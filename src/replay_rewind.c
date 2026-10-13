/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file replay_rewind.c
 * @par Purpose:
 *     Rewinding a replay while watching it.
 * @par Comment:
 *     Keyframes are taken every REWIND_KEYFRAME_INTERVAL turns while watching.
 *     Turns in between can be reached with resimulation.
 *     Kernel-accelerated memory diffing is used.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "replay_rewind.h"

#include <zlib.h>
#include "bflib_math.h"
#include "bflib_sound.h"
#include "ariadne.h"
#include "ariadne_tringls.h"
#include "config_keeperfx.h"
#include "game_legacy.h"
#include "kernel_memtracker.h"
#include "keeperfx.hpp"
#include "lua_base.h"
#include "player_data.h"
#include "replay.h"
#include "thing_data.h"
#include "thing_list.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define REWIND_KEYFRAME_INTERVAL 20
#define REWIND_MAX_SECTIONS KMT_MAX_RANGES
typedef uint64_t linebits_t;
#define REWIND_LINE_SIZE 64
#define REWIND_LINES_PER_PAGE (sizeof(linebits_t)*CHAR_BIT)
#define REWIND_PAGE_SIZE (REWIND_LINES_PER_PAGE * REWIND_LINE_SIZE)
#define REWIND_PAGE_MAX_ENCODED (sizeof(uint32_t) + sizeof(linebits_t) + REWIND_LINES_PER_PAGE * (1 + 8 * (1 + 8)))

struct MemSpanChunkAligned {
    unsigned char *ptr;
    size_t len;
    size_t snapshot_offset;
    TbBool kernel_tracked;
    size_t first_chunk;
};

// unaligned or small sections.
// These all go together at the end of the snapshot.
struct MemFragment {
    unsigned char* ptr;
    size_t len;
    size_t snapshot_offset;
};

/*
 * Ring buffer record: [RewindKeyframe][diffs...][lua][header offset].
 * A record may wrap to the start of the ring between those parts, but not within one.
 */
struct RewindKeyframe {
    GameTurn turn;
    struct ReplayReaderPos reader_pos;
    // ring offsets
    size_t next;
    size_t diff_start;
    size_t diff_end;
    size_t lua_off;
    size_t lua_len;
    size_t lua_raw_len;
};

#define RING_ALIGN 8
#define ring_align(n) (((n) + RING_ALIGN - 1) & ~(size_t)(RING_ALIGN - 1))
#define KEYFRAME_HEADER_SIZE ring_align(sizeof(struct RewindKeyframe))

static struct MemSpanChunkAligned spans[REWIND_MAX_SECTIONS];
static int spans_count;
static struct MemFragment fragments[REWIND_MAX_SECTIONS * 2];
static int fragments_count;

// size in bytes of an uncompressed rewind snapshot
static size_t snapshot_size;

// offset of the start of the MemFragment section of snapshots.
static size_t snapshot_fragments_start;

// kernel_memtracker init'd successfully
static TbBool kernel_tracking;

// one bool8 per chunk to filter out large unchanged regions before diffing
static unsigned char *chunk_dirty;

// buffer to copy the end fragments into for diffing
static unsigned char *fragment_stage;
static unsigned char *snapshot;

// turns whose state we can load directly (after delta-decompression)
static unsigned char *ring;
static size_t ring_cap;
static size_t ring_head;
// start of the oldest record
static size_t ring_tail;
// where records stopped before the head wrapped to 0, SIZE_MAX if the live records don't wrap
static size_t ring_wrap = SIZE_MAX;

// keyframe whose state snapshot can be loaded directly (after delta-decompression), not requiring additional simulation
static struct RewindKeyframe *snapshot_kf;
static struct RewindKeyframe *newest;
static int keyframes_count;

// diffs and lua are encoded here, then copied into the ring
static unsigned char *stage;
static size_t stage_cap;

// kfx_rewindable variables
#if defined(_WIN32)
REWIND_SECTION(".rwd$a") static char rwd_start[1];
REWIND_SECTION(".rwd$z") static char rwd_end[1];
#else
extern char __start_kfx_rwd[];
extern char __stop_kfx_rwd[];
#define rwd_start __start_kfx_rwd
#define rwd_end __stop_kfx_rwd
#endif
/******************************************************************************/

static void add_fragment(unsigned char *ptr, size_t len)
{
    if (len == 0)
        return;
    fragments[fragments_count].ptr = ptr;
    fragments[fragments_count].len = len;
    fragments_count++;
}

