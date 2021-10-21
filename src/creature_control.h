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

#include "ariadne.h"
#include "creature_groups.h"
#include "thing_stats.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SIZEXY            768
/** Max amount of spells casted at the creature at once. */
#define CREATURE_MAX_SPELLS_CASTED_AT 5
/** Max amount of creatures supported on any map. */
#define CREATURES_COUNT       256
/** Number of possible melee combat opponents. */
#define COMBAT_MELEE_OPPONENTS_LIMIT       4
/** Number of possible range combat opponents. */
#define COMBAT_RANGED_OPPONENTS_LIMIT      4
/** Amount of instances. */
#define CREATURE_INSTANCES_COUNT          48
/** Max amount of rooms needed for a creature to be attracted to a dungeon. */
#define ENTRANCE_ROOMS_COUNT               3

#define INVALID_CRTR_CONTROL (game.persons.cctrl_lookup[0])
/******************************************************************************/
#pragma pack(1)

struct Thing;
struct PlayerInfo;

enum CreatureSoundTypes {
    CrSnd_None      = 0,
    CrSnd_Hurt      = 1,
    CrSnd_Hit       = 2,
    CrSnd_Happy     = 3,
    CrSnd_Sad       = 4,
    CrSnd_Hang      = 5,
    CrSnd_Drop      = 6,
    CrSnd_Torture   = 7,
    CrSnd_Slap      = 8,
    CrSnd_Die       = 9,
    CrSnd_Foot      = 10,
    CrSnd_Fight     = 11,
};

enum CreatureControlFlags {
    CCFlg_Exists        = 0x01,
    CCFlg_NoCompControl = 0x02,
    CCFlg_PreventDamage = 0x04,
    CCFlg_Unknown08     = 0x08,
    CCFlg_Unknown10     = 0x10,
    CCFlg_IsInRoomList  = 0x20,
    CCFlg_Unknown40     = 0x40,
    CCFlg_Unknown80     = 0x80,
};

enum CreatureControlSpells {
    CCSpl_ChickenRel    = 0x01,// This is something related to chicken spell, but the spell itself is CSAfF_Chicken
    CCSpl_Freeze        = 0x02,
    CCSpl_Teleport      = 0x04,
    CCSpl_Unknown08     = 0x08,
    CCSpl_Unknown10     = 0x10,
    CCSpl_Unknown20     = 0x20,
    CCSpl_Unknown40     = 0x40,
    CCSpl_Unknown80     = 0x80,
};

enum CreatureControlMoodFlags {
    CCMoo_None          = 0x00,
    CCMoo_Angry         = 0x01,
    CCMoo_Livid         = 0x02,
};

enum CreatureCombatFlags {
    CmbtF_Melee         = 0x01,
    CmbtF_Ranged        = 0x02,
    CmbtF_Waiting       = 0x04,
    CmbtF_ObjctFight    = 0x08,
    CmbtF_DoorFight     = 0x10,
    CmbtF_Unknown20     = 0x20,
    CmbtF_Unknown40     = 0x40,
    CmbtF_Unknown80     = 0x80,
};

enum CreatureAngerReasons {
    AngR_None = 0,
    AngR_NotPaid,
    AngR_Hungry,
    AngR_NoLair,
    AngR_Other,
    AngR_ListEnd,
};

enum CreatureCombatStates {
    CmbtSt_Unset = 0,
    CmbtSt_Waiting,
    CmbtSt_Ranged,
    CmbtSt_Melee,
};

enum ObjectCombatStates {
    ObjCmbtSt_Unset = 0,
    ObjCmbtSt_Melee,
    ObjCmbtSt_Ranged,
};

struct CastedSpellData {
    unsigned char spkind;
    short duration;
    unsigned char field_3;
};

#define SIZEOF_CreatureControl 776

