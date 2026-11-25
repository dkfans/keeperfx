#ifndef RENDERER_OPENGL_PASSSHADERS_H
#define RENDERER_OPENGL_PASSSHADERS_H

/** @file PassShaders.h
 *     Map-fade, image present and zoom-box shaders.
 */

#include "kfx/renderer/opengl/shaders/LensShaders.h"

// Parchment transition. The elastic-pinch warp is a GLSL port of the CPU
// map_fade() (engine_redraw.c), kept continuous so the delta-time step stays
// smooth. The blend is software's: each side's palette index goes through
// fade table row step (parchment) or 32-step (world), then the pair goes
// through the map-fade ghost table, ghost[world_row][parchment_col].
// Units: 0 parchment, 1 world, 2 palette, 3 index lookup, 4 fade table,
// 5 map-fade ghost table (256x256 R8).
constexpr const char* MAPFADE_FRAGMENT_SHADER = R"glsl(
#version 330 core
)glsl" PALETTE_INDEX_COMMON R"glsl(
in vec2 v_uv;
uniform sampler2D u_parchment;
uniform sampler2D u_world;
uniform sampler2D u_ghost;
uniform float u_step;        // 0.0..32.0
uniform float u_warp_scale;  // pinch strength, 1.0 for the original 8-turn fade
out vec4 fragColor;

vec3 fetch_nearest(sampler2D tex, vec2 uv)
{
    ivec2 size = textureSize(tex, 0);
    return texelFetch(tex, clamp(ivec2(uv * vec2(size)), ivec2(0), size - 1), 0).rgb;
}

void main()
{
    float a6 = u_step;
    float fx = v_uv.x;
    float fy = 1.0 - v_uv.y;
    const float xmax = 320.0;
    float wp = (32.0 - a6) * u_warp_scale;
    float uv_px = clamp(fx + wp * (4.0 - 8.0 * fx) / xmax, 0.0, 1.0);
    float uv_py = clamp(fy + wp * 4.0 * (1.0 - 2.0 * fy) / xmax, 0.0, 1.0);
    float ww = a6 * u_warp_scale;
    float uv_wx = clamp(fx + ww * (4.0 - 8.0 * fx) / xmax, 0.0, 1.0);
    float uv_wy = clamp(fy + ww * 4.0 * (1.0 - 2.0 * fy) / xmax, 0.0, 1.0);

    vec3 c_parch = fetch_nearest(u_parchment, vec2(uv_px, 1.0 - uv_py));
    vec3 c_world = fetch_nearest(u_world,     vec2(uv_wx, 1.0 - uv_wy));

    int row = clamp(int(a6 + 0.5), 0, 32);
    int px1 = fade_index(row, palette_index(c_parch));
    int px2 = fade_index(32 - row, palette_index(c_world));
    int idx = int(texelFetch(u_ghost, ivec2(px1, px2), 0).r * 255.0 + 0.5);
    fragColor = vec4(palette_colour(idx), 1.0);
}
)glsl";

/******************************************************************************/
// Raw image present (BeginFrame/EndFrame/PresentImage port)
/******************************************************************************/

// FMV frame / splash bitmap blit to an arbitrary destination rect (screen
// pixels), same screen-space-to-NDC math as UI_VERTEX_SHADER. No discard on
// index 0 -- unlike sprites, these are opaque full-frame images.
constexpr const char* RAWIMAGE_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
uniform vec2 u_screen_size;
out vec2 v_uv;
void main()
{
    vec2 ndc;
    ndc.x = (a_pos.x / u_screen_size.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (a_pos.y / u_screen_size.y) * 2.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_uv = a_uv;
}
)glsl";

// Unit 0 = image (R8 palette-index). Unit 1 = palette (256x1 RGBA8).
constexpr const char* RAWIMAGE_BLIT_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_image;
uniform sampler2D u_palette;
out vec4 fragColor;
void main()
{
    float idx = texture(u_image, v_uv).r;
    vec4 pal = texture(u_palette, vec2(idx, 0.5));
    fragColor = vec4(pal.rgb, 1.0);
}
)glsl";