// splits into a chunk-aligned middle (span) and unaligned edges (fragments)
static void register_memtracker(void *ptr, size_t len)
{
    if ((spans_count >= REWIND_MAX_SECTIONS) || (fragments_count + 2 > REWIND_MAX_SECTIONS * 2))
    {
        ERRORLOG("Too many tracked memory sections");
        return;
    }
    if (!kernel_tracking) kernel_tracking = kernel_memtracker_init();
    const size_t align = kernel_memtracker_chunk_size();

    // pull out the juicy `MemSpanChunkAligned` and deal with whatever `MemFragment`s remain separately
    unsigned char *start = (unsigned char *)ptr;
    unsigned char *end = start + len;
    unsigned char *mid_start = (unsigned char *)(((uintptr_t)start + align - 1) / align * align);
    unsigned char *mid_end = (unsigned char *)((uintptr_t)end / align * align);
    if (mid_start >= mid_end)
    {
        add_fragment(start, end - start);
        return;
    }
    add_fragment(start, mid_start - start);
    add_fragment(mid_end, end - mid_end);
    struct MemSpanChunkAligned *span = &spans[spans_count++];
    span->ptr = mid_start;
    span->len = mid_end - mid_start;
    span->snapshot_offset = snapshot_size;
    span->first_chunk = kernel_memtracker_get_chunk_count();
    span->kernel_tracked = kernel_tracking && kernel_memtracker_add(span->ptr, span->len);
    snapshot_size += span->len;
}

static TbBool init_buffers(void)
{
    if (snapshot != NULL)
        return true;
    if (snapshot_size == 0)
    {
        register_memtracker(&game, sizeof(game));
        register_memtracker(Triangles, sizeof(Triangles));
        register_memtracker(rwd_start, rwd_end - rwd_start);

        // add the fragments to the snapshot size
        snapshot_fragments_start = snapshot_size;
        for (int i = 0; i < fragments_count; i++)
        {
            fragments[i].snapshot_offset = snapshot_size;
            snapshot_size += fragments[i].len;
        }
        snapshot_size = (snapshot_size + REWIND_PAGE_SIZE - 1) / REWIND_PAGE_SIZE * REWIND_PAGE_SIZE;
    }
    if (snapshot_size == 0) return false;

    snapshot = (unsigned char *)calloc(1, snapshot_size);
    fragment_stage = (unsigned char *)calloc(1, snapshot_size - snapshot_fragments_start);
    chunk_dirty = (unsigned char *)calloc(1, max(kernel_memtracker_get_chunk_count(), 1));
    ring_cap = (size_t)replay_rewind_buffer_kb * 1024;
    ring = (unsigned char *)malloc(ring_cap);
    if ((snapshot == NULL) || (fragment_stage == NULL) || (chunk_dirty == NULL) || (ring == NULL))
    {
        ERRORLOG("Cannot allocate replay rewind buffers");
        replay_rewind_reset();
        return false;
    }
    return true;
}

static void capture_snapshot(unsigned char *dst)
{
    for (int i = 0; i < spans_count; i++)
        memcpy(dst + spans[i].snapshot_offset, spans[i].ptr, spans[i].len);
    for (int i = 0; i < fragments_count; i++)
        memcpy(dst + fragments[i].snapshot_offset, fragments[i].ptr, fragments[i].len);
}

static void restore_snapshot(const unsigned char *src)
{
    stop_all_things_playing_samples();
    for (int i = 0; i < spans_count; i++)
        memcpy(spans[i].ptr, src + spans[i].snapshot_offset, spans[i].len);
    for (int i = 0; i < fragments_count; i++)
        memcpy(fragments[i].ptr, src + fragments[i].snapshot_offset, fragments[i].len);
    for (ThingIndex i = 0; i < THINGS_COUNT; i++)
        thing_get(i)->snd_emitter_id = 0;
}

static struct RewindKeyframe *oldest_keyframe(void)
{
    return (struct RewindKeyframe *)(ring + ring_tail);
}

static struct RewindKeyframe *kf_older(const struct RewindKeyframe *kf)
{
    size_t end = (unsigned char *)kf - ring;
    if (end == ring_tail)
        return NULL;
    if (end == 0)
        end = ring_wrap;
    return (struct RewindKeyframe *)(ring + *(size_t *)(ring + end - RING_ALIGN));
}

