/******************************************************************************/
// Free implementation of KeeperFX.
/******************************************************************************/
/** @file tier_stack.h
 *     KfxTierStack management — rebuild and load-event dispatch.
 * @par Purpose:
 *     Declares kfx_rebuild_tier_stack() and kfx_trigger_load_event(), which
 *     together form the single authoritative entry point for all resource
 *     loading and reloading in the game.
 *
 *     Rules enforced by this system:
 *       - No subsystem reload function is called directly from game event
 *         handlers (change_campaign, init_level, load_game_chunks).
 *       - Subsystems register themselves at startup via kfx_register_subsystem().
 *       - kfx_trigger_load_event() is the ONLY function that drives reloading.
 */
/******************************************************************************/
#ifndef KFX_TIER_STACK_H
#define KFX_TIER_STACK_H

#include "mod_location.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * Subsystem reload callback
 * --------------------------------------------------------------------- */

/**
 * Callback invoked by kfx_trigger_load_event() for a registered subsystem.
 *
 * @param event   The event that triggered this call.
 * @param stack   Pointer to the freshly rebuilt tier stack (read-only).
 * @param userdata Opaque pointer provided at registration time.
 */
typedef void (*KfxSubsystemReloadFn)(KfxLoadEvent event,
                                     const KfxTierStack *stack,
                                     void *userdata);

/* -----------------------------------------------------------------------
 * Tier stack lifecycle
 * --------------------------------------------------------------------- */

/**
 * Rebuilds the tier stack for the given event.
 *
 * Reads from: game_mods_config (all three mod lists), the global campaign
 * struct, and get_selected_level_number().  Only the tier slots relevant to
 * the event lifetime are rebuilt; slots for other lifetimes carry over.
 *
 * Called internally by kfx_trigger_load_event() before dispatching to
 * subsystems. External code should call kfx_trigger_load_event() instead.
 *
 * Also absorbs recheck_all_mod_exist() — mod directory probing runs here
 * as part of stack construction rather than as a separate public call.
 */
void kfx_rebuild_tier_stack(KfxLoadEvent event);

/* -----------------------------------------------------------------------
 * Subsystem registration
 * --------------------------------------------------------------------- */

/**
 * Register a subsystem reload callback with the event dispatch system.
 *
 * Registrations persist for the lifetime of the process. The callback is
 * invoked by kfx_trigger_load_event() whenever the event matches the
 * subsystem's declared lifetimes.
 *
 * @param reload_fn  Function to call on each relevant load event.
 * @param userdata   Opaque value forwarded to reload_fn unchanged.
 */
void kfx_register_subsystem(KfxSubsystemReloadFn reload_fn, void *userdata);

/* -----------------------------------------------------------------------
 * Load event dispatch — the single reload entry point
 * --------------------------------------------------------------------- */

/**
 * Triggers a load event:
 *   1. Calls kfx_rebuild_tier_stack(event) to update g_current_tier_stack.
 *   2. Iterates all registered subsystems and calls each reload_fn whose
 *      lifetimes intersect with the event.
 *
 * This is the ONLY function that should be called to initiate any
 * resource loading or reloading.  Direct calls to load_config(),
 * load_stats_files(), load_sounds_config() etc. from game event handlers
 * are replaced by a single call here.
 *
 * Lifetime → event mapping:
 *   KfxLoadEvent_Startup   → fires Startup + Campaign + Level lifetimes
 *   KfxLoadEvent_Campaign  → fires Campaign + Level lifetimes
 *   KfxLoadEvent_Level     → fires Level lifetime only
 *   KfxLoadEvent_LevelEnd  → fires Level lifetime (clear/unload semantics)
 *   KfxLoadEvent_SaveLoad  → fires Level lifetime; sets is_save_load flag
 */
void kfx_trigger_load_event(KfxLoadEvent event);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KFX_TIER_STACK_H */
