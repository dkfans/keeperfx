/******************************************************************************/
// Free implementation of KeeperFX.
/******************************************************************************/
/** @file mod_location.h
 *     Unified resource-layer abstraction — core type definitions.
 * @par Purpose:
 *     Declares the six-tier stack model, ModLocation descriptor, and
 *     KfxLoadEvent lifecycle that together replace all ad-hoc per-subsystem
 *     mod-iteration boilerplate.
 *
 *     All types are C-compatible so they can be included from both C and C++
 *     translation units.
 *
 * @par Tier order (low → high priority):
 *     Tier 0  ModTier_Base          fxdata/, data/, creatrs/
 *     Tier 1  ModTier_AfterBase     mods/<name>/  [after_base list]
 *     Tier 2  ModTier_Campaign      campgns/<name>/ subdirs
 *     Tier 3  ModTier_AfterCampaign mods/<name>/  [after_campaign list]
 *     Tier 4  ModTier_PerMap        map%05lu.* within campaign dirs
 *     Tier 5  ModTier_AfterMap      mods/<name>/  [after_map list]
 */
/******************************************************************************/
#ifndef KFX_MOD_LOCATION_H
#define KFX_MOD_LOCATION_H

#include <stddef.h>

#include "../../bflib_basics.h"
#include "../../config.h"
#include "../../globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * Tier types
 * --------------------------------------------------------------------- */

typedef enum KfxModTierType {
    ModTier_Base           = 0, /**< fxdata/, data/, creatrs/ — static, always present */
    ModTier_AfterBase      = 1, /**< mods/<name>/ from [after_base] list */
    ModTier_Campaign       = 2, /**< campgns/<name>/ subdirs — populated by load_campaign() */
    ModTier_AfterCampaign  = 3, /**< mods/<name>/ from [after_campaign] list */
    ModTier_PerMap         = 4, /**< map%05lu.* within campaign dirs */
    ModTier_AfterMap       = 5, /**< mods/<name>/ from [after_map] list */
} ModTierType;

/* -----------------------------------------------------------------------
 * Lifetime model
 * --------------------------------------------------------------------- */

/** Lifetime of a resource location: controls when it is (re)loaded. */
typedef enum KfxModLifetime {
    ModLifetime_Startup  = 0, /**< Load once at game boot; never reloaded. */
    ModLifetime_Campaign = 1, /**< Reload on campaign change; valid across levels. */
    ModLifetime_Level    = 2, /**< Reload at level start; cleared on LevelEnd. */
} ModLifetime;

/** Game event that triggers a reload sweep across registered subsystems. */
typedef enum KfxLoadEventType {
    KfxLoadEvent_Startup  = 0, /**< Game boot — loads all lifetimes. */
    KfxLoadEvent_Campaign = 1, /**< Campaign selected — reloads Campaign + Level lifetimes. */
    KfxLoadEvent_Level    = 2, /**< Fresh level/map start — reloads Level lifetime only. */
    KfxLoadEvent_LevelEnd = 3, /**< Level ended — clears Level-lifetime data explicitly. */
    KfxLoadEvent_SaveLoad = 4, /**< Save restored — reloads Level lifetime + sets is_save_load flag. */
} KfxLoadEvent;

/* -----------------------------------------------------------------------
 * Resource type
 * --------------------------------------------------------------------- */

/** Controls what the walker looks for at a given tier slot. */
typedef enum KfxModResType {
    ModRes_File,      /**< Probe: slot_dir/<built_fname>; must exist as a file. */
    ModRes_Directory, /**< Probe: slot_dir/ exists; pass directory path to delegate. */
    ModRes_ZipOrDir,  /**< Try directory first, then <slot_dir>.zip containing the file. */
} ModResourceType;

/* -----------------------------------------------------------------------
 * Path builder callback
 * --------------------------------------------------------------------- */

/**
 * Context provided to a ModPathBuilder callback.
 * Extensible: new fields can be added here without changing ModLocation.
 */
typedef struct KfxModBuildContext {
    LevelNumber  level_num;   /**< Current level number, e.g. from get_selected_level_number(). */
    const char  *slot_dir;    /**< Resolved base directory for this tier slot. */
    const char  *base_fname;  /**< Caller-supplied filename, e.g. "sounds.cfg". */
} ModBuildContext;

/**
 * Builder callback: constructs the concrete filename for a tier slot.
 * For example, the per-map builder writes "map00013.sounds.cfg" into @p out.
 * Pass NULL to use base_fname verbatim.
 */
typedef void (*ModPathBuilder)(char *out, size_t out_size, const ModBuildContext *ctx);

/* -----------------------------------------------------------------------
 * ModLocation — per-subsystem interest declaration
 * --------------------------------------------------------------------- */

/**
 * Declares one probing location in a subsystem's resource search.
 *
 * A subsystem declares a static array of ModLocation entries that covers
 * all tiers it cares about. The walker uses this array plus the live
 * KfxTierStack to find actual files.
 *
 * flag_offset: byte offset into ModExistState of the int field that gates
 *   this location. SIZE_MAX means no gate (base/campaign tiers always probed).
 */
typedef struct KfxModLocation {
    ModTierType      tier;        /**< Which tier this entry applies to. */
    short            fgroup;      /**< File group tag (TbFileGroups value); path resolution + mod subdir. */
    ModLifetime      lifetime;    /**< Controls when this location is active. */
    size_t           flag_offset; /**< offsetof(ModExistState, field); SIZE_MAX = no gate. */
    ModResourceType  res_type;    /**< What to look for at this location. */
    ModPathBuilder   builder;     /**< NULL = use base_fname verbatim. */
} ModLocation;

/* -----------------------------------------------------------------------
 * KfxTierStack — pre-built, ordered, flat list of all active tier slots
 * --------------------------------------------------------------------- */

/**
 * One resolved concrete slot in the tier stack.
 * Built by kfx_rebuild_tier_stack(); read-only during file searches.
 *
 * The `dir` is the fully resolved directory path for this slot.
 * The walker appends the filename (built from ModLocation.builder) to get
 * the final path.  builder and res_type are NOT on the slot — they live on
 * ModLocation so each subsystem can declare its own semantics.
 */
typedef struct KfxTierSlot {
    ModTierType      tier_type;
    ModLifetime      lifetime;
    short            fgroup;             /**< TbFileGroups value — used to match ModLocation entries. */
    char             dir[DISKPATH_SIZE]; /**< Fully resolved directory path (no trailing slash). */
    const char      *mod_name;           /**< Non-NULL for mod tiers; points into global ModConfigItem. */
} KfxTierSlot;

/** Maximum number of slots in one tier stack.
 *  6 tiers × 50 mods per tier + base + campaign slots = well under 256. */
#define KFX_TIER_STACK_MAX_SLOTS 256

/**
 * The global tier stack. Rebuilt whenever game state changes; never written
 * to during file searches (read-only between rebuild calls).
 */
typedef struct KfxTierStackType {
    KfxTierSlot slots[KFX_TIER_STACK_MAX_SLOTS];
    int         slot_count;

    /* State flags — set by kfx_rebuild_tier_stack(), read by subsystems. */
    int         is_campaign_active; /**< Non-zero when a campaign is loaded. */
    int         is_map_active;      /**< Non-zero when a level is active. */
    int         is_save_load;       /**< Non-zero during KfxLoadEvent_SaveLoad dispatch. */
    LevelNumber level_num;          /**< Current level number when is_map_active. */
} KfxTierStack;

/** The one global tier stack instance. */
extern KfxTierStack g_current_tier_stack;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KFX_MOD_LOCATION_H */