static void evict_oldest_keyframe(void)
{
    size_t next = oldest_keyframe()->next;
    keyframes_count--;
    if (keyframes_count == 0)
    {
        newest = NULL;
        ring_head = 0;
        ring_tail = 0;
        ring_wrap = SIZE_MAX;
        return;
    }
    if (next == ring_wrap)
        next = 0;
    if (next < ring_tail)
        ring_wrap = SIZE_MAX;
    ring_tail = next;
}

// reserves size contiguous bytes, evicting the oldest keyframes as needed but never the newest
static unsigned char *ring_alloc(size_t size, TbBool align)
{
    for (;;)
    {
        const size_t head = align ? ring_align(ring_head) : ring_head;
        size_t off = SIZE_MAX;
        if (keyframes_count == 0)
        {
            if (size <= ring_cap)
                off = 0;
        } else
        if (ring_head >= ring_tail)
        {
            if (head + size <= ring_cap)
                off = head;
            else if (size < ring_tail)
            {
                ring_wrap = ring_head;
                off = 0;
            }
        } else
        if (head + size < ring_tail)
            off = head;
        if (off != SIZE_MAX)
        {
            ring_head = off + size;
            return ring + off;
        }
        if (keyframes_count <= 1)
            return NULL;
        evict_oldest_keyframe();
    }
}

/*
 * Diff format, per changed page: u32 page index, u64 bitfield of changed lines;
 * per changed line: a u8 of changed words;
 * per changed word: a u8 of changed bytes;
 * per changed byte: xor of old and new byte
 */
// appends the XOR-delta of old_page and new_page to the ring, then old_page<-new_page
static TbBool diff_page(uint32_t pg, unsigned char *old_page, const unsigned char *new_page)
{
    const uint64_t *wa = (const uint64_t *)old_page;
    const uint64_t *wb = (const uint64_t *)new_page;
    linebits_t line_mask = 0;
    for (int ln = 0; ln < REWIND_LINES_PER_PAGE; ln++)
    {
        uint64_t any = 0;
        for (int w = 0; w < 8; w++)
            any |= wa[ln * 8 + w] ^ wb[ln * 8 + w];
        if (any != 0)
            line_mask |= (linebits_t)1 << ln;
    }
    if (line_mask == 0)
        return true;
    // reserve the worst case, then give back what's unused
    unsigned char *out = ring_alloc(REWIND_PAGE_MAX_ENCODED, false);
    if (out == NULL)
        return false;
    size_t len = 0;
    memcpy(out + len, &pg, sizeof(pg));
    len += sizeof(pg);
    memcpy(out + len, &line_mask, sizeof(line_mask));
    len += sizeof(line_mask);
    for (linebits_t lm = line_mask; lm != 0; lm &= lm - 1)
    {
        const int ln = LbCtz64(lm);
        unsigned char *word_mask = &out[len++];
        *word_mask = 0;
        for (int w = 0; w < 8; w++)
        {
            const uint64_t x = wa[ln * 8 + w] ^ wb[ln * 8 + w];
            if (x == 0)
                continue;
            *word_mask |= 1 << w;
            const unsigned char *xb = (const unsigned char *)&x;
            unsigned char *byte_mask = &out[len++];
            *byte_mask = 0;
            for (int i = 0; i < 8; i++)
            {
                if (xb[i] != 0)
                {
                    *byte_mask |= 1 << i;
                    out[len++] = xb[i];
                }
            }
        }
    }
    ring_head = out - ring + len;
    memcpy(old_page, new_page, REWIND_PAGE_SIZE);
    return true;
}

// add to ring: diff of last full snapshot against the current state
static TbBool diff_snapshot_against_live(void)
{
    const TbBool use_dirty = kernel_tracking && kernel_memtracker_collect(chunk_dirty);
    const size_t pages_per_chunk = kernel_memtracker_chunk_size() / REWIND_PAGE_SIZE;
    for (int i = 0; i < spans_count; i++)
    {
        const struct MemSpanChunkAligned *span = &spans[i];
        const uint32_t first_page = span->snapshot_offset / REWIND_PAGE_SIZE;
        for (size_t p = 0; p < span->len / REWIND_PAGE_SIZE; p++)
        {
            if (use_dirty && span->kernel_tracked && !chunk_dirty[span->first_chunk + p / pages_per_chunk])
                continue;
            if (!diff_page(first_page + p, snapshot + span->snapshot_offset + p * REWIND_PAGE_SIZE, span->ptr + p * REWIND_PAGE_SIZE))
                return false;
        }
    }
    for (int i = 0; i < fragments_count; i++)
        memcpy(fragment_stage + (fragments[i].snapshot_offset - snapshot_fragments_start), fragments[i].ptr, fragments[i].len);
    for (size_t off = snapshot_fragments_start; off < snapshot_size; off += REWIND_PAGE_SIZE)
    {
        if (!diff_page(off / REWIND_PAGE_SIZE, snapshot + off, fragment_stage + (off - snapshot_fragments_start)))
            return false;
    }
    kernel_memtracker_clear();
    return true;
}

