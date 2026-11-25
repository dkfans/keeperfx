#ifndef RENDERER_OPENGL_LENSSHADERS_H
#define RENDERER_OPENGL_LENSSHADERS_H

/** @file LensShaders.h
 *     Possession lens shaders.
 */

/******************************************************************************/
// Possession lens
/******************************************************************************/

// Shared by all lens composite passes: a static NDC unit quad, drawn with
// glViewport already set to the destination viewport sub-rect, so a_pos's
// -1..1 range maps exactly onto that sub-rect.
constexpr const char* LENS_COMPOSITE_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
out vec2 v_uv;
void main()
{
    gl_Position = vec4(a_pos, 0.0, 1.0);
    v_uv = a_uv;
}
)glsl";

// Addressing shared by the lens fragment shaders, matching the CPU lens
// buffers exactly: the destination pixel is counted from the viewport's
// top-left, and the source is read at (viewport_x + x, y) from the top of the
// captured scene -- software never offsets the source row by viewport_y.
// All reads are texelFetch, so no filtering or rounding is involved.
#define LENS_FRAGMENT_COMMON R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_scene;
uniform float u_src_x;       // viewport left edge, in scene texels
uniform vec2  u_view_size;   // viewport size, in pixels
out vec4 fragColor;

ivec2 lens_dest_pixel()
{
    ivec2 size = ivec2(u_view_size);
    ivec2 d = clamp(ivec2(v_uv * u_view_size), ivec2(0), size - 1);
    return ivec2(d.x, size.y - 1 - d.y);
}

vec3 lens_scene(ivec2 src)
{
    ivec2 size = textureSize(u_scene, 0);
    ivec2 t = ivec2(int(u_src_x) + src.x, size.y - 1 - src.y);
    return texelFetch(u_scene, clamp(t, ivec2(0), size - 1), 0).rgb;
}
)glsl"

// Palette index recovery shared by passes that apply the game's index tables
// to a captured RGB image. Every colour drawn from the frame palette maps back
// to its own index (see GLPaletteIndexLookup), so the table results are the
// ones software produces for those pixels.
#define PALETTE_INDEX_COMMON R"glsl(
uniform sampler2D u_palette;        // 256x1 RGBA8, the frame palette
uniform sampler2D u_index_lookup;   // 512x512 R8, 6-bit RGB -> palette index
uniform sampler2D u_fade_table;     // 256x64 R8, pixmap.fade_tables

int palette_index(vec3 rgb)
{
    ivec3 c = ivec3(clamp(rgb, 0.0, 1.0) * 63.0 + 0.5);
    int key = (c.r << 12) | (c.g << 6) | c.b;
    return int(texelFetch(u_index_lookup, ivec2(key & 511, key >> 9), 0).r * 255.0 + 0.5);
}

int fade_index(int row, int idx)
{
    return int(texelFetch(u_fade_table, ivec2(idx, row), 0).r * 255.0 + 0.5);
}

vec3 palette_colour(int idx)
{
    return texelFetch(u_palette, ivec2(idx, 0), 0).rgb;
}
)glsl"

