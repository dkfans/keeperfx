/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_creature.h
 *     Header file for thing_creature.c.
 * @par Purpose:
 *     Creatures related functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Mar 2009 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THINGCREATR_H
#define DK_THINGCREATR_H

#include "bflib_basics.h"
#include "globals.h"

#include "bflib_filelst.h"
#include "bflib_sprite.h"
#include "thing_list.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define CREATURE_TYPES_COUNT  32
#define DEAD_CREATURES_MAX_COUNT 64
#define NORMAL_FLYING_ALTITUDE 256

#define SWIPE_SPRITES_X 3
#define SWIPE_SPRITES_Y 2
#define SWIPE_SPRITE_FRAMES 5

#define TngGroup_LeaderIndex 0x0FFF
/******************************************************************************/
#pragma pack(1)

struct Thing;

enum ThingPickFlags {
    TPF_PickableCheck    = 0x01,
    TPF_OrderedPick      = 0x02,
    TPF_ReverseOrder     = 0x04,
};

enum CreatureDeathFlags {
    CrDed_Default        = 0x00, /**< Default value if no flags are needed. */
    CrDed_NoEffects      = 0x01, /**< Set if no special effects should accompany the creature death. */
    CrDed_DiedInBattle   = 0x02, /**< Set if the creature died during a battle. */
    CrDed_NoUnconscious  = 0x04, /**< Set if the creature isn't allowed to become unconscious. */
    CrDed_NotReallyDying = 0x08, /**< Set if it's not really death, it either transforms or leaves. */
};

struct CreatureStorage {
  unsigned char model;
  unsigned char explevel;
};

#pragma pack()
/******************************************************************************/
extern struct TbSprite *swipe_sprites;
extern struct TbSprite *end_swipe_sprites;
extern int creature_swap_idx[CREATURE_TYPES_COUNT];
extern unsigned long creature_create_errors;
/******************************************************************************/
struct Thing *create_creature(struct Coord3d *pos, ThingModel model, PlayerNumber owner);
long move_creature(struct Thing *thing);
TbBool kill_creature(struct Thing *creatng, struct Thing *killertng,
    PlayerNumber killer_plyr_idx, CrDeathFlags flags);
TbBool kill_creature_compat(struct Thing *creatng, struct Thing *killertng, PlayerNumber killer_plyr_idx,
      TbBool no_effects, TbBool died_in_battle, TbBool disallow_unconscious);
void update_creature_count(struct Thing *thing);
TngUpdateRet process_creature_state(struct Thing *thing);

TbBool create_random_evil_creature(MapCoord x, MapCoord y, PlayerNumber owner, CrtrExpLevel max_lv);
TbBool create_random_hero_creature(MapCoord x, MapCoord y, PlayerNumber owner, CrtrExpLevel max_lv);
TbBool create_owned_special_digger(MapCoord x, MapCoord y, PlayerNumber owner);

TbBool creature_increase_level(struct Thing *thing);
void set_creature_level(struct Thing *thing, long nlvl);
void init_creature_level(struct Thing *thing, long nlev);
long get_creature_speed(const struct Thing *thing);

TbBool control_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
TbBool control_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing);
void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
long get_human_controlled_creature_target(struct Thing *thing, long a2);
struct Thing *get_creature_near_for_controlling(unsigned char a1, long a2, long a3);

TbBool load_swipe_graphic_for_creature(const struct Thing *thing);
void free_swipe_graphic(void);
void draw_swipe_graphic(void);

long creature_available_for_combat_this_turn(struct Thing *thing);
TbBool set_creature_object_combat(struct Thing *crthing, struct Thing *obthing);
TbBool set_creature_door_combat(struct Thing *crthing, struct Thing *obthing);
void creature_fire_shot(struct Thing *firing,struct  Thing *target, ThingModel shot_model, char a2, unsigned char hit_type);
void creature_cast_spell_at_thing(struct Thing *caster, struct Thing *target, long a3, long a4);
void creature_cast_spell(struct Thing *caster, long trg_x, long trg_y, long a4, long a5);
unsigned int get_creature_blocked_flags_at(struct Thing *thing, struct Coord3d *newpos);