static TbBool stage_reserve(size_t need)
{
    if (need <= stage_cap)
        return true;
    size_t cap = (stage_cap > 0) ? stage_cap : 65536;
    while (cap < need)
        cap *= 2;
    unsigned char *grown = (unsigned char *)realloc(stage, cap);
    if (grown == NULL)
        return false;
    stage = grown;
    stage_cap = cap;
    return true;
}

static TbBool capture_lua(struct RewindKeyframe *kf)
{
    if (Lvl_script == NULL)
        return true;
    TbBool ok = true;
    size_t len = 0;
    const char *data = lua_get_serialised_data(&len);
    if ((data != NULL) && (len > 0))
    {
        ok = stage_reserve(compressBound(len));
        uLongf dst_len = stage_cap;
        if (ok && (compress2(stage, &dst_len, (const Bytef *)data, len, Z_BEST_SPEED) == Z_OK))
        {
            unsigned char *dst = ring_alloc(dst_len, false);
            ok = dst != NULL;
            if (ok)
            {
                memcpy(dst, stage, dst_len);
                kf->lua_off = dst - ring;
                kf->lua_len = dst_len;
                kf->lua_raw_len = len;
            }
        }
    }
    cleanup_serialized_data();
    return ok;
}

static TbBool restore_lua(const struct RewindKeyframe *kf)
{
    if (Lvl_script == NULL)
        return true;
    if (kf->lua_len > 0)
    {
        char *data = (char *)malloc(kf->lua_raw_len);
        if (data == NULL)
            return false;
        uLongf len = kf->lua_raw_len;
        TbBool ok = (uncompress((Bytef *)data, &len, ring + kf->lua_off, kf->lua_len) == Z_OK) && (len == kf->lua_raw_len)
            && lua_set_serialised_data(data, len);
        free(data);
        if (!ok)
            return false;
    }
    lua_set_random_seed(game.action_random_seed);
    return true;
}

static TbBool apply_diff(unsigned char *snap, const struct RewindKeyframe *kf)
{
    const unsigned char *p = ring + kf->diff_start;
    const unsigned char *end = ring + kf->diff_end;
    const uint32_t pages = snapshot_size / REWIND_PAGE_SIZE;
    while (p != end)
    {
        if (p == ring + ring_wrap)
        {
            p = ring;
            continue;
        }
        uint32_t pg;
        linebits_t line_mask;
        memcpy(&pg, p, sizeof(pg));
        p += sizeof(pg);
        memcpy(&line_mask, p, sizeof(line_mask));
        p += sizeof(line_mask);
        if (pg >= pages)
            return false;
        unsigned char *page = snap + (size_t)pg * REWIND_PAGE_SIZE;
        for (; line_mask != 0; line_mask &= line_mask - 1)
        {
            unsigned char *line = page + LbCtz64(line_mask) * REWIND_LINE_SIZE;
            for (unsigned word_mask = *p++; word_mask != 0; word_mask &= word_mask - 1)
            {
                unsigned char *word = line + LbCtz32(word_mask) * 8;
                for (unsigned byte_mask = *p++; byte_mask != 0; byte_mask &= byte_mask - 1)
                    word[LbCtz32(byte_mask)] ^= *p++;
            }
        }
    }
    return true;
}

// diffs are XOR, so their order doesn't matter: either way, walk back from the newer of the two
static TbBool move_snapshot_to(struct RewindKeyframe *target)
{
    struct RewindKeyframe *kf = (target->turn > snapshot_kf->turn) ? target : snapshot_kf;
    const struct RewindKeyframe *stop = (kf == target) ? snapshot_kf : target;
    for (; kf != stop; kf = kf_older(kf))
    {
        if ((kf == NULL) || !apply_diff(snapshot, kf))
            return false;
    }
    snapshot_kf = target;
    return true;
}

