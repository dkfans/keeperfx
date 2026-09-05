/******************************************************************************/
// Dungeon Keeper - Renderer Configuration
/******************************************************************************/
/** @file RendererSettings.h
 *     Run-time configurable renderer knobs. Ported from develop (Beat 4) --
 *     develop's own config surface for creature_outline_class_mask/shadow_type
 *     didn't exist on this branch at all before this beat; every hardcoded
 *     0.5/0.25/1.0 literal scattered through GLWorldViewRenderer.cpp for
 *     transpar4_alpha/transpar8_alpha/shade_scale/etc. (each individually
 *     disclosed as "g_renderer_settings doesn't exist on this branch") now
 *     has a real home.
 * @par Design:
 *     A C-compatible POD struct so that C translation units (config parsing)
 *     can read and modify settings without pulling in C++ headers. The active
 *     settings are stored in the global g_renderer_settings and applied to the
 *     active renderer via RendererApplySettings().
 *
 *     Default values (set by RendererSettings_Reset) preserve the original
 *     software-renderer behaviour exactly -- no visual change on first run.
 *
 * @par Persistence:
 *     Read/written from renderer_prefs.ini in PlatformManager_GetUserPrefDir().
 *
 * @par Not ported from develop (disclosed, not guessed at):
 *     develop's other RendererSettings consumers -- console_cmd.c's debug
 *     commands, RendMenuOverlay.c/h (a non-ImGui settings overlay), and
 *     ImGuiRendererPanel.cpp -- are separate front-end/menu UI surfaces, not
 *     part of the renderer subsystems this beat's plan scoped (and ImGui was
 *     already dropped from this branch's own architecture, see the
 *     upstreaming plan doc). Only the module itself (struct/Reset/Sanitize/
 *     Load/Save/Apply) and the GL-side fields Beat 4 actually wires up
 *     (creature_outline_mode/_alpha/_class_mask, shadow_type is declared but
 *     circle-shadow rendering itself is a later fast-follow, and the
 *     already-existing-but-hardcoded shade/filter/transpar/glow/fog uniforms
 *     from P5.7.2b/P5.7.3a) are ported this beat.
 */
/******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Palette mode constants (palette_mode field)
 * ------------------------------------------------------------------------- */
/** 256-colour DK indexed atlas: all sprite textures stored as GL_R8 palette
 *  indices; a palette LUT is uploaded once per palette change. Exact colour
 *  match with software renderer. Startup-only -- requires atlas re-upload. */
#define RENDERER_PALETTE_INDEXED    0

/** True-colour RGBA atlas: sprites pre-decoded to GL_RGBA8 at load time.
 *  Not implemented on this branch (no truecolour atlas path exists) --
 *  declared for struct/persistence parity with develop; selecting it is a
 *  no-op here. Startup-only. */
#define RENDERER_PALETTE_TRUECOLOUR 1

/* ---------------------------------------------------------------------------
 * Texture filter constants (tile_filter, ui_sprite_filter, shadow_filter)
 * ------------------------------------------------------------------------- */
#define RENDERER_FILTER_NEAREST 0   /**< GL_NEAREST -- pixel-art style (default) */
#define RENDERER_FILTER_LINEAR  1   /**< GL_LINEAR  -- bilinear smoothing        */

/* ---------------------------------------------------------------------------
 * Remap/tint mode constants (tint_mode field)
 * ------------------------------------------------------------------------- */
/** Palette remap performed on GPU via remap shader. Not implemented on this
 *  branch (sprite remap is CPU-resolved into the CLUT at submit time) --
 *  declared for parity; selecting it is a no-op here. */
#define RENDERER_TINT_GPU   0
/** Palette remap performed on CPU (this branch's only real path today). */
#define RENDERER_TINT_CPU   2

/* ---------------------------------------------------------------------------
 * Blend mode constants (glow_blend_mode field)
 * ------------------------------------------------------------------------- */
#define RENDERER_GLOW_ADDITIVE 0  /**< Additive blending (classic DK look, default) */
#define RENDERER_GLOW_SCREEN   1  /**< Screen blend (softer, no over-exposure)       */

