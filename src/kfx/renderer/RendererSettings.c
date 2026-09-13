/******************************************************************************/
// Dungeon Keeper - Renderer Configuration
/******************************************************************************/
/** @file RendererSettings.c
 *     Global renderer settings instance, reset, load and save functions.
 *     Ported from develop -- see RendererSettings.h's file header
 *     for what's deliberately not consumed yet on this branch.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/RendererSettings.h"
#include "kfx/renderer/RendererManager.h"
#include "kfx/platform/PlatformManager.h"
#include "thing_list.h" // TCls_* enum values for outline class mask default
#include <stdio.h>
#include <string.h>

RendererSettings g_renderer_settings;

void RendererSettings_Reset(void)
{
    /* Startup-only */
    g_renderer_settings.palette_mode          = RENDERER_PALETTE_INDEXED;
    g_renderer_settings.zoom_box_mode         = RENDERER_ZBM_OVERHEAD;

    /* Shade / brightness */
    g_renderer_settings.shade_fullbright      = 0.0f;
    g_renderer_settings.shade_ambient         = 0.0f;
    g_renderer_settings.shade_scale           = 1.0f;
    g_renderer_settings.shade_gamma           = 1.0f;

    /* Texture filtering */
    g_renderer_settings.tile_filter           = RENDERER_FILTER_NEAREST;
    g_renderer_settings.ui_sprite_filter      = RENDERER_FILTER_NEAREST;
    g_renderer_settings.shadow_filter         = RENDERER_FILTER_NEAREST;

    /* Transparency */
    g_renderer_settings.transpar4_alpha       = 0.5f;
    g_renderer_settings.transpar8_alpha       = 0.25f;

    /* Glow */
    g_renderer_settings.glow_intensity        = 1.0f;
    g_renderer_settings.glow_blend_mode       = RENDERER_GLOW_ADDITIVE;

    /* Shadow */
    g_renderer_settings.shadow_darkness_scale = 1.0f;
    g_renderer_settings.shadow_depth_test     = 0;
    g_renderer_settings.shadow_max_count      = 0;
    g_renderer_settings.shadow_colour_r       = 0.0f;
    g_renderer_settings.shadow_colour_g       = 0.0f;
    g_renderer_settings.shadow_colour_b       = 0.0f;
    g_renderer_settings.shadow_colour_a       = 1.0f;
    g_renderer_settings.shadow_type           = RENDERER_SHADOW_4;

    /* Remap/tint */
    g_renderer_settings.tint_mode             = RENDERER_TINT_CPU;
    g_renderer_settings.tint_strength         = 1.0f;

    /* Lighting pipeline */
    g_renderer_settings.lighting_mode         = RENDERER_LIGHTING_SOFTWARE;

    /* Darkness mode */
    g_renderer_settings.darkness_mode         = RENDERER_DARKNESS_PALETTE;
    g_renderer_settings.fog_speed             = 1.0f;
    g_renderer_settings.fog_density           = 0.4f;

    /* Lens colour fidelity -- default accurate (paletted, matches software) */
    g_renderer_settings.lens_color_mode       = RENDERER_LENS_COLOR_ACCURATE;

    /* Creature outline */
    g_renderer_settings.creature_outline_mode      = RENDERER_OUTLINE_SILHOUETTE;
    g_renderer_settings.creature_outline_alpha      = 0.5f;
    g_renderer_settings.creature_outline_class_mask = (1u << TCls_Creature) | (1u << TCls_DeadCreature);

    /* Debug */
    g_renderer_settings.wireframe             = 0;
    g_renderer_settings.show_depth            = 0;
    g_renderer_settings.debug_gui_hitboxes    = 0;
}

/* ---------------------------------------------------------------------------
 * Helpers for Sanitize
 * ------------------------------------------------------------------------- */
#define RS_CLAMP_I(field, lo, hi) \
    do { if (g_renderer_settings.field < (lo)) g_renderer_settings.field = (lo); \
         else if (g_renderer_settings.field > (hi)) g_renderer_settings.field = (hi); } while(0)

