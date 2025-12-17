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
#include "creature_graphics.h"
#include "creature_groups.h"
#include "thing_creature.h"
#include "thing_stats.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CREATURE_TYPES_MAX 128
#define CREATURE_STATES_MAX 256

#define MAX_SIZEXY            768
/** Max amount of spells casted at the creature at once. */
#define CREATURE_MAX_SPELLS_CASTED_AT 5
/** Number of possible melee combat opponents. */
#define COMBAT_MELEE_OPPONENTS_LIMIT       4
/** Number of possible range combat opponents. */
#define COMBAT_RANGED_OPPONENTS_LIMIT      4
/** Amount of instances. */
/** Max amount of rooms needed for a creature to be attracted to a dungeon. */
#define ENTRANCE_ROOMS_COUNT               3
#define INSTANCE_TYPES_MAX 2000
#define LAIR_ENEMY_MAX 5

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
    CrSnd_Piss      = 12,
};

enum CreatureControlFlags {
    CCFlg_Exists        = 0x01,
    CCFlg_NoCompControl = 0x02,
    CCFlg_PreventDamage = 0x04,
    CCFlg_RepositionedInWall = 0x08,
    CCFlg_AvoidCreatureCollision = 0x10,
    CCFlg_IsInRoomList  = 0x20,
    CCFlg_MoveX         = 0x40,
    CCFlg_MoveY         = 0x80,
    CCFlg_MoveZ         = 0x100,
};

/* The creature will not move if any of these flags are set. */
enum CreatureControlSpells {
    CCSpl_ChickenRel    = 0x01, // This is something related to chicken spell, but the spell itself is CSAfF_Chicken.
    CCSpl_Freeze        = 0x02, // Related to CSAfF_Freeze.
    CCSpl_Teleport      = 0x04, // Related to CSAfF_Teleport.
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
    ObjCmbtSt_MeleeSnipe,
    ObjCmbtSt_RangedSnipe,
};

struct CastedSpellData {
    SpellKind spkind;
    GameTurnDelta duration;
    CrtrExpLevel caster_level;
    PlayerNumber caster_owner;
};

struct CreatureControl {
    CctrlIndex index;
    unsigned short creature_control_flags;
    unsigned char creature_state_flags;
    unsigned char combat_flags;
    unsigned long wait_to_turn;
    short distance_to_destination;
    ThingIndex opponents_melee[COMBAT_MELEE_OPPONENTS_LIMIT];
    ThingIndex opponents_ranged[COMBAT_RANGED_OPPONENTS_LIMIT];
    unsigned char opponents_melee_count;
    unsigned char opponents_ranged_count;
    ThingIndex players_prev_creature_idx;
    ThingIndex players_next_creature_idx;
    unsigned short slap_turns;
    CrtrExpLevel exp_level;
    long exp_points;
    long prev_exp_points;
    struct Coord3d moveto_pos;
    long hunger_level;
    long temple_cure_gameturn;
    unsigned char hunger_amount;
    unsigned char hunger_loss;
    long thought_bubble_last_turn_drawn;
    unsigned char thought_bubble_display_timer;
    TbBool force_health_flower_displayed;
    TbBool force_health_flower_hidden;
    unsigned char paydays_owed;
    char paydays_advanced;
    long annoy_untrained_turn;
    unsigned long last_roar_turn;
   /** The game enumerates the elements of annoyance array periodically and looks for the highest value.
    * When the highest value is above CreatureModelConfig->annoy_level, the creature becomes angry/livid,
    * depending on how high the highest value is.
    */
    long annoyance_level[5];
    unsigned char mood_flags;
    unsigned char footstep_variant;
    unsigned char footstep_counter;
    /** Lair room index, that is the room which holds creature's lair object. */
    unsigned short lair_room_id;
    /** Lair object thing index. */
    unsigned short lairtng_idx;
    /** Index of a thing being dragged by the creature, or index of a thing which is dragging this thing.
     *  Specific case is determined by flags. */
    short dragtng_idx;
    ThingIndex arming_thing_id;
    ThingIndex pickup_object_id;
    ThingIndex pickup_creature_id;
    unsigned short next_in_group;
    unsigned short prev_in_group;
    unsigned long group_info;// offset 7A
    short last_work_room_id;
    /** Work room index, used when creature is working in a room. */
    short work_room_id;
    /** Target room index, used when creature is moving to a room or is attacking a room. */
    short target_room_id;
    long turns_at_job;
    short blocking_door_id;
    unsigned char move_flags;

