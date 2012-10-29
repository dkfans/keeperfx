/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file actionpt.c
 *     Action points support functions.
 * @par Purpose:
 *     Functions to maintain list of action points on map.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     21 May 2010 - 07 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "actionpt.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT struct ActionPoint *_DK_allocate_free_action_point_structure_with_number(long apt_num);
DLLIMPORT unsigned long _DK_action_point_get_players_within(long apt_idx);
DLLIMPORT long _DK_process_action_points(void);

/******************************************************************************/
struct ActionPoint *allocate_free_action_point_structure_with_number(long apt_num)
{
  return _DK_allocate_free_action_point_structure_with_number(apt_num);
}

struct ActionPoint *actnpoint_create_actnpoint(struct InitActionPoint *iapt)
{
  struct ActionPoint *apt;
  apt = allocate_free_action_point_structure_with_number(iapt->num);
  if (action_point_is_invalid(apt))
    return &game.action_points[0];
  apt->mappos.x.val = iapt->mappos.x.val;
  apt->mappos.y.val = iapt->mappos.y.val;
  apt->range = iapt->range;
  return apt;
}

struct ActionPoint *action_point_get(long apt_idx)
{
  if ((apt_idx < 1) || (apt_idx > ACTN_POINTS_COUNT))
    return &game.action_points[0];
  return &game.action_points[apt_idx];
}

struct ActionPoint *action_point_get_by_number(long apt_num)
{
  struct ActionPoint *apt;
  long apt_idx;
  for (apt_idx=0; apt_idx < ACTN_POINTS_COUNT; apt_idx++)
  {
    apt = &game.action_points[apt_idx];
    if (apt->num == apt_num)
      return apt;
  }
  return &game.action_points[0];
}

long action_point_number_to_index(long apt_num)
{
  struct ActionPoint *apt;
  long apt_idx;
  for (apt_idx=0; apt_idx < ACTN_POINTS_COUNT; apt_idx++)
  {
    apt = &game.action_points[apt_idx];
    if (apt->num == apt_num)
      return apt_idx;
  }
  return -1;
}

TbBool action_point_is_invalid(const struct ActionPoint *apt)
{
  return (apt == &game.action_points[0]) || (apt == NULL);
}

TbBool action_point_exists(const struct ActionPoint *apt)
{
  if (action_point_is_invalid(apt))
    return false;
  return ((apt->flags & 0x01) != 0);
}

TbBool action_point_exists_idx(long apt_idx)
{
  struct ActionPoint *apt;
  apt = action_point_get(apt_idx);
  if (action_point_is_invalid(apt))
    return false;
  return ((apt->flags & 0x01) != 0);
}

TbBool action_point_exists_number(long apt_num)
{
  struct ActionPoint *apt;
  apt = action_point_get_by_number(apt_num);
  if (action_point_is_invalid(apt))
    return false;
  return ((apt->flags & 0x01) != 0);
}

TbBool action_point_reset_idx(long apt_idx)
{
  struct ActionPoint *apt;
  apt = action_point_get(apt_idx);
  if (action_point_is_invalid(apt))
    return false;
  apt->activated = 0;
  return ((apt->flags & 0x01) != 0);
}

/**
 * Returns an action point activation bitmask.
 * Bits which are set in the bitmask corresponds to players which have triggered action point.
 */
unsigned long get_action_point_activated_by_players_mask(long apt_idx)
{
  struct ActionPoint *apt;
  apt = action_point_get(apt_idx);
  return apt->activated;
}

PlayerFlags action_point_get_players_within(long apt_idx)
{
  return _DK_action_point_get_players_within(apt_idx);
}

TbBool process_action_points(void)
{
  SYNCDBG(6,"Starting");
  struct ActionPoint *apt;
  long i;
  for (i=1; i < ACTN_POINTS_COUNT; i++)
  {
    apt = &game.action_points[i];
    if (apt->flags & 0x01)
    {
      if (((apt->num + game.play_gameturn) & 0x1F) == 0)
      {
        apt->activated = action_point_get_players_within(i);
//if (i==1) show_onscreen_msg(2*game.num_fps, "APT PLYRS %d", (int)apt->activated);
      }
    }
  }
  return true;
}

void clear_action_points(void)
{
    long i;
    for (i=0; i < ACTN_POINTS_COUNT; i++)
    {
        LbMemorySet(&game.action_points[i], 0, sizeof(struct ActionPoint));
    }
}

void delete_action_point_structure(struct ActionPoint *apt)
{
    if (apt->flags & 0x01)
    {
        LbMemorySet(apt, 0, sizeof(struct ActionPoint));
    }
}

void delete_all_action_point_structures(void)
{
  struct ActionPoint *apt;
  long i;
  for (i=1; i < ACTN_POINTS_COUNT; i++)
  {
    apt = &game.action_points[i];
    if (apt != NULL)
    {
      delete_action_point_structure(apt);
    }
  }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
