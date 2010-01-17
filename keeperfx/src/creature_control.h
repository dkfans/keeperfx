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
/** Percentage of creature pay increase for every experience level. */
#define CREATURE_PAY_INCREASE_ON_EXP      35
/** Percentage of creature damage increase for every experience level. */
#define CREATURE_DAMAGE_INCREASE_ON_EXP   20
/** Percentage of creature parameter increase for every experience level.
 *  Used for all parameters that have no separate definition, ie. strength,
 *  health, dexterity...*/
#define CREATURE_PROPERTY_INCREASE_ON_EXP 35
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;
struct PlayerInfo;

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
    unsigned short flgfield_0;
    unsigned char field_2;
    unsigned char field_3;
    unsigned char field_4;
    long field_5;
char field_9[22];
    unsigned short thing_idx;
unsigned short field_21;
    unsigned char explevel;
unsigned char field_24[9];
    unsigned short field_2D;
    unsigned short field_2F;
    unsigned short field_31;
unsigned char field_33[6];
    long field_39;
unsigned char field_3D[41];
    unsigned char field_66;
unsigned char field_67[7];
    short field_6E;
unsigned char field_70[6];
    unsigned short next_in_group;
    unsigned short prev_in_group;
    unsigned short field_7A;
unsigned char field_7C[2];
    short field_7E;
    short field_80;
    long field_82;
unsigned char field_86[3];
    short field_89;
    unsigned char field_8B;
unsigned char field_8C[14];
  union {
  struct {
    short word_9A;
    short word_9C;
    };
    unsigned long long_9A;
  };
    unsigned char field_9E[13];
    unsigned char field_AB;
unsigned char field_AC;
    unsigned char field_AD;
unsigned char field_AE[3];
    unsigned char field_B1;
unsigned char field_B2[5];
    unsigned char teleport_x;
    unsigned char teleport_y;
unsigned short field_B9;
    struct CoordDelta3d pos_BB;
    unsigned char bloody_footsteps_turns;
    short field_C2;
    short max_speed;
    short max_health;
    short field_C8;
    short field_CA;
unsigned char field_CC[2];
    unsigned long field_CE;
unsigned char field_D2;
unsigned char field_D3;
    unsigned short field_D4;
    unsigned short field_D6;
    unsigned short field_D8;
    unsigned short field_DA;
    unsigned char target_x;
    unsigned char target_y;
long field_DE[48];
    char instances[50];
  unsigned short field_1D0;
    char field_1D2;
  unsigned char field_1D3[21];
    char field_1E8;
unsigned char field_1E9[148];
unsigned char field_27D;
unsigned char field_27E;
unsigned char field_27F;
    short field_280;
    short field_282;
unsigned char field_284[2];
    unsigned char field_286;
    unsigned char field_287;
unsigned char field_288[22];
unsigned char field_29E[18];
    unsigned char field_2B0;
unsigned char field_2B1[2];
unsigned short field_2B3[3];
unsigned short field_2B9[3];

unsigned char field_2BF[2];
unsigned short field_2C1;
unsigned short field_2C3;
unsigned short field_2C5;
unsigned char field_2C7[40];
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
  unsigned short job_stress;
  short annoy_job_stress;
  unsigned short jobs_anger;
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

struct CreatureSound {
    long index;
    long count;
};

struct CreatureSounds {
    struct CreatureSound snd01;
    struct CreatureSound snd02;
    struct CreatureSound snd03;
    struct CreatureSound snd04;
    struct CreatureSound snd05;
    struct CreatureSound snd06;
    struct CreatureSound snd07;
    struct CreatureSound snd08;
    struct CreatureSound snd09;
    struct CreatureSound snd10;
    struct CreatureSound snd11;
};
#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
extern struct CreatureSounds creature_sounds[];
/******************************************************************************/
struct CreatureControl *creature_control_get(long cctrl_idx);
struct CreatureControl *creature_control_get_from_thing(const struct Thing *thing);
TbBool creature_control_invalid(const struct CreatureControl *cctrl);
void clear_creature_instance(struct Thing *thing);

struct Thing *create_and_control_creature_as_controller(struct PlayerInfo *player, long a2, struct Coord3d *pos);
TbBool disband_creatures_group(struct Thing *thing);
void play_creature_sound(struct Thing *thing, long snd_idx, long a3, long a4);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
