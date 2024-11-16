#ifndef CREATURE_TREE_API_H
#define CREATURE_TREE_API_H

#include <stdbool.h>
#include <stdint.h>

#include "globals.h"
#include "thing_creature.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NearbyCreature
{
    ThingIndex index;
    uint64_t distance;
} NearbyCreature;

bool add_creature_to_tree
    (
    Thing *creature
    );

bool clear_creature_tree();

bool get_nearby_creatures_in_visual_range
    (
    Thing* creature,
    uint32_t* result_count,
    NearbyCreature** creature_indices //!< Caller should not free this.
    );

bool nearest_creature_search
    (
    Coord3d *position,
    uint32_t radius,
    uint32_t *result_count,
    uint16_t **creature_indices //!< Caller should free this.
    );

#ifdef __cplusplus
}
#endif

#endif // CREATURE_TREE_API_H