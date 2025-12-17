/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_groups.h
 *     Header file for creature_groups.c.
 * @par Purpose:
 *     Creature grouping and groups support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRGROUPS_H
#define DK_CRTRGROUPS_H

#include "globals.h"
#include "bflib_basics.h"
#include "map_locations.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GROUP_MEMBERS_COUNT 30
#define FAMILIAR_MAX 8

enum TriggerFlags {
    TrgF_CREATE_PARTY                  =  0x00,
    TrgF_CREATE_CREATURE               =  0x01,
    TrgF_CREATE_OBJECT                 =  0x02,
    TrgF_ADD_TO_PARTY                  =  0x03,
    TrgF_DELETE_FROM_PARTY             =  0x04,
    TrgF_CREATE_EFFECT_GENERATOR       =  0x05,
    TrgF_COMMAND_MASK                  =  0x0F,

    TrgF_DISABLED                      =  0x40,
    TrgF_REUSABLE                      =  0x80,
};

enum FollowBehaviour {
    FlwB_None               = 0,
    FlwB_FollowLeader       = 1,
    FlwB_MatchWorkRoom      = 2,
    FlwB_JoinCombatOrFollow = 3,
};
/******************************************************************************/
#pragma pack(1)

struct Thing;

enum MemberPosFlags
{
        MpF_OCCUPIED = 1,
        MpF_AVAIL    = 2,
};

/** Used for storing group members positions around leader.
 */
struct MemberPos {
    SubtlCodedCoords stl_num;
    unsigned char flags;
};

struct PartyMember {
  unsigned char flags;
  ThingModel crtr_kind;
  unsigned char objectv;
  long countdown;
  CrtrExpLevel exp_level;
  unsigned short carried_gold;
  unsigned short is_active;
  PlayerNumber target;
};

struct Party {
  char prtname[100];
  struct PartyMember members[GROUP_MEMBERS_COUNT];
  unsigned long members_num;
};

#pragma pack()
/******************************************************************************/
struct Thing* get_best_creature_to_lead_group(struct Thing* grptng);
long get_no_creatures_in_group(const struct Thing *grptng);
TbBool get_free_position_behind_leader(struct Thing *leadtng, struct Coord3d *pos);

TbBool add_creature_to_group(struct Thing *crthing, struct Thing *grthing);
long add_creature_to_group_as_leader(struct Thing *thing1, struct Thing *thing2);
TbBool remove_creature_from_group(struct Thing *thing);
TbBool remove_creature_from_group_without_leader_consideration(struct Thing *creatng);

TbBool creature_is_group_member(const struct Thing *thing);
TbBool creature_is_group_leader(const struct Thing *thing);
struct Thing *get_group_leader(const struct Thing *thing);
struct Thing *get_first_follower_creature_in_group(const struct Thing *grptng);
struct Thing *get_last_follower_creature_in_group(const struct Thing *grptng);
TbBool make_group_member_leader(struct Thing *leadtng);

TbBool create_party(const char *prtname);
int get_party_index_of_name(const char *prtname);
TbBool add_member_to_party(int party_id, long crtr_model, CrtrExpLevel exp_level, long carried_gold, long objctv_id, long countdown, PlayerNumber target);
TbBool delete_member_from_party(int party_id, long crtr_model, CrtrExpLevel exp_level);
long process_obey_leader(struct Thing *thing);
void leader_find_positions_for_followers(struct Thing *leadtng);

struct Thing *script_process_new_party(struct Party *party, PlayerNumber plyr_idx, TbMapLocation location, long copies_num);
struct Thing *script_process_new_tunneller_party(PlayerNumber plyr_idx, long prty_id, TbMapLocation location, TbMapLocation heading, CrtrExpLevel exp_level, unsigned long carried_gold);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