#define RS_CLAMP_F(field, lo, hi) \
    do { if (g_renderer_settings.field < (lo)) g_renderer_settings.field = (lo); \
         else if (g_renderer_settings.field > (hi)) g_renderer_settings.field = (hi); } while(0)

void RendererSettings_Sanitize(void)
{
    /* Int enum fields */
    RS_CLAMP_I(palette_mode,              0, 1);
    RS_CLAMP_I(darkness_mode,             0, 2);
    RS_CLAMP_I(lens_color_mode,           0, 1);
    RS_CLAMP_I(lighting_mode,             0, 1);
    RS_CLAMP_I(tile_filter,               0, 1);
    RS_CLAMP_I(ui_sprite_filter,          0, 1);
    RS_CLAMP_I(shadow_filter,             0, 1);
    RS_CLAMP_I(glow_blend_mode,           0, 1);
    RS_CLAMP_I(zoom_box_mode,             0, 1);
    RS_CLAMP_I(shadow_depth_test,         0, 1);
    RS_CLAMP_I(shadow_max_count,          0, 1024);
    RS_CLAMP_I(shadow_type,               0, 5);
    RS_CLAMP_I(creature_outline_mode,     0, 2);
    RS_CLAMP_I(wireframe,                 0, 1);
    RS_CLAMP_I(show_depth,                0, 1);
    RS_CLAMP_I(debug_gui_hitboxes,        0, 1);

    /* tint_mode has non-contiguous values (0, 2): default invalid to CPU */
    if (g_renderer_settings.tint_mode != RENDERER_TINT_GPU &&
        g_renderer_settings.tint_mode != RENDERER_TINT_CPU)
        g_renderer_settings.tint_mode = RENDERER_TINT_CPU;

    /* Float fields */
    RS_CLAMP_F(shade_ambient,             0.0f, 1.0f);
    RS_CLAMP_F(shade_scale,               0.1f, 3.0f);
    RS_CLAMP_F(shade_gamma,               0.1f, 3.0f);
    RS_CLAMP_F(glow_intensity,            0.0f, 2.0f);
    RS_CLAMP_F(transpar4_alpha,           0.0f, 1.0f);
    RS_CLAMP_F(transpar8_alpha,           0.0f, 1.0f);
    RS_CLAMP_F(shadow_darkness_scale,     0.0f, 2.0f);
    RS_CLAMP_F(shadow_colour_r,           0.0f, 1.0f);
    RS_CLAMP_F(shadow_colour_g,           0.0f, 1.0f);
    RS_CLAMP_F(shadow_colour_b,           0.0f, 1.0f);
    RS_CLAMP_F(shadow_colour_a,           0.0f, 1.0f);
    RS_CLAMP_F(creature_outline_alpha,    0.0f, 1.0f);
    RS_CLAMP_F(fog_speed,                 0.1f, 5.0f);
    RS_CLAMP_F(fog_density,               0.0f, 1.0f);
    RS_CLAMP_F(tint_strength,             0.0f, 1.0f);

    /* Float-backed boolean: snap to 0.0f or 1.0f */
    g_renderer_settings.shade_fullbright =
        (g_renderer_settings.shade_fullbright >= 0.5f) ? 1.0f : 0.0f;
}

#undef RS_CLAMP_I
#undef RS_CLAMP_F

#define RENDERER_PREFS_FILENAME "renderer_prefs.ini"

static void build_prefs_path(char* buf, size_t buflen)
{
    snprintf(buf, buflen, "%s/" RENDERER_PREFS_FILENAME,
             PlatformManager_GetUserPrefDir());
}