// Mist lens, following CMistFade::Render(): virtual 640x480 coordinates in
// 16.16 fixed point, two wrapped layers of the 256x256 density texture,
// n = (primary+secondary)>>3 clamped to 32, then each pixel's index goes
// through fade table row (mist_lightness + n).
// Units: 0 scene, 1 mist density, 2 palette, 3 index lookup, 4 fade table.
constexpr const char* LENS_MIST_FRAGMENT_SHADER = LENS_FRAGMENT_COMMON PALETTE_INDEX_COMMON R"glsl(
uniform sampler2D u_mist;
uniform vec2 u_pos;          // primary layer offset, 0..255
uniform vec2 u_sec;          // secondary layer offset, 0..255
uniform float u_lightness;   // fade table row base
void main()
{
    ivec2 d = lens_dest_pixel();

    ivec2 size = ivec2(u_view_size);
    int scale_x = (640 << 16) / size.x;
    int scale_y = (480 << 16) / size.y;
    int vx = (d.x * scale_x) >> 16;
    int vy = (d.y * scale_y) >> 16;

    int pos_x = int(u_pos.x), pos_y = int(u_pos.y);
    int sec_x = int(u_sec.x), sec_y = int(u_sec.y);
    int p2 = (pos_x + vx) & 255;
    int c2 = (pos_y + vy) & 255;
    int c1 = (sec_y + 0x10000 - vx) & 255;
    int p1 = (sec_x + 0x10000 - vy) & 255;

    int k = int(texelFetch(u_mist, ivec2(p2, c2), 0).r * 255.0 + 0.5);
    int i = int(texelFetch(u_mist, ivec2(p1, c1), 0).r * 255.0 + 0.5);
    int n = min((k + i) >> 3, 32);

    int row = min(int(u_lightness) + n, 63);
    int idx = fade_index(row, palette_index(lens_scene(d)));
    fragColor = vec4(palette_colour(idx), 1.0);
}
)glsl";

// Displacement/flyeye distortion through the CPU's own lookup table
// (DisplaceLookupEntry/FlyeyeLookupEntry: src_x,src_y per destination pixel,
// rows from the top). Unit 0 = captured scene. Unit 1 = GL_RG16UI table,
// sized viewport_w x viewport_h.
constexpr const char* LENS_REMAP_FRAGMENT_SHADER = LENS_FRAGMENT_COMMON R"glsl(
uniform usampler2D u_remap;
void main()
{
    ivec2 d = lens_dest_pixel();
    ivec2 table_size = textureSize(u_remap, 0);
    uvec2 src = texelFetch(u_remap, clamp(d, ivec2(0), table_size - 1), 0).rg;
    fragColor = vec4(lens_scene(ivec2(src)), 1.0);
}
)glsl";

// Overlay composite, following OverlayEffect's Render(): the overlay is
// stretched to the viewport with 16.16 nearest sampling, rows from the top;
// index 255 is transparent; otherwise the result is the index-number blend
// (overlay * alpha + scene * (256 - alpha)) >> 8, as software computes it.
// Units: 0 scene, 1 overlay (R8 palette index), 2 palette, 3 index lookup.
constexpr const char* LENS_OVERLAY_FRAGMENT_SHADER = LENS_FRAGMENT_COMMON PALETTE_INDEX_COMMON R"glsl(
uniform sampler2D u_overlay;
uniform int u_alpha;         // 0..256
void main()
{
    ivec2 d = lens_dest_pixel();
    vec3 scene = lens_scene(d);

    ivec2 size = ivec2(u_view_size);
    ivec2 overlay_size = textureSize(u_overlay, 0);
    int ox = min((d.x * ((overlay_size.x << 16) / size.x)) >> 16, overlay_size.x - 1);
    int oy = min((d.y * ((overlay_size.y << 16) / size.y)) >> 16, overlay_size.y - 1);

    int ov = int(texelFetch(u_overlay, ivec2(ox, oy), 0).r * 255.0 + 0.5);
    if (ov == 255)
    {
        fragColor = vec4(scene, 1.0);
        return;
    }
    int idx = (ov * u_alpha + palette_index(scene) * (256 - u_alpha)) >> 8;
    fragColor = vec4(palette_colour(idx), 1.0);
}
)glsl";

// Plain copy of the captured scene into the viewport, for a lens with no
// pixel effect (palette-only or custom lens) -- the software fallback's
// CopyBuffer(). Unit 0 = captured scene.
constexpr const char* LENS_COPY_FRAGMENT_SHADER = LENS_FRAGMENT_COMMON R"glsl(
void main()
{
    fragColor = vec4(lens_scene(lens_dest_pixel()), 1.0);
}
)glsl";

/******************************************************************************/
// Parchment transition
/******************************************************************************/

#endif // RENDERER_OPENGL_LENSSHADERS_H
