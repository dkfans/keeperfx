#include <utility>
#include <cmath>

#include "CreatureTree.hpp"

namespace Creature
{

// Initialize static members
CreatureTreeManager* CreatureTreeManager::mInstance = nullptr;

CreatureTreeManager* CreatureTreeManager::getInstance()
{
    if (mInstance == nullptr)
    {
        mInstance = new CreatureTreeManager();
    }
    return mInstance;
}

CreatureTreeManager::~CreatureTreeManager()
{
}

CreatureTreeManager::CreatureTreeManager()
{
}

//! Add creature into the R-tree.
bool CreatureTreeManager::addCreature
    (
    const CreatureNode& node
    )
{
    if (mIndexSet.count(node.index) != 0)
    {
        return false;
    }

    mIndexSet.insert(node.index);
    mRTree.insert(node);
    return true;
}

bool CreatureTreeManager::clearTree()
{
    mIndexSet.clear();
    mRTree.clear();
    mNearbyCreatures.clear();
    return true;
}

//! Find nearby creatures in the visual range of the specified creature
bool CreatureTreeManager::getNearbyCreaturesInVisualRange
	(
	Thing* creature,
    uint32_t visulRange,
	uint32_t* resultCount,
	NearbyCreature** creature_indices
	)
{
    bool ok = true;
    if (mNearbyCreatures.count(creature->index) == 0) // not found in cache.
    {
        Point<int> pos(creature->mappos);
        std::vector<CreatureNode> nodes;
        bool ok = mRTree.nearest(pos.data, visulRange, std::back_inserter(nodes));

        if (!ok)
        {
            return false;
        }

        std::vector<NearbyCreature> nearbyCreatures;
        for (auto node : nodes)
        {
            int center[2];
            node.bbox.center(center);
            const uint64_t distance = hypot(center[0] - pos.x, center[1] - pos.y);
            nearbyCreatures.push_back({ node.index, distance });
        }
        mNearbyCreatures.insert({ creature->index, nearbyCreatures });
    }

    *creature_indices = mNearbyCreatures[creature->index].size() > 0 ?
        mNearbyCreatures[creature->index].data() : NULL;
    *resultCount = mNearbyCreatures[creature->index].size();

    return ok;
}

//! Find creatures in the specified radius.
bool CreatureTreeManager::nearestSearch
    (
    const Point<int>& position,
    uint32_t radius,
    std::vector<ThingIndex>& results
    )
{
    std::vector<CreatureNode> nodes;
    bool ok = mRTree.nearest(position.data, radius, std::back_inserter(nodes));

    if (ok)
    {
        for (auto node : nodes)
        {
            results.push_back(node.index);
        }
    }

    return ok;
}

size_t CreatureTreeManager::getCount()
{
    return mRTree.count();
}

}