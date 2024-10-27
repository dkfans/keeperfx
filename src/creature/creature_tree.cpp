#include "creature_tree.h"
#include "CreatureTree.hpp"
#include "config_creature.h"
#include "map_data.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace Creature;

bool add_creature_to_tree
    (
    Thing* creature
    )
{
    CreatureNode node;
    node.bbox.min[0] = creature->mappos.x.val - ( creature->solid_size_xy / 2 );
    node.bbox.min[1] = creature->mappos.y.val - ( creature->solid_size_xy / 2 );
    node.bbox.max[0] = creature->mappos.x.val + ( creature->solid_size_xy / 2 );
    node.bbox.max[1] = creature->mappos.y.val + ( creature->solid_size_xy / 2 );
    node.index = creature->index;
    return CreatureTreeManager::getInstance()->addCreature(node);
}

bool clear_creature_tree()
    {
    return CreatureTreeManager::getInstance()->clearTree();
    }

bool get_nearby_creatures_in_visual_range
    (
    Thing* creature,
    uint32_t* result_count,
    NearbyCreature** creature_indices
    )
{
    const CreatureStats* stats = creature_stats_get_from_thing(creature);
    bool ok = CreatureTreeManager::getInstance()->getNearbyCreaturesInVisualRange
            (creature, subtile_coord(stats->visual_range, 0), result_count, creature_indices);
    return ok;
}

bool nearest_creature_search
    (
    Coord3d* position,
    uint32_t radius,
    uint32_t* result_count,
    ThingIndex** creature_indices
    )
{
    if (!position || !result_count || !creature_indices)
    {
        return false;
    }

    Point<int> pos(*position);
    std::vector<ThingIndex> results;
    bool ok = CreatureTreeManager::getInstance()->nearestSearch(
        pos,
        radius,
        results);

    if (!ok)
    {
        return false;
    }
    if (results.size() > 0)
    {
        ThingIndex* resultPtr = (ThingIndex*)malloc(results.size() * sizeof(ThingIndex));
        memcpy(resultPtr, results.data(), results.size() * sizeof(ThingIndex));
        *creature_indices = resultPtr;
    }
    *result_count = results.size();
    return ok;
}

#ifdef __cplusplus
}
#endif