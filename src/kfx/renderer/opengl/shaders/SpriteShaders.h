#ifndef RENDERER_OPENGL_SPRITESHADERS_H
#define RENDERER_OPENGL_SPRITESHADERS_H

/** @file SpriteShaders.h
 *     Creature sprite and creature shadow shaders.
 */

// Keeper-sprite (creature/object) shaders. Every variant picks the source
// texel for a destination pixel with kspr_source_texel(), the GLSL form of
// sprite_scale_axis() in SpriteScale.h, so GL lands on the same pixels as the
// software scaling tables. The quad always covers whole pixels.
#define KSPR_SOURCE_TEXEL_GLSL R"glsl(
// local = position inside the quad in pixels; map = phase x/y, step x/y (16.16);
// src = content size in source pixels.
ivec2 kspr_source_texel(vec2 local, vec4 map, vec2 src, float flip)
{
    ivec2 q     = ivec2(floor(local));
    ivec2 phase = ivec2(map.xy);
    ivec2 step  = max(ivec2(map.zw), ivec2(1));
    ivec2 j     = max(((q + 1) << 16) - phase - 1, ivec2(0)) / step;
    ivec2 last  = max(ivec2(src) - 1, ivec2(0));
    j = min(j, last);
    if (flip > 0.5)
        j.x = last.x - j.x;
    return j;
}
)glsl"

#define KSPR_GLOW_GLSL R"glsl(
// Per-strength RGB step, (dR*4, dG*4, dB*4) / 255 from compute_alpha_tables().
// Black's step is subtracted.
const vec3 k_glow_step[8] = vec3[8](
    vec3(16.0, 16.0, 16.0) / 255.0,  // white:  dR=4, dG=4, dB=4
    vec3(24.0, 16.0,  0.0) / 255.0,  // yellow: dR=6, dG=4, dB=0
    vec3(24.0,  4.0,  4.0) / 255.0,  // red:    dR=6, dG=1, dB=1
    vec3( 8.0,  8.0, 24.0) / 255.0,  // blue:   dR=2, dG=2, dB=6
    vec3( 8.0, 24.0,  8.0) / 255.0,  // green:  dR=2, dG=6, dB=2
    vec3(12.0,  0.0, 12.0) / 255.0,  // purple: dR=3, dG=0, dB=3
    vec3( 8.0,  8.0,  8.0) / 255.0,  // black:  dR=-2, dG=-2, dB=-2
    vec3(24.0, 12.0,  4.0) / 255.0   // orange: dR=6, dG=3, dB=1
);
// False when the pixel isn't drawn by this pass; the darken pass draws only black.
bool kspr_glow_delta(int px, bool darken, mat3 palette_xform, out vec3 delta)
{
    delta = vec3(0.0);
    if (px < 1 || px > 64) return false;
    int code   = px - 1;
    int family = code / 8;
    int row    = code % 8;
    if (row == 0 || (family == 6) != darken) return false;
    delta = clamp(palette_xform * (k_glow_step[family] * float(row)), 0.0, 1.0);
    return true;
}
)glsl"

constexpr const char* KSPR_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2  a_pos;
layout(location = 1) in vec2  a_local;  // position inside the quad, pixels
layout(location = 2) in vec4  a_map;    // phase x/y, step x/y
layout(location = 3) in vec2  a_src;    // src_w, content_h
layout(location = 4) in float a_flip;
uniform vec2  u_viewport;
uniform float u_z_ndc;
out vec2 v_local;
flat out vec4  v_map;
flat out vec2  v_src;
flat out float v_flip;
void main()
{
    vec2 ndc;
    ndc.x = a_pos.x / u_viewport.x * 2.0 - 1.0;
    ndc.y = 1.0 - a_pos.y / u_viewport.y * 2.0;
    gl_Position = vec4(ndc, u_z_ndc, 1.0);
    v_local = a_local;
    v_map   = a_map;
    v_src   = a_src;
    v_flip  = a_flip;
}
)glsl";

