/******************************************************************************/
// Free implementation of KeeperFX.
/******************************************************************************/
/** @file tier_stack.cpp
 *     KfxTierStack management — rebuild and load-event dispatch.
 *
 * @par Slot construction per event:
 *   KfxLoadEvent_Startup   → rebuild all tiers (Startup + Campaign + Level)
 *   KfxLoadEvent_Campaign  → rebuild Campaign + Level tiers
 *   KfxLoadEvent_Level     → rebuild Level tiers only
 *   KfxLoadEvent_LevelEnd  → clear Level tiers (no new slots added)
 *   KfxLoadEvent_SaveLoad  → same as Level; also sets is_save_load flag
 */
/******************************************************************************/

#include "../../pre_inc.h"
#include "tier_stack.h"
#include "../../config.h"
#include "../../config_mods.h"
#include "../../config_keeperfx.h"
#include "../../config_campaigns.h"
#include "../../bflib_basics.h"
#include "../../bflib_fileio.h"
#include "../../game_merge.h"
#include "../../globals.h"

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "../../post_inc.h"

/* Forward declaration for C function that builds file paths */
extern "C" char *prepare_file_path_mod(const char *mod_dir, short fgroup, const char *fname);
extern "C" char *prepare_file_path(short fgroup, const char *fname);
/* recheck_all_mod_exist() is internal to config_mods.c; not in the public header */
extern "C" void recheck_all_mod_exist(void);

/* -----------------------------------------------------------------------
 * Global tier stack definition
 * --------------------------------------------------------------------- */

KfxTierStack g_current_tier_stack;

/* -----------------------------------------------------------------------
 * Subsystem registry
 * --------------------------------------------------------------------- */

#define KFX_MAX_SUBSYSTEMS 64

static struct {
    KfxSubsystemReloadFn reload_fn;
    void                *userdata;
} s_subsystems[KFX_MAX_SUBSYSTEMS];

static int s_subsystem_count = 0;

/* -----------------------------------------------------------------------
 * Slot-appending helpers
 * --------------------------------------------------------------------- */

/** Append a slot to the stack, skipping if dir is empty. */
static void push_slot(KfxTierStack *stack, ModTierType tier, ModLifetime lt,
                      short fgroup, const char *dir, const char *mod_name)
{
    if (!dir || dir[0] == '\0')
        return;
    if (stack->slot_count >= KFX_TIER_STACK_MAX_SLOTS)
        return;

    KfxTierSlot *s = &stack->slots[stack->slot_count++];
    s->tier_type = tier;
    s->lifetime  = lt;
    s->fgroup    = fgroup;
    strncpy(s->dir, dir, DISKPATH_SIZE - 1);
    s->dir[DISKPATH_SIZE - 1] = '\0';
    s->mod_name  = mod_name;
}

/* -----------------------------------------------------------------------
 * Mapping: FGrp → ModExistState flag + lifetime
 * (mirrors what recheck_block_mod_list_exist probes, extended to include
 *  the correct tier lifetime for each FGrp)
 * --------------------------------------------------------------------- */

typedef struct {
    short        fgroup;   /* TbFileGroups value */
    ModLifetime  lifetime;
    ptrdiff_t    flag_offset; /* offsetof(struct ModExistState, field) */
} FGroupFlag;

static const FGroupFlag kModFGroupFlags[] = {
    { FGrp_FxData,     ModLifetime_Startup,  offsetof(struct ModExistState, fx_data)    },
    { FGrp_StdData,    ModLifetime_Startup,  offsetof(struct ModExistState, std_data)   },
    { FGrp_CmpgConfig, ModLifetime_Campaign, offsetof(struct ModExistState, cmpg_config)},
    { FGrp_CmpgLvls,   ModLifetime_Level,    offsetof(struct ModExistState, cmpg_lvls)  },
    { FGrp_CrtrData,   ModLifetime_Startup,  offsetof(struct ModExistState, crtr_data)  },
    { FGrp_CmpgCrtrs,  ModLifetime_Campaign, offsetof(struct ModExistState, cmpg_crtrs) },
    { FGrp_LrgSound,   ModLifetime_Level,    offsetof(struct ModExistState, sound)      },
    /* FGrp_Main — mod root dir; three entries so each after_* tier gets a slot */
    { FGrp_Main,       ModLifetime_Startup,  offsetof(struct ModExistState, mod_dir)    },
    { FGrp_Main,       ModLifetime_Campaign, offsetof(struct ModExistState, mod_dir)    },
    { FGrp_Main,       ModLifetime_Level,    offsetof(struct ModExistState, mod_dir)    },
};

static const int kModFGroupCount = (int)(sizeof(kModFGroupFlags)/sizeof(kModFGroupFlags[0]));

/* -----------------------------------------------------------------------
 * Tier 0/1 — Base and after_base mod slots (Startup lifetime)
 * --------------------------------------------------------------------- */

