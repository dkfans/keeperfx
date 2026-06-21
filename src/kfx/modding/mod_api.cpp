/******************************************************************************/
// Free implementation of KeeperFX.
/******************************************************************************/
/** @file mod_api.cpp
 *     C-compatible public API implementation — thin wrappers around ModWalker.
 */
/******************************************************************************/

#include "../../pre_inc.h"
#include "mod_api.h"
#include "ModWalker.hpp"
#include "../../config.h"
#include "../../config_crtrmodel.h"
#include "../../globals.h"

#include <new>
#include <stdlib.h>
#include <string.h>

/* -----------------------------------------------------------------------
 * Walker handle lifecycle
 * --------------------------------------------------------------------- */

KfxModHandle kfx_mod_create_walker(const ModLocation *locs, size_t n_locs)
{
    void *mem = malloc(sizeof(ModWalker));
    if (!mem) return nullptr;
    return new (mem) ModWalker(locs, n_locs);
}

void kfx_mod_release_walker(KfxModHandle handle)
{
    if (!handle) return;
    static_cast<ModWalker *>(handle)->~ModWalker();
    free(handle);
}

/* -----------------------------------------------------------------------
 * Walk operations (explicit stack)
 * --------------------------------------------------------------------- */

void kfx_mod_visit_all(KfxModHandle handle,
                       const KfxTierStack *stack,
                       const char *base_fname,
                       ModOnFoundFn cb, void *userdata)
{
    static_cast<ModWalker *>(handle)->visitAll(stack, base_fname, cb, userdata);
}

TbBool kfx_mod_find_first(KfxModHandle handle,
                           const KfxTierStack *stack,
                           const char *base_fname,
                           char *out_path, size_t out_size)
{
    return static_cast<ModWalker *>(handle)->findFirst(stack, base_fname, out_path, out_size)
               ? 1 : 0;
}

/* -----------------------------------------------------------------------
 * Convenience wrappers
 * --------------------------------------------------------------------- */

void kfx_mod_visit(KfxModHandle h, const char *fname, ModOnFoundFn cb, void *ud)
{
    kfx_mod_visit_all(h, &g_current_tier_stack, fname, cb, ud);
}

void kfx_mod_visit_batch(KfxModHandle h,
                         const KfxModBatchEntry *entries, size_t n_entries)
{
    static_cast<ModWalker *>(h)->visitBatch(&g_current_tier_stack, entries, n_entries);
}

TbBool kfx_mod_find(KfxModHandle h, const char *fname, char *out, size_t sz)
{
    return kfx_mod_find_first(h, &g_current_tier_stack, fname, out, sz);
}

/* -----------------------------------------------------------------------
 * Creature reload helper
 * --------------------------------------------------------------------- */

TbBool kfx_reload_single_creature(int crtr_model)
{
    return load_default_creaturemodel_config((ThingModel)crtr_model, CnfLd_Standard);
}

/* -----------------------------------------------------------------------
 * Load progress observer — thread-local scope stack
 * --------------------------------------------------------------------- */

struct KfxLoadObserverScope {
    KfxLoadObserverFn    fn;
    void                *userdata;
    KfxLoadObserverScope *prev;
};

static thread_local KfxLoadObserverScope *s_observer_top = nullptr;

KfxLoadObserverScope *kfx_load_observer_push(KfxLoadObserverFn fn, void *userdata)
{
    KfxLoadObserverScope *scope =
        static_cast<KfxLoadObserverScope *>(malloc(sizeof(KfxLoadObserverScope)));
    if (!scope) return nullptr;
    scope->fn       = fn;
    scope->userdata = userdata;
    scope->prev     = s_observer_top;
    s_observer_top  = scope;
    return scope;
}

void kfx_load_observer_pop(KfxLoadObserverScope *scope)
{
    if (!scope) return;
    /* Walk the stack to find and unlink it (handles out-of-order pops safely). */
    KfxLoadObserverScope **cur = &s_observer_top;
    while (*cur && *cur != scope)
        cur = &(*cur)->prev;
    if (*cur == scope)
        *cur = scope->prev;
    free(scope);
}

void kfx_load_notify(const KfxLoadNotification *notif)
{
    if (!notif) return;
    KfxLoadObserverScope *top = s_observer_top;
    if (top && top->fn)
        top->fn(notif, top->userdata);
}

#include "../../post_inc.h"
