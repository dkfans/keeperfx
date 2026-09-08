#ifndef RENDERER_OPENGL_GLSHADERS_H
#define RENDERER_OPENGL_GLSHADERS_H

/**
 * @brief Shader source code for the OpenGL renderer.
 * 
 * I really don't like this.
 * 
 * The eventual plan is the define shaders like this;;
 * 
 * inline constexpr std::string_view UI_VERTEX_SHADER = R"( 
 * foo...) and have a shader manager that can compile and cache them. 
 * This will allow us to have a single source of truth for shaders,
 *  and will also allow us to have a single place to manage shader compilation and caching.
 * 
 * It'll also allow us to implement a VFS and open up the possibility of having shaders be loaded from mod files
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

// World rendering shaders (P5.7.2a: tile geometry only, no sprites/shadows)
constexpr const char* WORLD_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec3  a_pos;
layout(location = 1) in vec2  a_uv;
layout(location = 2) in float a_shade;
layout(location = 3) in vec2  a_stl;       // subtile coords for lightmap (mode 1)
layout(location = 4) in float a_camera_z;  // camera-space Z for perspective correction
layout(location = 5) in float a_layer;     // texture array layer (atlas variation)
layout(location = 6) in vec3  aWorldPos;   // pre-projection world-space position
out vec2  v_uv;
out float v_shade;
out vec2  v_stl;
flat out float v_layer;
out vec3  vWorldPos;
void main()
{
    // Perspective-correct interpolation trick: multiply clip-space position by
    // camera_z (= gl_Position.w).  The rasterizer divides by w, restoring the
    // original NDC position, but now it also perspective-corrects all varyings
    // (v_uv, v_shade, v_stl) using the per-vertex w values.
    // When camera_z == 1.0 (unknown depth), this degrades to affine (no-op).
    float w = max(a_camera_z, 1.0);
    gl_Position = vec4(a_pos.xy * w, a_pos.z * w, w);
    v_uv        = a_uv;
    v_shade     = a_shade;
    v_stl       = a_stl;
    v_layer     = a_layer;
    vWorldPos   = aWorldPos;
}
)glsl";

constexpr const char* WORLD_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2  v_uv;
in float v_shade;
in vec2  v_stl;                         // subtile coords [0..511], mode 1 only
flat in float v_layer;                  // texture array layer (atlas variation)
in vec3  vWorldPos;                     // reserved for future dynamic lighting / shadow mapping
uniform sampler2DArray u_tile_atlas;    // R8 palette-index atlas array (unit 0)
uniform sampler2D  u_palette;           // RGBA8 256×1 palette (unit 1)
uniform usampler2D u_lightmap;          // R16UI subtile_lightness map (unit 2), mode 1
uniform sampler2D  u_fade_table;        // R8 256x64 fade/remap LUT (unit 3), palette mode
uniform float      u_fullbright;        // 0=normal shading, 1=bypass shade
uniform float      u_ambient;           // darkness floor added to shade [0,1]
uniform float      u_shade_scale;       // brightness multiplier (1.0=original)
uniform float      u_shade_gamma;       // shade curve exponent  (1.0=linear)
uniform int        u_lighting_mode;     // 0=software-accurate, 1=modern (Phase 3+)
uniform int        u_darkness_mode;     // 0=linear, 1=palette LUT, 2=animated fog
uniform int        u_tile_filter;       // 0=nearest, 1=palette-correct bilinear
uniform float      u_missing_tile;      // 1.0 = no valid atlas bound — show diagnostic
uniform float      u_time;             // seconds since start (fog animation)
uniform float      u_fog_speed;        // fog scroll speed multiplier
uniform float      u_fog_density;      // fog opacity [0,1]
out vec4 fragColor;

// Simple 2D hash for procedural noise (fog mode)
float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// Value noise with smooth interpolation
float noise2d(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);  // smoothstep
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Fractal Brownian Motion — 3 octaves
float fbm(vec2 p)
{
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < 3; i++)
    {
        v += a * noise2d(p);
        p *= 2.0;
        a *= 0.5;
    }
    return v;
}

void main()
{
    // When the tile atlas slot is missing (atlas not yet loaded or variation out of
    // range), show a magenta/black checkerboard so the problem is immediately visible
    // rather than a silent black tile.
    if (u_missing_tile > 0.5)
    {
        ivec2 cell = ivec2(gl_FragCoord.xy / 8.0);
        fragColor  = ((cell.x + cell.y) & 1) == 0
                     ? vec4(1.0, 0.0, 1.0, 1.0)  // magenta
                     : vec4(0.0, 0.0, 0.0, 1.0);  // black
        return;
    }

    // --- Compute raw shade (fade table row) ---
    // v_shade = (S>>16) / 32.0 where S = shade_intensity<<8.
    // This directly maps to fade table rows: 0.0 = row 0 (black),
    // 1.0 = row 32 (full brightness).  Used as-is for palette LUT modes.
    float raw_shade;
    if (u_lighting_mode == 0) {
        raw_shade = mix(v_shade, 1.0, u_fullbright);
    } else {
        vec2 lm_uv  = v_stl / vec2(511.0, 511.0);
        uint lm_raw = texture(u_lightmap, lm_uv).r;
        raw_shade = float(lm_raw) / 8192.0;
        raw_shade = mix(raw_shade, 1.0, u_fullbright);
    }

    // Pre-compute fade_v for palette-mode darkness so bilinear path can use it
    // per-neighbour without repeating the arithmetic.
    // Only valid when u_darkness_mode != 0 (avoids unit-3 access before load).
    float fade_v = 0.0;
    if (u_darkness_mode != 0)
    {
        float fade_row = raw_shade * 32.0;
        // pixmap.fade_tables is 64 rows x 256 columns (not 256 rows) --
        // matches UI_REMAP_FRAGMENT_SHADER's identical /64.0 normalisation.
        fade_v = (clamp(fade_row, 0.0, 63.0) + 0.5) / 64.0;
    }

    // --- Sample palette index from atlas (always needed) ---
    float pal_idx;  // raw palette index [0,1] from R8 atlas (nearest or non-bilinear)
    vec4  col;      // decoded RGBA colour (after palette lookup + optional shading)

    if (u_tile_filter == 1) {
        // Palette-correct bilinear: sample 4 neighbours, decode each, then lerp.
        vec2 tex_size = vec2(textureSize(u_tile_atlas, 0).xy);
        vec2 px   = v_uv * tex_size - 0.5;
        vec2 f    = fract(px);
        vec2 base = (floor(px) + 0.5) / tex_size;
        vec2 st   = 1.0 / tex_size;
        float layer = v_layer;
        float idx00 = texture(u_tile_atlas, vec3(base, layer)).r;
        float idx10 = texture(u_tile_atlas, vec3(base + vec2(st.x, 0.0), layer)).r;
        float idx01 = texture(u_tile_atlas, vec3(base + vec2(0.0, st.y), layer)).r;
        float idx11 = texture(u_tile_atlas, vec3(base + st, layer)).r;

        if (u_darkness_mode != 0) {
            // Palette-mode darkness: shade each neighbour through the fade table
            // independently before blending.  This prevents shading discontinuities
            // at tile boundaries (the "crawling" artifact seen when idx00 alone was
            // used for the LUT, snapping to a new palette row on each texel crossing).
            float r00 = texture(u_fade_table, vec2(idx00, fade_v)).r;
            float r10 = texture(u_fade_table, vec2(idx10, fade_v)).r;
            float r01 = texture(u_fade_table, vec2(idx01, fade_v)).r;
            float r11 = texture(u_fade_table, vec2(idx11, fade_v)).r;
            vec4 c00 = texture(u_palette, vec2(r00, 0.5));
            vec4 c10 = texture(u_palette, vec2(r10, 0.5));
            vec4 c01 = texture(u_palette, vec2(r01, 0.5));
            vec4 c11 = texture(u_palette, vec2(r11, 0.5));
            col = mix(mix(c00, c10, f.x), mix(c01, c11, f.x), f.y);
        } else {
            vec4 c00 = texture(u_palette, vec2(idx00, 0.5));
            vec4 c10 = texture(u_palette, vec2(idx10, 0.5));
            vec4 c01 = texture(u_palette, vec2(idx01, 0.5));
            vec4 c11 = texture(u_palette, vec2(idx11, 0.5));
            col = mix(mix(c00, c10, f.x), mix(c01, c11, f.x), f.y);
        }
        pal_idx = idx00;  // kept for LINEAR shade multiply below
    } else {
        pal_idx = texture(u_tile_atlas, vec3(v_uv, v_layer)).r;
        col = texture(u_palette, vec2(pal_idx, 0.5));
    }

    // Palette index 0 = void/transparent in DK1 paletted data.
    // The same convention applies here as in the sprite/UI shader (which also discards index 0).
    // Without this, world-geometry tiles whose data is all-zero bytes (e.g. the entrance-portal
    // centre column, or unclaimed portal-floor tiles whose atlas variation is empty) render as
    // a solid opaque black quad, which is visually wrong.  Making them transparent is correct:
    // the portal centre is a gateway void, and unset tiles should not occlude anything beneath.
    if (pal_idx < 0.5 / 255.0)
        discard;

    // --- Palette fade-table lookup (PALETTE and FOG modes, nearest path only) ---
    // In the bilinear path (u_tile_filter == 1) each neighbour is already shaded
    // above, so col already contains the fully shaded and blended colour.
    // In the nearest path we do the LUT lookup here.
    // Only sampled when darkness_mode != LINEAR: avoids reading unit 3 when the
    // fade-table texture may not yet be loaded, which would produce undefined colours.
    // Replicates the software renderer's pixmap.fade_tables[] exactly:
    //   fade_tables[row * 256 + palette_index] → remapped palette index.
    // Row 0 = fully dark, row 32 = full brightness (identity).
    // The texture is 256 wide x 64 tall -- all 64 rows are fade data.
    float remapped = pal_idx;      // fallback for LINEAR mode: identity remap
    vec3 pal_color = col.rgb;      // fallback for LINEAR mode: direct colour
    if (u_darkness_mode != 0 && u_tile_filter != 1)
    {
        remapped   = texture(u_fade_table, vec2(pal_idx, fade_v)).r;
        pal_color  = texture(u_palette,    vec2(remapped, 0.5)).rgb;
    }
    else if (u_darkness_mode != 0)
    {
        // Bilinear path: col already contains the blended shaded colour.
        // pal_color mirrors col for the fog/overlay code below.
        pal_color = col.rgb;
    }

    // --- Apply darkness mode ---
    if (u_darkness_mode == 1)
    {
        // Palette LUT: exact software renderer output — no further processing.
        fragColor = vec4(pal_color, 1.0);
    }
    else if (u_darkness_mode == 2)
    {
        // Animated fog: uses the palette LUT as the base colour (preserving
        // the authentic green-tinted darkness), then overlays scrolling fog
        // in dark areas for atmosphere.
        float fog_mask = 1.0 - clamp(raw_shade, 0.0, 1.0);

        if (fog_mask > 0.01)
        {
            vec2 fog_uv = gl_FragCoord.xy * 0.008
                        + vec2(u_time * u_fog_speed * 0.4,
                               u_time * u_fog_speed * 0.25);
            float n = fbm(fog_uv);
            vec3 fog_color = vec3(0.06, 0.10, 0.06);
            float fog_alpha = smoothstep(0.25, 0.65, n) * fog_mask * u_fog_density;
            fragColor = vec4(mix(pal_color, fog_color, fog_alpha), 1.0);
        }
        else
        {
            fragColor = vec4(pal_color, 1.0);
        }
    }
    else
    {
        // LINEAR: classic linear RGB multiply with tuning knobs.
        float shade = max(raw_shade, u_ambient);
        shade = pow(clamp(shade * u_shade_scale, 0.0, 1.0), u_shade_gamma);
        fragColor = vec4(col.rgb * shade, 1.0);
    }
}
)glsl";

// Flat-colour polygon shaders (QK_PolyMode0, QK_PolyMode4, QK_BasicPolygon)
constexpr const char* FLATPOLY_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec3 a_pos;    // x,y = screen pixel; z = NDC depth [-1,1]
layout(location = 1) in vec3 a_color;  // linear RGB [0,1]
uniform vec2 u_viewport;
out vec3 v_color;
void main()
{
    float ndc_x = a_pos.x / u_viewport.x * 2.0 - 1.0;
    float ndc_y = 1.0 - a_pos.y / u_viewport.y * 2.0;
    gl_Position = vec4(ndc_x, ndc_y, a_pos.z, 1.0);
    v_color = a_color;
}
)glsl";

constexpr const char* FLATPOLY_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec3 v_color;
out vec4 fragColor;
void main()
{
    fragColor = vec4(v_color, 1.0);
}
)glsl";

// Keeper-sprite (creature/object) shaders (P5.7.3a: core path only -- no
// depth-fail outline. Beat 3 added the non-instanced glow programs below,
// verbatim from develop, so the atlas-full/unknown-sprite-id fallback path
// (render_keepersprite_gpu()) gets the same family-aware additive glow the
// instanced fragment shader already computed inline via its flag bit).
constexpr const char* KSPR_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
uniform vec2  u_viewport;
uniform float u_z_ndc;
out vec2 v_uv;
void main()
{
    vec2 ndc;
    ndc.x = a_pos.x / u_viewport.x * 2.0 - 1.0;
    ndc.y = 1.0 - a_pos.y / u_viewport.y * 2.0;
    gl_Position = vec4(ndc, u_z_ndc, 1.0);
    v_uv = a_uv;
}
)glsl";

constexpr const char* KSPR_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_sprite;    // GL_R8  palette-index map (256x256)
uniform sampler2D u_palette;   // GL_RGBA8 colour table  (256x1)
uniform float     u_alpha;     // 1.0=solid, 0.5=transpar4, 0.25=transpar8
out vec4 fragColor;
void main()
{
    float idx = texture(u_sprite, v_uv).r;
    if (idx < (0.5 / 255.0)) discard;
    vec4 color = texture(u_palette, vec2(idx, 0.5));
    fragColor = vec4(color.rgb, u_alpha);
}
)glsl";

// Non-instanced additive-glow fragment shader (Beat 3), ported verbatim from
// develop. Reuses KSPR_VERTEX_SHADER -- no palette needed, the glow colour
// comes entirely from the DK glow-encoding index baked into the sprite's own
// pixels (1-64: family = code/8, row = code%8, intensity scales with row).
// Drawn with glBlendFunc(GL_ONE, GL_ONE) so the RGB delta adds directly onto
// the framebuffer -- see render_keepersprite_gpu()'s additive branch.
constexpr const char* KSPR_GLOW_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_sprite;
out vec4 fragColor;

// Per-row additive RGB step for each of the 8 glow families (8-bit normalised).
// Order: white, yellow, red, blue, green, purple, black/darken, orange.
// Values = (dR*4, dG*4, dB*4) / 255  where dR/dG/dB are from compute_alpha_tables().
const vec3 k_glow_step[8] = vec3[8](
    vec3(16.0, 16.0, 16.0) / 255.0,  // white:  dR=4, dG=4, dB=4
    vec3(24.0, 16.0,  0.0) / 255.0,  // yellow: dR=6, dG=4, dB=0
    vec3(24.0,  4.0,  4.0) / 255.0,  // red:    dR=6, dG=1, dB=1
    vec3( 8.0,  8.0, 24.0) / 255.0,  // blue:   dR=2, dG=2, dB=6
    vec3( 8.0, 24.0,  8.0) / 255.0,  // green:  dR=2, dG=6, dB=2
    vec3(12.0,  0.0, 12.0) / 255.0,  // purple: dR=3, dG=0, dB=3
    vec3( 0.0,  0.0,  0.0) / 255.0,  // black/darken -- no additive contribution
    vec3(24.0, 12.0,  4.0) / 255.0   // orange: dR=6, dG=3, dB=1
);

void main()
{
    // Sprite pixels 1-64 are DK glow-encoding indices.
    // code = px-1; family = code/8 (0=white..7=orange); row = code%8 (intensity)
    // Row 0 = no glow; family 6 = darken (no additive contribution).
    int px = int(texture(u_sprite, v_uv).r * 255.0 + 0.5);
    if (px < 1 || px > 64) discard;
    int code   = px - 1;
    int family = code / 8;
    int row    = code % 8;
    if (row == 0 || family == 6) discard;
    vec3 glow = clamp(k_glow_step[family] * float(row), 0.0, 1.0);
    fragColor = vec4(glow, 1.0);
}
)glsl";

// Atlas variant: u_sprite is a GL_TEXTURE_2D_ARRAY;
// u_layer selects the pre-decoded layer for this sprite.
constexpr const char* KSPR_ARRAY_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2DArray u_sprite;   // GL_R8 texture array, one layer per unique sprite
uniform sampler2D      u_clut;     // GL_RGBA8 256xN CLUT -- row 0=identity, rows 1..N-1=remaps
uniform float          u_alpha;    // 1.0=solid, 0.5=transpar4, 0.25=transpar8
uniform float          u_layer;    // layer index in the sprite array
uniform float          u_clut_v;   // V texcoord selecting the CLUT row
out vec4 fragColor;
void main()
{
    float idx = texture(u_sprite, vec3(v_uv, u_layer)).r;
    if (idx < (0.5 / 255.0)) discard;
    vec4 color = texture(u_clut, vec2(idx, u_clut_v));
    fragColor = vec4(color.rgb, color.a * u_alpha);
}
)glsl";

// Array-atlas variant of the glow shader -- additive sprites cached in atlas.
// Same glow families/steps as KSPR_GLOW_FRAGMENT_SHADER (see there for docs).
constexpr const char* KSPR_ARRAY_GLOW_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2DArray u_sprite;
uniform float          u_layer;
out vec4 fragColor;

const vec3 k_glow_step[8] = vec3[8](
    vec3(16.0, 16.0, 16.0) / 255.0,
    vec3(24.0, 16.0,  0.0) / 255.0,
    vec3(24.0,  4.0,  4.0) / 255.0,
    vec3( 8.0,  8.0, 24.0) / 255.0,
    vec3( 8.0, 24.0,  8.0) / 255.0,
    vec3(12.0,  0.0, 12.0) / 255.0,
    vec3( 0.0,  0.0,  0.0) / 255.0,
    vec3(24.0, 12.0,  4.0) / 255.0
);

void main()
{
    int px = int(texture(u_sprite, vec3(v_uv, u_layer)).r * 255.0 + 0.5);
    if (px < 1 || px > 64) discard;
    int code   = px - 1;
    int family = code / 8;
    int row    = code % 8;
    if (row == 0 || family == 6) discard;
    vec3 glow = clamp(k_glow_step[family] * float(row), 0.0, 1.0);
    fragColor = vec4(glow, 1.0);
}
)glsl";

// Depth-fail outline shaders (Beat 4) -- draw a flat owner-colour silhouette
// only where the creature sprite is occluded by geometry (depth test =
// GL_GREATER, set at the draw call site, not here).
constexpr const char* KSPR_OUTLINE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_sprite;
uniform vec4      u_outline_color;
out vec4 fragColor;
void main()
{
    float idx = texture(u_sprite, v_uv).r;
    if (idx < (0.5 / 255.0)) discard;
    fragColor = u_outline_color;
}
)glsl";

// Array-atlas variant of the outline shader (sampler2DArray).
constexpr const char* KSPR_ARRAY_OUTLINE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2DArray u_sprite;
uniform float          u_layer;
uniform vec4           u_outline_color;
out vec4 fragColor;
void main()
{
    float idx = texture(u_sprite, vec3(v_uv, u_layer)).r;
    if (idx < (0.5 / 255.0)) discard;
    fragColor = u_outline_color;
}
)glsl";

// Edge-detect variant of the outline shader (sampler2D). Emits outline
// colour only at sprite boundary pixels (where at least one cardinal
// neighbour has palette index 0). Texel step is 1/256 -- the atlas tile
// dimension is compile-time fixed at 256x256.
constexpr const char* KSPR_EDGE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_sprite;
uniform vec4      u_outline_color;
out vec4 fragColor;
const float kStep = 1.0 / 256.0;
const float kThr  = 0.5 / 255.0;
void main()
{
    float idx = texture(u_sprite, v_uv).r;
    if (idx < kThr) discard;
    float l = texture(u_sprite, v_uv + vec2(-kStep, 0.0)).r;
    float r = texture(u_sprite, v_uv + vec2( kStep, 0.0)).r;
    float u = texture(u_sprite, v_uv + vec2(0.0, -kStep)).r;
    float d = texture(u_sprite, v_uv + vec2(0.0,  kStep)).r;
    if (l >= kThr && r >= kThr && u >= kThr && d >= kThr) discard;
    fragColor = u_outline_color;
}
)glsl";

// Edge-detect variant -- array-atlas (sampler2DArray + u_layer).
constexpr const char* KSPR_ARRAY_EDGE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2DArray u_sprite;
uniform float          u_layer;
uniform vec4           u_outline_color;
out vec4 fragColor;
const float kStep = 1.0 / 256.0;
const float kThr  = 0.5 / 255.0;
void main()
{
    float idx = texture(u_sprite, vec3(v_uv, u_layer)).r;
    if (idx < kThr) discard;
    float l = texture(u_sprite, vec3(v_uv + vec2(-kStep, 0.0), u_layer)).r;
    float r = texture(u_sprite, vec3(v_uv + vec2( kStep, 0.0), u_layer)).r;
    float u = texture(u_sprite, vec3(v_uv + vec2(0.0, -kStep), u_layer)).r;
    float d = texture(u_sprite, vec3(v_uv + vec2(0.0,  kStep), u_layer)).r;
    if (l >= kThr && r >= kThr && u >= kThr && d >= kThr) discard;
    fragColor = u_outline_color;
}
)glsl";

constexpr const char* KSPR_INST_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_corner;  // unit quad corner, (0,0)..(1,1)
layout(location = 1) in vec4 a_rect;    // instance: dst x, y, w, h (screen px)
layout(location = 2) in vec2 a_uvext;   // instance: uv extent (src_w/dim, src_h/dim)
layout(location = 3) in vec4 a_misc;    // instance: layer, clut_v, alpha, z_ndc
layout(location = 4) in uint a_flags;   // instance: bit0 = flip_h, bit1 = additive
uniform vec2 u_viewport;
out vec2 v_uv;
flat out vec3 v_lca;    // layer, clut_v, alpha
flat out uint v_flags;
void main()
{
    vec2 px = a_rect.xy + a_corner * a_rect.zw;
    vec2 ndc;
    ndc.x = px.x / u_viewport.x * 2.0 - 1.0;
    ndc.y = 1.0 - px.y / u_viewport.y * 2.0;
    gl_Position = vec4(ndc, a_misc.w, 1.0);
    float u = ((a_flags & 1u) != 0u) ? (1.0 - a_corner.x) : a_corner.x;
    v_uv    = vec2(u * a_uvext.x, a_corner.y * a_uvext.y);
    v_lca   = a_misc.xyz;
    v_flags = a_flags;
}
)glsl";

constexpr const char* KSPR_INST_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
flat in vec3 v_lca;    // layer, clut_v, alpha
flat in uint v_flags;  // bit1 = additive glow
uniform sampler2DArray u_sprite;   // GL_R8 decode atlas, one layer per sprite
uniform sampler2D      u_clut;     // 256xN CLUT -- row 0 identity, rows 1..N remaps
out vec4 fragColor;

// Per-row additive RGB step for each of the 8 glow families (8-bit normalised).
// Order: white, yellow, red, blue, green, purple, black/darken, orange.
// Values = (dR*4, dG*4, dB*4) / 255  where dR/dG/dB are from compute_alpha_tables().
const vec3 k_glow_step[8] = vec3[8](
    vec3(16.0, 16.0, 16.0) / 255.0,
    vec3(24.0, 16.0,  0.0) / 255.0,
    vec3(24.0,  4.0,  4.0) / 255.0,
    vec3( 8.0,  8.0, 24.0) / 255.0,
    vec3( 8.0, 24.0,  8.0) / 255.0,
    vec3(12.0,  0.0, 12.0) / 255.0,
    vec3( 0.0,  0.0,  0.0) / 255.0,
    vec3(24.0, 12.0,  4.0) / 255.0
);

void main()
{
    float idx = texture(u_sprite, vec3(v_uv, v_lca.x)).r;
    if ((v_flags & 2u) != 0u)
    {
        // Additive glow: alpha 0 makes (ONE, ONE_MINUS_SRC_ALPHA) act as (ONE, ONE).
        int px = int(idx * 255.0 + 0.5);
        if (px < 1 || px > 64) discard;
        int code   = px - 1;
        int family = code / 8;
        int row    = code % 8;
        if (row == 0 || family == 6) discard;
        vec3 glow = clamp(k_glow_step[family] * float(row), 0.0, 1.0);
        fragColor = vec4(glow, 0.0);
    }
    else
    {
        if (idx < (0.5 / 255.0)) discard;
        vec4 c = texture(u_clut, vec2(idx, v_lca.y));
        float a = c.a * v_lca.z;
        fragColor = vec4(c.rgb * a, a);  // premultiplied
    }
}
)glsl";

// Instanced depth-fail outline (Beat 4): flat owner colour where the sprite
// is behind geometry (drawn with glDepthFunc(GL_GREATER) before the main
// instanced pass -- see flush_keeper_sprite_instances()).
constexpr const char* KSPR_INST_OUTLINE_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec2 a_corner;
layout(location = 1) in vec4 a_rect;
layout(location = 2) in vec2 a_uvext;
layout(location = 3) in vec3 a_lzf;     // instance: layer, z_ndc, flip (0/1)
layout(location = 4) in vec4 a_color;   // instance: outline rgba
uniform vec2 u_viewport;
out vec2 v_uv;
flat out float v_layer;
flat out vec4  v_color;
void main()
{
    vec2 px = a_rect.xy + a_corner * a_rect.zw;
    vec2 ndc;
    ndc.x = px.x / u_viewport.x * 2.0 - 1.0;
    ndc.y = 1.0 - px.y / u_viewport.y * 2.0;
    gl_Position = vec4(ndc, a_lzf.y, 1.0);
    float u = (a_lzf.z > 0.5) ? (1.0 - a_corner.x) : a_corner.x;
    v_uv    = vec2(u * a_uvext.x, a_corner.y * a_uvext.y);
    v_layer = a_lzf.x;
    v_color = a_color;
}
)glsl";

constexpr const char* KSPR_INST_OUTLINE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
flat in float v_layer;
flat in vec4  v_color;
uniform sampler2DArray u_sprite;
out vec4 fragColor;
void main()
{
    float idx = texture(u_sprite, vec3(v_uv, v_layer)).r;
    if (idx < (0.5 / 255.0)) discard;
    fragColor = vec4(v_color.rgb * v_color.a, v_color.a);  // premultiplied
}
)glsl";

// Instanced edge-detect: only emit colour at sprite boundary pixels.
// Shares the same vertex shader and VAO as the instanced outline.
constexpr const char* KSPR_INST_EDGE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
flat in float v_layer;
flat in vec4  v_color;
uniform sampler2DArray u_sprite;
out vec4 fragColor;
const float kStep = 1.0 / 256.0;
const float kThr  = 0.5 / 255.0;
void main()
{
    float idx = texture(u_sprite, vec3(v_uv, v_layer)).r;
    if (idx < kThr) discard;
    float l = texture(u_sprite, vec3(v_uv + vec2(-kStep, 0.0), v_layer)).r;
    float r = texture(u_sprite, vec3(v_uv + vec2( kStep, 0.0), v_layer)).r;
    float u = texture(u_sprite, vec3(v_uv + vec2(0.0, -kStep), v_layer)).r;
    float d = texture(u_sprite, vec3(v_uv + vec2(0.0,  kStep), v_layer)).r;
    if (l >= kThr && r >= kThr && u >= kThr && d >= kThr) discard;
    fragColor = vec4(v_color.rgb * v_color.a, v_color.a);  // premultiplied
}
)glsl";

// Creature-shadow shaders (P5.7.4). Samples the same GL_TEXTURE_2D_ARRAY
// the keeper-sprite atlas already builds (resolve_atlas_layer(), shared
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
uniform vec2 u_viewport;
out vec2 v_uv;
flat out float v_layer;
flat out float v_darken;
void main()
{
    float ndc_x = a_pos.x / u_viewport.x * 2.0 - 1.0;
    float ndc_y = 1.0 - a_pos.y / u_viewport.y * 2.0;
    gl_Position = vec4(ndc_x, ndc_y, a_z_ndc, 1.0);
    v_uv = a_uv;
    v_layer = a_layer;
    v_darken = a_darken;
}
)glsl";

constexpr const char* SHADOW_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
flat in float v_layer;
flat in float v_darken;
uniform sampler2DArray u_sprite;
out vec4 fragColor;
void main()
{
    float idx = texture(u_sprite, vec3(v_uv, v_layer)).r;
    if (idx < (0.5 / 255.0)) discard;
    float keep = 1.0 - v_darken;
    fragColor = vec4(keep, keep, keep, 1.0);
}
)glsl";

/******************************************************************************/
// Possession lens (P5.8a)
/******************************************************************************/

