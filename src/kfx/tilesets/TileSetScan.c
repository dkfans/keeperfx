/** @file TileSetScan.c
 *     Which tile texture sets a level uses. */
#include "kfx/tilesets/TileSetScan.h"

uint32_t tile_sets_scan(const unsigned char *ext, const unsigned char *ext_initial, size_t slab_count,
                        const unsigned char *packs, size_t pack_count)
{
    uint32_t mask = 1u;
    for (size_t i = 0; i < slab_count; i++)
    {
        mask |= 1u << (ext[i] & TILE_SET_INDEX_MASK);
        mask |= 1u << (ext_initial[i] & TILE_SET_INDEX_MASK);
    }
    for (size_t i = 0; i < pack_count; i++)
        mask |= 1u << (packs[i] & TILE_SET_INDEX_MASK);
    return mask;
}

int32_t tile_set_alias_index(uint32_t level_texture_id)
{
    if (level_texture_id + 1 >= TILE_SET_COUNT)
        return -1;
    return (int32_t)level_texture_id + 1;
}

int tile_set_needs_request(int32_t set, unsigned char state, int is_alias)
{
    if (set <= 0 || set >= TILE_SET_COUNT)
        return 0;
    return !is_alias && state == TileSet_Absent;
}
