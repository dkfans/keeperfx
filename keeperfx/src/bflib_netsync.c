/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsync.cpp
 *     Function definitions for LbNetsync*.
 * @par Purpose:
 *     Algorithms and data structures for synchronization of client data from
 *     server data.
 * @par Comment:
 *     None.
 * @author   The KeeperFX Team
 * @date     10 April 2010 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "bflib_netsync.h"
#include "bflib_memory.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

static float self_information(const char * buffer, size_t len)
{
    size_t counts[0x100];
    size_t i;
    float nat;

    LbMemorySet(counts, 0, sizeof(counts));

    for (i = 0; i < len; ++i) {
        counts[((unsigned) buffer[i]) & 0xFF] += 1; //handles other char sizes than 8 bit.. probably redundant
    }

    nat = 0.0f;
    for (int i = 0; i < 0x100; ++i) {
        if (counts[i] == 0) {
            continue;
        }

        float p = (float) counts[i] / len;
        nat -= log(p);
    }

    return nat;
}

static enum DeltaEncoding encode(enum DeltaEncoding encoding, char * code,
    const char * old_state, const char * new_state, size_t len)
{
    size_t i;

    if (encoding == DELTA_NONE) {
        LbMemoryCopy(code, new_state, len);
        return DELTA_NONE;
    }

    //encode
    for (i = 0; i < len; ++i) {
        code[i] = new_state[i] - old_state[i];
    }

    if (encoding == DELTA_PREVSTATE) {
        return DELTA_PREVSTATE;
    }

    //encoding == DELTA_SELECTBEST is implied from here

    //compare information content and select best
    if (self_information(new_state, len) <= self_information(code, len)) {
        LbMemoryCopy(code, new_state, len); //undo
        return DELTA_NONE;
    }

    return DELTA_PREVSTATE;
}

static void decode(enum DeltaEncoding encoding, const char * code,
    const char * old_state, char * new_state, size_t len)
{
    size_t i;

    if (encoding == DELTA_NONE) {
        LbMemoryCopy(new_state, code, len);
    }
    else {
        for (i = 0; i < len; ++i) {
            new_state[i] = code[i] + old_state[i];
        }
    }
}

size_t LbNetsyncBufferSize(const struct NetsyncInstr ** instr)
{
    size_t size;
    unsigned i;

    for (i = 0, size = 0; instr[i] != NULL; ++i) {
        if (instr[i]->len == 0) {
            continue;
        }

        size += instr[i]->len;
        if (instr[i]->encoding == DELTA_SELECTBEST) {
            size += sizeof (NetsyncHeader);
        }
    }

    return size;
}

void LbNetsyncCollect(const struct NetsyncInstr ** instr, char * out_buffer,
    const char * old_state, char * new_state)
{
    unsigned i;
    NetsyncHeader * header;

    NETDBG(5, "Collecting data for synchronization");

    for (i = 0; instr[i] != NULL; ++i) {
        if (instr[i]->len == 0) {
            continue;
        }

        NETDBG(6, "Collecting data item %u", i);

        //reserve header
        if (instr[i]->encoding == DELTA_SELECTBEST) {
            header = new_state;
            new_state += sizeof (*header);
        }

        //copy body
        if (instr[i]->ptr != NULL) {
            LbMemoryCopy(new_state, instr[i]->ptr, instr[i]->len);
        }

        //call on_collect
        if (instr[i]->on_collect != NULL) {
            instr[i]->on_collect(new_state, instr[i]->ptr, instr[i]->len);
        }

        //TODO: work here
    }
}

void LbNetsyncRestore(const struct NetsyncInstr ** instr, const char * in_buffer,
    const char * old_state, char * new_state)
{
}

#ifdef __cplusplus
};
#endif

