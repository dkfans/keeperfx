/******************************************************************************/
// Free implementation of KeeperFX.
/******************************************************************************/
/** @file ModWalker.cpp
 *     Implementation of ModWalker — tier stack traversal.
 */
/******************************************************************************/

#include "../../pre_inc.h"
#include "ModWalker.hpp"
#include "../../bflib_fileio.h"

#include <string.h>
#include <stdio.h>

/* -----------------------------------------------------------------------
 * Constructor
 * --------------------------------------------------------------------- */

ModWalker::ModWalker(const ModLocation *locs, size_t count)
    : locs_(locs), loc_count_(count)
{
}

/* -----------------------------------------------------------------------
 * Slot → location matching
 * --------------------------------------------------------------------- */

const ModLocation *ModWalker::findLocForSlot(const KfxTierSlot &slot) const
{
    for (size_t i = 0; i < loc_count_; i++) {
        if (locs_[i].tier   == slot.tier_type &&
            locs_[i].fgroup == slot.fgroup)
        {
            return &locs_[i];
        }
    }
    return nullptr;
}

/* -----------------------------------------------------------------------
 * trySlot — resolve a single path candidate
 * --------------------------------------------------------------------- */

bool ModWalker::trySlot(const KfxTierSlot &slot,
                        const ModLocation &loc,
                        const char *base_fname,
                        char *out, size_t out_size)
{
    char built[512] = {};

    /* Build the filename */
    if (loc.builder) {
        ModBuildContext ctx;
        ctx.level_num  = g_current_tier_stack.level_num;
        ctx.slot_dir   = slot.dir;
        ctx.base_fname = base_fname;
        loc.builder(built, sizeof(built), &ctx);
    } else {
        snprintf(built, sizeof(built), "%s", base_fname ? base_fname : "");
    }

    switch (loc.res_type) {
    case ModRes_File: {
        /* Probe: slot.dir/built */
        char path[512];
        if (built[0])
            snprintf(path, sizeof(path), "%s/%s", slot.dir, built);
        else
            snprintf(path, sizeof(path), "%s", slot.dir);

        if (LbFileExists(path)) {
            snprintf(out, out_size, "%s", path);
            return true;
        }
        return false;
    }

    case ModRes_Directory: {
        /* Probe the directory itself */
        if (LbFileExists(slot.dir)) {
            snprintf(out, out_size, "%s", slot.dir);
            return true;
        }
        return false;
    }

    case ModRes_ZipOrDir: {
        /* Try directory first, then .zip */
        char path[512];
        if (built[0])
            snprintf(path, sizeof(path), "%s/%s", slot.dir, built);
        else
            snprintf(path, sizeof(path), "%s", slot.dir);

        if (LbFileExists(path)) {
            snprintf(out, out_size, "%s", path);
            return true;
        }
        /* Try .zip: slot.dir.zip/<built> — just report the zip path for now */
        char zip_path[512];
        snprintf(zip_path, sizeof(zip_path), "%s.zip", slot.dir);
        if (LbFileExists(zip_path)) {
            snprintf(out, out_size, "%s", zip_path);
            return true;
        }
        return false;
    }
    }

    return false;
}

/* -----------------------------------------------------------------------
 * visitAll — accumulate
 * --------------------------------------------------------------------- */

void ModWalker::visitAll(const KfxTierStack *stack,
                         const char *base_fname,
                         ModOnFoundFn cb, void *userdata) const
{
    if (!stack || !cb) return;

    char path[512];
    for (int i = 0; i < stack->slot_count; i++) {
        const KfxTierSlot &slot = stack->slots[i];
        const ModLocation *loc = findLocForSlot(slot);
        if (!loc) continue;
        if (!trySlot(slot, *loc, base_fname, path, sizeof(path))) continue;
        cb(path, userdata);
    }
}

/* -----------------------------------------------------------------------
 * visitBatch — single pass, multiple filenames
 * --------------------------------------------------------------------- */

void ModWalker::visitBatch(const KfxTierStack *stack,
                           const KfxModBatchEntry *entries, size_t n_entries) const
{
    if (!stack || !entries || n_entries == 0) return;

    char path[512];
    for (int i = 0; i < stack->slot_count; i++) {
        const KfxTierSlot &slot = stack->slots[i];
        const ModLocation *loc = findLocForSlot(slot);
        if (!loc) continue;
        for (size_t ei = 0; ei < n_entries; ei++) {
            if (!entries[ei].cb) continue;
            if (!trySlot(slot, *loc, entries[ei].base_fname, path, sizeof(path))) continue;
            entries[ei].cb(path, entries[ei].userdata);
        }
    }
}

/* -----------------------------------------------------------------------
 * findFirst — highest-priority match
 * --------------------------------------------------------------------- */

bool ModWalker::findFirst(const KfxTierStack *stack,
                          const char *base_fname,
                          char *out_path, size_t out_size) const
{
    if (!stack) return false;

    /* Iterate in reverse: last slot = highest tier = wins */
    char path[512];
    for (int i = stack->slot_count - 1; i >= 0; i--) {
        const KfxTierSlot &slot = stack->slots[i];
        const ModLocation *loc = findLocForSlot(slot);
        if (!loc) continue;
        if (!trySlot(slot, *loc, base_fname, path, sizeof(path))) continue;
        snprintf(out_path, out_size, "%s", path);
        return true;
    }
    return false;
}

#include "../../post_inc.h"
