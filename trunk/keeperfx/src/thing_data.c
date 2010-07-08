/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_data.c
 *     Thing struct support functions.
 * @par Purpose:
 *     Functions to maintain thing structure.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_data.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_delete_thing_structure(struct Thing *thing, long a2);
DLLIMPORT struct Thing *_DK_allocate_free_thing_structure(unsigned char a1);
DLLIMPORT unsigned char _DK_i_can_allocate_free_thing_structure(unsigned char a1);

/******************************************************************************/
struct Thing *allocate_free_thing_structure(unsigned char free_flags)
{
    struct Thing *thing;
    long i;
    TbBool check_again;
    //return _DK_allocate_free_thing_structure(a1);
    check_again = true;
    while (check_again)
    {
        i = game.free_things[THINGS_COUNT-1];
        if (i < THINGS_COUNT-1)
        {
          thing = thing_get(game.free_things[i]);
          LbMemorySet(thing, 0, sizeof(struct Thing));
          if (!thing_is_invalid(thing))
          {
              thing->field_0 |= 0x01;
              thing->index = game.free_things[i];
              game.free_things[THINGS_COUNT-1]++;
          }
          return thing;
        }
        check_again = false;
        if ((free_flags & 0x01) != 0)
        {
          thing = thing_get(game.thing_lists[3].index);
          if (!thing_is_invalid(thing))
          {
              delete_thing_structure(thing, 0);
              check_again = true;
          } else
          {
              ERRORLOG("Cannot even free up effect thing!");
          }
        }
    }
    ERRORLOG("Cannot allocate a structure!");
    return NULL;
}

unsigned char i_can_allocate_free_thing_structure(unsigned char a1)
{
  return _DK_i_can_allocate_free_thing_structure(a1);
}

void delete_thing_structure(struct Thing *thing, long a2)
{
  _DK_delete_thing_structure(thing, a2);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