constexpr const char* KSPR_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_flip;
uniform sampler2D u_sprite;    // GL_RG8 palette index (R) + coverage (G), 256x256
uniform sampler2D u_palette;   // GL_RGBA8 colour table  (256x1)
uniform float     u_alpha;     // 1.0=solid, 0.5=transpar4, 0.25=transpar8
out vec4 fragColor;
)glsl" KSPR_SOURCE_TEXEL_GLSL R"glsl(
void main()
{
    vec4 texel = texelFetch(u_sprite, kspr_source_texel(v_local, v_map, v_src, v_flip), 0);
    if (texel.g < 0.5) discard;
    vec4 color = texture(u_palette, vec2(texel.r, 0.5));
    fragColor = vec4(color.rgb, u_alpha);
}
)glsl";

// Non-instanced glow fragment shader (see KSPR_GLOW_GLSL). Reuses KSPR_VERTEX_SHADER.
// Drawn with glBlendFunc(GL_ONE, GL_ONE): added, or reverse-subtracted when u_darken is set.
constexpr const char* KSPR_GLOW_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_flip;
uniform sampler2D u_sprite;
// Glow steps are engine-palette colours; this moves them into the palette drawn with.
uniform mat3 u_palette_xform;
uniform bool u_darken;
out vec4 fragColor;
)glsl" KSPR_SOURCE_TEXEL_GLSL KSPR_GLOW_GLSL R"glsl(
void main()
{
    int px = int(texelFetch(u_sprite, kspr_source_texel(v_local, v_map, v_src, v_flip), 0).r * 255.0 + 0.5);
    vec3 delta;
    if (!kspr_glow_delta(px, u_darken, u_palette_xform, delta)) discard;
    fragColor = vec4(delta, u_darken ? 0.0 : 1.0);
}
)glsl";

// Atlas variant: u_sprite is a GL_TEXTURE_2D_ARRAY;
// u_layer selects the pre-decoded layer for this sprite.
constexpr const char* KSPR_ARRAY_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_flip;
uniform sampler2DArray u_sprite;   // GL_RG8 texture array (index, coverage), many frames packed per layer
uniform sampler2D      u_clut;     // GL_RGBA8 256xN CLUT -- row 0=identity, rows 1..N-1=remaps
uniform float          u_alpha;    // 1.0=solid, 0.5=transpar4, 0.25=transpar8
uniform float          u_layer;    // layer index in the sprite array
uniform vec2           u_origin;   // sprite's top-left texel in the layer
uniform float          u_clut_v;   // V texcoord selecting the CLUT row
out vec4 fragColor;
)glsl" KSPR_SOURCE_TEXEL_GLSL R"glsl(
void main()
{
    ivec2 t = kspr_source_texel(v_local, v_map, v_src, v_flip);
    vec4 texel = texelFetch(u_sprite, ivec3(t + ivec2(u_origin), int(u_layer + 0.5)), 0);
    if (texel.g < 0.5) discard;
    vec4 color = texture(u_clut, vec2(texel.r, u_clut_v));
    fragColor = vec4(color.rgb, color.a * u_alpha);
}
)glsl";

// Array-atlas variant of the glow shader -- glow sprites cached in atlas.
constexpr const char* KSPR_ARRAY_GLOW_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_flip;
uniform sampler2DArray u_sprite;
uniform float          u_layer;
uniform vec2           u_origin;
// Glow steps are engine-palette colours; this moves them into the palette drawn with.
uniform mat3 u_palette_xform;
uniform bool u_darken;
out vec4 fragColor;
)glsl" KSPR_SOURCE_TEXEL_GLSL KSPR_GLOW_GLSL R"glsl(
void main()
{
    ivec2 t = kspr_source_texel(v_local, v_map, v_src, v_flip);
    int px = int(texelFetch(u_sprite, ivec3(t + ivec2(u_origin), int(u_layer + 0.5)), 0).r * 255.0 + 0.5);
    vec3 delta;
    if (!kspr_glow_delta(px, u_darken, u_palette_xform, delta)) discard;
    fragColor = vec4(delta, u_darken ? 0.0 : 1.0);
}
)glsl";

// Depth-fail outline shaders -- draw a flat owner-colour silhouette
// only where the creature sprite is occluded by geometry (depth test =
// GL_GREATER, set at the draw call site, not here).
constexpr const char* KSPR_OUTLINE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_flip;
uniform sampler2D u_sprite;
uniform vec4      u_outline_color;
out vec4 fragColor;
)glsl" KSPR_SOURCE_TEXEL_GLSL R"glsl(
void main()
{
    if (texelFetch(u_sprite, kspr_source_texel(v_local, v_map, v_src, v_flip), 0).g < 0.5) discard;
    fragColor = u_outline_color;
}
)glsl";

