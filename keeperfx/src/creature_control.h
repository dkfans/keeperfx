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
#include "ariadne.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SIZEXY            768
/** Max amount of spells casted at the creature at once. */
#define CREATURE_MAX_SPELLS_CASTED_AT 5
/** Max amount of creatures supported on any map. */
#define CREATURES_COUNT       256
/** Percentage of creature pay increase for every experience level. */
#define CREATURE_PAY_INCREASE_ON_EXP       35
/** Percentage of creature damage increase for every experience level. */
#define CREATURE_DAMAGE_INCREASE_ON_EXP    20
/** Percentage of spell range/area of effect increase for every experience level. */
#define CREATURE_RANGE_INCREASE_ON_EXP     6
/** Percentage of creature job value increase for every experience level. */
#define CREATURE_JOB_VALUE_INCREASE_ON_EXP 35
/** Percentage of creature health increase for every experience level. */
#define CREATURE_HEALTH_INCREASE_ON_EXP    35
/** Percentage of creature strength increase for every experience level. */
#define CREATURE_STRENGTH_INCREASE_ON_EXP  35
/** Percentage of creature dexterity increase for every experience level. */
#define CREATURE_DEXTERITY_INCREASE_ON_EXP 15
/** Percentage of creature defense increase for every experience level. */
#define CREATURE_DEFENSE_INCREASE_ON_EXP   20
/** Percentage of creature parameter increase for every experience level.
 *  Used for all parameters that have no separate definition. */
#define CREATURE_PROPERTY_INCREASE_ON_EXP  35
/** Number of possible melee combat opponents. */
#define COMBAT_MELEE_OPPONENTS_LIMIT       4
/** Number of possible range combat opponents. */
#define COMBAT_RANGED_OPPONENTS_LIMIT      4
/** Amount of instances. */
#define CREATURE_INSTANCES_COUNT          48

#define INVALID_CRTR_CONTROL (game.persons.cctrl_lookup[0])
/******************************************************************************/
#pragma pack(1)

struct Thing;
struct PlayerInfo;

enum CreatureSoundTypes {
    CrSnd_SlappedOuch  = 1,
    CrSnd_PrisonMoan   = 4,
    CrSnd_HandPick     = 5,
};

enum CreatureControlFlags {
    CCFlg_Exists        = 0x01,
    CCFlg_NoCompControl = 0x02,
    CCFlg_Immortal      = 0x04,
    CCFlg_Unknown08     = 0x08,
    CCFlg_Unknown10     = 0x10,
    CCFlg_IsInRoomList  = 0x20,
    CCFlg_Unknown40     = 0x40,
    CCFlg_Unknown80     = 0x80,
};

enum CreatureControlSpells {
    CCSpl_Unknown01     = 0x01,
    CCSpl_Freeze        = 0x02,
    CCSpl_Teleport      = 0x04,
    CCSpl_Unknown08     = 0x08,
    CCSpl_Unknown10     = 0x10,
    CCSpl_Unknown20     = 0x20,
    CCSpl_Unknown40     = 0x40,
    CCSpl_Unknown80     = 0x80,
};

enum CreatureCombatFlags {
    CmbtF_Melee         = 0x01,
    CmbtF_Ranged        = 0x02,
    CmbtF_Waiting       = 0x04,
    CmbtF_Unknown08     = 0x08,
    CmbtF_Unknown10     = 0x10,
    CmbtF_Unknown20     = 0x20,
    CmbtF_Unknown40     = 0x40,
    CmbtF_Unknown80     = 0x80,
};

enum CreatureCombatStates {
    CmbtSt_Waiting      = 1,
    CmbtSt_Ranged       = 2,
    CmbtSt_Melee        = 3,
};

struct CastedSpellData {
    unsigned char spkind;
    short field_1;
    unsigned char field_3;
};

#define SIZEOF_CreatureControl 776