struct Thing *get_enemy_dungeon_heart_creature_can_see(struct Thing *thing);
void food_eaten_by_creature(struct Thing *crthing, struct Thing *obthing);
short creature_take_wage_from_gold_pile(struct Thing *crthing,struct Thing *obthing);
struct Thing *get_creature_near(unsigned short pos_x, unsigned short pos_y);
struct Thing *get_creature_near_with_filter(unsigned short pos_x, unsigned short pos_y, Thing_Filter filter, FilterParam param);
void anger_apply_anger_to_creature(struct Thing *thing, long anger, AnnoyMotive reason, long a3);
void apply_damage_to_thing_and_display_health(struct Thing *thing, HitPoints dmg, PlayerNumber inflicting_plyr_idx);
void process_creature_standing_on_corpses_at(struct Thing *thing, struct Coord3d *pos);
long creature_instance_has_reset(const struct Thing *thing, long a2);
void set_creature_instance(struct Thing *thing, CrInstance inst_idx, long a2, long a3, struct Coord3d *pos);
unsigned short find_next_annoyed_creature(unsigned char a1, unsigned short a2);
void draw_creature_view(struct Thing *thing);

TbBool remove_creature_from_group(struct Thing *thing);
long add_creature_to_group_as_leader(struct Thing *thing1, struct Thing *thing2);
TbBool creature_is_group_member(const struct Thing *thing);
TbBool creature_is_group_leader(const struct Thing *thing);
struct Thing *get_group_leader(const struct Thing *thing);
void set_first_creature(struct Thing *thing);
void remove_first_creature(struct Thing *thing);
long player_list_creature_filter_needs_to_be_placed_in_room(const struct Thing *thing, MaxTngFilterParam param, long maximizer);

TbBool creature_has_lair_room(const struct Thing *creatng);
struct Room *get_creature_lair_room(const struct Thing *creatng);
TbBool remove_creature_lair(struct Thing *thing);

TbBool creature_affected_by_spell(const struct Thing *thing, SpellKind spkind);
void apply_spell_effect_to_thing(struct Thing *thing, SpellKind spell_idx, long spell_lev);
void terminate_thing_spell_effect(struct Thing *thing, SpellKind spkind);
void process_thing_spell_effects(struct Thing *thing);
void delete_effects_attached_to_creature(struct Thing *creatng);
long get_spell_duration_left_on_thing(const struct Thing *thing, SpellKind spkind);

void anger_set_creature_anger_all_types(struct Thing *thing, long a2);
void change_creature_owner(struct Thing *thing, PlayerNumber nowner);
struct Thing *find_players_next_creature_of_breed_and_gui_job(long breed_idx, long job_idx, PlayerNumber plyr_idx, unsigned char pick_flags);
struct Thing *pick_up_creature_of_breed_and_gui_job(long breed_idx, long job_idx, PlayerNumber owner, unsigned char pick_flags);
struct Thing *find_players_creature_dragging_thing(PlayerNumber plyr_idx, const struct Thing *dragtng);
struct Thing *find_creature_dragging_thing(const struct Thing *dragtng);
struct Thing *find_players_highest_score_creature_in_fight_not_affected_by_spell(PlayerNumber plyr_idx, PowerKind pwkind);

void init_creature_scores(void);
long get_creature_thing_score(const struct Thing *thing);
TbBool add_creature_score_to_owner(struct Thing *thing);
TbBool remove_creature_score_from_owner(struct Thing *thing);
long calculate_melee_damage(const struct Thing *thing);
long calculate_shot_damage(const struct Thing *thing, ThingModel shot_model);
long project_creature_shot_damage(const struct Thing *thing, ThingModel shot_model);

long update_creature_levels(struct Thing *thing);
TngUpdateRet update_creature(struct Thing *thing);
TbBool creature_stats_debug_dump(void);
/******************************************************************************/
TbBool thing_is_creature(const struct Thing *thing);
TbBool thing_is_creature_special_digger(const struct Thing *thing);
TbBool creature_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx);
TbBool creature_is_invisible(const struct Thing *thing);
TbBool creature_can_see_invisible(const struct Thing *thing);
int get_creature_health_permil(const struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
