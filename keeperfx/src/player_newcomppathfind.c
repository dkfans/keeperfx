/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_newcompchecks.c
 *     New computer player generalized pathfinding routines.
 * @par Purpose:
 *     Cleanly separates computer player checks that weren't in the original
 *     game.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Mar 2009 - 14 July 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "globals.h"

#include "player_newcomp.h"

struct SlabPathNode
{
	struct SlabPoint position;
	union
	{
		int parent_index;
		int free_index;
	};
	int cost;

	union
	{
		int state; //dfs
	} algo;
};

struct SlabPathContext
{
	struct SlabPathNode* nodes;
	int node_capacity;
	int next_free_node;
	void* userdata;
	unsigned char visited[85][85];
};

static struct SlabPathContext default_slabpath_context; //shared context for singlethreaded algorithms
static struct SlabPathNode null_node;

static struct SlabPathNode* get_node(struct SlabPathContext* context, int index)
{
	if (index >= 0)
		return context->nodes + index;
	return &null_node;
}

static void free_node(struct SlabPathContext* context, int index)
{
	struct SlabPathNode* node;
	if (index < 0)
		return;

	node = context->nodes + index;
	node->free_index = context->next_free_node;
	context->next_free_node = index;
}

static int alloc_node(struct SlabPathContext* context)
{
	int i;
	int selected_index;
	struct SlabPathNode* selected_node;
	int original_capacity = context->node_capacity;

	if (NULL == context->nodes)
	{
		context->node_capacity = 10;
		context->next_free_node = -1;
	}
	else if (context->next_free_node < 0)
	{
		context->node_capacity *= 2;
	}

	if (context->node_capacity != original_capacity)
	{
		size_t size = context->node_capacity * sizeof(*context->nodes);
		SYNCDBG(9, "Allocating %d bytes", size);
		context->nodes = (struct SlabPathNode*)realloc(context->nodes, size);
		if (NULL == context->nodes)
		{
			context->node_capacity = 0;
			//TODO: handle memory leak on failure, not that it matters since it only would happen during bugged circumstances
			ERRORLOG("Failure to allocate %d bytes", size);
			return -1;
		}

		context->nodes[original_capacity].free_index = context->next_free_node;
		for (i = original_capacity + 1; i < context->node_capacity; ++i)
		{
			context->nodes[i].free_index = i - 1;
		}
		context->next_free_node = context->node_capacity - 1;
	}

	selected_index = context->next_free_node;
	selected_node = context->nodes + selected_index;
	context->next_free_node = selected_node->free_index;

	return selected_index;
}

/************************************************************************/
/* Depth first search.                                                  */
/************************************************************************/
static int dfs_node(struct SlabPathContext* context, struct SlabPoint* pos, int parent, int cost)
{
	int index = alloc_node(context);
	struct SlabPathNode* node = get_node(context, index);
	if (NULL == node)
		return -1;

	node->parent_index = parent;
	node->cost = cost;
	node->position.x = pos->x;
	node->position.y = pos->y;
	node->algo.state = 0;

	return index;
}

static int dfs_child(struct SlabPathContext* context, MapSlabCoord x, MapSlabCoord y, int top, SlabPathNodeFunc cost)
{
	int g;
	struct SlabPoint pos;
	struct SlabPathNode* top_node;

	if (context->visited[y][x])
		return top;
	context->visited[y][x] = 1;

	top_node = get_node(context, top);
	pos.x = x;
	pos.y = y;
	g = cost(&top_node->position, &pos, context->userdata);
	if (g == SLABPATH_NONODE)
		return top;
	if (g == SLABPATH_GOAL)
		return -1;
	g += top_node->cost;

	return dfs_node(context, &pos, top, g);
}

int slabpath_dfs(struct SlabPoint* start, struct SlabPathEntry* path, int path_capacity, SlabPathNodeFunc cost, void* userdata, struct SlabPathContext* context)
{
	int out_index;
	int top, parent;
	struct SlabPathNode* node;

	if (NULL == context)
		context = &default_slabpath_context;
	context->userdata = userdata;

	memset(context->visited, 0, sizeof(context->visited));
	top = dfs_node(context, start, -1, 0);
	context->visited[start->y][start->x] = 1;

	if (top >= 0)
	{
		for (;;)
		{
			int oldtop = top;
			node = get_node(context, top);

			switch (node->algo.state)
			{
			case 0:
				node->algo.state = 1;
				top = dfs_child(context, node->position.x - 1, node->position.y, top, cost);
				break;
			case 1:
				node->algo.state = 2;
				top = dfs_child(context, node->position.x + 1, node->position.y, top, cost);
				break;
			case 2:
				node->algo.state = 3;
				top = dfs_child(context, node->position.x, node->position.y - 1, top, cost);
				break;
			case 3:
				node->algo.state = 4;
				top = dfs_child(context, node->position.x, node->position.y + 1, top, cost);
				break;
			default:
				parent = node->parent_index;
				free_node(context, top);
				top = parent;
				if (top < 0) return 0; //stack exhausted without any goal found
				break;
			}

			if (top >= 0) continue; //fast execution path
			
			//else top < 0: goal was found

			if (NULL == context->nodes)
				return false;

			out_index = 0;
			top = oldtop;
			node = get_node(context, top); //need to do again as nodes might have been reallocated

			if (node && out_index < path_capacity)
			{
				//reconstruct goal position
				struct SlabPoint* goal = &path[out_index].position;
				goal->x = node->position.x;
				goal->y = node->position.y;

				switch (node->algo.state)
				{
				case 1:
					goal->x -= 1;
					break;
				case 2:
					goal->x += 1;
					break;
				case 3:
					goal->y -= 1;
					break;
				default:
					goal->y += 1;
					break;
				}

				path[out_index].cost = node->cost; //i.e. not 100% accurate
			}
			out_index += 1;

			//free stack and unwind path
			while (top >= 0)
			{
				node = get_node(context, top);
				if (out_index < path_capacity)
				{
					path[out_index].cost = node->cost;
					path[out_index].position.x = node->position.x;
					path[out_index].position.y = node->position.y;
				}
				out_index += 1;

				oldtop = node->parent_index;
				free_node(context, top);
				top = oldtop;
			}

			return out_index;
		}
	}

	return 0;
}

/************************************************************************/
/* Uniform cost search.                                                 */
/************************************************************************/
int slabpath_ucs(struct SlabPoint* start, struct SlabPathEntry* path, int path_capacity, SlabPathNodeFunc isnode, void* userdata, struct SlabPathContext* context)
{
	if (NULL == context)
		context = &default_slabpath_context;
	context->userdata = userdata;

	ERRORLOG("Unimplemented");

	return false;
}

/************************************************************************/
/* Breadth first search.                                                */
/************************************************************************/
int slabpath_bfs(struct SlabPoint* start, struct SlabPathEntry* path, int path_capacity, SlabPathNodeFunc cost, void* userdata, struct SlabPathContext* context)
{
	if (NULL == context)
		context = &default_slabpath_context;
	context->userdata = userdata;

	ERRORLOG("Unimplemented");

	return false;
}

//TODO: will need different context because of added data structure requirements
/*
TbBool slabpath_astar(struct SlabPoint* start, struct SlabPoint* goal, int* goal_cost, SlabPathNodeFunc cost, SlabPathNodeFunc heuristic, void* userdata, struct SlabPathContext* context)
{

}
*/