// Array-atlas variant of the outline shader (sampler2DArray).
constexpr const char* KSPR_ARRAY_OUTLINE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_flip;
uniform sampler2DArray u_sprite;
uniform float          u_layer;
uniform vec2           u_origin;
uniform vec4           u_outline_color;
out vec4 fragColor;
)glsl" KSPR_SOURCE_TEXEL_GLSL R"glsl(
void main()
{
    ivec2 t = kspr_source_texel(v_local, v_map, v_src, v_flip);
    if (texelFetch(u_sprite, ivec3(t + ivec2(u_origin), int(u_layer + 0.5)), 0).g < 0.5) discard;
    fragColor = u_outline_color;
}
)glsl";

// Edge-detect variant of the outline shader (sampler2D). Emits outline
// colour only at sprite boundary pixels (where at least one cardinal
// neighbouring source texel is transparent).
constexpr const char* KSPR_EDGE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_flip;
uniform sampler2D u_sprite;
uniform vec4      u_outline_color;
out vec4 fragColor;
const float kThr = 0.5;
)glsl" KSPR_SOURCE_TEXEL_GLSL R"glsl(
float coverage(ivec2 t)
{
    ivec2 size = textureSize(u_sprite, 0);
    if (any(lessThan(t, ivec2(0))) || any(greaterThanEqual(t, size)))
        return 0.0;
    return texelFetch(u_sprite, t, 0).g;
}
void main()
{
    ivec2 t = kspr_source_texel(v_local, v_map, v_src, v_flip);
    if (coverage(t) < kThr) discard;
    if (coverage(t + ivec2(-1, 0)) >= kThr && coverage(t + ivec2(1, 0)) >= kThr &&
        coverage(t + ivec2(0, -1)) >= kThr && coverage(t + ivec2(0, 1)) >= kThr) discard;
    fragColor = u_outline_color;
}
)glsl";

// Edge-detect variant -- array-atlas (sampler2DArray + u_layer).
constexpr const char* KSPR_ARRAY_EDGE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_flip;
uniform sampler2DArray u_sprite;
uniform float          u_layer;
uniform vec2           u_origin;
uniform vec4           u_outline_color;
out vec4 fragColor;
const float kThr = 0.5;
)glsl" KSPR_SOURCE_TEXEL_GLSL R"glsl(
float coverage(ivec2 t)
{
    if (any(lessThan(t, ivec2(0))) || t.x >= int(v_src.x))
        return 0.0;
    return texelFetch(u_sprite, ivec3(t + ivec2(u_origin), int(u_layer + 0.5)), 0).g;
}
void main()
{
    ivec2 t = kspr_source_texel(v_local, v_map, v_src, v_flip);
    if (coverage(t) < kThr) discard;
    if (coverage(t + ivec2(-1, 0)) >= kThr && coverage(t + ivec2(1, 0)) >= kThr &&
        coverage(t + ivec2(0, -1)) >= kThr && coverage(t + ivec2(0, 1)) >= kThr) discard;
    fragColor = u_outline_color;
}
)glsl";

constexpr const char* KSPR_INST_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_corner;  // unit quad corner, (0,0)..(1,1)
layout(location = 1) in vec4 a_rect;    // instance: dst x, y, w, h (whole screen px)
layout(location = 2) in vec2 a_src;     // instance: src_w, content_h
layout(location = 3) in vec4 a_misc;    // instance: layer, clut_v, alpha, z_ndc
layout(location = 4) in uint a_flags;   // instance: bit0 = flip_h, bit1 = glow, bit2 = darken pass
layout(location = 5) in vec4 a_map;     // instance: phase x/y, step x/y
layout(location = 6) in vec2 a_origin;  // instance: sprite's top-left texel in the layer
uniform vec2 u_viewport;
out vec2 v_local;
flat out vec4 v_map;
flat out vec2 v_src;
flat out vec3 v_lca;    // layer, clut_v, alpha
flat out uint v_flags;
flat out vec2 v_origin;
void main()
{
    vec2 px = a_rect.xy + a_corner * a_rect.zw;
    vec2 ndc;
    ndc.x = px.x / u_viewport.x * 2.0 - 1.0;
    ndc.y = 1.0 - px.y / u_viewport.y * 2.0;
    gl_Position = vec4(ndc, a_misc.w, 1.0);
    v_local = a_corner * a_rect.zw;
    v_map   = a_map;
    v_src   = a_src;
    v_lca   = a_misc.xyz;
    v_flags = a_flags;
    v_origin = a_origin;
}
)glsl";