// Shared by all three lens composite passes: a static NDC unit quad, drawn
// with glViewport already set to the destination viewport sub-rect, so
// a_pos's -1..1 range maps exactly onto that sub-rect. v_uv (0..1) is only
// used for sampling the source scene/effect textures, not for positioning.
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

// Truecolor mist blend -- NOT a port of develop's palette-index-exact
// LENS_MIST_FRAGMENT_SHADER/LENS_MIST_ACCURATE_FRAGMENT_SHADER (this
// branch's captured scene texture is already RGB, the palette index is
// gone by this point). Reproduces the CPU CMistFade::Render() density
// calculation exactly (same two-layer wrap-around texture sampling, same
// (primary+secondary)>>3 combine clamped to 0..32), then blends the scene
// toward a grey target driven by mist_lightness (0=dark, 63=light, per
// config_lenses.c's own field comment) rather than reproducing
// pixmap.fade_tables' exact palette-index remap.
// Unit 0 = captured scene. Unit 1 = 256x256 R8 mist density texture.
constexpr const char* LENS_MIST_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_scene;
uniform sampler2D u_mist;
uniform vec2 u_src_off;      // (viewport_x / tex_w, 0) -- see the CPU source-Y asymmetry note below
uniform vec2 u_src_scale;    // (viewport_w / tex_w, viewport_h / tex_h)
uniform vec2 u_pos;          // primary layer offset, 0..255 wrapped
uniform vec2 u_sec;          // secondary layer offset, 0..255 wrapped
uniform float u_lightness;   // 0..63
out vec4 fragColor;
void main()
{
    // CPU (draw_creature_view()) only offsets the SOURCE sample by
    // viewport_x, never by viewport_y -- the destination write is
    // positioned by both, via dst_offset. Replicated here bug-for-bug via
    // u_src_off's Y component always being 0 -- see the P5.8a plan.
    vec2 src_uv = u_src_off + v_uv * u_src_scale;
    vec3 scene = texture(u_scene, src_uv).rgb;

    // Reference-space (640x480) virtual coords, matching CMistFade::Render()'s
    // fixed-point scale_x/scale_y.
    vec2 virtual_xy = v_uv * vec2(640.0, 480.0);

    float p2 = mod(u_pos.x + virtual_xy.x, 256.0);
    float c2 = mod(u_pos.y + virtual_xy.y, 256.0);
    float c1 = mod(u_sec.y - virtual_xy.x, 256.0);
    float p1 = mod(u_sec.x - virtual_xy.y, 256.0);

    float primary   = texture(u_mist, vec2((p2 + 0.5) / 256.0, (c2 + 0.5) / 256.0)).r * 255.0;
    float secondary = texture(u_mist, vec2((p1 + 0.5) / 256.0, (c1 + 0.5) / 256.0)).r * 255.0;

    float density = clamp((primary + secondary) / 8.0, 0.0, 32.0) / 32.0;
    vec3 fog_color = vec3(u_lightness / 63.0);

    fragColor = vec4(mix(scene, fog_color, density), 1.0);
}
)glsl";

