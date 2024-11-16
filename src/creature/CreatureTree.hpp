#ifndef CREATURE_TREE_HPP
#define CREATURE_TREE_HPP

#include <THST/RTree.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "creature_tree.h"

namespace Creature
{

template <typename T> struct Point
{
	union
	{
		T data[2];
		struct
		{
			T x, y;
		};
	};

	Point(const Coord3d& coord)
		: x(static_cast<T>( coord.x.val ))
		, y(static_cast<T>( coord.y.val ))
	{
	}

	template <typename Y>
	Point(Y x, Y y)
	{
		this->x = static_cast<T>( x );
		this->y = static_cast<T>( y );
	}
};

struct CreatureNode
{
	spatial::BoundingBox<int, 2> bbox;
	ThingIndex index;

	bool operator==(const CreatureNode& other) const
	{
		return index == other.index;
	}
};

// use a custom indexable for custom objects
struct Indexable
{
	const int* min(const CreatureNode& value) const { return value.bbox.min; }
	const int* max(const CreatureNode& value) const { return value.bbox.max; }
};

class CreatureTreeManager
{
public:
    CreatureTreeManager(const CreatureTreeManager& obj) = delete;
    static CreatureTreeManager* getInstance();
	virtual ~CreatureTreeManager();

	bool addCreature
		(
		const CreatureNode& node
		);

	bool clearTree();

	bool getNearbyCreaturesInVisualRange
		(
		Thing* creature,
		uint32_t visulRange,
		uint32_t* resultCount,
		NearbyCreature** creature_indices
		);

	bool nearestSearch
		(
		const Point<int>& position,
		uint32_t radius,
		std::vector<ThingIndex>& results
		);

	size_t getCount();

private:
    CreatureTreeManager();

private:
    static CreatureTreeManager* mInstance;
	spatial::RTree<int, CreatureNode, 2, 4, 2, Indexable> mRTree;
	std::unordered_set<ThingIndex> mIndexSet;
	std::unordered_map<ThingIndex, std::vector<NearbyCreature>> mNearbyCreatures;
};

}

#endif // CREATURE_TREE_HPP