struct CreatureControl {
    unsigned char index;
    unsigned char flgfield_1;
    unsigned char field_2;
    unsigned char combat_flags;
    unsigned char field_4;
    long field_5;
    short field_9;
    short opponents_melee[COMBAT_MELEE_OPPONENTS_LIMIT];
    short opponents_ranged[COMBAT_RANGED_OPPONENTS_LIMIT];
    unsigned char opponents_melee_count;
    unsigned char opponents_ranged_count;
    unsigned short field_1D;
    unsigned short players_next_creature_idx;
unsigned short field_21;
    unsigned char explevel;
    long exp_points;
    long prev_exp_points;
unsigned char field_2C;
    struct Coord3d moveto_pos;
unsigned char field_33[6];
    long hunger_level;
    long field_3D;
unsigned char field_41[9];
    long field_4A;
unsigned char field_4E[8];
   /** The game enumerates the elements of annoyance array periodically and looks for the highest value.
    * When the highest value is above CreatureStats->annoy_level, the creature becomes angry/enraged,
    * depending on how high the highest value is.
    */
    long annoyance_level[4];
    unsigned char field_66;
unsigned char field_67;
    /** Lair room index, that is the room which holds creature's lair object. */
    unsigned short lair_room_id;
    /** Lair object thing index. */
    unsigned short lairtng_idx;
    short field_6C;
    short field_6E;
    unsigned short field_70;
    unsigned short pickup_object_id;
    unsigned short pickup_creature_id;
    unsigned short next_in_group;
    unsigned short prev_in_group;
    unsigned short group_leader;// offset 7A
    short last_work_room_id;
    /** Work room index, used when creature is working in a room. */
    short work_room_id;
    /** Target room index, used when creature is moving to a room or is attacking a room. */
    short target_room_id;
    long field_82;
unsigned char field_86[2];
unsigned char field_88;
// Hard to tell where exactly, but somewhere here a kind-specific, job-specific or owner-specific data starts
  union {
  struct {
    char sbyte_89x;
    unsigned char byte_8Ax;
    unsigned char byte_8Bx;
    unsigned char byte_8Cx;
    short word_8Dx;
    unsigned char byte_8F;
    unsigned short word_90[5];
    } party;
  struct {
    long stack_update_turn;
    short word_8Dx;
    unsigned char byte_8F;
    unsigned char byte_90;
    unsigned char byte_91;
    unsigned char byte_92;
    unsigned char byte_93;
    unsigned char last_did_job;
    unsigned char field_95;
    unsigned char field_96[4];
  } digger;
  struct {
    char sbyte_89;
    unsigned char byte_8A;
    unsigned char byte_8B;
    unsigned char byte_8C;
    short word_8D;
    unsigned short word_8F;
    short word_91;
    short word_93;
    unsigned char field_95;
    unsigned char field_96[4];
    };
  struct {
    long long_89;
    long long_8D;
    long long_91;
    unsigned char field_95x;
    unsigned char field_96x[4];
  };
  };