void replay_rewind_capture(void)
{
    if (!replay.load_enable || (replay_rewind_buffer_kb == 0))
        return;
    const GameTurn turn = get_gameturn();
    if ((newest != NULL) && (turn < newest->turn + REWIND_KEYFRAME_INTERVAL))
        return;
    if (!init_buffers())
        return;
    const TbBool first = newest == NULL;
    if (first)
    {
        capture_snapshot(snapshot);
        kernel_memtracker_clear();
    }
    else if (!move_snapshot_to(newest))
    {
        ERRORLOG("Cannot rebuild replay rewind keyframe; rewind disabled");
        replay_rewind_reset();
        return;
    }
    struct RewindKeyframe *kf = (struct RewindKeyframe *)ring_alloc(KEYFRAME_HEADER_SIZE, true);
    if (kf == NULL)
    {
        ERRORLOG("Replay rewind buffer is too small for a keyframe; rewind disabled");
        replay_rewind_reset();
        return;
    }
    memset(kf, 0, sizeof(*kf));
    kf->turn = turn;
    replay_save_reader_pos(&kf->reader_pos);
    keyframes_count++;
    newest = kf;
    snapshot_kf = kf;
    kf->diff_start = ring_head;
    TbBool ok = first || diff_snapshot_against_live();
    kf->diff_end = ring_head;
    ok = ok && capture_lua(kf);
    size_t *trailer = ok ? (size_t *)ring_alloc(RING_ALIGN, true) : NULL;
    if (trailer == NULL)
    {
        ERRORLOG("Cannot store replay rewind keyframe; rewind disabled");
        replay_rewind_reset();
        return;
    }
    *trailer = (unsigned char *)kf - ring;
    kf->next = ring_head;
}

void replay_rewind_step(void)
{
    if (keyframes_count == 0)
        return;
    const GameTurn turn = get_gameturn();
    if (turn <= oldest_keyframe()->turn)
        return;
    const GameTurn target = turn - 1;
    struct RewindKeyframe *kf = newest;
    while ((kf->turn > target) && (kf_older(kf) != NULL))
        kf = kf_older(kf);
    // pause is legitimate game state in single player, but not during replays (which skip pauses)
    const TbBool paused = flag_is_set(game.operation_flags, GOF_Paused);
    if (!move_snapshot_to(kf))
    {
        ERRORLOG("Cannot rebuild replay rewind keyframe; rewind disabled");
        replay_rewind_reset();
        return;
    }
    restore_snapshot(snapshot);
    if (!restore_lua(kf) || !replay_restore_reader_pos(&kf->reader_pos))
    {
        ERRORLOG("Cannot restore replay rewind keyframe; rewind disabled");
        set_flag_value(game.operation_flags, GOF_Paused, paused);
        replay_rewind_reset();
        return;
    }
    clear_flag(game.operation_flags, GOF_Paused);
    const TbBool sound_disabled = SoundDisabled;
    SoundDisabled = true;
    while ((get_gameturn() < target) && replay.load_enable && !quit_game && !exit_keeper)
    {
        load_packets_for_turn(replay.pckt_gameturn);
        replay.pckt_gameturn++;
        update();
    }
    SoundDisabled = sound_disabled;
    set_flag_value(game.operation_flags, GOF_Paused, paused);
}

TbBool replay_rewind_available(void)
{
    return replay.load_enable && (keyframes_count > 0);
}

TbBool replay_rewind_at_limit(void)
{
    return (keyframes_count > 0) && (get_gameturn() <= oldest_keyframe()->turn);
}

size_t replay_rewind_buffer_used(void)
{
    if (keyframes_count == 0)
        return 0;
    return (ring_head > ring_tail) ? ring_head - ring_tail : ring_wrap - ring_tail + ring_head;
}

void replay_rewind_reset(void)
{
    newest = NULL;
    keyframes_count = 0;
    free(ring);
    free(stage);
    ring = NULL;
    stage = NULL;
    ring_cap = 0;
    ring_head = 0;
    ring_tail = 0;
    ring_wrap = SIZE_MAX;
    stage_cap = 0;
    snapshot_kf = NULL;
    free(snapshot);
    free(fragment_stage);
    free(chunk_dirty);
    snapshot = NULL;
    fragment_stage = NULL;
    chunk_dirty = NULL;
    kernel_memtracker_disarm();
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
