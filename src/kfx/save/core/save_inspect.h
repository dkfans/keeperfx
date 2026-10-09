/******************************************************************************/
/** @file save_inspect.h
 *     Cheap check of a savegame file: structure and title, without reading the whole file.
 */
/******************************************************************************/
#ifndef KFX_SAVE_INSPECT_H
#define KFX_SAVE_INSPECT_H

#include <stdint.h>

#include "save_schema.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** Copies len bytes starting at offset into dst. Returns 0 on success. */
typedef int (*SaveReadAtFn)(void *context, uint32_t offset, void *dst, uint32_t len);

/** Checks that a savegame can be loaded as far as its structure goes: the file header, every chunk
 *  header, no unknown required chunk, and META, SCHM, GAME and ILVL all present. Only the headers and
 *  the META and SCHM chunks are read, and the other chunks' contents and checksums are not checked.
 *
 *  meta_desc and meta describe the title record in META (zeroed by the caller). *meta_ok says whether
 *  it was read, which can be true even when the result is an error, so a damaged or too new save can
 *  still be listed by name. */
enum SaveResult save_inspect_game(SaveReadAtFn read_at, void *context, uint32_t file_len,
    const struct SaveStructDesc *meta_desc, void *meta, int *meta_ok, struct SaveError *err);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