  union {
  struct {
        long start_gameturn;
        long long_9Ex;
        long long_A2x;
  } tortured;
  struct {
        long start_gameturn;
        long long_9Ex;
        long long_A2x;
  } idle;
  struct {
    unsigned char byte_9A;
    unsigned char byte_9B;
    unsigned char byte_9C;
    unsigned char byte_9D;
    unsigned char byte_9E;
    unsigned char byte_9F;
    unsigned char byte_A0;
    unsigned char byte_A1;
    unsigned char byte_A2;
    unsigned char byte_A3;
    unsigned char byte_A4;
    unsigned char byte_A5;
    };
  struct {
    short word_9A;
    short word_9C;
    short word_9E;
    short word_A0;
    short battle_enemy_idx;
    short word_A4;
    };
  struct {
    short word_9A_cp2;
    long long_9C;
    long long_A0;
    short word_A4_cp2;
    };
  struct {
    long long_9A;
    long long_9E;
    long long_A2;
    };
  struct {
    unsigned char byte_9A_cp2;
    long long_9B;
    short word_9F_cp2;
    long long_A1;
    unsigned char byte_A5_cp2;
    };
  struct {
    unsigned char byte_9A_cp3;
    short word_9B;
    short word_9D;
    short word_9F;
    short word_A1;
    short word_A3;
    unsigned char byte_A5_cp3;
    };
  struct {
    unsigned char mode;// offset 9A
    unsigned char train_timeout;
    unsigned char pole_stl_x;
    unsigned char pole_stl_y;
    unsigned char search_timeout;
    short partner_idx;
    long partner_field9;
    unsigned char byte_A5x;
    } training;
  };
  union {
  struct {
      unsigned char combat_state_id;
      unsigned char byte_A7;
    };
  struct {
      short word_A6;
    };
  };
    unsigned char field_A8;
    unsigned char fight_til_death;
    unsigned char field_AA;
    unsigned char affected_by_spells;
    unsigned short spell_flags; // Sometimes treated as two bytes, but it's a short (AC + AD)
unsigned char field_AE;
    short field_AF;
    unsigned char field_B1;
    long field_B2;
    unsigned char field_B6;
    unsigned char teleport_x;
    unsigned char teleport_y;
unsigned short field_B9;
    struct CoordDelta3d pos_BB;
    unsigned char bloody_footsteps_turns;
    short field_C2;
    short max_speed;
    short max_health;
    short move_speed;
    short field_CA;
    short field_CC;
    unsigned long field_CE;
    unsigned char instance_id;
unsigned char field_D3;
    unsigned short field_D4;
    unsigned short field_D6;
    unsigned short field_D8;
    unsigned short field_DA;
    unsigned char target_x;
    unsigned char target_y;
long field_DE[48];
    char instances[CREATURE_INSTANCES_COUNT];
    unsigned short field_1CE;
  unsigned short field_1D0;
    char field_1D2;
unsigned char shot_model;
  struct CastedSpellData casted_spells[CREATURE_MAX_SPELLS_CASTED_AT];
    char field_1E8;
unsigned char field_1E9[46];
    struct Ariadne arid;
unsigned char field_27D;
unsigned char field_27E;
unsigned char field_27F;
    short field_280;
    short field_282;
unsigned char field_284[2];
    unsigned char field_286;
    unsigned char field_287;
    struct Coord3d flee_pos;
    long field_28E;
unsigned char field_292[12];
unsigned char field_29E[12];
    unsigned short next_in_room;
    unsigned short prev_in_room;//field_2AC
short field_2AE;
    unsigned char field_2B0;
short field_2B1;
unsigned short field_2B3[3];
unsigned short field_2B9[3];

unsigned char field_2BF[2];
unsigned short field_2C1;
unsigned short field_2C3;
unsigned short field_2C5;
    long field_2C7;
unsigned char field_2CB[32];
    long field_2EB;
    unsigned long field_2EF;
    short battle_prev_creatr;
    short battle_next_creatr;
    unsigned char battle_id;
    unsigned char field_2F8;
    unsigned char field_2F9;
    unsigned long field_2FA;
    unsigned long field_2FE;
    unsigned char field_302;
unsigned char field_303[3];
unsigned char field_306[2];
};

struct CreatureStatsOLD { // sizeof = 230
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
  unsigned char fear_wounded;
  unsigned char defense;
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
  unsigned char partner_training;
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

struct CreatureStats { // These stats are not compatible with original DK - they have more fields
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
    unsigned char fear_wounded;
    unsigned char defense;
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
    unsigned char partner_training;
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
    // New fields go there; don't change earlier fields.
    unsigned short fear_stronger;
    unsigned short fear_noflee_factor;
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

#pragma pack()
/******************************************************************************/
extern struct CreatureSounds creature_sounds[];
/******************************************************************************/
struct CreatureControl *creature_control_get(long cctrl_idx);
struct CreatureControl *creature_control_get_from_thing(const struct Thing *thing);
TbBool creature_control_invalid(const struct CreatureControl *cctrl);
TbBool creature_control_exists(const struct CreatureControl *cctrl);
TbBool creature_control_exists_in_thing(const struct Thing *thing);
void clear_creature_instance(struct Thing *thing);
long i_can_allocate_free_control_structure(void);
struct CreatureControl *allocate_free_control_structure(void);
void delete_control_structure(struct CreatureControl *cctrl);
void delete_all_control_structures(void);

struct Thing *create_and_control_creature_as_controller(struct PlayerInfo *player, long a2, struct Coord3d *pos);
TbBool disband_creatures_group(struct Thing *thing);
void play_creature_sound(struct Thing *thing, long snd_idx, long a3, long a4);
void reset_creature_eye_lens(struct Thing *thing);
TbBool creature_can_gain_experience(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
