#include "tst_main.h"

#include <map_data.h>
#include <slab_data.h>

extern "C"
{
extern void reveal_seen_slabs(int slb_x, int slb_y, unsigned char owner, unsigned char sight_distance);
extern void reveal_seen_slabs2(int cx, int cy, unsigned char owner, unsigned char sight_distance);
}

static int sight_colored(MapSlabCoord x, MapSlabCoord y)
{
    if (subtile_revealed(slab_subtile(x, 1), slab_subtile(y, 1), 0))
        return 32;
    else
        return 30;
}

static const char* sight_fn(MapSlabCoord x, MapSlabCoord y)
{
    const char *sym = tst_slab_to_symbol(x, y);
    if (subtile_revealed(slab_subtile(x, 1), slab_subtile(y, 1), 0))
        return "o";
    return sym;
}

ADD_TEST(test_sight_1)
{
    const char * map ="#####|\n"
                      "#....|\n"
                      "#..x.|\n"
                      "#....|\n"
                      "#....|\n";

    const char *expected = "#####|\n"
                           "#..o.|\n"
                           "#.ooo|\n"
                           "#..o.|\n"
                           "#....|\n";
    const char *result;
    MapSlabCoord slb_x, slb_y, max_x, max_y;

    tst_init_map();

    tst_parse_map(map, &slb_x, &slb_y, &max_x, &max_y);
    reveal_seen_slabs(slb_x, slb_y, 0,2 );
    result = tst_print_map(max_x, max_y, &sight_fn);
    if (strcmp(result, expected) != 0)
    {
        printf("\n%s\n", tst_print_colored_map(max_x, max_y, &sight_colored));
    }
    CU_ASSERT_STRING_EQUAL(result, expected);
}

ADD_TEST(test_sight_2)
{
    const char * map ="#######|\n"
                      "#......|\n"
                      "#..x  .|\n"
                      "#......|\n"
                      "#......|\n";

    const char *expected = "#######|\n"
                           "#..ooo.|\n"
                           "#.ooooo|\n"
                           "#..ooo.|\n"
                           "#......|\n";
    const char *result;
    MapSlabCoord slb_x, slb_y, max_x, max_y;

    tst_init_map();

    tst_parse_map(map, &slb_x, &slb_y, &max_x, &max_y);
    reveal_seen_slabs(slb_x, slb_y, 0,2 );
    result = tst_print_map(max_x, max_y, &sight_fn);
    if (strcmp(result, expected) != 0)
    {
        printf("\n%s\n", tst_print_colored_map(max_x, max_y, &sight_colored));
    }
    CU_ASSERT_STRING_EQUAL(result, expected);
}
/*
ADD_TEST(test_sight_3)
{
    const char * map ="#######|\n"
                      "#...~~.|\n"
                      "#...~~.|\n"
                      "#..x...|\n"
                      "#......|\n";
    MapSlabCoord slb_x, slb_y, max_x, max_y;

    tst_init_map();

    tst_parse_map(map, &slb_x, &slb_y, &max_x, &max_y);
    reveal_seen_slabs(slb_x, slb_y, 0, 2);
}
*/