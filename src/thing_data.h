/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_data.h
 *     Header file for thing_data.c.
 * @par Purpose:
 *     Thing struct support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THING_DATA_H
#define DK_THING_DATA_H

#include "globals.h"
#include "bflib_basics.h"

/** Max amount of creatures supported on any map. */
#define CREATURES_COUNT       1024


#ifdef __cplusplus
extern "C" {
#endif

TbBool is_non_synchronized_thing_class(unsigned char class_id);

typedef unsigned short Thingid;

/******************************************************************************/
/** Enums for thing->alloc_flags bit fields. */
enum ThingAllocFlags {
    TAlF_Exists            = 0x01,
    TAlF_IsInMapWho        = 0x02,
    TAlF_IsInStrucList     = 0x04,
    TAlF_InDungeonList     = 0x08,
    TAlF_IsInLimbo         = 0x10,
    TAlF_IsControlled      = 0x20,
    TAlF_IsFollowingLeader = 0x40,
    TAlF_IsDragged         = 0x80,
};

/** Enum for specifying thing allocation pool type. */
enum ThingAllocationPool {
    ThingAllocation_Synced = 0,    /**< Allocate from synced thing pool */
    ThingAllocation_Unsynced = 1   /**< Allocate from unsynced thing pool */
};

/** Enums for thing->state_flags bit fields. */
enum ThingFlags1 {
    TF1_IsDragged1     = 0x01,
    TF1_InCtrldLimbo   = 0x02,
    TF1_PushAdd        = 0x04,
    TF1_PushOnce       = 0x08,
    TF1_DoFootsteps    = 0x10,
};

enum ThingFlags2 {
    TF2_CreatureIsMoving              = 0x01,
    TF2_Spectator           = 0x02,
    TF2_SummonedCreature    = 0x04,
};

enum ThingRenderingFlags {
    TRF_Invisible      = 0x01, // Not Drawn
    TRF_Unshaded       = 0x02, // Not shaded

    TRF_Tint_1         = 0x04, // Tint1 (used to draw enemy creatures when they are blinking to owners color)
    TRF_Tint_2         = 0x08, // Tint2 (not used?)
    TRF_Tint_Flags     = 0x0C, // Tint flags

    TRF_Transpar_8     = 0x10, // Used on chicken effect when creature is turned to chicken
    TRF_Transpar_4     = 0x20, // Used for Invisible creatures and traps -- more transparent
    TRF_Transpar_Alpha = 0x30,
    TRF_Transpar_Flags = 0x30,

    TRF_AnimateOnce    = 0x40,
    TRF_BeingHit       = 0x80,    // Being hit (draw red sometimes)
};

 /**
  * Used for EffectElementConfigStats->size_change and Thing->size_change.
  *
  * See effect_element_stats[] for setting of size_change.
  */
enum ThingSizeChange {
  TSC_DontChangeSize         = 0x00, /**< Default behaviour. */
  TSC_ChangeSize             = 0x01, /**< Used when creature changing to/from chicken, and by TngEffElm_Cloud3. */
  TSC_ChangeSizeContinuously = 0x02, /**< Used by TngEffElm_IceShard. */
};


enum ThingMovementFlags {
    TMvF_Default            = 0x000, // Default.
    TMvF_IsOnWater          = 0x001, // The creature is walking on water.
    TMvF_IsOnLava           = 0x002, // The creature is walking on lava.
    TMvF_BeingSacrificed    = 0x004, // For creature falling in the temple pool, this informs its sacrificed state.
    TMvF_ZeroVerticalVelocity          = 0x008, // thing->veloc_base.z.val = 0;
    TMvF_GoThroughWalls     = 0x010,
    TMvF_Flying             = 0x020, // The creature is flying and can navigate in the air.
    TMvF_Immobile           = 0x040, // The creature cannot move.
    TMvF_IsOnSnow           = 0x080, // The creature leaves footprints on snow path.
    TMvF_MagicFall          = 0x100, // The creature does a free fall with magical effect, ie. it was just created with some initial velocity.
    TMvF_Grounded           = 0x200, // For creature which are normally flying, this informs that its grounded due to spells or its condition.
};

/******************************************************************************/
#pragma pack(1)

struct Room;

struct Thing {
    unsigned char alloc_flags;
    unsigned char state_flags;
    unsigned short next_on_mapblk;
    unsigned short prev_on_mapblk;
    unsigned char owner;
    unsigned char active_state;
    unsigned char continue_state;
    int32_t creation_turn;
    struct Coord3d mappos;
    union {
//TCls_Empty
//TCls_Object
      struct {
        int32_t gold_stored;
        short unusedparam;
      } valuable;
      struct {
        short life_remaining;
        char freshness_state;
        unsigned char possession_startup_timer;
        TbBool some_chicken_was_sacrificed;
        unsigned short angle;
      } food;
      struct {
        unsigned char box_kind;
      } custom_box;
      struct {
        short belongs_to;
        short cssize;
        short spr_size;
      } lair;
      struct {
        short belongs_to;
        short cssize;
        short spr_size;
      } torturer;
      struct {
        unsigned char state;
      } call_to_arms_flag;
      struct {
        unsigned char countdown_UNUSED;
        unsigned char beat_direction;
      } heart;
      struct {
        unsigned char number;
      } hero_gate;
      struct {
        KeepPwrLevel power_level;
      } lightning;
      struct {
        short belongs_to;
        unsigned char shspeed;
      } armor;
      struct {
        short belongs_to;
        unsigned char effect_slot;
      } disease;
      struct {
        int32_t room_idx;
      } roomflag;
      struct {
      short unused3;
      int32_t last_turn_drawn;
      unsigned char display_timer;
      }roomflag2; // both roomflag and roomflag2 are used in same function on same object but have 2 bytes overlapping between room_idx and last_turn_drawn
//TCls_Shot
      struct {
        unsigned char dexterity;
        short damage;
        unsigned char hit_type;
        short target_idx;
        CrtrExpLevel shot_level;
        struct Coord3d originpos;
        int num_wind_affected;
        CctrlIndex wind_affected_creature[CREATURES_COUNT];  //list of wind affected Creatures
      } shot;
      struct {
        int32_t x;
        short target_idx;
        unsigned char posint;
      } shot_lizard;
      struct {
        unsigned char range;
      } shot_lizard2;// both shot_lizard and shot_lizard2 are used in same function on same object but have 1 byte overlapping between x and range
//TCls_EffectElem
//TCls_DeadCreature
      struct {
          CrtrExpLevel exp_level;
          unsigned char laid_to_rest;
      } corpse;
//TCls_Creature
      struct {
        int32_t gold_carried;
        short health_bar_turns;
        short volley_repeat;
        TbBool volley_fire;
      } creature;
//TCls_Effect
      struct {
        int parent_class_id;
        ThingModel parent_model;
        unsigned char hit_type;
      } shot_effect;
      struct {
        int32_t number;
      } price_effect;
//TCls_EffectGen
      struct {
      short range;
      int32_t generation_delay;
      } effect_generator;
//TCls_Trap
      struct {
        unsigned char num_shots;
        unsigned char revealed;
        TbBool wait_for_rearm;
        TbBool volley_fire;
        GameTurn rearm_turn;
        GameTurn shooting_finished_turn;
        short volley_repeat;
        unsigned short volley_delay;
        unsigned short firing_at;
        unsigned char flag_number;
      } trap;
//TCls_Door
      struct {
      short orientation;
      unsigned char opening_counter;
      short closing_counter;
      unsigned char is_locked;
      PlayerBitFlags revealed;
      } door;
//TCls_unusedparam10
//TCls_unusedparam11
//TCls_AmbientSnd
//TCls_CaveIn
      struct {
        unsigned char x;
        unsigned char y;
        short time;
        ThingModel model;
      }cave_in;
    };
    ThingModel model;
    unsigned short index;
    /** Parent index. The parent may either be a thing, or a slab index.
     * What it means depends on thing class, ie. it's thing index for shots
     *  and slab number for objects.
     */
    short parent_idx;
    unsigned char class_id;
    unsigned char fall_acceleration;
    unsigned char bounce_angle;
    short inertia_floor;
    short inertia_air;
    unsigned short movement_flags;
    struct CoordDelta3d veloc_push_once;
    struct CoordDelta3d veloc_base;
    struct CoordDelta3d veloc_push_add;
    struct CoordDelta3d velocity;
    // Push when moving; needs to be signed
    short anim_speed;
    int32_t anim_time; // animation time (measured in 1/256 of a frame)
    unsigned short anim_sprite;
    unsigned short sprite_size;
    unsigned char current_frame;
    unsigned char max_frames;
    char transformation_speed;
    unsigned short sprite_size_min;
    unsigned short sprite_size_max;
    unsigned char rendering_flags;
    unsigned char draw_class; /**< See enum ObjectsDrawClasses for valid values. */
    unsigned char size_change; /**< See enum ThingSizeChange for valid values. */
    unsigned char tint_colour;
    short move_angle_xy;
    short move_angle_z;
    unsigned short clipbox_size_xy;
    unsigned short clipbox_size_z;
    unsigned short solid_size_xy;
    unsigned short solid_size_z;
    HitPoints health;
    unsigned short floor_height;
    unsigned short light_id;
    CctrlIndex ccontrol_idx;
    unsigned char snd_emitter_id;
    short next_of_class;
    short prev_of_class;
    uint32_t flags; //ThingAddFlags
    int32_t last_turn_drawn;
    float time_spent_displaying_hurt_colour; // Used for delta time interpolated render position
    unsigned short previous_floor_height;
    unsigned short interp_floor_height;
    struct Coord3d previous_mappos;
    struct Coord3d interp_mappos;
    int32_t interp_minimap_pos_x;
    int32_t interp_minimap_pos_y;
    int32_t previous_minimap_pos_x;
    int32_t previous_minimap_pos_y;
    uint32_t random_seed;
    int32_t interp_minimap_update_turn;
    PlayerNumber holding_player;
};

#define INVALID_THING (game.things.lookup[0])

/** Macro used for debugging problems related to things.
 * Should be executed in every function which changes a thing.
 * Can be defined to any SYNCLOG routine, making complete trace of usage on a thing.
 */
#define TRACE_THING(thing)

enum ThingAddFlags //named this way because they were part of the ThingAdd structure
{
    TAF_ROTATED_SHIFT = 16,
    TAF_ROTATED_MASK = 0x070000,
};

#pragma pack()
/******************************************************************************/
#define allocate_free_thing_structure(class_id) allocate_free_thing_structure_f(class_id, __func__)
struct Thing *allocate_free_thing_structure_f(unsigned char class_id, const char *func_name);
TbBool i_can_allocate_free_thing_structure(unsigned char class_id);
#define delete_thing_structure(thing, deleting_everything) delete_thing_structure_f(thing, deleting_everything, __func__)
void delete_thing_structure_f(struct Thing *thing, TbBool deleting_everything, const char *func_name);

#define thing_get(tng_idx) thing_get_f(tng_idx, __func__)
struct Thing *thing_get_f(ThingIndex tng_idx, const char *func_name);
TbBool thing_exists(const struct Thing *thing);
short thing_is_invalid(const struct Thing *thing);
struct Thing* get_parent_thing(const struct Thing* thing);

TbBool thing_is_in_limbo(const struct Thing* thing);
TbBool thing_is_dragged_or_pulled(const struct Thing *thing);
struct PlayerInfo *get_player_thing_is_controlled_by(const struct Thing *thing);

void set_thing_draw(struct Thing *thing, long anim, long speed, long scale, char animate_once, char start_frame, unsigned char draw_class);

void query_thing(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