constexpr const char* KSPR_INST_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4 v_map;
flat in vec2 v_src;
flat in vec3 v_lca;    // layer, clut_v, alpha
flat in uint v_flags;  // bit0 = flip_h, bit1 = glow, bit2 = darken pass
flat in vec2 v_origin;
uniform sampler2DArray u_sprite;   // GL_RG8 decode atlas (index, coverage), many frames packed per layer
uniform sampler2D      u_clut;     // 256xN CLUT -- row 0 identity, rows 1..N remaps
// Glow steps are engine-palette colours; this moves them into the palette drawn with.
uniform mat3 u_palette_xform;
out vec4 fragColor;
)glsl" KSPR_SOURCE_TEXEL_GLSL KSPR_GLOW_GLSL R"glsl(
void main()
{
    float flip = ((v_flags & 1u) != 0u) ? 1.0 : 0.0;
    ivec2 t = kspr_source_texel(v_local, v_map, v_src, flip);
    vec4 texel = texelFetch(u_sprite, ivec3(t + ivec2(v_origin), int(v_lca.x + 0.5)), 0);
    float idx = texel.r;
    if ((v_flags & 2u) != 0u)
    {
        // Glow: alpha 0 makes (ONE, ONE_MINUS_SRC_ALPHA) act as (ONE, ONE).
        // The darken pass is drawn with a reverse-subtract blend.
        vec3 delta;
        if (!kspr_glow_delta(int(idx * 255.0 + 0.5), (v_flags & 4u) != 0u, u_palette_xform, delta)) discard;
        fragColor = vec4(delta, 0.0);
    }
    else
    {
        if (texel.g < 0.5) discard;
        vec4 c = texture(u_clut, vec2(idx, v_lca.y));
        float a = c.a * v_lca.z;
        fragColor = vec4(c.rgb * a, a);  // premultiplied
    }
}
)glsl";

// Instanced depth-fail outline: flat owner colour where the sprite
// is behind geometry (drawn with glDepthFunc(GL_GREATER) before the main
// instanced pass -- see flush_keeper_sprite_instances()).
constexpr const char* KSPR_INST_OUTLINE_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_corner;
layout(location = 1) in vec4 a_rect;
layout(location = 2) in vec2 a_src;
layout(location = 3) in vec3 a_lzf;     // instance: layer, z_ndc, flip (0/1)
layout(location = 4) in vec4 a_color;   // instance: outline rgba
layout(location = 5) in vec4 a_map;
layout(location = 6) in vec2 a_origin;  // instance: sprite's top-left texel in the layer
uniform vec2 u_viewport;
out vec2 v_local;
flat out vec4  v_map;
flat out vec2  v_src;
flat out float v_layer;
flat out float v_flip;
flat out vec4  v_color;
flat out vec2  v_origin;
void main()
{
    vec2 px = a_rect.xy + a_corner * a_rect.zw;
    vec2 ndc;
    ndc.x = px.x / u_viewport.x * 2.0 - 1.0;
    ndc.y = 1.0 - px.y / u_viewport.y * 2.0;
    gl_Position = vec4(ndc, a_lzf.y, 1.0);
    v_local = a_corner * a_rect.zw;
    v_map   = a_map;
    v_src   = a_src;
    v_layer = a_lzf.x;
    v_flip  = a_lzf.z;
    v_color = a_color;
    v_origin = a_origin;
}
)glsl";