static void add_base_slots(KfxTierStack *stack)
{
    static const short kBaseFGroups[] = {
        FGrp_FxData, FGrp_StdData, FGrp_CrtrData, FGrp_LrgSound,
    };
    for (int i = 0; i < (int)(sizeof(kBaseFGroups)/sizeof(kBaseFGroups[0])); i++) {
        const char *dir = prepare_file_path(kBaseFGroups[i], NULL);
        push_slot(stack, ModTier_Base, ModLifetime_Startup, kBaseFGroups[i], dir, NULL);
    }
}

static void add_after_base_slots(KfxTierStack *stack)
{
    const struct ModsConfig *mc = get_loaded_mods_conf();
    char mod_dir[DISKPATH_SIZE];

    for (int i = 0; i < mc->after_base_cnt; i++) {
        const struct ModConfigItem *item = &mc->after_base_item[i];
        if (!item->state.mod_dir) continue;

        snprintf(mod_dir, sizeof(mod_dir), "%s/%s", MODS_DIR_NAME, item->name);

        for (int f = 0; f < kModFGroupCount; f++) {
            const FGroupFlag *ff = &kModFGroupFlags[f];
            if (ff->lifetime != ModLifetime_Startup) continue;
            const int *flag = (const int *)((const char *)&item->state + ff->flag_offset);
            if (!*flag) continue;
            const char *dir = prepare_file_path_mod(mod_dir, (short)ff->fgroup, NULL);
            push_slot(stack, ModTier_AfterBase, ff->lifetime, ff->fgroup, dir, item->name);
        }
    }
}

/* -----------------------------------------------------------------------
 * Tier 2/3 — Campaign and after_campaign slots (Campaign lifetime)
 * --------------------------------------------------------------------- */

static void add_campaign_slots(KfxTierStack *stack)
{
    if (!stack->is_campaign_active) return;

    static const short kCmpgFGroups[] = {
        FGrp_CmpgConfig, FGrp_CmpgCrtrs, FGrp_LandView,
    };
    for (int i = 0; i < (int)(sizeof(kCmpgFGroups)/sizeof(kCmpgFGroups[0])); i++) {
        const char *dir = prepare_file_path(kCmpgFGroups[i], NULL);
        push_slot(stack, ModTier_Campaign, ModLifetime_Campaign, kCmpgFGroups[i], dir, NULL);
    }
}

static void add_after_campaign_slots(KfxTierStack *stack)
{
    if (!stack->is_campaign_active) return;

    const struct ModsConfig *mc = get_loaded_mods_conf();
    char mod_dir[DISKPATH_SIZE];

    for (int i = 0; i < mc->after_campaign_cnt; i++) {
        const struct ModConfigItem *item = &mc->after_campaign_item[i];
        if (!item->state.mod_dir) continue;

        snprintf(mod_dir, sizeof(mod_dir), "%s/%s", MODS_DIR_NAME, item->name);

        for (int f = 0; f < kModFGroupCount; f++) {
            const FGroupFlag *ff = &kModFGroupFlags[f];
            if (ff->lifetime != ModLifetime_Campaign) continue;
            const int *flag = (const int *)((const char *)&item->state + ff->flag_offset);
            if (!*flag) continue;
            const char *dir = prepare_file_path_mod(mod_dir, (short)ff->fgroup, NULL);
            push_slot(stack, ModTier_AfterCampaign, ff->lifetime, ff->fgroup, dir, item->name);
        }
    }
}

/* -----------------------------------------------------------------------
 * Tier 4/5 — Per-map and after_map slots (Level lifetime)
 * --------------------------------------------------------------------- */

static void add_per_map_slots(KfxTierStack *stack)
{
    if (!stack->is_campaign_active) return;

    /* Per-map files live in the same campaign level-directory;
     * subsystems provide builders that produce the map-specific filename. */
    static const short kPerMapFGroups[] = {
        FGrp_CmpgLvls, FGrp_CmpgConfig, FGrp_CmpgCrtrs,
    };
    for (int i = 0; i < (int)(sizeof(kPerMapFGroups)/sizeof(kPerMapFGroups[0])); i++) {
        const char *dir = prepare_file_path(kPerMapFGroups[i], NULL);
        push_slot(stack, ModTier_PerMap, ModLifetime_Level, kPerMapFGroups[i], dir, NULL);
    }
}

