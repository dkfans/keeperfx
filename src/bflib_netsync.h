/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsync.h
 *     Header file for bflib_netsync.c.
 * @par Purpose:
 *     Algorithms and data structures for synchronization of client data from
 *     server data.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   The KeeperFX Team
 * @date     09 October 2010 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef BFLIB_NETSYNC_H
#define BFLIB_NETSYNC_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

enum DeltaEncoding
{
    DELTA_SELECTBEST,   //select delta encoding freely (for instruction)
    DELTA_NONE,         //no delta encoding, straight copy
    DELTA_PREVSTATE     //delta compared to last state
};

typedef void (*NetsyncFixupFn)(char *destptr, char * srcptr, size_t len);

//we need to write header if encoding in instruction was DELTA_SELECTBEST
typedef char NetsyncHeader;

/**
 * Network sync instruction. Contains a pointer to a block of data which must be synced.
 *
 * Collection process:
 * 1. Block header (if any) is reserved in new state buffer.
 * 2. Block body of len bytes are reserved in new state buffer. (Conceptually.)
 * 3. If ptr != NULL, len bytes a copied to block body from ptr.
 * 4. If on_collect != NULL, call on_collect with destptr = block body.
 *  This allows special case code that handles e.g. pointers. It can also allow
 *  complete customization of resultant data if ptr == NULL.
 * 5. Perform specified delta encoding on block body. If DELTA_SELECTBEST, try
 *  different (select the one with least self information content).
 * 6. Update block header (if any).
 * 7. Instruction has finished.
 *
 * Restoration is essentially the reverse. The meaning of destptr in on_restore
 * is ptr (swapped).
 */
struct NetsyncInstr
{
    char * ptr;
    size_t len;
    enum DeltaEncoding encoding; //probably best to leave at DELTA_SELECTBEST
    NetsyncFixupFn on_collect; //called after data has been copied to new state buffer
    NetsyncFixupFn on_restore; //called after data has been restored from new state buffer
};

/**
 * Computes the size of the buffers necessary to hold the delta.
 * @param instr The instructions.
 * @return
 */
size_t LbNetsyncBufferSize(const struct NetsyncInstr ** instr);

/**
 * Collects data and encodes it, for synchronization purposes.
 * @param instr The instructions (null terminated array).
 * @param out_buffer The encoded buffer. Send over network or whatever.
 * @param old_state The previous state buffer. Must be NULL if there is no previous state.
 * @param new_state The new state buffer. Save this to use as old state for next call.
 */
void LbNetsyncCollect(const struct NetsyncInstr ** instr, char * out_buffer,
    const char * old_state, char * new_state);

/**
 * Restores data previously collected and encoded.
 * @param instr The instructions (null terminated array).
 * @param in_buffer The encoded buffer. Received from network or wherever.
 * @param old_state The previous state buffer. Must be NULL if there is no previous state.
 * @param new_state The new state buffer. Save this to use as old state for next call.
 */
void LbNetsyncRestore(const struct NetsyncInstr ** instr, const char * in_buffer,
    const char * old_state, char * new_state);

#ifdef __cplusplus
};
#endif

#endif //BFLIB_NETSYNC_H

