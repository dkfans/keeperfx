/******************************************************************************/
// Free implementation of KeeperFX.
/******************************************************************************/
/** @file ModWalker.hpp
 *     C++ walker that traverses the KfxTierStack using a ModLocation array.
 *
 * @par Usage:
 *     C code accesses this through the C API in mod_api.h.
 *     C++ subsystems (e.g. SoundManager) can hold a ModWalker member directly.
 *
 * @par Walk patterns:
 *     visitAll()  — accumulate: visit every matching path, lowest tier first.
 *     findFirst() — find-first: return highest-priority match (reverse order).
 *
 * @par Slot matching:
 *     A slot in the tier stack matches a ModLocation entry when both
 *     tier_type and fgroup are equal.  The stack was already built with
 *     existence probing, so no additional flag checks are needed here.
 */
/******************************************************************************/
#ifndef KFX_MODWALKER_HPP
#define KFX_MODWALKER_HPP

#include "mod_location.h"
#include "mod_api.h"

#include <stddef.h>

/**
 * Walks a KfxTierStack against a fixed ModLocation declaration array.
 *
 * One instance is typically held per subsystem (either as a C++ member or
 * allocated via kfx_mod_create_walker()).
 */
class ModWalker {
public:
    ModWalker(const ModLocation *locs, size_t count);

    /**
     * Accumulate pattern: call cb for every found path across ALL active
     * tiers in low→high priority order.
     *
     * @param stack      Tier stack to search.
     * @param base_fname Base filename (e.g. "sounds.cfg").
     * @param cb         Called once per found path.
     * @param userdata   Forwarded to cb.
     */
    void visitAll(const KfxTierStack *stack,
                  const char *base_fname,
                  ModOnFoundFn cb, void *userdata) const;

    /**
     * Find-first pattern: return the highest-priority match.
     * Searches in reverse tier order (last slot wins, i.e. highest tier).
     *
     * @param stack      Tier stack to search.
     * @param base_fname Base filename.
     * @param out_path   Receives the resolved path on success.
     * @param out_size   Capacity of out_path.
     * @return           true if a match was found.
     */
    bool findFirst(const KfxTierStack *stack,
                   const char *base_fname,
                   char *out_path, size_t out_size) const;

    /**
     * Batch accumulate: traverse the stack ONCE and try every entry in the
     * array at each matching slot.  Equivalent to calling visitAll() for
     * each entry but with a single loop over the tier stack.
     *
     * @param stack    Tier stack to search.
     * @param entries  Array of (base_fname, cb, userdata) tuples.
     * @param n_entries Number of entries.
     */
    void visitBatch(const KfxTierStack *stack,
                    const KfxModBatchEntry *entries, size_t n_entries) const;

private:
    const ModLocation *locs_;
    size_t             loc_count_;

    /**
     * Attempt to resolve a file at the given tier slot using the location's
     * builder and resource type.
     *
     * @param slot       Candidate tier slot.
     * @param loc        ModLocation supplying builder and res_type.
     * @param base_fname Base filename.
     * @param out        Buffer to receive the resolved path.
     * @param out_size   Capacity of out.
     * @return           true if the path exists.
     */
    static bool trySlot(const KfxTierSlot &slot,
                        const ModLocation &loc,
                        const char *base_fname,
                        char *out, size_t out_size);

    /**
     * Find the ModLocation entry (if any) that matches the given slot's
     * (tier_type, fgroup) pair.
     */
    const ModLocation *findLocForSlot(const KfxTierSlot &slot) const;
};

#endif /* KFX_MODWALKER_HPP */
