#ifndef RENDERER_OPENGL_UISHADERS_H
#define RENDERER_OPENGL_UISHADERS_H

/** @file UIShaders.h
 *     Interface (sprite, font, solid) shaders.
 */

constexpr const char* UI_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;
layout(location = 3) in float a_z;
uniform vec2 u_screen_size;
out vec2 v_uv;
out vec4 v_color;
void main()
{
    vec2 ndc;
    ndc.x = (a_pos.x / u_screen_size.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (a_pos.y / u_screen_size.y) * 2.0;
    gl_Position = vec4(ndc, a_z, 1.0);
    v_uv = a_uv;
    v_color = a_color;
}
)glsl";

// Unit 0 = sprite atlas (R8 palette-index). Unit 1 = palette (256x1 RGBA8).
constexpr const char* UI_SPRITE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
in vec4 v_color;
uniform sampler2D u_sprite_atlas;
uniform sampler2D u_palette;
out vec4 fragColor;
void main()
{
    float idx = texture(u_sprite_atlas, v_uv).r;
    if (idx < (0.5 / 255.0)) discard;
    vec4 pal = texture(u_palette, vec2(idx, 0.5));
    fragColor = vec4(pal.rgb * v_color.rgb, v_color.a);
}
)glsl";

// Unit 0 = sprite atlas, mask-only (index 0 discarded); outputs flat vertex colour.
constexpr const char* UI_SPRITE_COLORED_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
in vec4 v_color;
uniform sampler2D u_sprite_atlas;
out vec4 fragColor;
void main()
{
    float idx = texture(u_sprite_atlas, v_uv).r;
    if (idx < (0.5 / 255.0)) discard;
    fragColor = v_color;
}
)glsl";

// Unit 0 = sprite atlas. Unit 1 = CLUT (256xN RGBA8).
// The CLUT row already contains the final current-palette colour for each
// source palette index, so this supports arbitrary 256-byte cmap tables.
constexpr const char* UI_CLUT_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
in vec4 v_color;
uniform sampler2D u_sprite_atlas;
uniform sampler2D u_clut;
uniform float u_clut_v;
out vec4 fragColor;
void main()
{
    float idx = texture(u_sprite_atlas, v_uv).r;
    if (idx < (0.5 / 255.0)) discard;
    vec4 color = texture(u_clut, vec2(idx, u_clut_v));
    fragColor = vec4(color.rgb * v_color.rgb, color.a * v_color.a);
}
)glsl";

// Unit 0 = sprite atlas. Unit 1 = palette. Unit 2 = fade table.
// pixmap.fade_tables is 64 rows x 256 columns (not 256 rows, unlike develop's
// table) -- u_remap_row indexes one of those 64 rows.
constexpr const char* UI_REMAP_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
in vec4 v_color;
uniform sampler2D u_sprite_atlas;
uniform sampler2D u_palette;
uniform sampler2D u_fade_table;
uniform float u_remap_row;
out vec4 fragColor;
void main()
{
    float idx_f = texture(u_sprite_atlas, v_uv).r;
    if (idx_f < (0.5 / 255.0)) discard;
    float remap_y = (u_remap_row + 0.5) / 64.0;
    float remapped_f = texture(u_fade_table, vec2(idx_f, remap_y)).r;
    vec4 pal = texture(u_palette, vec2(remapped_f, 0.5));
    fragColor = vec4(pal.rgb * v_color.rgb, v_color.a);
}
)glsl";

// Unit 0 = font atlas (R8 palette-index, same atlas as sprites -- glyphs are
// TbSprites too). Same shape as UI_SPRITE_FRAGMENT_SHADER.
constexpr const char* UI_FONT_FRAGMENT_SHADER = UI_SPRITE_FRAGMENT_SHADER;

constexpr const char* UI_SOLID_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec4 v_color;
out vec4 fragColor;
void main()
{
    fragColor = v_color;
}
)glsl";

#endif // RENDERER_OPENGL_UISHADERS_H
