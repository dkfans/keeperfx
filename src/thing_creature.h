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
#include "map_locations.h"
#include "packets.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define DEAD_CREATURES_MAX_COUNT 128
#define CREATURE_NAME_MAX 25
/** The standard altitude at which a creature is flying.
 * Should be over one tile, to allow flying creatures leave water areas. */
#define NORMAL_FLYING_ALTITUDE (256+16)

#define SWIPE_SPRITES_X 3
#define SWIPE_SPRITES_Y 2
#define SWIPE_SPRITE_FRAMES 5

/******************************************************************************/
#pragma pack(1)

struct Thing;

enum ThingPickFlags {
    TPF_None             = 0x00,
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
    CrDed_NoRebirth      = 0x10, /**< Set if the death blocks it from resurrecting */
};

struct CreatureStorage {
    ThingModel model;
    CrtrExpLevel exp_level;
    unsigned char count;
    char creature_name[CREATURE_NAME_MAX];
};

#pragma pack()
/******************************************************************************/
extern struct TbSpriteSheet *swipe_sprites;
extern unsigned long creature_create_errors;
/******************************************************************************/
struct Thing *create_creature(struct Coord3d *pos, ThingModel model, PlayerNumber owner);
TbBool creature_count_below_map_limit(TbBool temp_creature);
long move_creature(struct Thing *thing);
struct Thing* kill_creature(struct Thing *creatng, struct Thing *killertng,
    PlayerNumber killer_plyr_idx, CrDeathFlags flags);
void update_creature_count(struct Thing *thing);
TngUpdateRet process_creature_state(struct Thing *thing);

struct Thing *create_owned_special_digger(MapCoord x, MapCoord y, PlayerNumber owner);

TbBool creature_increase_level(struct Thing *thing);
TbBool creature_change_multiple_levels(struct Thing *thing, int count);
void set_creature_level(struct Thing *thing, CrtrExpLevel exp_level);
void init_creature_level(struct Thing *thing, CrtrExpLevel exp_level);
long get_creature_speed(const struct Thing *thing);

TbBool control_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
TbBool thing_can_be_controlled_as_controller(struct Thing* thing);
TbBool control_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing);
void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
void prepare_to_controlled_creature_death(struct Thing* thing);
ThingIndex process_player_use_instance(struct Thing *thing, CrInstance inst_id, struct Packet *packet);
ThingIndex get_human_controlled_creature_target(struct Thing *thing, CrInstance inst_id, struct Packet *packet);
struct Thing *get_creature_near_for_controlling(PlayerNumber plyr_idx, MapCoord x, MapCoord y);

TbBool load_swipe_graphic_for_creature(const struct Thing *thing);
void free_swipe_graphic(void);
void draw_swipe_graphic(void);

long creature_available_for_combat_this_turn(struct Thing *thing);
TbBool set_creature_object_combat(struct Thing *crthing, struct Thing *obthing);
TbBool set_creature_object_snipe(struct Thing* crthing, struct Thing* obthing);
TbBool set_creature_door_combat(struct Thing *crthing, struct Thing *obthing);
void thing_fire_shot(struct Thing *firing,struct  Thing *target, ThingModel shot_model, CrtrExpLevel shot_level, unsigned char hit_type);
void creature_cast_spell_at_thing(struct Thing *caster, struct Thing *target, SpellKind spl_idx, CrtrExpLevel shot_level);
void creature_cast_spell(struct Thing *caster, SpellKind spl_idx, CrtrExpLevel shot_level, MapSubtlCoord trg_x, MapSubtlCoord trg_y);

void thing_summon_temporary_creature(struct Thing* creatng, ThingModel model, char level, char count, GameTurn duration, long spl_idx);
void level_up_familiar(struct Thing* famlrtng);
void teleport_familiar_to_summoner(struct Thing* famlrtng, struct Thing* creatng);
void add_creature_to_summon_list(struct Dungeon* dungeon, ThingIndex famlrtng);
void remove_creature_from_summon_list(struct Dungeon* dungeon, ThingIndex famlrtng);

unsigned int get_creature_blocked_flags_at(struct Thing *thing, struct Coord3d *newpos);

struct Thing *get_enemy_soul_container_creature_can_see(struct Thing *thing);
TbBool thing_can_be_eaten(struct Thing *thing);
void food_eaten_by_creature(struct Thing *foodtng, struct Thing *creatng);
void anger_apply_anger_to_creature_f(struct Thing *thing, long anger, AnnoyMotive reason, long a3, const char *func_name);
#define anger_apply_anger_to_creature(thing, anger, reason, a3) anger_apply_anger_to_creature_f(thing, anger, reason, a3, __func__)
HitPoints apply_damage_to_thing_and_display_health(struct Thing *thing, HitPoints dmg, PlayerNumber inflicting_plyr_idx);
void process_creature_standing_on_corpses_at(struct Thing *thing, struct Coord3d *pos);
long creature_instance_has_reset(const struct Thing *thing, long a2);
void set_creature_instance(struct Thing *thing, CrInstance inst_idx, long targtng_idx, const struct Coord3d *pos);
unsigned short find_next_annoyed_creature(PlayerNumber plyr_idx, unsigned short current_annoyed_creature_idx);
void draw_creature_view(struct Thing *thing);