constexpr const char* KSPR_INST_OUTLINE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_layer;
flat in float v_flip;
flat in vec4  v_color;
flat in vec2  v_origin;
uniform sampler2DArray u_sprite;
out vec4 fragColor;
)glsl" KSPR_SOURCE_TEXEL_GLSL R"glsl(
void main()
{
    ivec2 t = kspr_source_texel(v_local, v_map, v_src, v_flip);
    if (texelFetch(u_sprite, ivec3(t + ivec2(v_origin), int(v_layer + 0.5)), 0).g < 0.5) discard;
    fragColor = vec4(v_color.rgb * v_color.a, v_color.a);  // premultiplied
}
)glsl";

// Instanced edge-detect: only emit colour at sprite boundary pixels.
// Shares the same vertex shader and VAO as the instanced outline.
constexpr const char* KSPR_INST_EDGE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_local;
flat in vec4  v_map;
flat in vec2  v_src;
flat in float v_layer;
flat in float v_flip;
flat in vec4  v_color;
flat in vec2  v_origin;
uniform sampler2DArray u_sprite;
out vec4 fragColor;
const float kThr = 0.5;
)glsl" KSPR_SOURCE_TEXEL_GLSL R"glsl(
float coverage(ivec2 t)
{
    if (any(lessThan(t, ivec2(0))) || t.x >= int(v_src.x))
        return 0.0;
    return texelFetch(u_sprite, ivec3(t + ivec2(v_origin), int(v_layer + 0.5)), 0).g;
}
void main()
{
    ivec2 t = kspr_source_texel(v_local, v_map, v_src, v_flip);
    if (coverage(t) < kThr) discard;
    if (coverage(t + ivec2(-1, 0)) >= kThr && coverage(t + ivec2(1, 0)) >= kThr &&
        coverage(t + ivec2(0, -1)) >= kThr && coverage(t + ivec2(0, 1)) >= kThr) discard;
    fragColor = vec4(v_color.rgb * v_color.a, v_color.a);  // premultiplied
}
)glsl";

// Creature-shadow shaders. Samples the same GL_TEXTURE_2D_ARRAY
// the keeper-sprite cache already builds (resolve_kspr_cache(), shared
// cache) as a binary silhouette mask, discarding transparent texels the
// same way KSPR_ARRAY_FRAGMENT_SHADER does. Where the mask is solid, the
// draw call (glBlendFunc(GL_ZERO, GL_SRC_COLOR)) multiplies the
// destination colour by this shader's output -- an approximation of
// trig_render_md10's destination-dependent palette darken (software has
// no RGBA framebuffer to sample the same way; see the .cpp file header).
constexpr const char* SHADOW_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2  a_pos;     // screen px
layout(location = 1) in vec2  a_uv;      // atlas UV [0,1]
layout(location = 2) in float a_layer;   // atlas layer index
layout(location = 3) in float a_z_ndc;
layout(location = 4) in float a_darken;  // 0=no shadow, 1=fully black
layout(location = 5) in vec4  a_rect;    // sprite's texels in the layer: x0, y0, x1, y1
uniform vec2 u_viewport;
out vec2 v_uv;
flat out float v_layer;
flat out float v_darken;
flat out vec4  v_rect;
void main()
{
    float ndc_x = a_pos.x / u_viewport.x * 2.0 - 1.0;
    float ndc_y = 1.0 - a_pos.y / u_viewport.y * 2.0;
    gl_Position = vec4(ndc_x, ndc_y, a_z_ndc, 1.0);
    v_uv = a_uv;
    v_layer = a_layer;
    v_darken = a_darken;
    v_rect = a_rect;
}
)glsl";

constexpr const char* SHADOW_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
flat in float v_layer;
flat in float v_darken;
flat in vec4  v_rect;
uniform sampler2DArray u_sprite;
out vec4 fragColor;
void main()
{
    // The shadow quad can span more than the cached frame; never read a neighbour.
    vec2 px = v_uv * vec2(textureSize(u_sprite, 0).xy);
    if (any(lessThan(px, v_rect.xy)) || any(greaterThanEqual(px, v_rect.zw))) discard;
    if (texture(u_sprite, vec3(v_uv, v_layer)).g < 0.5) discard;
    float keep = 1.0 - v_darken;
    fragColor = vec4(keep, keep, keep, 1.0);
}
)glsl";

#endif // RENDERER_OPENGL_SPRITESHADERS_H