void RendererSettings_Load(void)
{
    char path[600];
    build_prefs_path(path, sizeof(path));

    FILE* f = fopen(path, "r");
    if (!f)
        return; // first run -- silently use defaults

    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        char key[64];
        // Support both int and float values in the same sscanf pass.
        float fval;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;
        if (sscanf(line, " %63[^= \t] = %f", key, &fval) != 2)
            continue;
        int ival = (int)fval;

        if      (strcmp(key, "palette_mode")               == 0) g_renderer_settings.palette_mode               = ival;
        else if (strcmp(key, "darkness_mode")              == 0) g_renderer_settings.darkness_mode              = ival;
        else if (strcmp(key, "lens_color_mode")            == 0) g_renderer_settings.lens_color_mode            = ival;
        else if (strcmp(key, "fog_speed")                  == 0) g_renderer_settings.fog_speed                  = fval;
        else if (strcmp(key, "fog_density")                == 0) g_renderer_settings.fog_density                = fval;
        else if (strcmp(key, "tile_filter")                == 0) g_renderer_settings.tile_filter                = ival;
        else if (strcmp(key, "ui_sprite_filter")           == 0) g_renderer_settings.ui_sprite_filter           = ival;
        else if (strcmp(key, "shadow_filter")              == 0) g_renderer_settings.shadow_filter              = ival;
        else if (strcmp(key, "zoom_box_mode")              == 0) g_renderer_settings.zoom_box_mode              = ival;
        else if (strcmp(key, "shade_fullbright")           == 0) g_renderer_settings.shade_fullbright           = fval;
        else if (strcmp(key, "shade_ambient")              == 0) g_renderer_settings.shade_ambient              = fval;
        else if (strcmp(key, "shade_scale")                == 0) g_renderer_settings.shade_scale                = fval;
        else if (strcmp(key, "shade_gamma")                == 0) g_renderer_settings.shade_gamma                = fval;
        else if (strcmp(key, "glow_intensity")             == 0) g_renderer_settings.glow_intensity             = fval;
        else if (strcmp(key, "glow_blend_mode")            == 0) g_renderer_settings.glow_blend_mode            = ival;
        else if (strcmp(key, "transpar4_alpha")            == 0) g_renderer_settings.transpar4_alpha            = fval;
        else if (strcmp(key, "transpar8_alpha")            == 0) g_renderer_settings.transpar8_alpha            = fval;
        else if (strcmp(key, "shadow_darkness_scale")      == 0) g_renderer_settings.shadow_darkness_scale      = fval;
        else if (strcmp(key, "shadow_depth_test")          == 0) g_renderer_settings.shadow_depth_test          = ival;
        else if (strcmp(key, "shadow_max_count")           == 0) g_renderer_settings.shadow_max_count           = ival;
        else if (strcmp(key, "shadow_colour_r")            == 0) g_renderer_settings.shadow_colour_r            = fval;
        else if (strcmp(key, "shadow_colour_g")            == 0) g_renderer_settings.shadow_colour_g            = fval;
        else if (strcmp(key, "shadow_colour_b")            == 0) g_renderer_settings.shadow_colour_b            = fval;
        else if (strcmp(key, "shadow_colour_a")            == 0) g_renderer_settings.shadow_colour_a            = fval;
        else if (strcmp(key, "shadow_type")                == 0) g_renderer_settings.shadow_type                = ival;
        else if (strcmp(key, "creature_outline_mode")      == 0) g_renderer_settings.creature_outline_mode      = ival;
        // Backward compat: old creature_outline_enable (0->None, 1->Silhouette)
        else if (strcmp(key, "creature_outline_enable")    == 0) g_renderer_settings.creature_outline_mode      = ival ? RENDERER_OUTLINE_SILHOUETTE : RENDERER_OUTLINE_NONE;
        else if (strcmp(key, "creature_outline_alpha")     == 0) g_renderer_settings.creature_outline_alpha     = fval;
        else if (strcmp(key, "lighting_mode")              == 0) g_renderer_settings.lighting_mode              = ival;
        else if (strcmp(key, "tint_mode")                  == 0) g_renderer_settings.tint_mode                  = ival;
        else if (strcmp(key, "tint_strength")              == 0) g_renderer_settings.tint_strength              = fval;
        // Debug knobs are included so developers can persist them across sessions.
        else if (strcmp(key, "wireframe")                  == 0) g_renderer_settings.wireframe                  = ival;
        else if (strcmp(key, "show_depth")                 == 0) g_renderer_settings.show_depth                 = ival;
        else if (strcmp(key, "debug_gui_hitboxes")         == 0) g_renderer_settings.debug_gui_hitboxes         = ival;
    }

    fclose(f);
    RendererSettings_Sanitize();
    RendererApplySettings(&g_renderer_settings);
}