TbBool creature_is_for_dungeon_diggers_list(const struct Thing *creatng);
TbBool creature_kind_is_for_dungeon_diggers_list(PlayerNumber plyr_idx, ThingModel crmodel);
void set_first_creature(struct Thing *thing);
void remove_first_creature(struct Thing *thing);
long player_list_creature_filter_needs_to_be_placed_in_room_for_job(const struct Thing *thing, MaxTngFilterParam param, long maximizer);
void recalculate_player_creature_digger_lists(PlayerNumber plr_idx);
void recalculate_all_creature_digger_lists();

TbBool creature_has_lair_room(const struct Thing *creatng);
struct Room *get_creature_lair_room(const struct Thing *creatng);
TbBool remove_creature_lair(struct Thing *thing);

TbBool creature_affected_by_slap(const struct Thing *thing);
TbBool creature_under_spell_effect_f(const struct Thing *thing, unsigned long spell_flags, const char *func_name);
#define creature_under_spell_effect(thing, spell_flags) creature_under_spell_effect_f(thing, spell_flags, __func__)
TbBool creature_is_immune_to_spell_effect_f(const struct Thing *thing, unsigned long spell_flags, const char *func_name);
#define creature_is_immune_to_spell_effect(thing, spell_flags) creature_is_immune_to_spell_effect_f(thing, spell_flags, __func__)

TbBool set_thing_spell_flags_f(struct Thing *thing, SpellKind spell_idx, GameTurnDelta duration, CrtrExpLevel spell_level, const char *func_name);
#define set_thing_spell_flags(thing, spell_idx, duration, spell_level) set_thing_spell_flags_f(thing, spell_idx, duration, spell_level, __func__)
TbBool clear_thing_spell_flags_f(struct Thing *thing, unsigned long spell_flags, const char *func_name);
#define clear_thing_spell_flags(thing, spell_flags) clear_thing_spell_flags_f(thing, spell_flags, __func__)
void clean_spell_effect_f(struct Thing *thing, unsigned long spell_flags, const char *func_name);
#define clean_spell_effect(thing, spell_flags) clean_spell_effect_f(thing, spell_flags, __func__)

TbResult script_use_spell_on_creature(PlayerNumber plyr_idx, struct Thing *thing, SpellKind spkind, CrtrExpLevel spell_level);
TbResult script_use_spell_on_creature_with_criteria(PlayerNumber plyr_idx, ThingModel crmodel, short criteria, SpellKind spkind, CrtrExpLevel spell_level);
void apply_spell_effect_to_thing(struct Thing *thing, SpellKind spell_idx, CrtrExpLevel spell_level, PlayerNumber plyr_idx);
void first_apply_spell_effect_to_thing(struct Thing *thing, SpellKind spell_idx, CrtrExpLevel spell_level, PlayerNumber plyr_idx);
void reapply_spell_effect_to_thing(struct Thing *thing, SpellKind spell_idx, CrtrExpLevel spell_level, PlayerNumber plyr_idx, int slot_idx);
void terminate_thing_spell_effect(struct Thing *thing, SpellKind spell_idx);
void process_thing_spell_damage_or_heal_effects(struct Thing *thing, SpellKind spell_idx, CrtrExpLevel caster_level, PlayerNumber caster_owner);
void process_thing_spell_effects(struct Thing *thing);
void process_thing_spell_effects_while_blocked(struct Thing *thing);
void delete_armour_effects_attached_to_creature(struct Thing *thing);
void delete_disease_effects_attached_to_creature(struct Thing *thing);
void delete_familiars_attached_to_creature(struct Thing* sumntng);

CrInstance get_available_instance_with_spell_effect(const struct Thing *thing, unsigned long spell_flags);
SpellKind get_spell_kind_from_instance(CrInstance inst_idx);

GameTurnDelta get_spell_duration_left_on_thing_f(const struct Thing *thing, SpellKind spell_idx, const char *func_name);
#define get_spell_duration_left_on_thing(thing, spell_idx) get_spell_duration_left_on_thing_f(thing, spell_idx, __func__)