  union // Union on diggers, heroes and normal creatures
  {
      struct {
        long stack_update_turn;
        SubtlCodedCoords working_stl;
        SubtlCodedCoords task_stl;
        unsigned short task_idx;
        unsigned char consecutive_reinforcements;
        unsigned char last_did_job;
        unsigned char task_stack_pos;
        unsigned short task_repeats;
      } digger;
      struct {
        char hero_state;
        unsigned char hero_gate_creation_turn;
        TbBool hero_state_reset_flag;
        TbBool ready_for_attack_flag;
        long look_for_enemy_dungeon_turn;
        long wait_time;
      } hero;
      struct {
        char unusedparam;
        unsigned char unused;
        TbBool navigation_map_changed;
        TbBool unusedparam2;
      } regular_creature;
  };
  struct {
      unsigned char objective;
      unsigned char original_objective;
      char target_plyr_idx;
      PlayerBitFlags player_broken_into_flags;
      long tunnel_steps_counter;
      unsigned char tunnel_dig_direction;
      SubtlCodedCoords member_pos_stl[5];
  } party;
  struct {
      short countdown;
      struct Coord3d pos;
  } patrol;

  union // Jobs union
  {
      struct {
        GameTurn start_gameturn;
        GameTurn state_start_turn;
        GameTurn torturer_start_turn;
        ThingIndex assigned_torturer;
        unsigned char vis_state;
      } tortured;
      struct {
        GameTurn start_gameturn;
      } idle;
      struct {
        unsigned char job_stage;
        unsigned char effect_id;
        PlayerNumber previous_owner;
        MapSubtlCoord stl_9D_x;
        MapSubtlCoord stl_9D_y;
      } scavenge;
      struct {
        unsigned char mode;// offset 9A
        unsigned char train_timeout;
        MapSubtlCoord pole_stl_x;
        MapSubtlCoord pole_stl_y;
        unsigned char search_timeout;
        short partner_idx;
        long partner_creation;
      } training;
      struct {
        GameTurn seen_enemy_turn;
        long battle_enemy_crtn;
        ThingIndex battle_enemy_idx;
        ThingIndex seen_enemy_idx;
        unsigned char state_id;
        unsigned char attack_type;
        unsigned char seen_enemy_los;
      } combat;
      struct {
        GameTurn start_gameturn;
        GameTurn last_mood_sound_turn;
      } imprison;
      struct {
        unsigned char job_stage;
        unsigned char swing_weapon_counter;
        MapSubtlCoord stl_x;
        MapSubtlCoord stl_y;
        unsigned char work_timer;
      } workshop;
      struct {
        ThingIndex foodtng_idx;
      } eating;
      struct {
        unsigned char job_stage;
        long random_thinking_angle;
      } research;
      struct {
        short enemy_idx;
        GameTurn enemy_creation_turn;
        GameTurn turn_looked_for_enemy;
      } seek_enemy;
      struct {
        GameTurn last_mood_sound_turn;
      }mood;
      struct {
        unsigned char persuade_count;
      }persuade;
      struct {
        RoomIndex room_idx;
      }evacuate;
      struct {
        short animation_counter;
        short animation_duration;
      }sacrifice;
  };

