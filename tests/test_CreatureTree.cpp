#include <doctest/doctest.h>

#include <iostream>
#include <vector>

#include "creature/CreatureTree.hpp"
#include "thing_data.h"

using namespace Creature;

Thing* make_creature(uint16_t index, uint32_t x, uint32_t y, uint16_t size)
{
    Thing* obj = new Thing();

    obj->index = index;
    obj->mappos.x.val = x;
    obj->mappos.y.val = y;
    obj->solid_size_xy = size;

    return obj;
}

void print_creature(Thing* obj)
{
    MESSAGE("Creature: index=" << obj->index << ", x=" << obj->mappos.x.val << ", y=" <<
        obj->mappos.x.val << ", xy size=" << obj->solid_size_xy << ".");
}

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


TEST_CASE("Test nearest search for creature")
{
    MESSAGE("Creating creatures:\n");
    Thing* creature1 = make_creature(100, 5000, 5000, 300);
    print_creature(creature1);
    Thing* creature2 = make_creature(101, 5000, 8000, 300);
    print_creature(creature2);

    bool ok = add_creature_to_tree(creature1);
    CHECK(ok);
    ok = add_creature_to_tree(creature2);
    CHECK(ok);

    // Test if the count is correct.
    using namespace Creature;
    CHECK(CreatureTreeManager::getInstance()->getCount() == 2u);
    // Test if clearTree() works.
    CreatureTreeManager::getInstance()->clearTree();
    CHECK(CreatureTreeManager::getInstance()->getCount() == 0u);

    // Add them back.
    ok = add_creature_to_tree(creature1);
    CHECK(ok);
    ok = add_creature_to_tree(creature2);
    CHECK(ok);

    Thing* creature3 = make_creature(102, 5000, 8000 + 150 + 1, 300); // not in 3000 radius.
    print_creature(creature3);
    ok = add_creature_to_tree(creature3);
    CHECK(ok);

    Thing* creature4 = make_creature(103, 2001, 5000, 300);
    print_creature(creature4);
    ok = add_creature_to_tree(creature4);
    CHECK(ok);

    Thing* creature5 = make_creature(104, 8000, 5000, 300);
    print_creature(creature5);
    ok = add_creature_to_tree(creature5);
    CHECK(ok);

    Thing* creature6 = make_creature(105, 1000, 5000, 300); // not in 3000 radius.
    print_creature(creature6);
    ok = add_creature_to_tree(creature6);
    CHECK(ok);

    Thing* creature7 = make_creature(106, 8000 + 150 + 1, 5000, 300); // not in 3000 radius.
    print_creature(creature7);
    ok = add_creature_to_tree(creature7);
    CHECK(ok);

    Thing* creature8 = make_creature(107, 3000, 3000, 300);
    print_creature(creature8);
    ok = add_creature_to_tree(creature8);
    CHECK(ok);

    Thing* creature9 = make_creature(108, 7121, 7121, 300);
    print_creature(creature9);
    ok = add_creature_to_tree(creature9);
    CHECK(ok);

    Thing* creature10 = make_creature(109, 7121 + 150 + 1, 7121 + 150 + 1, 300); // not in 3000 radius.
    print_creature(creature10);
    ok = add_creature_to_tree(creature10);
    CHECK(ok);

    // Verify that we cannot add the same thing.
    ok = add_creature_to_tree(creature10);
    CHECK(!ok);

    Point<int> pos(creature1->mappos.x.val, creature1->mappos.y.val);
    std::vector<uint16_t> results;
    MESSAGE("Test nearestSearch() with radius=" << 3000 <<  ".");
    ok = CreatureTreeManager::getInstance()->nearestSearch(pos, 3000, results);
    CHECK(ok);
    CHECK(results.size() == 6);
    MESSAGE("Result of nearest search: \n");
    for (int i = 0; i < results.size(); ++i)
    {
        MESSAGE("Creature index=" << results[i] <<  ".");
        CHECK(results[i] != creature3->index);
        CHECK(results[i] != creature6->index);
        CHECK(results[i] != creature7->index);
        CHECK(results[i] != creature10->index);
    }

    MESSAGE("Test getNearbyCreaturesInVisualRange() with radius=" << 3000 << ".");
    NearbyCreature* nearbyCreatures = NULL;
    uint32_t count = 0;
    ok = CreatureTreeManager::getInstance()->getNearbyCreaturesInVisualRange(creature1, 3000, &count, &nearbyCreatures);
    CHECK(ok);
    CHECK(count == 6);
    for (int i = 0; i < count; ++i)
    {
        MESSAGE("Creature index=" << nearbyCreatures[i].index << ", distance: " << nearbyCreatures[i].distance);
        if (nearbyCreatures[i].index == 107)
        {
            CHECK(nearbyCreatures[i].distance == 2828);
        }
    }

    delete creature1;
    delete creature2;
    delete creature3;
    delete creature4;
    delete creature5;
    delete creature6;
    delete creature7;
    delete creature8;
    delete creature9;
    delete creature10;
}