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
 * @date     09 October 2010 - ?
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

    LbMemorySet(counts, 0, sizeof(counts));

    for (i = 0; i < len; i++) {
        counts[((unsigned) buffer[i]) & 0xFF] += 1; //handles other char sizes than 8 bit.. probably redundant
    }

    float nat = 0.0f;
    for (i = 0; i < 0x100; i++) {
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
    if (encoding == DELTA_NONE) {
        LbMemoryCopy(code, new_state, len);
        return DELTA_NONE;
    }

    //encode
    for (size_t i = 0; i < len; ++i)
    {
        code[i] = new_state[i] - old_state[i];
    }

    if (encoding == DELTA_PREVSTATE) {
        return DELTA_PREVSTATE;
    }

    //encoding == DELTA_SELECTBEST is implied from here

    //compare information content and select best
    if (self_information(new_state, len) <= self_information(code, len)) {
        NETDBG(6, "No delta selected");
        LbMemoryCopy(code, new_state, len); //undo
        return DELTA_NONE;
    }

    NETDBG(6, "Delta to previous state selected");
    return DELTA_PREVSTATE;
}

static void decode(enum DeltaEncoding encoding, const char * code,
    const char * old_state, char * new_state, size_t len)
{
    if (encoding == DELTA_NONE) {
        LbMemoryCopy(new_state, code, len);
    }
    else {
        //delta decode
        for (size_t i = 0; i < len; ++i)
        {
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
    NetsyncHeader * header = NULL;

    NETDBG(5, "Collecting data for synchronization");

    for (unsigned int i = 0; instr[i] != NULL; ++i)
    {
        if (instr[i]->len == 0) {
            continue;
        }

        NETDBG(6, "Collecting data item %u", i);

        //reserve header
        if (instr[i]->encoding == DELTA_SELECTBEST) {
            header = (NetsyncHeader*) out_buffer;
            out_buffer  += sizeof (*header);
            old_state   += sizeof (*header);
            new_state   += sizeof (*header);
        }

        //copy body to new state
        if (instr[i]->ptr != NULL) {
            LbMemoryCopy(new_state, instr[i]->ptr, instr[i]->len);
        }

        //call on_collect to alter new state
        if (instr[i]->on_collect != NULL) {
            instr[i]->on_collect(new_state, instr[i]->ptr, instr[i]->len);
        }

        //do (any) encoding
        enum DeltaEncoding enc;
        if (old_state == NULL)
        {
            enc = encode(DELTA_NONE, out_buffer, NULL, new_state, instr[i]->len);
        }
        else {
            enc = encode(instr[i]->encoding, out_buffer, old_state, new_state, instr[i]->len);
        }

        //save new encoding in header (if necessary)
        if (instr[i]->encoding == DELTA_SELECTBEST) {
            *header = (NetsyncHeader) enc;
        }

        //adjust pointers for next instruction
        out_buffer  += instr[i]->len;
        old_state   += instr[i]->len;
        new_state   += instr[i]->len;
    }
}

void LbNetsyncRestore(const struct NetsyncInstr ** instr, const char * in_buffer,
    const char * old_state, char * new_state)
{
    NetsyncHeader header = 0; //removing unit warning

    NETDBG(5, "Restoring data for synchronization");

    for (unsigned int i = 0; instr[i] != NULL; ++i)
    {
        if (instr[i]->len == 0) {
            continue;
        }

        NETDBG(6, "Restoring data item %u", i);

        //get header
        if (instr[i]->encoding == DELTA_SELECTBEST) {
            header = *(NetsyncHeader*) in_buffer;
            in_buffer   += sizeof (header);
            old_state   += sizeof (header);
            new_state   += sizeof (header);
        }

        //determine coding used
        enum DeltaEncoding enc;
        if (old_state == NULL)
        {
            enc = DELTA_NONE;
        }
        else {
            enc = instr[i]->encoding;
            enc = enc == DELTA_SELECTBEST? (enum DeltaEncoding) header : enc;
        }

        //do (any) decoding
        decode(enc, in_buffer, old_state, new_state, instr[i]->len);

        //copy new state to body
        if (instr[i]->ptr != NULL) {
            LbMemoryCopy(instr[i]->ptr, new_state, instr[i]->len);
        }

        //call on_restore to alter body
        if (instr[i]->on_restore != NULL) {
            instr[i]->on_restore(instr[i]->ptr, new_state, instr[i]->len);
        }

        //adjust pointers for next instruction
        in_buffer   += instr[i]->len;
        old_state   += instr[i]->len;
        new_state   += instr[i]->len;
    }
}

#ifdef __cplusplus
};
#endif

