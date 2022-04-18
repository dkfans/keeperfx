/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_merge.c
 *     Module which merges all elements of the game into single Game structure.
 * @par Purpose:
 *     Allows easy saving and loading of game data.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     21 Oct 2009 - 25 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "game_merge.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct GameAdd gameadd;
struct IntralevelData intralvl;
unsigned long game_flags2 = 0;
/******************************************************************************/
/******************************************************************************/
/**
 * Informs if we're going to emulate overflow for integer values with given amount of bits.
 * @param nbits Amount of bits for which we want to know the overflow emulation state.
 * @return Overflow emulation state.
 */
TbBool emulate_integer_overflow(unsigned short nbits)
{
    if (nbits == 8)
        return (gameadd.classic_bugs_flags & ClscBug_Overflow8bitVal) != 0;
    return false;
}

/**
 * Returns the loaded level number.
 */
LevelNumber get_loaded_level_number(void)
{
  return game.loaded_level_number;
}

/**
 * Sets the loaded level number. Does not make any cleanup or loading.
 */
LevelNumber set_loaded_level_number(LevelNumber lvnum)
{
  if (lvnum > 0)
    game.loaded_level_number = lvnum;
  return game.loaded_level_number;
}

/**
 * Returns the continue level number.
 */
LevelNumber get_continue_level_number(void)
{
  return game.continue_level_number;
}

/**
 * Sets the continue level number. The level informs of campaign progress.
 * Levels which are not part of campaign will be ignored.
 */
LevelNumber set_continue_level_number(LevelNumber lvnum)
{
  if (is_singleplayer_like_level(lvnum))
    game.continue_level_number = lvnum;
  return game.continue_level_number;
}

/**
 * Returns the selected level number. Selected level is loaded when staring game.
 */
LevelNumber get_selected_level_number(void)
{
  return game.selected_level_number;
}

/**
 * Sets the selected level number. Selected level is loaded when staring game.
 */
LevelNumber set_selected_level_number(LevelNumber lvnum)
{
  if (lvnum >= 0)
    game.selected_level_number = lvnum;
  return game.selected_level_number;
}

/**
 * Returns if the given bonus level is visible in land view screen.
 */
TbBool is_bonus_level_visible(struct PlayerInfo *player, LevelNumber bn_lvnum)
{
    int i = storage_index_for_bonus_level(bn_lvnum);
    if (i < 0)
    {
        // This hapens quite often - status of bonus level is checked even
        // if there's no such bonus level. So no log message here.
        return false;
  }
  int n = i / 8;
  int k = (1 << (i % 8));
  if ((n < 0) || (n >= BONUS_LEVEL_STORAGE_COUNT))
  {
    WARNLOG("Bonus level %d has invalid store position.",(int)bn_lvnum);
    return false;
  }
  return ((intralvl.bonuses_found[n] & k) != 0);
}

/**
 * Makes the bonus level visible on the land map screen.
 */
TbBool set_bonus_level_visibility(LevelNumber bn_lvnum, TbBool visible)
{
    int i = storage_index_for_bonus_level(bn_lvnum);
    if (i < 0)
    {
        WARNLOG("Can't set state of non-existing bonus level %d.", (int)bn_lvnum);
        return false;
  }
  int n = i / 8;
  int k = (1 << (i % 8));
  if ((n < 0) || (n >= BONUS_LEVEL_STORAGE_COUNT))
  {
    WARNLOG("Bonus level %d has invalid store position.",(int)bn_lvnum);
    return false;
  }
  set_flag_byte(&intralvl.bonuses_found[n], k, visible);
  return true;
}

/**
 * Makes a bonus level for specified SP level visible on the land map screen.
 */
TbBool set_bonus_level_visibility_for_singleplayer_level(struct PlayerInfo *player, unsigned long sp_lvnum, short visible)
{
    long bn_lvnum = bonus_level_for_singleplayer_level(sp_lvnum);
    if (!set_bonus_level_visibility(bn_lvnum, visible))
    {
        if (visible)
            WARNMSG("Couldn't store bonus award for level %d", sp_lvnum);
        return false;
  }
  if (visible)
    SYNCMSG("Bonus award for level %d enabled",sp_lvnum);
  return true;
}

void hide_all_bonus_levels(struct PlayerInfo *player)
{
    for (int i = 0; i < BONUS_LEVEL_STORAGE_COUNT; i++)
        intralvl.bonuses_found[i] = 0;
}

/**
 * Returns if the given extra level is visible in land view screen.
 */
unsigned short get_extra_level_kind_visibility(unsigned short elv_kind)
{
    LevelNumber ex_lvnum = get_extra_level(elv_kind);
    if (ex_lvnum <= 0)
        return LvSt_Hidden;
    switch (elv_kind)
    {
    case ExLv_FullMoon:
        if (is_full_moon)
            return LvSt_Visible;
        if (is_near_full_moon)
            return LvSt_HalfShow;
        break;
    case ExLv_NewMoon:
        if (is_new_moon)
            return LvSt_Visible;
        if (is_near_new_moon)
            return LvSt_HalfShow;
        break;
  }
  return LvSt_Hidden;
}

/**
 * Returns if the given extra level is visible in land view screen.
 */
short is_extra_level_visible(struct PlayerInfo *player, long ex_lvnum)
{
    int i = array_index_for_extra_level(ex_lvnum);
    switch (i + 1)
    {
    case ExLv_FullMoon:
        return is_full_moon;
    case ExLv_NewMoon:
        return is_new_moon;
  }
  return false;
}

void update_extra_levels_visibility(void)
{
}

struct ThingAdd *get_thingadd(Thingid thing_idx)
{
    return &gameadd.things[thing_idx];
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