void RendererSettings_Save(void)
{
    char path[600];
    build_prefs_path(path, sizeof(path));

    FILE* f = fopen(path, "w");
    if (!f)
        return;

    fprintf(f, "# KeeperFX renderer preferences -- auto-generated, safe to edit\n\n");

    fprintf(f, "palette_mode             = %d\n",   g_renderer_settings.palette_mode);
    fprintf(f, "darkness_mode           = %d\n",   g_renderer_settings.darkness_mode);
    fprintf(f, "fog_speed               = %.4f\n", g_renderer_settings.fog_speed);
    fprintf(f, "fog_density             = %.4f\n", g_renderer_settings.fog_density);
    fprintf(f, "lens_color_mode         = %d\n",   g_renderer_settings.lens_color_mode);
    fprintf(f, "\n");
    fprintf(f, "tile_filter             = %d\n",   g_renderer_settings.tile_filter);
    fprintf(f, "ui_sprite_filter        = %d\n",   g_renderer_settings.ui_sprite_filter);
    fprintf(f, "shadow_filter           = %d\n",   g_renderer_settings.shadow_filter);
    fprintf(f, "\n");
    fprintf(f, "zoom_box_mode           = %d\n",   g_renderer_settings.zoom_box_mode);
    fprintf(f, "\n");
    fprintf(f, "shade_fullbright        = %.4f\n", g_renderer_settings.shade_fullbright);
    fprintf(f, "shade_ambient           = %.4f\n", g_renderer_settings.shade_ambient);
    fprintf(f, "shade_scale             = %.4f\n", g_renderer_settings.shade_scale);
    fprintf(f, "shade_gamma             = %.4f\n", g_renderer_settings.shade_gamma);
    fprintf(f, "\n");
    fprintf(f, "glow_intensity          = %.4f\n", g_renderer_settings.glow_intensity);
    fprintf(f, "glow_blend_mode         = %d\n",   g_renderer_settings.glow_blend_mode);
    fprintf(f, "\n");
    fprintf(f, "transpar4_alpha         = %.4f\n", g_renderer_settings.transpar4_alpha);
    fprintf(f, "transpar8_alpha         = %.4f\n", g_renderer_settings.transpar8_alpha);
    fprintf(f, "\n");
    fprintf(f, "shadow_darkness_scale   = %.4f\n", g_renderer_settings.shadow_darkness_scale);
    fprintf(f, "shadow_depth_test       = %d\n",   g_renderer_settings.shadow_depth_test);
    fprintf(f, "shadow_max_count        = %d\n",   g_renderer_settings.shadow_max_count);
    fprintf(f, "shadow_colour_r         = %.4f\n", g_renderer_settings.shadow_colour_r);
    fprintf(f, "shadow_colour_g         = %.4f\n", g_renderer_settings.shadow_colour_g);
    fprintf(f, "shadow_colour_b         = %.4f\n", g_renderer_settings.shadow_colour_b);
    fprintf(f, "shadow_colour_a         = %.4f\n", g_renderer_settings.shadow_colour_a);
    fprintf(f, "shadow_type             = %d\n",   g_renderer_settings.shadow_type);
    fprintf(f, "\n");
    fprintf(f, "creature_outline_mode   = %d\n",   g_renderer_settings.creature_outline_mode);
    fprintf(f, "creature_outline_alpha  = %.4f\n", g_renderer_settings.creature_outline_alpha);
    fprintf(f, "\n");
    fprintf(f, "lighting_mode           = %d\n",   g_renderer_settings.lighting_mode);
    fprintf(f, "tint_mode               = %d\n",   g_renderer_settings.tint_mode);
    fprintf(f, "tint_strength           = %.4f\n", g_renderer_settings.tint_strength);
    fprintf(f, "\n");
    fprintf(f, "# debug knobs\n");
    fprintf(f, "wireframe               = %d\n",   g_renderer_settings.wireframe);
    fprintf(f, "show_depth              = %d\n",   g_renderer_settings.show_depth);
    fprintf(f, "debug_gui_hitboxes      = %d\n",   g_renderer_settings.debug_gui_hitboxes);

    fclose(f);
}

#include "post_inc.h"