/* ---------------------------------------------------------------------------
 * Zoom-box render mode constants (zoom_box_mode field)
 * ------------------------------------------------------------------------- */
/** Default: flat palette-indexed overhead map tiles. */
#define RENDERER_ZBM_OVERHEAD   0
/** Picture-in-picture isometric 3D view via FBO. Not implemented on this
 *  branch -- declared for parity; selecting it is a no-op here. */
#define RENDERER_ZBM_ISOMETRIC  1

/* ---------------------------------------------------------------------------
 * Darkness mode constants (darkness_mode field)
 * ------------------------------------------------------------------------- */
/** Linear RGB multiply -- clean fade toward black. */
#define RENDERER_DARKNESS_LINEAR     0
/** Palette fade-table LUT -- samples the original DK fade table on the GPU.
 *  Reproduces the non-linear hue shifts of the software renderer. This
 *  branch's world shader already implements this mode (P5.7.2a's
 *  u_darkness_mode uniform) -- default here, matching what was hardcoded. */
#define RENDERER_DARKNESS_PALETTE    1
/** Animated fog -- dark areas receive a scrolling noise-based fog overlay
 *  instead of a flat colour multiply. Purely cosmetic enhancement; this
 *  branch's world shader already implements this mode too. */
#define RENDERER_DARKNESS_FOG        2

/* ---------------------------------------------------------------------------
 * Lens colour-fidelity mode constants (lens_color_mode field)
 * ------------------------------------------------------------------------- */
/** Accurate: possessed-creature lens effects reproduced in 8-bit
 *  palette-index space, bit-for-bit identical to the software renderer. */
#define RENDERER_LENS_COLOR_ACCURATE  0
/** Truecolor: lens effects operate directly on the decoded RGBA scene. Not
 *  consumed by this branch's lens code (P5.8a) yet -- declared for parity. */
#define RENDERER_LENS_COLOR_TRUECOLOR 1

/* ---------------------------------------------------------------------------
 * Lighting pipeline mode constants (lighting_mode field)
 * ------------------------------------------------------------------------- */
/** Software-accurate lighting: per-vertex Gouraud shade from the DK fade
 *  table. Emulates the software renderer exactly. This branch's world
 *  shader already implements this mode (P5.7.2a's u_lighting_mode). */
#define RENDERER_LIGHTING_SOFTWARE 0
/** Modern GPU lighting: per-fragment lightmap sampling + dynamic point
 *  lights. This branch's world shader already implements this mode too
 *  (the lightmap texture/u_lightmap path). */
#define RENDERER_LIGHTING_MODERN   1

/* ---------------------------------------------------------------------------
 * Shadow type constants (shadow_type field)
 * ------------------------------------------------------------------------- */
/** Disable creature shadows entirely. */
#define RENDERER_SHADOW_OFF    0
/** Projected sprite shadow from the nearest 1 light source per creature. */
#define RENDERER_SHADOW_1      1
#define RENDERER_SHADOW_2      2
#define RENDERER_SHADOW_3      3
/** Projected sprite shadow from up to 4 light sources per creature. Default. */
#define RENDERER_SHADOW_4      4
/** Pre-baked soft circle blob under each creature. Declared for parity;
 *  the instanced circle-shadow GL pass itself is a later fast-follow on
 *  this branch (Beat 4 disclosed this as lower priority -- visual variety,
 *  not a correctness bug) -- selecting it currently falls back to
 *  RENDERER_SHADOW_4's existing behaviour, same as develop's own documented
 *  software-renderer fallback. */
#define RENDERER_SHADOW_CIRCLE 5

/* ---------------------------------------------------------------------------
 * Creature outline mode constants (creature_outline_mode field)
 * ------------------------------------------------------------------------- */
/** No depth-fail outline -- occluded creatures are not highlighted. */
#define RENDERER_OUTLINE_NONE       0
/** Full silhouette fill -- every visible occluded pixel is coloured. */
#define RENDERER_OUTLINE_SILHOUETTE 1
/** Edge highlight -- only boundary pixels of the occluded sprite are shown. */
#define RENDERER_OUTLINE_EDGE       2

