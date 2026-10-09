/******************************************************************************/
/** @file save_internal.h
 *     Shared by the files that make up the schema code in this directory. Not for use outside it.
 */
/******************************************************************************/
#ifndef KFX_SAVE_INTERNAL_H
#define KFX_SAVE_INTERNAL_H

#include <cstddef>
#include <cstdint>

constexpr int SAVE_MAX_DEPTH = 16;
constexpr size_t SAVE_MAX_DIMS = 3;

/** Whether an array of this stored type can be copied as bytes: little endian host, same width in memory. */
bool save_fast_copy_ok(uint8_t type, uint32_t elem);

#endif
