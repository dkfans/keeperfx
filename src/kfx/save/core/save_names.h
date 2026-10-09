/******************************************************************************/
/** @file save_names.h
 *     NAMS chunk: per-domain code names of config-table entries, and the
 *     refuse-only check that a save's name_domain-tagged fields still resolve
 *     to the same entry in this build's config tables.
 */
/******************************************************************************/
#ifndef KFX_SAVE_NAMES_H
#define KFX_SAVE_NAMES_H

#include <stdint.h>

#include "kfx/save/core/save_codec.h"

struct Game;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** Writes the NAMS chunk body (not yet wrapped in a SaveChunk) from this
 *  build's config tables, reached through g. Does not touch any global. */
enum SaveResult save_names_write(const struct Game *game_state, struct SaveBuffer *out, struct SaveError *err);

/** Checks every name_domain-tagged field of a just-decoded Game against the
 *  NAMS chunk body the file carried (nams_data/nams_len, as returned by
 *  save_chunk_unpack for SCID_Names). Refuses (does not remap) on the first
 *  used index whose file name doesn't match this build's name at that same
 *  position in its domain. */
enum SaveResult save_names_check(const uint8_t *nams_data, uint32_t nams_len,
    const struct Game *game_state, struct SaveError *err);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