/* ---------------------------------------------------------------------------
 * Main settings struct
 * ------------------------------------------------------------------------- */
typedef struct RendererSettings {
    /** Atlas colour mode. RENDERER_PALETTE_INDEXED (0) or
     *  RENDERER_PALETTE_TRUECOLOUR (1). Default: INDEXED. TRUECOLOUR is a
     *  no-op on this branch (see the constant's own doc comment). */
    int   palette_mode;

    /* --- Shade / brightness knobs (applied as uniforms each frame) --- */

    /** Fullbright toggle. 0 = normal shading; 1 = bypass v_shade (all
     *  geometry rendered at maximum brightness). Default: 0. */
    float shade_fullbright;

    /** Ambient darkness floor. Fraction of full brightness that even the
     *  darkest geometry receives. 0.0 = pure black in shadow (original),
     *  1.0 = no shading at all. Range [0.0, 1.0]. Default: 0.0. */
    float shade_ambient;

    /** Overall brightness multiplier applied after DK's v_shade lookup.
     *  1.0 = original brightness. Range > 0. Default: 1.0. */
    float shade_scale;

    /** Shade curve gamma. Values > 1.0 brighten mid-tones; < 1.0 deepen
     *  shadows. Applied as pow(shade, 1/shade_gamma) to the shade fraction.
     *  1.0 = linear (original). Default: 1.0. */
    float shade_gamma;

    /* --- Texture filtering --- */

    /** World tile texture filter. NEAREST (0) or LINEAR (1). Default: NEAREST. */
    int   tile_filter;

    /** UI/panel sprite texture filter. Not yet consumed by GLUIRenderer.
     *  Default: NEAREST. */
    int   ui_sprite_filter;

    /** Shadow texture filter. Not yet consumed by the shadow pass.
     *  Default: NEAREST. */
    int   shadow_filter;

    /* --- Transparency --- */

    /** Alpha value for Lb_SPRITE_TRANSPAR4 sprites (one-quarter alpha).
     *  Default: 0.5 (matches original visual weight). */
    float transpar4_alpha;

    /** Alpha value for Lb_SPRITE_TRANSPAR8 sprites (one-eighth alpha).
     *  Default: 0.25. */
    float transpar8_alpha;

    /* --- Glow / additive passes --- */

    /** Scale multiplier for k_glow_step contribution. 1.0 = original,
     *  0.0 = no glow. Not yet consumed by the glow shaders (Beat 3 ported
     *  them with a fixed k_glow_step table) -- declared for parity.
     *  Default: 1.0. */
    float glow_intensity;

    /** Glow blend equation. ADDITIVE (0) or SCREEN (1). Not yet consumed --
     *  Beat 3's glow shaders are hardcoded additive. Default: ADDITIVE. */
    int   glow_blend_mode;

    /* --- Shadow pass --- */

    /** Multiplier on the u_darkness uniform. 1.0 = original shadow depth.
     *  Not yet consumed by the shadow pass. Default: 1.0. */
    float shadow_darkness_scale;

    /** Shadow depth test. 0 = disabled (original); 1 = wall-clipped (future).
     *  Not yet consumed. Default: 0. */
    int   shadow_depth_test;

    /** Maximum creature shadows rendered per frame. Not yet consumed.
     *  0 = unlimited (default). */
    int   shadow_max_count;

    /** RGBA colour of the shadow tint. Not yet consumed by the shadow pass.
     *  Default: (0, 0, 0, 1) = original black shadow. */
    float shadow_colour_r;
    float shadow_colour_g;
    float shadow_colour_b;
    float shadow_colour_a;

    /** Creature shadow rendering style. See the RENDERER_SHADOW_* constants
     *  above. Default: RENDERER_SHADOW_4 (this branch's only implemented
     *  shadow path so far -- CIRCLE falls back to it, see that constant's
     *  own doc comment). */
    int   shadow_type;

    /** Zoom-box render mode. Not consumed on this branch (no picture-in-
     *  picture isometric zoom-box exists) -- declared for parity.
     *  Default: RENDERER_ZBM_OVERHEAD. */
    int   zoom_box_mode;

    /* --- Palette remap / tint mode --- */

    /** Remap mode. Default: RENDERER_TINT_CPU (this branch's only real
     *  path -- see that constant's own doc comment). */
    int   tint_mode;

    /** Tint strength. Not yet consumed. 1.0 = full remap applied.
     *  Default: 1.0. */
    float tint_strength;

    /* --- Debug / developer knobs --- */

    /** Wireframe mode. Not yet consumed. 0 = off; 1 = GL_LINE. Default: 0. */
    int   wireframe;

    /** Depth buffer visualisation. Not yet consumed. Default: 0. */
    int   show_depth;

    /** GUI hit-box debug overlay. Not yet consumed on this branch (no
     *  equivalent debug overlay exists). Default: 0. */
    int   debug_gui_hitboxes;

    /* --- Lighting pipeline --- */

    /** Lighting pipeline mode. See the RENDERER_LIGHTING_* constants above
     *  -- both are already implemented by this branch's world shader
     *  (P5.7.2a). Default: RENDERER_LIGHTING_SOFTWARE. */
    int   lighting_mode;

    /* --- Darkness mode --- */

    /** How dark areas are shaded. See the RENDERER_DARKNESS_* constants
     *  above -- all three are already implemented by this branch's world
     *  shader. Default: RENDERER_DARKNESS_PALETTE, matching develop's own
     *  default -- NOTE this is a real, disclosed visual-default change:
     *  before this beat the uniform was hardcoded to LINEAR (0) at shader
     *  init (there was no settings module to read a real default from). */
    int   darkness_mode;

    /** Fog animation speed (darkness_mode == FOG only). 1.0 = default speed. */
    float fog_speed;

    /** Fog density / opacity (darkness_mode == FOG only). Range [0,1].
     *  0.0 = transparent; 1.0 = fully opaque fog. Default: 0.4. */
    float fog_density;

    /* --- Lens colour fidelity (possession eye-lens effects) --- */

    /** Colour mode for possessed-creature lens effects. Not yet consumed by
     *  this branch's lens code (P5.8a is always palette-accurate) --
     *  declared for parity. Default: RENDERER_LENS_COLOR_ACCURATE. */
    int   lens_color_mode;

    /* --- Creature depth-fail outline --- */

    /** Outline mode for occluded creatures. See the RENDERER_OUTLINE_*
     *  constants above. Default: RENDERER_OUTLINE_SILHOUETTE. */
    int   creature_outline_mode;

    /** Alpha of the depth-fail creature outline. Range [0.0, 1.0].
     *  Default: 0.5. */
    float creature_outline_alpha;

    /** Bitmask of ThingClass (TCls_*) values that receive the depth-fail
     *  outline. Bit N is set iff things with class_id == N should be
     *  outlined. Evaluated by the engine (engine_render.c) before passing
     *  wants_outline to the renderer; the renderer itself never inspects
     *  ThingClass. Default: (1<<TCls_Creature)|(1<<TCls_DeadCreature). */
    unsigned int creature_outline_class_mask;

} RendererSettings;

/** Global active renderer settings. Modified by config parsing; applied to
 *  the renderer via RendererApplySettings(). */
extern RendererSettings g_renderer_settings;

/** Reset g_renderer_settings to all defaults.
 *  Defaults preserve exact original software-renderer visual behaviour. */
void RendererSettings_Reset(void);

/** Clamp all fields in g_renderer_settings to their valid ranges.
 *  Call after loading from file to prevent out-of-range enum values from
 *  causing array-index or UB issues. Also rounds float-backed boolean
 *  fields (shade_fullbright) to 0.0f/1.0f. */
void RendererSettings_Sanitize(void);

/** Load renderer preferences from the platform user-pref directory.
 *  File: <PlatformManager_GetUserPrefDir()>/renderer_prefs.ini
 *  Missing file is silently ignored (first run). Only fields present in the
 *  file are updated; absent fields retain whatever value Reset() left them
 *  at. Calls RendererApplySettings() on success. */
void RendererSettings_Load(void);

/** Save the current g_renderer_settings to the platform user-pref directory.
 *  File: <PlatformManager_GetUserPrefDir()>/renderer_prefs.ini */
void RendererSettings_Save(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
