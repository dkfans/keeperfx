/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_control.h
 *     Header file for creature_control.c.
 * @par Purpose:
 *     CreatureControl structure support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     23 Apr 2009 - 16 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRCTRL_H
#define DK_CRTRCTRL_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CREATURES_COUNT       256

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Creatures { // sizeof = 16
  unsigned short numfield_0;
  unsigned short numfield_2;
  unsigned char field_4[2];
  unsigned char field_6;
  unsigned char field_7;
  unsigned char field_8;
  unsigned char field_9[3];
  unsigned char field_C[4];
};

struct CreatureControl {
    unsigned short field_0;
    unsigned char field_2;
    unsigned char field_3;
    unsigned char field_4;
    long field_5;
char field_9[22];
    unsigned short thing_idx;
unsigned char field_21[2];
    unsigned char explevel;
unsigned char field_24[74];
    short field_6E;
unsigned char field_70[10];
    unsigned short field_7A;
unsigned char field_7C[15];
    unsigned char field_8B;
unsigned char field_8C[31];
    unsigned char field_AB;
unsigned char field_AC;
    unsigned char field_AD;
unsigned char field_AE[3];
    unsigned char field_B1;
unsigned char field_B2[7];
unsigned short field_B9;
    struct CoordDelta3d pos_BB;
    unsigned char bloody_footsteps_turns;
    short field_C2;
    short field_C4;
    short max_health;
    short field_C8;
    short field_CA;
unsigned char field_CC[2];
    unsigned long field_CE;
unsigned char field_D2;
unsigned char field_D3[7];
    unsigned short field_DA;
unsigned char field_DC[194];
    char instances[74];
    char field_1E8;
unsigned char field_1E9[153];
    short field_282;
unsigned char field_284[2];
    unsigned char field_286;
    unsigned char field_287;
unsigned char field_288[22];
unsigned char field_29E[18];
    unsigned char field_2B0;
unsigned char field_2B1[62];
    unsigned long field_2EF;
unsigned char field_2F3[11];
    unsigned long field_2FE;
    unsigned char field_302;
unsigned char field_303[3];
unsigned char field_306[2];
};

struct CreatureStats { // sizeof = 230
  unsigned short job_primary;
  unsigned short job_secondary;
  unsigned short jobs_not_do;
  unsigned char eye_effect;
  unsigned short health;
  unsigned char heal_requirement;
  unsigned char heal_threshold;
  unsigned char strength;
  unsigned char armour;
  unsigned char dexterity;
  unsigned char fear;
  unsigned char defence;
  unsigned char luck;
  unsigned char recovery;
  unsigned short hunger_rate;
  unsigned char hunger_fill;
  unsigned short annoy_level;
  unsigned char lair_size;
  unsigned char hurt_by_lava;
  unsigned char sleep_exp_slab;
  unsigned char sleep_experience;
short exp_for_hitting;
short gold_hold;
short training_cost;
short scavenger_cost;
short scavenge_require;
  unsigned char scavenge_value;
  unsigned long to_level[10];
  unsigned char base_speed;
short grow_up;
  unsigned char grow_up_level;
  unsigned char entrance_force;
short max_angle_change;
short eye_height;
short field_57[14];
short field_73;
  unsigned short size_xy;
  unsigned short size_yz;
  unsigned short walking_anim_speed;
  unsigned char flying;
  unsigned char immune_to_gas;
  unsigned char attack_preference;
short field_of_view;
  unsigned char instance_spell[10];
  unsigned char instance_level[10];
  unsigned char research_value;
  unsigned char humanoid_creature;
  unsigned char piss_on_dead;
  unsigned char training_value;
short pay;
  unsigned char manufacture_value;
  unsigned char hearing;
  unsigned char entrance_rooms[3];
  unsigned char entrance_slabs_req[3];
  unsigned char visual_range;
  unsigned char real_training;
short torture_time;
short annoy_no_lair;
short annoy_no_hatchery;
short annoy_woken_up;
short annoy_on_dead_friend;
short annoy_sulking;
short annoy_no_salary;
short annoy_slapped;
short annoy_on_dead_enemy;
short annoy_in_temple;
short annoy_sleeping;
short annoy_got_wage;
short annoy_in_torture;
short annoy_win_battle;
short annoy_untrained_time;
short annoy_untrained;
short field_C4;
short annoy_queue;
short annoy_will_not_do_job;
short job_stress;
short annoy_job_stress;
short jobs_anger;
short annoy_others_leaving;
    unsigned char slaps_to_kill;
short lair_enemy;
short hero_vs_keeper_cost;
    unsigned char rebirth;
    unsigned char can_see_invisible;
    unsigned char can_go_locked_doors;
    unsigned char bleeds;
    unsigned char affected_by_wind;
    unsigned short thing_size_xy;
    unsigned short thing_size_yz;
short annoy_eat_food;
short annoy_in_hand;
short damage_to_boulder;
};

struct Persons {
    struct CreatureControl *cctrl_lookup[CREATURES_COUNT];
    struct CreatureControl *cctrl_end;
};

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
struct CreatureControl *creature_control_get(long cctrl_idx);
struct CreatureControl *creature_control_get_from_thing(struct Thing *thing);
TbBool creature_control_invalid(struct CreatureControl *cctrl);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
