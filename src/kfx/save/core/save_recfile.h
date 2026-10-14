/******************************************************************************/
/** @file save_recfile.h
 *     Files that hold a run of records of one type: a schema chunk and a records chunk. The high
 *     score table, the network config and the continue files are all this.
 */
/******************************************************************************/
#ifndef KFX_SAVE_RECFILE_H
#define KFX_SAVE_RECFILE_H

#include <stdint.h>

#include "save_schema.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** Encodes records[0..count) (elem_size bytes each) as a file of the given kind. */
enum SaveResult save_recfile_build(struct SaveBuffer *out, uint32_t kind, const struct SaveStructDesc *desc,
    const void *records, uint32_t elem_size, uint32_t count, struct SaveError *err);

/** Decodes a file made by save_recfile_build into a malloc'd array of *count records, elem_size bytes
 *  each, to be freed with free(). *records is NULL when there are none or on any error. */
enum SaveResult save_recfile_parse(const uint8_t *file, uint32_t len, uint32_t kind, const struct SaveStructDesc *desc,
    uint32_t elem_size, void **records, uint32_t *count, struct SaveError *err);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