static void add_after_map_slots(KfxTierStack *stack)
{
    if (!stack->is_campaign_active) return;

    const struct ModsConfig *mc = get_loaded_mods_conf();
    char mod_dir[DISKPATH_SIZE];

    for (int i = 0; i < mc->after_map_cnt; i++) {
        const struct ModConfigItem *item = &mc->after_map_item[i];
        if (!item->state.mod_dir) continue;

        snprintf(mod_dir, sizeof(mod_dir), "%s/%s", MODS_DIR_NAME, item->name);

        for (int f = 0; f < kModFGroupCount; f++) {
            const FGroupFlag *ff = &kModFGroupFlags[f];
            if (ff->lifetime != ModLifetime_Level) continue;
            const int *flag = (const int *)((const char *)&item->state + ff->flag_offset);
            if (!*flag) continue;
            const char *dir = prepare_file_path_mod(mod_dir, (short)ff->fgroup, NULL);
            push_slot(stack, ModTier_AfterMap, ff->lifetime, ff->fgroup, dir, item->name);
        }
    }
}

/* -----------------------------------------------------------------------
 * Stack rebuild
 * --------------------------------------------------------------------- */

void kfx_rebuild_tier_stack(KfxLoadEvent event)
{
    /* Run mod existence checks — same probe logic as before, now an internal step */
    recheck_all_mod_exist();

    LevelNumber level_num = get_selected_level_number();
    int has_campaign = (campaign.fname[0] != '\0');

    /* Preserve the slot count split point so we can remove and re-add per tier */
    int startup_end = 0;
    int campaign_end = 0;

    if (event == KfxLoadEvent_Startup) {
        /* Full rebuild — reset everything */
        memset(&g_current_tier_stack, 0, sizeof(g_current_tier_stack));
        g_current_tier_stack.is_campaign_active = has_campaign;
        g_current_tier_stack.is_map_active      = (level_num > 0);
        g_current_tier_stack.is_save_load        = 0;
        g_current_tier_stack.level_num           = level_num;

        add_base_slots(&g_current_tier_stack);
        add_after_base_slots(&g_current_tier_stack);
        startup_end = g_current_tier_stack.slot_count;

        add_campaign_slots(&g_current_tier_stack);
        add_after_campaign_slots(&g_current_tier_stack);
        campaign_end = g_current_tier_stack.slot_count;

        add_per_map_slots(&g_current_tier_stack);
        add_after_map_slots(&g_current_tier_stack);
        return;
    }

    /* Find the boundary between Startup and Campaign slots so we can rebuild in-place */
    startup_end = 0;
    for (int i = 0; i < g_current_tier_stack.slot_count; i++) {
        if (g_current_tier_stack.slots[i].lifetime == ModLifetime_Startup)
            startup_end = i + 1;
        else
            break;
    }

    if (event == KfxLoadEvent_Campaign) {
        g_current_tier_stack.is_campaign_active = has_campaign;
        g_current_tier_stack.is_map_active      = (level_num > 0);
        g_current_tier_stack.is_save_load        = 0;
        g_current_tier_stack.level_num           = level_num;

        /* Discard everything after Startup slots; rebuild Campaign + Level */
        g_current_tier_stack.slot_count = startup_end;
        add_campaign_slots(&g_current_tier_stack);
        add_after_campaign_slots(&g_current_tier_stack);
        add_per_map_slots(&g_current_tier_stack);
        add_after_map_slots(&g_current_tier_stack);
        return;
    }

    /* For Level / LevelEnd / SaveLoad — keep Startup + Campaign slots, rebuild Level */
    campaign_end = startup_end;
    for (int i = startup_end; i < g_current_tier_stack.slot_count; i++) {
        if (g_current_tier_stack.slots[i].lifetime == ModLifetime_Campaign)
            campaign_end = i + 1;
        else
            break;
    }

    g_current_tier_stack.is_map_active = (level_num > 0);
    g_current_tier_stack.level_num     = level_num;
    g_current_tier_stack.is_save_load  = (event == KfxLoadEvent_SaveLoad) ? 1 : 0;

    /* Discard Level slots; rebuild unless this is a LevelEnd clear */
    g_current_tier_stack.slot_count = campaign_end;
    if (event != KfxLoadEvent_LevelEnd) {
        add_per_map_slots(&g_current_tier_stack);
        add_after_map_slots(&g_current_tier_stack);
    }
}



/* -----------------------------------------------------------------------
 * Subsystem registration
 * --------------------------------------------------------------------- */

void kfx_register_subsystem(KfxSubsystemReloadFn reload_fn, void *userdata)
{
    if (s_subsystem_count >= KFX_MAX_SUBSYSTEMS)
        return;
    s_subsystems[s_subsystem_count].reload_fn = reload_fn;
    s_subsystems[s_subsystem_count].userdata  = userdata;
    s_subsystem_count++;
}

/* -----------------------------------------------------------------------
 * Load event dispatch
 * --------------------------------------------------------------------- */

void kfx_trigger_load_event(KfxLoadEvent event)
{
    kfx_rebuild_tier_stack(event);

    for (int i = 0; i < s_subsystem_count; i++) {
        s_subsystems[i].reload_fn(event, &g_current_tier_stack, s_subsystems[i].userdata);
    }
}