struct CreatureControl {
    unsigned char index;
    unsigned char flgfield_1;
    unsigned char flgfield_2;
    unsigned char combat_flags;
    unsigned char party_objective;
    unsigned long wait_to_turn;
    short distance_to_destination;
    short opponents_melee[COMBAT_MELEE_OPPONENTS_LIMIT];
    short opponents_ranged[COMBAT_RANGED_OPPONENTS_LIMIT];
    unsigned char opponents_melee_count;
    unsigned char opponents_ranged_count;
    unsigned short players_prev_creature_idx;
    unsigned short players_next_creature_idx;
    unsigned short slap_turns;
    unsigned char explevel;
    long exp_points;
    long prev_exp_points;
unsigned char field_2C;
    struct Coord3d moveto_pos;
unsigned char field_33[2];
    short field_35;
unsigned char field_37[2];
    long hunger_level;
    long temple_cure_gameturn;
    unsigned char hunger_amount;
    unsigned char hunger_loss;
    long field_43;
    unsigned char field_47;
    unsigned char paydays_owed;
    unsigned char prepayments_received;
    long annoy_untrained_turn;
    unsigned long last_roar_turn;
   /** The game enumerates the elements of annoyance array periodically and looks for the highest value.
    * When the highest value is above CreatureStats->annoy_level, the creature becomes angry/livid,
    * depending on how high the highest value is.
    */
    long annoyance_level[5];
    unsigned char mood_flags;
unsigned char field_67;
    /** Lair room index, that is the room which holds creature's lair object. */
    unsigned short lair_room_id;
    /** Lair object thing index. */
    unsigned short lairtng_idx;
    short field_6C;
    /** Index of a thing being dragged by the creature, or index of a thing which is dragging this thing.
     *  Specific case is determined by flags. */
    short dragtng_idx;
    unsigned short arming_thing_id;
    unsigned short pickup_object_id;
    unsigned short pickup_creature_id;
    unsigned short next_in_group;
    unsigned short prev_in_group;
    unsigned short group_info;// offset 7A
    short last_work_room_id;
    /** Work room index, used when creature is working in a room. */
    short work_room_id;
    /** Target room index, used when creature is moving to a room or is attacking a room. */
    short target_room_id;
    long turns_at_job;
    short blocking_door_id;
    unsigned char move_flags;
// Hard to tell where exactly, but somewhere here a kind-specific, job-specific or owner-specific data starts
  union {
  struct {
    char target_plyr_idx;
    unsigned char byte_8Ax;
    long long_8B;
    unsigned char byte_8F;
    unsigned short member_pos_stl[5];
  } party;
  struct {
    long stack_update_turn;
    short working_stl;
    unsigned short task_stl;
    unsigned short task_idx;
    unsigned char byte_93;
    unsigned char last_did_job;
    unsigned char task_stack_pos;
    unsigned short task_repeats;
    unsigned char field_98[2];
  } digger;
  struct {
    short word_89;
    short word_8B;
    short word_8D;
    unsigned char byte_8F;
    unsigned short word_90[5];
  } patrol;
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
        short assigned_torturer;
        unsigned char vis_state;
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
    unsigned char byte_9A_scv;
    unsigned char byte_9B_scv;
    unsigned char byte_9C_scv;
    unsigned char stl_9D_x;
    unsigned char stl_9D_y;
    unsigned char byte_9F_scv;
    unsigned char byte_A0_scv;
    unsigned char byte_A1_scv;
    unsigned char byte_A2_scv;
    unsigned char byte_A3_scv;
    unsigned char byte_A4_scv;
    unsigned char byte_A5_scv;
  } scavenge;
  struct {
    unsigned char mode;// offset 9A
    unsigned char train_timeout;
    unsigned char pole_stl_x;
    unsigned char pole_stl_y;
    unsigned char search_timeout;
    short partner_idx;
    long partner_creation;
    unsigned char byte_A5x;
  } training;
  struct {
    long seen_enemy_turn;
    long battle_enemy_crtn;
    short battle_enemy_idx;
    short seen_enemy_idx;
    unsigned char state_id;
    unsigned char attack_type;
    unsigned char seen_enemy_los;
  } combat;
  struct {
    unsigned long start_gameturn;
    unsigned long last_mood_sound_turn;
  } imprison;
  struct {
    short word_9A;
    short word_9C;
    short word_9E;
    long long_A0;
    short word_A4;
    short assigned_torturer;
  };
  struct {
    short word_9A_cp2;
    long long_9C;
    long long_A0_cp2;
    short word_A4_cp2;
    short word_A6_cp2;
  };
  struct {
    long long_9A;
    long long_9E;
    long long_A2;
  };
  struct {
    unsigned long last_mood_sound_turn;
    long long_9E_cp2;
    long long_A2_cp2;
    short word_A6_cp3;
  };
  struct {
    unsigned char byte_9A_cp2;
    long long_9B;
    short word_9F_cp2;
    long long_A1;
    unsigned char byte_A5_cp2;
    short word_A6_cp4;
  };
  struct {
    unsigned char byte_9A_cp3;
    short word_9B;
    short word_9D;
    short word_9F;
    short word_A1;
    short word_A3;
    unsigned char byte_A5_cp3;
    short word_A6_cp5;
  };
  };
    unsigned char fight_til_death;
    unsigned char field_AA;
    unsigned char stateblock_flags;
    unsigned short spell_flags; // Sometimes treated as two bytes, but it's a short (AC + AD)
    unsigned char field_AE;
    short force_visible;
    unsigned char field_B1;
    long field_B2;
    unsigned char disease_caster_plyridx;
    unsigned char teleport_x;
    unsigned char teleport_y;
    unsigned short field_B9;
    struct CoordDelta3d moveaccel;
    unsigned char bloody_footsteps_turns;
    short kills_num;
    short max_speed;
    short max_health;
    short move_speed;
    short orthogn_speed;
    short field_CC;
    unsigned long field_CE;
    unsigned char instance_id;
    unsigned char inst_repeat;
    unsigned short inst_turn;
    unsigned short inst_action_turns;
    unsigned short inst_total_turns;
    unsigned short targtng_idx;
    unsigned char targtstl_x;
    unsigned char targtstl_y;
    unsigned long instance_use_turn[CREATURE_INSTANCES_COUNT];
    char instance_available[CREATURE_INSTANCES_COUNT];
    unsigned short instance_anim_step_turns;
    unsigned short collided_door_subtile;
    char fighting_player_idx;
    unsigned char shot_model;
    struct CastedSpellData casted_spells[CREATURE_MAX_SPELLS_CASTED_AT];
    /** Current active skill instance. */
    char active_instance_id;
    unsigned char field_1E9;
    struct Navigation navi;
