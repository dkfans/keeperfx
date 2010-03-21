/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_objects.h
 *     Header file for thing_objects.c.
 * @par Purpose:
 *     Things of class 'object' handling functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Nov 2009 - 01 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THINGOBJCT_H
#define DK_THINGOBJCT_H

#include "globals.h"

#include "thing_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Objects {
    unsigned char field_0;
    unsigned char field_1;
    unsigned char field_2;
    unsigned char field_3;
    unsigned char field_4;
    short field_5;
    short field_7;
    short field_9;
    short field_B;
    short field_D;
    unsigned char field_F;
    unsigned char field_10;
    unsigned char field_11;
    unsigned char field_12;
    unsigned char field_13;
    unsigned char field_14;
    unsigned char field_15;
};

#ifdef __cplusplus
#pragma pack()
#endif

#define OBJECT_TYPES_COUNT  135
/******************************************************************************/
extern Thing_Class_Func object_state_functions[];
extern Thing_Class_Func object_update_functions[];
/******************************************************************************/
DLLIMPORT extern struct Objects _DK_objects[OBJECT_TYPES_COUNT];
#define objects_data _DK_objects
DLLIMPORT extern unsigned char _DK_object_to_special[OBJECT_TYPES_COUNT];
#define object_to_special _DK_object_to_special
DLLIMPORT extern unsigned char _DK_object_to_magic[OBJECT_TYPES_COUNT];
#define object_to_magic _DK_object_to_magic
DLLIMPORT extern unsigned char _DK_workshop_object_class[OBJECT_TYPES_COUNT];
#define workshop_object_class _DK_workshop_object_class
DLLIMPORT extern unsigned char _DK_object_to_door_or_trap[OBJECT_TYPES_COUNT];
#define object_to_door_or_trap _DK_object_to_door_or_trap
/******************************************************************************/
struct Objects *get_objects_data_for_thing(struct Thing *thing);
struct Objects *get_objects_data(unsigned int tmodel);
unsigned int get_workshop_object_class_for_thing(struct Thing *thing);
unsigned int get_workshop_object_class(unsigned int tmodel);

int box_thing_to_special(const struct Thing *thing);
int book_thing_to_magic(const struct Thing *thing);
int box_thing_to_door_or_trap(const struct Thing *thing);

TbBool thing_is_special(const struct Thing *thing);
#define is_dungeon_special thing_is_special
TbBool thing_is_door_or_trap(const struct Thing *thing);
TbBool thing_is_dungeon_heart(const struct Thing *thing);
TbBool thing_is_mature_food(const struct Thing *thing);
TbBool object_is_mature_food(const struct Thing *thing);
TbBool object_is_gold(const struct Thing *thing);
TbBool object_is_gold_pile(const struct Thing *thing);
TbBool thing_is_gold_hoarde(struct Thing *thing);

long update_object(struct Thing *thing);

struct Thing *create_gold_pot_at(long pos_x, long pos_y, long plyr_idx);

long remove_gold_from_hoarde(struct Thing *thing, struct Room *room, long amount);
struct Thing *find_gold_hoarde_at(unsigned short stl_x, unsigned short stl_y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