// Displacement/flyeye distortion -- samples the captured scene through a
// precomputed per-pixel remap table (identical layout to
// DisplaceLookupEntry/FlyeyeLookupEntry: src_x,src_y in viewport-local pixel
// space), so the distortion shape is pixel-identical to the CPU tables, not
// a procedural re-derivation. Unit 0 = captured scene. Unit 1 = GL_RG16UI
// remap table, sized viewport_w x viewport_h.
constexpr const char* LENS_REMAP_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_scene;
uniform usampler2D u_remap;
uniform vec2 u_src_off;      // (viewport_x / tex_w, 0) -- see the mist shader's asymmetry note
uniform vec2 u_tex_size;     // captured scene texture size, texels
out vec4 fragColor;
void main()
{
    ivec2 remap_size = textureSize(u_remap, 0);
    ivec2 texel = ivec2(v_uv * vec2(remap_size));
    texel = clamp(texel, ivec2(0), remap_size - ivec2(1));
    uvec2 src_xy = texelFetch(u_remap, texel, 0).rg;

    vec2 src_uv = u_src_off + vec2(src_xy) / u_tex_size;
    fragColor = vec4(texture(u_scene, src_uv).rgb, 1.0);
}
)glsl";

// Overlay alpha composite -- overlay stretches to exactly fill the viewport
// (matches CPU's stretch-to-fit scale_x/scale_y), so v_uv doubles as the
// overlay's own UV with no extra offset math. Palette index 255 is the
// transparent sentinel (OverlayEffect.cpp), passed through unblended.
// Unit 0 = captured scene. Unit 1 = R8 palette-index overlay texture.
// Unit 2 = 256x1 RGBA8 palette (shared with the world/UI passes).
constexpr const char* LENS_OVERLAY_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_scene;
uniform sampler2D u_overlay;
uniform sampler2D u_palette;
uniform vec2 u_src_off;
uniform vec2 u_src_scale;
uniform float u_alpha;       // 0..1
out vec4 fragColor;
void main()
{
    vec2 src_uv = u_src_off + v_uv * u_src_scale;
    vec3 scene = texture(u_scene, src_uv).rgb;

    float idx = texture(u_overlay, v_uv).r;
    if (idx * 255.0 > 254.5)
    {
        fragColor = vec4(scene, 1.0);
        return;
    }
    vec3 overlay_rgb = texture(u_palette, vec2(idx, 0.5)).rgb;
    fragColor = vec4(mix(scene, overlay_rgb, u_alpha), 1.0);
}
)glsl";

