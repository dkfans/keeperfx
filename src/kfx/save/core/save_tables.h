/******************************************************************************/
/** @file save_tables.h
 *     The list of root struct tables.
 */
/******************************************************************************/
#ifndef KFX_SAVE_TABLES_H
#define KFX_SAVE_TABLES_H

#include "save_schema.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const struct SaveStructDesc *const *save_root_structs(uint32_t *count);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
