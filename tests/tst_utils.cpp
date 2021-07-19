#include "tst_main.h"
#include "../src/engine_render.h"

#include <ariadne.h>
#include <front_simple.h>
#include <game_legacy.h>
#include <map_blocks.h>
#include <room_util.h>
#include <slab_data.h>

extern "C" {

extern void init_lookups(void);

unsigned char tmp_scratch[1024 * 1024 * 16]; //16Mb
static char map_data[256*256*12]; // This should be enough

void tst_parse_map(const char *data, MapSlabCoord *src_x, MapSlabCoord *src_y, MapSlabCoord *max_x, MapSlabCoord *max_y)
{
    MapSlabCoord slab_x = 0;
    MapSlabCoord slab_y = 0;
    *max_x = 0;
    *max_y = 0;

    for (; *data != 0; data++)
    {
        if ((slab_x == 0) || (slab_y == 0))
        {
            switch (*data)
            {
                case '#':
                    slab_x++;
                    break;
                case '|':
                    if (slab_x > *max_x)
                        *max_x = slab_x;
                    slab_x = 0;
                    slab_y++;
                    if (slab_y > MAP_SIZE_SLB)
                    {
                        CU_FAIL_FATAL("Map is too big");
                        return;
                    }
                    break;
                case '\n':
                    break;
                default:
                    CU_FAIL_FATAL("Map walls should be Rock");
                    return;
            }
            continue;
        }
        switch (*data)
        {
            case '|': // Next line
                if (slab_x > *max_x)
                    *max_x = slab_x;
                slab_x = 0;
                slab_y++;
                if (slab_y > MAP_SIZE_SLB)
                {
                    CU_FAIL_FATAL("Map is too big");
                    return;
                }
                continue;
            case '#': // Rock
                replace_slab_from_script(slab_x, slab_y, SlbT_ROCK);
                break;
            case '.': // Earth
                replace_slab_from_script(slab_x, slab_y, SlbT_EARTH);
                break;
            case '~': // Water
                replace_slab_from_script(slab_x, slab_y, SlbT_WATER);
                break;
            case 'X': // Claimed path with mark
                *src_x = slab_x;
                *src_y = slab_y;
                // Fallthrough
            case '_':
                replace_slab_from_script(slab_x, slab_y, SlbT_CLAIMED);
                break;
            case 'x': // Empty path with mark
                *src_x = slab_x;
                *src_y = slab_y;
                // Fallthrough
            case ' ':
                replace_slab_from_script(slab_x, slab_y, SlbT_PATH);
                break;
            case '\n':
                slab_x--;
                break;
            default:
                CU_FAIL_FATAL("Unknown symbol");
        }
        slab_x++;
    }
    if (slab_y > *max_y)
        *max_y = slab_y;
}

void tst_init_map()
{
    init_lookups(); // Used by columns data
    init_navigation(); // Used by triangulation
    game.neutral_player_num = 6; // Otherwise things are weird
    scratch = tmp_scratch;
}

/*
 * This function returns a slab symbol at given location
 */
const char* tst_slab_to_symbol(MapSlabCoord x, MapSlabCoord y)
{
    struct SlabMap *slab_map = get_slabmap_block(x, y);
    switch(slab_map->kind)
    {
        case SlbT_ROCK:
            return "#";
        case SlbT_EARTH:
            return ".";
        case SlbT_PATH:
            return " ";
        case SlbT_WATER:
            return "~";
        case SlbT_CLAIMED:
            return "_";
        default:
            return "?";
    }
}

const char *tst_print_map(MapSlabCoord max_x, MapSlabCoord max_y, const char* (*test_fn)(MapSlabCoord x, MapSlabCoord y))
{
    map_data[0] = 0;
    for (MapSlabCoord y = 0; y < max_y; y++)
    {
        for (MapSlabCoord x = 0; x < max_x; x++)
        {
            strcat(map_data, test_fn(x,y));
        }
        strcat(map_data, "|\n");
    }
    return map_data;
}

const char *tst_print_colored_map(MapSlabCoord max_x, MapSlabCoord max_y, int (*test_fn)(MapSlabCoord x, MapSlabCoord y))
{
    map_data[0] = 0;
    for (MapSlabCoord y = 0; y < max_y; y++)
    {
        for (MapSlabCoord x = 0; x < max_x; x++)
        {
            switch (test_fn(x, y))
            {
                case 1:
                    strcat(map_data, "\x1B[48;5;52m");
                    break;
                case 2:
                    strcat(map_data, "\x1B[48;5;88m");
                    break;
                case 3:
                    strcat(map_data, "\x1B[48;5;124m");
                    break;
                case 11: // green
                    strcat(map_data, "\x1B[48;5;22m");
                    break;
                case 12:
                    strcat(map_data, "\x1B[48;5;28m");
                    break;
                case 13:
                    strcat(map_data, "\x1B[48;5;34m");
                    break;
                case 21:
                    strcat(map_data, "\x1B[48;5;18m");
                    break;
                case 22:
                    strcat(map_data, "\x1B[48;5;19m");
                    break;
                case 23:
                    strcat(map_data, "\x1B[48;5;20m");
                    break;
                case 31:
                    strcat(map_data, "\x1B[48;5;236m");
                    break;
                case 32:
                    strcat(map_data, "\x1B[48;5;240m");
                    break;
                case 33:
                    strcat(map_data, "\x1B[48;5;243m");
                    break;
                case 0: //fallthrough
                case 10://fallthrough
                case 20://fallthrough
                case 30://fallthrough
                    strcat(map_data, "\x1B[48;5;233m");
                default:
                    break;
            }
            strcat(map_data, tst_slab_to_symbol(x, y));
            strcat(map_data, "\x1B[0m");
        }
        strcat(map_data, "|\n");
    }
    return map_data;
}

}