/******************************************************************************/
// Parchment transition (P5.8b)
/******************************************************************************/

// Adapted directly from origin/develop's actual GLMapFadePass fragment
// shader (fetched via `git show`, not re-derived) -- a GLSL port of the
// CPU map_fade()'s elastic-pinch UV warp + additive weighted blend
// (engine_redraw.c), not a plain crossfade. Uses P5.8a's
// LENS_COMPOSITE_VERTEX_SHADER (a generic fullscreen-quad-to-viewport
// vertex shader -- no map-fade-specific vertex work needed, reused as-is).
// Unit 0 = captured parchment view. Unit 1 = captured 3D world view.
constexpr const char* MAPFADE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 v_uv;
uniform sampler2D u_parchment;
uniform sampler2D u_world;
uniform float u_step;        // 0.0..32.0
out vec4 fragColor;
void main()
{
    float a6 = u_step;
    float fx = v_uv.x;
    float fy = 1.0 - v_uv.y;
    const float xmax = 320.0;
    float wp = 32.0 - a6;
    float uv_px = clamp(fx + wp * (4.0 - 8.0 * fx) / xmax, 0.0, 1.0);
    float uv_py = clamp(fy + wp * 4.0 * (1.0 - 2.0 * fy) / xmax, 0.0, 1.0);
    float ww = a6;
    float uv_wx = clamp(fx + ww * (4.0 - 8.0 * fx) / xmax, 0.0, 1.0);
    float uv_wy = clamp(fy + ww * 4.0 * (1.0 - 2.0 * fy) / xmax, 0.0, 1.0);
    float samp_py = 1.0 - uv_py;
    float samp_wy = 1.0 - uv_wy;
    float f_parch = wp / 32.0;
    float f_world = ww / 32.0;
    vec3 c_parch = texture(u_parchment, vec2(uv_px, samp_py)).rgb * f_parch;
    vec3 c_world = texture(u_world,     vec2(uv_wx, samp_wy)).rgb * f_world;
    fragColor = vec4(clamp(c_parch + c_world, 0.0, 1.0), 1.0);
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

// Beat 10: window-frame overlay (compressed_window_draw(), front_landview.c)
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

// Beat 10: landview zoom-in/out transition (frontzoom_to_point(),
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

#endif // RENDERER_OPENGL_GLSHADERS_H