// Window-frame overlay (compressed_window_draw(), front_landview.c)
// -- same as RAWIMAGE_BLIT_FRAGMENT_SHADER but index 0 is transparent instead
// of opaque black, so this draws over whatever the opaque present already put
// on screen (the zoomed landview background) instead of replacing it.
constexpr const char* RAWIMAGE_TRANSPARENT_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_image;
uniform sampler2D u_palette;
out vec4 fragColor;
void main()
{
    float idx = texture(u_image, v_uv).r;
    if (idx < (0.5 / 255.0)) discard;
    vec4 pal = texture(u_palette, vec2(idx, 0.5));
    fragColor = vec4(pal.rgb, 1.0);
}
)glsl";

// Coverage variant of RAWIMAGE_TRANSPARENT_FRAGMENT_SHADER: transparency
// comes from an explicit per-pixel coverage map (unit 2, R8: 255 = opaque,
// 0 = transparent) instead of the index-0 key, so a source image may
// legitimately paint with palette index 0 (e.g. the landview window frame's
// black stone) without being treated as see-through.
constexpr const char* RAWIMAGE_TRANSPARENT_COVERAGE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_image;
uniform sampler2D u_palette;
uniform sampler2D u_coverage;
out vec4 fragColor;
void main()
{
    float cov = texture(u_coverage, v_uv).r;
    if (cov < 0.5) discard;
    float idx = texture(u_image, v_uv).r;
    vec4 pal = texture(u_palette, vec2(idx, 0.5));
    fragColor = vec4(pal.rgb, 1.0);
}
)glsl";

// Landview zoom-in/out transition (frontzoom_to_point(),
// front_landview.c). Reuses RAWIMAGE_VERTEX_SHADER over a full-screen quad;
// v_uv * u_screen_size recovers the destination screen pixel, which maps
// back to a source texel via the same linear "centre + (pixel - screen
// centre) * scale" relationship the CPU nearest-neighbour zoom loop computes
// per quadrant -- one continuous formula replaces that split (the CPU
// version only needed 4 quadrants to keep its rounding direction consistent
// around the centre pixel while writing into a real framebuffer; a GPU
// texture fetch has no equivalent constraint). floor() before normalizing
// matches the CPU's `>>8` truncation (round toward the source pixel's
// top-left, not nearest) even though GL_NEAREST would otherwise round to
// nearest.
constexpr const char* RAWIMAGE_ZOOM_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_image;
uniform sampler2D u_palette;
uniform vec2 u_screen_size;
uniform vec2 u_zoom_center_map;
uniform vec2 u_zoom_screen_center;
uniform float u_zoom_scale;
uniform vec2 u_src_size;
out vec4 fragColor;
void main()
{
    vec2 screen_px = v_uv * u_screen_size;
    vec2 src_px = u_zoom_center_map + (screen_px - u_zoom_screen_center) * u_zoom_scale;
    vec2 src_uv = (floor(src_px) + 0.5) / u_src_size;
    if (src_uv.x < 0.0 || src_uv.x > 1.0 || src_uv.y < 0.0 || src_uv.y > 1.0)
    {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    float idx = texture(u_image, src_uv).r;
    vec4 pal = texture(u_palette, vec2(idx, 0.5));
    fragColor = vec4(pal.rgb, 1.0);
}
)glsl";

// Zoom-box terrain tiles (draw_zoom_box_terrain(), gui_parchment.c): flat 2D
// textured quads sampling the world tile atlas (GLTileAtlas, GL_TEXTURE_2D_ARRAY,
// R8 palette-indexed -- same content the isometric world view samples, layer =
// tile_id / TEXTURE_BLOCKS_COUNT).
constexpr const char* ZOOMBOX_TILES_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec3 a_uvw;
uniform vec2 u_screen_size;
out vec3 v_uvw;
void main()
{
    vec2 ndc;
    ndc.x = (a_pos.x / u_screen_size.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (a_pos.y / u_screen_size.y) * 2.0;
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_uvw = a_uvw;
}
)glsl";

// Unit 0 = tile atlas (R8 palette-index, texture array). Unit 1 = palette (256x1 RGBA8).
constexpr const char* ZOOMBOX_TILES_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec3 v_uvw;
uniform sampler2DArray u_atlas;
uniform sampler2D u_palette;
out vec4 fragColor;
void main()
{
    float idx = texture(u_atlas, v_uvw).r;
    vec4 pal = texture(u_palette, vec2(idx, 0.5));
    fragColor = vec4(pal.rgb, 1.0);
}
)glsl";

#endif // RENDERER_OPENGL_PASSSHADERS_H
