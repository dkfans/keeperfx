/** @file TileSetLoader.h
 *     Background loading of tile texture sets. */
#ifndef KFX_TILESETS_TILESETLOADER_H
#define KFX_TILESETS_TILESETLOADER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TILE_SET_LETTER_BYTES 557056u
#define TILE_SET_BYTES (2u * TILE_SET_LETTER_BYTES)
#define TILE_SET_FILL_COLOUR 130
#define TILE_SET_PATH_SIZE 4096

/** One set to load. Paths are resolved on the game thread. */
struct TileSetJob {
    int32_t set;
    uint32_t level_serial;
    char path_a[TILE_SET_PATH_SIZE];
    char path_b[TILE_SET_PATH_SIZE];
};

/** A loaded set. The receiver owns buffer and releases it with free(). */
struct TileSetResult {
    int32_t set;
    uint32_t level_serial;
    unsigned char *buffer;
    int32_t result_a; /**< LbFileLoadAtMax() result for path_a. */
    int32_t result_b; /**< LbFileLoadAtMax() result for path_b. */
};

void TileSetLoader_Queue(const struct TileSetJob *job);
int  TileSetLoader_TakeDone(struct TileSetResult *out);
void TileSetLoader_WaitIdle(void);
void TileSetLoader_Shutdown(void);

#ifdef __cplusplus
}
#endif
#endif