unsigned char field_211[6];
    /* Creature movement path data. */
    struct Ariadne arid;
    /* State backup when a creature temporarily changes its state due to being slapped. */
    unsigned char active_state_bkp;
    /* State backup when a creature temporarily changes its state due to being slapped. */
    unsigned char continue_state_bkp;
unsigned char field_27F;
    short conscious_back_turns;
    short countdown_282; // signed
    unsigned short field_284;
    unsigned char joining_age;
    unsigned char blood_type;
    struct Coord3d flee_pos;
    long flee_start_turn;
    struct MemberPos followers_pos[GROUP_MEMBERS_COUNT];
    unsigned short next_in_room;
    unsigned short prev_in_room;//field_2AC
short field_2AE;
    unsigned char field_2B0; // 7 == heal
    unsigned short job_assigned;
    unsigned short spell_tngidx_armour[3];
    unsigned short spell_tngidx_disease[3];
unsigned char field_2BF[2];
unsigned short shot_shift_x;
unsigned short shot_shift_y;
unsigned short shot_shift_z;
    unsigned long tasks_check_turn;
    unsigned long wander_around_check_turn;
    unsigned long job_primary_check_turn;
    unsigned long job_secondary_check_turn;
    unsigned long healing_sleep_check_turn;
    unsigned long garden_eat_check_turn;
    unsigned long temple_pray_check_turn;
    unsigned long sulking_sleep_check_turn;
    unsigned long job_assigned_check_turn;
    unsigned long disease_start_turn;
    unsigned long armageddon_teleport_turn;
    short battle_prev_creatr;
    short battle_next_creatr;
    unsigned char battle_id;
    unsigned char alarm_stl_x;
    unsigned char alarm_stl_y;
    unsigned long field_2FA;
    unsigned long field_2FE;
    unsigned char field_302;
    long field_303;
    unsigned char field_307;
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
  unsigned char sleep_recovery;
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
  TbBool entrance_force;
  short max_angle_change;
  short eye_height;
