/** @file TileSetScan.h
 *     Which tile texture sets a level uses. Pure functions, no game state. */
#ifndef KFX_TILESETS_TILESETSCAN_H
#define KFX_TILESETS_TILESETSCAN_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number of tile texture sets: set 0 is the level's own, set v is standard pack v-1. */
#define TILE_SET_COUNT 32
/** Slab texture bytes select a set with these bits. */
#define TILE_SET_INDEX_MASK 0x1F

/** Where a set's tile data is. */
enum TileSetState {
    TileSet_Absent   = 0, /**< Not loaded; its tiles point at set 0. */
    TileSet_Pending  = 1, /**< Queued or loading on the loader thread. */
    TileSet_Resident = 2, /**< Loaded (or the alias of set 0). */
};

/** Mask of the sets in use: bit 0 always, plus every slab value and player pack. */
uint32_t tile_sets_scan(const unsigned char *ext, const unsigned char *ext_initial, size_t slab_count,
                        const unsigned char *packs, size_t pack_count);
/** The set that holds the same pack as set 0, or -1 if there is none. */
int32_t tile_set_alias_index(uint32_t level_texture_id);
/** 1 if the set must be queued for loading. */
int tile_set_needs_request(int32_t set, unsigned char state, int is_alias);

#ifdef __cplusplus
}
#endif
#endif
