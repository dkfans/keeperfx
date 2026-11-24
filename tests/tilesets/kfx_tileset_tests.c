/** @file kfx_tileset_tests.c
 *     Standalone tests for tile set loading. No game code, no game data. */
#include "pre_inc.h"
#include "bflib_dernc.h"
#include "kfx/tilesets/TileSetScan.h"
#include "kfx/tilesets/TileSetLoader.h"
#include "post_inc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_checks;
static int g_fails;

#define CHECK(cond, ...) \
    do { \
        g_checks++; \
        if (!(cond)) { \
            g_fails++; \
            printf("FAIL %s:%d: ", __FILE__, __LINE__); \
            printf(__VA_ARGS__); \
            printf("\n"); \
        } \
    } while (0)

static void write_file(const char *path, size_t len, unsigned char fill)
{
    FILE *f = fopen(path, "wb");
    for (size_t i = 0; i < len; i++)
        fputc(fill, f);
    fclose(f);
}

static void test_load_at_max(void)
{
    unsigned char buf[64];
    const char *path = "kfx_tileset_test_file.dat";

    write_file(path, 32, 7);
    memset(buf, 0xAA, sizeof(buf));
    CHECK(LbFileLoadAtMax(path, buf, sizeof(buf)) == 32, "a file within the limit loads");
    CHECK(buf[0] == 7 && buf[31] == 7 && buf[32] == 0xAA, "only the file's bytes are written");

    write_file(path, 65, 9);
    memset(buf, 0xAA, sizeof(buf));
    CHECK(LbFileLoadAtMax(path, buf, sizeof(buf)) == LbLoadAtMax_TooLarge, "a file over the limit is refused");
    CHECK(buf[0] == 0xAA && buf[63] == 0xAA, "a refused file writes nothing");

    remove(path);
    CHECK(LbFileLoadAtMax(path, buf, sizeof(buf)) == LbLoadAtMax_Missing, "a missing file is reported");
}

static void test_scan(void)
{
    unsigned char ext[6]     = {0, 0, 3, 0, 0, 0};
    unsigned char initial[6] = {0, 0, 0, 0, 0, 7};
    unsigned char packs[3]   = {0, 12, 0};
    CHECK(tile_sets_scan(ext, initial, 6, packs, 3) == ((1u << 0) | (1u << 3) | (1u << 7) | (1u << 12)),
          "set 0, both slab arrays and player packs are in the mask");

    unsigned char none[4] = {0, 0, 0, 0};
    CHECK(tile_sets_scan(none, none, 4, none, 4) == 1u, "an untouched level needs only set 0");

    unsigned char high[1] = {0x20 | 5};
    CHECK(tile_sets_scan(high, none, 1, NULL, 0) == ((1u << 0) | (1u << 5)), "values are masked to 0x1F");
}

static void alias_and_request_rules(void)
{
    CHECK(tile_set_alias_index(0) == 1, "pack 0 is set 1");
    CHECK(tile_set_alias_index(30) == 31, "pack 30 is set 31");
    CHECK(tile_set_alias_index(31) == -1, "pack 31 has no set");

    CHECK(tile_set_needs_request(4, TileSet_Absent, 0) == 1, "an absent set is queued");
    CHECK(tile_set_needs_request(4, TileSet_Pending, 0) == 0, "a pending set is not queued twice");
    CHECK(tile_set_needs_request(4, TileSet_Resident, 0) == 0, "a resident set is not queued");
    CHECK(tile_set_needs_request(4, TileSet_Absent, 1) == 0, "the alias of set 0 is never queued");
    CHECK(tile_set_needs_request(0, TileSet_Absent, 0) == 0, "set 0 is never queued");
    CHECK(tile_set_needs_request(32, TileSet_Absent, 0) == 0, "out-of-range sets are ignored");
}

static void loader_drops_nothing_and_tags_serial(void)
{
    struct TileSetJob job;
    memset(&job, 0, sizeof(job));
    job.set = 5;
    job.level_serial = 42;
    snprintf(job.path_a, sizeof(job.path_a), "kfx_tileset_a.dat");
    snprintf(job.path_b, sizeof(job.path_b), "kfx_tileset_b_missing.dat");
    write_file(job.path_a, TILE_SET_LETTER_BYTES, 3);
    remove(job.path_b);

    TileSetLoader_Queue(&job);
    TileSetLoader_WaitIdle();

    struct TileSetResult res;
    CHECK(TileSetLoader_TakeDone(&res) == 1, "a finished job is on the done list");
    CHECK(res.set == 5 && res.level_serial == 42, "the result keeps the set and level serial");
    CHECK(res.result_a == (int32_t)TILE_SET_LETTER_BYTES, "file a loaded in full");
    CHECK(res.result_b == LbLoadAtMax_Missing, "file b reported missing");
    CHECK(res.buffer != NULL && res.buffer[0] == 3 && res.buffer[TILE_SET_LETTER_BYTES - 1] == 3,
          "file a's bytes are in the first half");
    CHECK(res.buffer != NULL && res.buffer[TILE_SET_LETTER_BYTES] == TILE_SET_FILL_COLOUR
          && res.buffer[TILE_SET_BYTES - 1] == TILE_SET_FILL_COLOUR, "a missing file leaves colour 130");
    free(res.buffer);
    CHECK(TileSetLoader_TakeDone(&res) == 0, "the done list is empty afterwards");

    remove(job.path_a);
    TileSetLoader_Shutdown();
}

int main(void)
{
    test_load_at_max();
    test_scan();
    alias_and_request_rules();
    loader_drops_nothing_and_tags_serial();
    printf("%d checks, %d failed\n", g_checks, g_fails);
    // Leave a running worker behind: process exit must still end cleanly.
    struct TileSetJob job;
    memset(&job, 0, sizeof(job));
    job.set = 1;
    TileSetLoader_Queue(&job);
    return g_fails ? 1 : 0;
}