    unsigned char fight_til_death;
    TbBool fighting_at_same_position;
    TbBool called_to_arms;
    TbBool exp_level_up;
    unsigned char stateblock_flags;
    unsigned long spell_flags;
    short force_visible;
    unsigned char frozen_on_hit;
    long last_piss_turn;
    unsigned char disease_caster_plyridx;
    MapSubtlCoord teleport_x;
    MapSubtlCoord teleport_y;
    unsigned short corpse_to_piss_on;
    struct CoordDelta3d moveaccel;
    unsigned char bloody_footsteps_turns;
    short kills_num;
    short kills_num_allied;
    short kills_num_enemy;
    short max_speed;
    HitPoints max_health;
    short move_speed;
    short orthogn_speed;
    short roll;
    unsigned long anim_time;
    CrInstance instance_id;
    TbBool inst_repeat;
    unsigned short inst_turn;
    unsigned short inst_action_turns; /* Turn when instance should be fired*/
    unsigned short inst_total_turns;
    unsigned short targtng_idx;
    MapSubtlCoord targtstl_x;
    MapSubtlCoord targtstl_y;
    unsigned long instance_use_turn[INSTANCE_TYPES_MAX];
    TbBool instance_available[INSTANCE_TYPES_MAX];
    unsigned short instance_anim_step_turns;
    SubtlCodedCoords collided_door_subtile;
    char fighting_player_idx;
    ThingModel shot_model;
    struct CastedSpellData casted_spells[CREATURE_MAX_SPELLS_CASTED_AT];
    /** Current active skill instance. */
    CrInstance active_instance_id;
    char head_bob;
    struct Navigation navi;
    /* Creature movement path data. */
    struct Ariadne arid;
    /* State backup when a creature temporarily changes its state due to being slapped. */
    unsigned char active_state_bkp;
    /* State backup when a creature temporarily changes its state due to being slapped. */
    unsigned char continue_state_bkp;
    unsigned char cowers_from_slap_turns;
    short conscious_back_turns;
    short countdown; // signed
    unsigned short damage_wall_coords;
    unsigned char joining_age;
    unsigned char blood_type;
    char creature_name[CREATURE_NAME_MAX];
    struct Coord3d flee_pos;
    long flee_start_turn;
    struct MemberPos followers_pos[GROUP_MEMBERS_COUNT];
    unsigned short next_in_room;
    unsigned short prev_in_room;
    EffectOrEffElModel spell_aura;
    GameTurnDelta spell_aura_duration;
    unsigned short job_assigned;
    unsigned short spell_thing_index_armour[3];
    unsigned short spell_thing_index_disease[3];
    short shot_shift_x;
    short shot_shift_y;
    short shot_shift_z;
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
    BattleIndex battle_id;
    MapSubtlCoord alarm_stl_x;
    MapSubtlCoord alarm_stl_y;
    unsigned long alarm_over_turn;
    unsigned long lava_escape_since;
    unsigned char stopped_for_hand_turns;
    long following_leader_since;
    unsigned char follow_leader_fails;
    GameTurn dropped_turn;
    unsigned long timebomb_countdown;
    ThingIndex timebomb_countdown_id;
    ThingIndex timebomb_target_id;
    GameTurn unsummon_turn;
    ThingIndex summoner_idx;
    SpellKind summon_spl_idx;
    ThingIndex familiar_idx[FAMILIAR_MAX];
    SpellKind active_disease_spell;
    SpellKind active_teleport_spell;
    SpellKind active_timebomb_spell;
    short vertical_speed;
    GameTurnDelta hand_blocked_turns;
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
    struct CreatureSound piss;
};

extern int creature_swap_idx[CREATURE_TYPES_MAX];

#pragma pack()
/******************************************************************************/
struct CreatureControl *creature_control_get(long cctrl_idx);
struct CreatureControl *creature_control_get_from_thing(const struct Thing *thing);
TbBool creature_control_invalid(const struct CreatureControl *cctrl);
TbBool creature_control_exists(const struct CreatureControl *cctrl);
void clear_creature_instance(struct Thing *thing);
long i_can_allocate_free_control_structure(void);
struct CreatureControl *allocate_free_control_structure(void);
void delete_control_structure(struct CreatureControl *cctrl);
void delete_all_control_structures(void);

struct Thing *create_and_control_creature_as_controller(struct PlayerInfo *player, ThingModel crmodel, struct Coord3d *pos);

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
