#include "tst_main.h"

#include <map_blocks.h>
#include <map_data.h>
#include <slab_data.h>

extern "C" {

extern void reveal_seen_slabs(int slb_x, int slb_y, unsigned char owner, unsigned char sight_distance);

}

static void parse_map(const char *data, MapSlabCoord *src_x, MapSlabCoord *src_y)
{
    MapSlabCoord slab_x = 0;
    MapSlabCoord slab_y = 0;
    for (;*data != 0; data++)
    {
        struct SlabMap *slab_data = get_slabmap_block(slab_x, slab_y);
        switch (*data)
        {
            case '|': // Next line
                slab_x = 0;
                slab_y ++;
                if (slab_y > MAP_SIZE_SLB)
                {
                    CU_FAIL("Map is too big");
                    return;
                }
                continue;
            case '#': // Rock
                break;
            case '.': // Earth
                break;
            case '~': // Water
                break;
            case 'x': // Empty path with mark
                *src_x = slab_x;
                *src_y = slab_y;
                // Fallthrough
            case ' ':
                break;
        }
        slab_x++;
    }
}

ADD_TEST(test_sight_3)
{
    const char * map ="#######|"
                      "#...~~.|"
                      "#...~~.|"
                      "#..x...|"
                      "#......|";
    MapSlabCoord slb_x, slb_y;
    reset_visibility_cache(0, 0);
    parse_map(map, &slb_x, &slb_y);
    reveal_seen_slabs(3, 3, 0, 2);
    CU_ASSERT(0);
}