void anger_set_creature_anger_all_types(struct Thing *thing, long new_value);
void change_creature_owner(struct Thing *thing, PlayerNumber nowner);
struct Thing *find_players_next_creature_of_breed_and_gui_job(long breed_idx, long job_idx, PlayerNumber plyr_idx, unsigned char pick_flags);
struct Thing *pick_up_creature_of_model_and_gui_job(long breed_idx, long job_idx, PlayerNumber owner, unsigned char pick_flags);
void go_to_next_creature_of_model_and_gui_job(long crmodel, long job_idx, unsigned char pick_flags);
struct Thing *find_creature_dragging_thing(const struct Thing *dragtng);
struct Thing *find_players_highest_score_creature_in_fight_not_affected_by_spell(PlayerNumber plyr_idx, SpellKind spell_kind);
int claim_neutral_creatures_in_sight(struct Thing *creatng, struct Coord3d *pos, int can_see_slabs);
TbBool change_creature_owner_if_near_dungeon_heart(struct Thing *creatng);

void init_creature_scores(void);
long get_creature_thing_score(const struct Thing *thing);
TbBool add_creature_score_to_owner(struct Thing *thing);
TbBool remove_creature_score_from_owner(struct Thing *thing);
long calculate_melee_damage(struct Thing *thing, short damage_percent);
long project_melee_damage(const struct Thing *thing);
long calculate_shot_damage(struct Thing *thing, ThingModel shot_model);

long update_creature_levels(struct Thing *thing);
TngUpdateRet update_creature(struct Thing *thing);
TbBool creature_stats_debug_dump(void);

void create_light_for_possession(struct Thing *creatng);
void illuminate_creature(struct Thing *creatng);

long get_spell_slot(const struct Thing *thing, SpellKind spkind);
TbBool free_spell_slot(struct Thing *thing, int slot_idx);

void controlled_creature_pick_thing_up(struct Thing *creatng, struct Thing *picktng, PlayerNumber plyr_idx);
void controlled_creature_drop_thing(struct Thing *creatng, struct Thing *droptng, PlayerNumber plyr_idx);
void direct_control_pick_up_or_drop(PlayerNumber plyr_idx, struct Thing *creatng);
void display_controlled_pick_up_thing_name(struct Thing *picktng, unsigned long timeout, PlayerNumber plyr_idx);
struct Thing *controlled_get_thing_to_pick_up(struct Thing *creatng);
TbBool thing_is_pickable_by_digger(struct Thing *picktng, struct Thing *creatng);
struct Thing *controlled_get_trap_to_rearm(struct Thing *creatng);
void controlled_continue_looking_excluding_diagonal(struct Thing *creatng, MapSubtlCoord *stl_x, MapSubtlCoord *stl_y);

short get_creature_eye_height(const struct Thing *creatng);

void query_creature(struct PlayerInfo *player, ThingIndex index, TbBool reset, TbBool zoom);
TbBool creature_can_be_queried(struct PlayerInfo *player, struct Thing *creatng);
/******************************************************************************/
TbBool thing_is_creature(const struct Thing *thing);
TbBool thing_is_dead_creature(const struct Thing *thing);
TbBool thing_is_creature_digger(const struct Thing *thing);
TbBool thing_is_creature_special_digger(const struct Thing* thing);
TbBool thing_is_creature_spectator(const struct Thing *thing);
TbBool creature_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx);
TbBool creature_is_invisible(const struct Thing *thing);
TbBool creature_can_see_invisible(const struct Thing *thing);
TbBool creature_can_be_transferred(const struct Thing* thing);
HitPoints get_creature_health_permil(const struct Thing *thing);
/******************************************************************************/
struct Thing *script_create_new_creature(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, long carried_gold, CrtrExpLevel exp_level, char spawn_type);
struct Thing *script_create_creature_at_location(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, char spawn_type);
void script_process_new_creatures(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, long copies_num, long carried_gold, CrtrExpLevel exp_level, char spawn_type);
PlayerNumber get_appropriate_player_for_creature(struct Thing *creatng);
struct Thing* script_get_creature_by_criteria(PlayerNumber plyr_idx, ThingModel crmodel, short criteria);
void script_move_creature_with_criteria(PlayerNumber plyr_idx, ThingModel crmodel, long select_id, TbMapLocation location, ThingModel effect_id, long count);
void script_move_creature(struct Thing* thing, TbMapLocation location, ThingModel effect_id);
TbBool script_change_creatures_annoyance(PlayerNumber plyr_idx, ThingModel crmodel, long operation, long anger);
/******************************************************************************/
void throw_out_gold(struct Thing* thing, long amount);
ThingModel get_random_creature_kind_with_model_flags(unsigned long model_flags);
ThingModel get_random_appropriate_creature_kind(ThingModel original_model);
TbBool grow_up_creature(struct Thing *thing, ThingModel grow_up_model, CrtrExpLevel grow_up_level);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
