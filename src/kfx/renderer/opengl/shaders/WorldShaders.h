#ifndef RENDERER_OPENGL_WORLDSHADERS_H
#define RENDERER_OPENGL_WORLDSHADERS_H

/** @file WorldShaders.h
 *     World tile shaders.
 */

// World rendering shaders (tile geometry only, no sprites/shadows)
constexpr const char* WORLD_VERTEX_SHADER = R"glsl(
#version 330 core
layout(location = 0) in vec3  a_pos;
layout(location = 1) in vec2  a_uv;
layout(location = 2) in float a_shade;
layout(location = 3) in vec2  a_stl;       // subtile coords for lightmap (mode 1)
layout(location = 4) in float a_camera_z;  // camera-space Z for perspective correction
layout(location = 5) in float a_layer;     // texture array layer (atlas variation)
layout(location = 6) in vec3  aWorldPos;   // pre-projection world-space position
layout(location = 7) in float a_tile;      // tile index within the atlas layer
out vec2  v_uv;
out float v_shade;
out vec2  v_stl;
flat out float v_layer;
flat out int v_tile;
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
    v_tile      = int(a_tile + 0.5);
    vWorldPos   = aWorldPos;
}
)glsl";

constexpr const char* WORLD_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2  v_uv;                          // texel coords within the tile, wrapped below
in float v_shade;
in vec2  v_stl;                         // subtile coords [0..511], mode 1 only
flat in float v_layer;                  // texture array layer (atlas variation)
flat in int v_tile;                     // tile index within the layer
in vec3  vWorldPos;                     // reserved for future dynamic lighting / shadow mapping
uniform sampler2DArray u_tile_atlas;    // R8 palette-index atlas array (unit 0)
uniform int        u_set_layer[32];     // atlas layer of each tile set, -1 = none
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

// Palette index of a tile texel. The coordinates wrap within the 32x32 tile, as the
// software rasteriser masks them, so scrolled liquid textures repeat.
float tile_index(ivec2 texel)
{
    int cols = textureSize(u_tile_atlas, 0).x / 32;
    ivec2 origin = ivec2(v_tile % cols, v_tile / cols) * 32;
    int layer = u_set_layer[clamp(int(v_layer + 0.5), 0, 31)];
    if (layer < 0)
        return 0.0;
    return texelFetch(u_tile_atlas, ivec3(origin + (texel & 31), layer), 0).r;
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
        vec2 px    = v_uv - 0.5;
        vec2 f     = fract(px);
        ivec2 base = ivec2(floor(px));
        float idx00 = tile_index(base);
        float idx10 = tile_index(base + ivec2(1, 0));
        float idx01 = tile_index(base + ivec2(0, 1));
        float idx11 = tile_index(base + ivec2(1, 1));

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
        pal_idx = tile_index(ivec2(floor(v_uv)));
        col = texture(u_palette, vec2(pal_idx, 0.5));
    }

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

#endif // RENDERER_OPENGL_WORLDSHADERS_H
