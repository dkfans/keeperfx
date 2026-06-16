/******************************************************************************/
// Free implementation of KeeperFX.
/******************************************************************************/
/** @file mod_api.h
 *     C-compatible public API for the unified resource-layer abstraction.
 *
 * @par Purpose:
 *     Provides the C API used by C translation units to:
 *       - Create/release walker handles backed by ModWalker objects
 *       - Invoke accumulate or find-first searches against the current tier stack
 *       - Register subsystem reload callbacks
 *       - Convenience wrappers that use g_current_tier_stack implicitly
 *
 *     C++ subsystems use IModSubsystem directly instead of this API.
 */
/******************************************************************************/
#ifndef KFX_MOD_API_H
#define KFX_MOD_API_H

#include "mod_location.h"
#include "tier_stack.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * Callback type for accumulate (visitAll) pattern
 * --------------------------------------------------------------------- */

/**
 * Called once for every found path during a kfx_mod_visit* call.
 *
 * @param path     Fully resolved path to the found file or directory.
 * @param userdata Opaque pointer provided at walk time.
 */
typedef void (*ModOnFoundFn)(const char *path, void *userdata);

/* -----------------------------------------------------------------------
 * Walker handle lifecycle
 * --------------------------------------------------------------------- */

/** Opaque handle to a ModWalker instance. */
typedef void *KfxModHandle;

/**
 * Create a walker backed by the given ModLocation array.
 *
 * The caller owns the returned handle and must release it with
 * kfx_mod_release_walker() when done.  The locs array must remain valid
 * for the lifetime of the handle (static arrays are ideal).
 *
 * @param locs   Array of ModLocation entries describing what to search.
 * @param n_locs Number of entries in locs.
 * @return       Opaque handle, or NULL on allocation failure.
 */
KfxModHandle kfx_mod_create_walker(const ModLocation *locs, size_t n_locs);

/**
 * Release a walker handle returned by kfx_mod_create_walker().
 * Passing NULL is safe (no-op).
 */
void kfx_mod_release_walker(KfxModHandle handle);

/* -----------------------------------------------------------------------
 * Walk operations (explicit stack)
 * --------------------------------------------------------------------- */

/**
 * Accumulate pattern: call cb for every matching path found across ALL
 * active tiers, in tier order (lowest priority first).
 *
 * @param handle   Walker handle.
 * @param stack    Tier stack to search (usually &g_current_tier_stack).
 * @param base_fname Base filename to search for (e.g. "sounds.cfg").
 * @param cb       Callback invoked for each found path.
 * @param userdata Forwarded to cb.
 */
void kfx_mod_visit_all(KfxModHandle handle,
                       const KfxTierStack *stack,
                       const char *base_fname,
                       ModOnFoundFn cb, void *userdata);

/**
 * Find-first pattern: return the highest-priority match across all tiers.
 * Searches in reverse tier order (highest priority wins).
 *
 * @param handle    Walker handle.
 * @param stack     Tier stack to search.
 * @param base_fname Base filename (e.g. "rules.cfg").
 * @param out_path  Buffer to receive the resolved path on success.
 * @param out_size  Size of out_path buffer.
 * @return          Non-zero if a match was found; zero otherwise.
 */
TbBool kfx_mod_find_first(KfxModHandle handle,
                           const KfxTierStack *stack,
                           const char *base_fname,
                           char *out_path, size_t out_size);

/* -----------------------------------------------------------------------
 * Convenience wrappers — use g_current_tier_stack implicitly
 * --------------------------------------------------------------------- */

/**
 * Equivalent to kfx_mod_visit_all(h, &g_current_tier_stack, fname, cb, ud).
 */
void kfx_mod_visit(KfxModHandle h, const char *fname,
                   ModOnFoundFn cb, void *ud);

/**
 * Equivalent to kfx_mod_find_first(h, &g_current_tier_stack, fname, out, sz).
 */
TbBool kfx_mod_find(KfxModHandle h, const char *fname,
                    char *out, size_t sz);

/* -----------------------------------------------------------------------
 * Creature reload helper
 * --------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
 * Batch walk — single stack traversal for multiple filenames
 * --------------------------------------------------------------------- */

/**
 * One entry in a batch walk: associates a base filename with its callback.
 */
typedef struct KfxModBatchEntry {
    const char   *base_fname; /**< Filename to search for at each slot. */
    ModOnFoundFn  cb;         /**< Called when a path is found. */
    void         *userdata;   /**< Forwarded to cb. */
} KfxModBatchEntry;

/**
 * Batch accumulate: traverse g_current_tier_stack ONCE and, for each
 * matching slot, try every entry's base_fname.  Equivalent to calling
 * kfx_mod_visit() for each entry but with a single loop over the stack.
 *
 * @param handle   Walker handle (must share the same ModLocation array
 *                 that all entries' filenames are to be searched with).
 * @param entries  Array of batch entries.
 * @param n_entries Number of entries.
 */
void kfx_mod_visit_batch(KfxModHandle handle,
                         const KfxModBatchEntry *entries, size_t n_entries);

/**
 * Reload the config for one creature model through the unified tier stack.
 * Replaces direct calls to load_default_creaturemodel_config() from Lua
 * scripts that swap creatures at runtime.
 *
 * @param crtr_model  Creature model index to reload.
 * @return  True if at least one config file was loaded successfully.
 */
TbBool kfx_reload_single_creature(int crtr_model);

/* -----------------------------------------------------------------------
 * Load progress observer — thread-safe per-thread callback scope stack
 * --------------------------------------------------------------------- */

/**
 * Whether a loading step is beginning or has finished.
 */
typedef enum KfxLoadStep {
    KfxLoadStep_Begin, /**< About to load this item. */
    KfxLoadStep_Done,  /**< Finished loading this item (base + all mod overrides). */
} KfxLoadStep;

/**
 * Payload delivered to a load observer on every step event.
 */
typedef struct KfxLoadNotification {
    KfxLoadStep  step;
    const char  *label;   /**< Human-readable name, typically the config filename. */
    int          current; /**< 1-based index within the current batch (0 = unbatched). */
    int          total;   /**< Total items in the current batch (0 = unbatched). */
} KfxLoadNotification;

/** Callback type for load observers. */
typedef void (*KfxLoadObserverFn)(const KfxLoadNotification *notif, void *userdata);

/** Opaque scope handle returned by kfx_load_observer_push(). */
typedef struct KfxLoadObserverScope KfxLoadObserverScope;

/**
 * Register a load observer for the calling thread.
 *
 * The callback is invoked synchronously on the same thread that performs
 * loading.  Multiple scopes may be nested; only the innermost (most recently
 * pushed) observer is called.
 *
 * @param fn       Callback to invoke on each loading step.
 * @param userdata Forwarded to fn unchanged.
 * @return         Opaque scope handle; pass to kfx_load_observer_pop() to
 *                 unregister.  NULL on allocation failure.
 */
KfxLoadObserverScope *kfx_load_observer_push(KfxLoadObserverFn fn, void *userdata);

/**
 * Unregister a load observer.  Must be called from the same thread as push.
 * Passing NULL is a no-op.
 */
void kfx_load_observer_pop(KfxLoadObserverScope *scope);

/**
 * Fire a load notification to the current thread's innermost observer.
 * Called by the loading system; not intended for external callers.
 */
void kfx_load_notify(const KfxLoadNotification *notif);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KFX_MOD_API_H */