short field_57[14];
short field_73;
  unsigned short size_xy;
  unsigned short size_yz;
  unsigned short walking_anim_speed;
  TbBool flying;
  TbBool immune_to_gas;
  unsigned char attack_preference;
  short field_of_view;
  unsigned char learned_instance_id[10];
  unsigned char learned_instance_level[10];
  unsigned char research_value;
  TbBool humanoid_creature;
  TbBool piss_on_dead;
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
  TbBool can_see_invisible;
  TbBool can_go_locked_doors;
  TbBool bleeds;
  TbBool affected_by_wind;
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
    unsigned char sleep_recovery;
    unsigned short hunger_rate;
    unsigned char hunger_fill;
    unsigned short annoy_level;
    unsigned char lair_size;
    unsigned char hurt_by_lava;
    unsigned char sleep_exp_slab;
    short sleep_experience;
    short exp_for_hitting;
    short gold_hold;
    short training_cost;
    short scavenger_cost;
    short scavenge_require;
    unsigned char scavenge_value;
    unsigned long to_level[CREATURE_MAX_LEVEL];
    unsigned char base_speed;
    short grow_up;
    unsigned char grow_up_level;
    TbBool entrance_force;
    short max_angle_change;
    short eye_height;
  short field_57[14];
  short field_73;
    unsigned short size_xy;
    unsigned short size_yz;
    unsigned short walking_anim_speed;
    TbBool flying;
    TbBool immune_to_gas;
    unsigned char attack_preference;
    short field_of_view;
    /** Instance identifiers of the instances creature can learn. */
    unsigned char learned_instance_id[LEARNED_INSTANCES_COUNT];
    /** Required level to use the instances creature can learn. Scaled 1..CREATURE_MAX_LEVEL. */
    unsigned char learned_instance_level[LEARNED_INSTANCES_COUNT];
    unsigned char research_value;
    TbBool humanoid_creature;
    TbBool piss_on_dead;
    unsigned char training_value;
    short pay;
    unsigned char manufacture_value;
    unsigned char hearing;
    unsigned char entrance_rooms[ENTRANCE_ROOMS_COUNT];
    unsigned char entrance_slabs_req[ENTRANCE_ROOMS_COUNT];
    unsigned char visual_range;
    unsigned char partner_training;
    /** Minimal game turns a creature must be tortured before it gets a chance to be broken */
    short torture_break_time;
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
    /* Annoyance caused by tries to assign creature to a job it won't do */
    short annoy_will_not_do_job;
    /* Job kinds which cause stress for the creature */
    unsigned short job_stress;
    /* Amount of annoyance given to creature under stressful job */
    short annoy_job_stress;
    /* Job kinds which the creature will start when it is angry */
    unsigned short jobs_anger;
    short annoy_others_leaving;
    unsigned char slaps_to_kill;
    short lair_enemy;
    short hero_vs_keeper_cost;
    TbBool rebirth;
    TbBool can_see_invisible;
    TbBool can_go_locked_doors;
    TbBool bleeds;
    TbBool affected_by_wind;
    unsigned short thing_size_xy;
    unsigned short thing_size_yz;
    short annoy_eat_food;
    short annoy_in_hand;
    short damage_to_boulder;
    // New fields go there; don't change earlier fields.
    unsigned short fear_stronger;
    unsigned short fearsome_factor;
    short entrance_score;
    short annoy_going_postal;
    short toking_recovery;
    TbBool illuminated;
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
    struct CreatureSound foot;
    struct CreatureSound hit;
    struct CreatureSound happy;
    struct CreatureSound sad;
    struct CreatureSound hurt;
    struct CreatureSound die;
    struct CreatureSound hang;
    struct CreatureSound drop;
    struct CreatureSound torture;
    struct CreatureSound slap;
    struct CreatureSound fight;
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
struct Thing *get_group_last_member(struct Thing *thing);

void play_creature_sound(struct Thing *thing, long snd_idx, long a3, long a4);
void stop_creature_sound(struct Thing *thing, long snd_idx);
void play_creature_sound_and_create_sound_thing(struct Thing *thing, long snd_idx, long a2);
struct CreatureSound *get_creature_sound(struct Thing *thing, long snd_idx);
void reset_creature_eye_lens(struct Thing *thing);
TbBool creature_can_gain_experience(const struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
