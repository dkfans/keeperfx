/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_hand.c
 *     power_hand support functions.
 * @par Purpose:
 *     Functions to power_hand.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "power_hand.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "bflib_vidraw.h"
#include "bflib_sound.h"

#include "magic.h"
#include "power_specials.h"
#include "power_process.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "room_garden.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_shots.h"
#include "thing_traps.h"
#include "thing_physics.h"
#include "thing_stats.h"
#include "thing_navigate.h"
#include "creature_graphics.h"
#include "creature_states.h"
#include "creature_states_mood.h"
#include "creature_states_combt.h"
#include "creature_states_tresr.h"
#include "creature_states_gardn.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_effects.h"
#include "player_instances.h"
#include "player_states.h"
#include "kjm_input.h"
#include "front_input.h"
#include "frontend.h"
#include "gui_draw.h"
#include "engine_render.h"
#include "engine_arrays.h"
#include "sounds.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_can_thing_be_picked_up2_by_player(const struct Thing *thing, unsigned char plyr_idx);
DLLIMPORT void _DK_stop_creatures_around_hand(char a1, unsigned short value, unsigned short a3);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Thing *create_gold_for_hand_grab(struct Thing *thing, long owner)
{
    //return _DK_create_gold_for_hand_grab(thing, owner);
    struct Thing *objtng;
    objtng = INVALID_THING;
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(owner);
    if (dungeon->field_14BC != thing->index)
    {
        dungeon->field_14BC = thing->index;
        GoldAmount gold_req;
        if (lbKeyOn[KC_LCONTROL])
        {
            gold_req = thing->long_13;
        }
        else
        {
            gold_req = thing->long_13 / 4 + 1;
        }
        if (gold_req <= 100)
            gold_req = 100;
        dungeon->field_14BE = gold_req;
    }
    struct Coord3d pos;
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val;
    GoldAmount gold_picked;
    {
        struct Room *room;
        room = get_room_thing_is_on(thing);
        if (!room_is_invalid(room))
            gold_picked = remove_gold_from_hoarde(thing, room, dungeon->field_14BE);
        else
            gold_picked = 0;
    }
    if (gold_picked > 0)
    {
        objtng = create_object(&pos, 43, game.neutral_player_num, -1);
        if (!thing_is_invalid(objtng))
        {
            objtng->long_13 = gold_picked;
            pos.z.val += 128;
            struct Thing *efftng;
            efftng = create_effect_element(&pos, TngEffElm_Price, owner);
            if (!thing_is_invalid(efftng))
                efftng->long_13 = gold_picked;
        }
    }
    return objtng;
}

/**
 *
 * @param thing
 * @param plyr_idx
 */
unsigned long object_is_pickable_by_hand_for_use(const struct Thing *thing, long plyr_idx)
{
    struct SlabMap *slb;
    if (thing_is_special_box(thing))
    {
        slb = get_slabmap_thing_is_on(thing);
        if ((slabmap_owner(slb) != plyr_idx) || thing_is_dragged_or_pulled(thing))
            return false;
        return true;
    }
    return false;
}

TbBool thing_is_picked_up(const struct Thing *thing)
{
    return (((thing->alloc_flags & TAlF_IsInLimbo) != 0) || ((thing->state_flags & TF1_InCtrldLimbo) != 0));
}

TbBool thing_is_picked_up_by_player(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (((thing->alloc_flags & TAlF_IsInLimbo) == 0) && ((thing->state_flags & TF1_InCtrldLimbo) == 0))
        return false;
    if (thing_is_in_power_hand_list(thing, plyr_idx))
        return true;
    return thing_is_in_computer_power_hand_list(thing, plyr_idx);
}

TbBool thing_is_picked_up_by_owner(const struct Thing *thing)
{
    return thing_is_picked_up_by_player(thing, thing->owner);
}

TbBool thing_is_picked_up_by_enemy(const struct Thing *thing)
{
    if (((thing->alloc_flags & TAlF_IsInLimbo) == 0) && ((thing->state_flags & TF1_InCtrldLimbo) == 0))
        return false;
    return !thing_is_in_power_hand_list(thing, thing->owner) && !thing_is_in_computer_power_hand_list(thing, thing->owner);
}

/**
 * Returns if a thing can be picked up by players hand.
 * @see can_thing_be_picked_up_by_player()
 * @param player
 * @param thing
 * @return
 */
TbBool thing_is_pickable_by_hand(struct PlayerInfo *player, const struct Thing *thing)
{
    if (!thing_exists(thing))
        return false;
    return can_thing_be_picked_up_by_player(thing, player->id_number);
}

TbBool armageddon_blocks_creature_pickup(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if ((game.armageddon_cast_turn != 0) && (game.armageddon.count_down + game.armageddon_cast_turn <= game.play_gameturn)) {
        return true;
    }
    return false;
}

long can_thing_be_picked_up_by_player(const struct Thing *thing, PlayerNumber plyr_idx)
{
    // Some things can be picked not to be placed in hand, but for direct use
    if (thing_is_object(thing))
    {
        if (object_is_pickable_by_hand_for_use(thing, plyr_idx))
            return true;
    }
    // Other things are pickable only for placing in hand
    return can_cast_spell(plyr_idx, PwrK_HAND, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, CastChk_Default);
}

long can_thing_be_picked_up2_by_player(const struct Thing *thing, PlayerNumber plyr_idx)
{
    //TODO: rewrite, then give it better name
    return _DK_can_thing_be_picked_up2_by_player(thing, plyr_idx);
}

void set_power_hand_offset(struct PlayerInfo *player, struct Thing *thing)
{
    //_DK_set_power_hand_offset(player, thing);
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);

    if (thing->class_id == TCls_Creature)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
      if ((cctrl->spell_flags & CSAfF_Chicken) != 0) {
          dungeon->field_43 = 11;
          dungeon->field_53 = 56;
      } else
      {
          struct CreaturePickedUpOffset *pickoffs;
          pickoffs = get_creature_picked_up_offset(thing);
          dungeon->field_43 = pickoffs->field_4;
          dungeon->field_53 = pickoffs->field_6;
      }
    } else
    if (thing->class_id == TCls_Object)
    {
      if (object_is_mature_food(thing))
      {
          dungeon->field_43 = 11;
          dungeon->field_53 = 56;
      } else
      {
          dungeon->field_43 = 60;
          dungeon->field_53 = 40;
      }
    } else
    {
        dungeon->field_43 = 60;
        dungeon->field_53 = 40;
    }
}

struct Thing *process_object_being_picked_up(struct Thing *thing, long plyr_idx)
{
  struct PlayerInfo *player;
  struct Thing *picktng;
  struct Coord3d pos;
  struct PowerConfigStats *powerst;
  long i;
  switch (thing->model)
  {
    case 3:
    case 6:
    case 43:
    case 136:
      i = thing->creature.gold_carried;
      if (i != 0)
      {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val + 128;
        create_price_effect(&pos, thing->owner, i);
      }
      powerst = get_power_model_stats(PwrK_PICKUPGOLD);
      thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
      picktng = thing;
      break;
    case 10:
      i = UNSYNC_RANDOM(3);
      powerst = get_power_model_stats(PwrK_PICKUPFOOD);
      thing_play_sample(thing, powerst->select_sound_idx+i, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
      i = convert_td_iso(122);
      set_thing_draw(thing, i, 256, -1, -1, 0, 2);
      remove_food_from_food_room_if_possible(thing);
      picktng = thing;
      break;
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
      picktng = create_gold_for_hand_grab(thing, plyr_idx);
      break;
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
      player = get_player(plyr_idx);
      activate_dungeon_special(thing, player);
      picktng = NULL;
      break;
    default:
      ERRORLOG("Picking up invalid object");
      picktng = NULL;
      break;
  }
  return picktng;
}

void set_power_hand_graphic(long plyr_idx, long a2, long a3)
{
  struct PlayerInfo *player;
  struct Thing *thing;
  player = get_player(plyr_idx);
  if (player->hand_busy_until_turn >= game.play_gameturn)
  {
    if ((a2 == 786) || (a2 == 787))
      player->hand_busy_until_turn = 0;
  }
  if (player->hand_busy_until_turn < game.play_gameturn)
  {
    if (player->field_C != a2)
    {
      player->field_C = a2;
      thing = thing_get(player->hand_thing_idx);
      if ((a2 == 782) || (a2 == 781))
      {
        set_thing_draw(thing, a2, a3, gameadd.crtr_conf.sprite_size, 0, 0, 2);
      } else
      {
        set_thing_draw(thing, a2, a3, gameadd.crtr_conf.sprite_size, 1, 0, 2);
      }
      thing = get_first_thing_in_power_hand(player);
      set_power_hand_offset(player,thing);
    }
  }
}

TbBool power_hand_is_empty(const struct PlayerInfo *player)
{
    const struct Dungeon *dungeon;
    dungeon = get_dungeon(player->id_number);
    return (dungeon->num_things_in_hand <= 0);
}

TbBool power_hand_is_full(const struct PlayerInfo *player)
{
    const struct Dungeon *dungeon;
  dungeon = get_dungeon(player->id_number);
  return (dungeon->num_things_in_hand >= MAX_THINGS_IN_HAND);
}

struct Thing *get_first_thing_in_power_hand(struct PlayerInfo *player)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(player->id_number);
    return thing_get(dungeon->things_in_hand[0]);
}

/** Removes a thing from player's power hand list without any further processing.
 *
 * @param thing
 * @param plyr_idx
 * @return
 */
TbBool remove_first_thing_from_power_hand_list(PlayerNumber plyr_idx)
{
  struct Dungeon *dungeon;
  long i;
  long num_in_hand;
  dungeon = get_dungeon(plyr_idx);
  num_in_hand = dungeon->num_things_in_hand;
  if (num_in_hand > MAX_THINGS_IN_HAND)
      num_in_hand = MAX_THINGS_IN_HAND;
  if (num_in_hand > 0)
  {
      for (i = 0; i < num_in_hand-1; i++)
      {
        dungeon->things_in_hand[i] = dungeon->things_in_hand[i+1];
      }
      num_in_hand--;
      dungeon->num_things_in_hand = num_in_hand;
      dungeon->things_in_hand[num_in_hand] = 0;
      return true;
  }
  return false;
}

/** Removes a thing from player's power hand list without any further processing.
 *
 * @param thing
 * @param plyr_idx
 * @return
 */
TbBool remove_thing_from_power_hand_list(struct Thing *thing, PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    long i;
    long num_in_hand;
    dungeon = get_dungeon(plyr_idx);
    num_in_hand = dungeon->num_things_in_hand;
    if (num_in_hand > MAX_THINGS_IN_HAND)
        num_in_hand = MAX_THINGS_IN_HAND;
    for (i = 0; i < num_in_hand; i++)
    {
        if (dungeon->things_in_hand[i] == thing->index)
        {
            for ( ; i < num_in_hand-1; i++)
            {
                dungeon->things_in_hand[i] = dungeon->things_in_hand[i+1];
            }
            num_in_hand--;
            dungeon->num_things_in_hand = num_in_hand;
            dungeon->things_in_hand[num_in_hand] = 0;
            return true;
        }
    }
    return false;
}

/** Puts a thing into player's power hand list without any further processing.
 * Originally was named dump_thing_in_power_hand().
 * @param thing
 * @param plyr_idx
 * @return
 */
TbBool insert_thing_into_power_hand_list(struct Thing *thing, PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    long i;
    struct PowerConfigStats *powerst;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon->num_things_in_hand >= MAX_THINGS_IN_HAND)
      return false;
    // Move all things in list up, to free position 0
    for (i = MAX_THINGS_IN_HAND-1; i > 0; i--)
    {
      dungeon->things_in_hand[i] = dungeon->things_in_hand[i-1];
    }
    dungeon->num_things_in_hand++;
    dungeon->things_in_hand[0] = thing->index;
    powerst = get_power_model_stats(PwrK_HAND);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    if (thing->class_id == TCls_Creature) {
        remove_all_traces_of_combat(thing);
    }
    if (thing->class_id == TCls_Creature)
    {
        powerst = get_power_model_stats(PwrK_PICKUPCRTR);
        thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        if (is_my_player_number(thing->owner)) {
            play_creature_sound(thing, CrSnd_Hang, 3, 1);
        }
    }
    return true;
}

/** Checks if given thing is placed in power hand of given player.
 *
 * @param thing
 * @param plyr_idx
 * @return
 */
TbBool thing_is_in_power_hand_list(const struct Thing *thing, PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    long i;
    dungeon = get_dungeon(plyr_idx);
    for (i = 0; i < dungeon->num_things_in_hand; i++)
    {
        if (dungeon->things_in_hand[i] == thing->index)
        {
            return true;
        }
    }
    return false;
}

void place_thing_in_limbo(struct Thing *thing)
{
    remove_thing_from_mapwho(thing);
    thing->field_4F |= TF4F_Unknown01;
    thing->alloc_flags |= TAlF_IsInLimbo;
}

void remove_thing_from_limbo(struct Thing *thing)
{
    thing->alloc_flags &= ~TAlF_IsInLimbo;
    thing->field_4F &= ~TF4F_Unknown01;
    place_thing_in_mapwho(thing);
}

void draw_power_hand(void)
{
    SYNCDBG(17,"Starting");
    struct PlayerInfo *player;
    struct CreaturePickedUpOffset *pickoffs;
    struct Thing *thing;
    struct Thing *picktng;
    struct Room *room;
    struct RoomData *rdata;
    player = get_my_player();
    if ((player->flgfield_6 & PlaF6_Unknown01) != 0)
        return;
    if (game.small_map_state == 2)
        return;
    lbDisplay.DrawFlags = 0x00;
    if (player->view_type != PVT_DungeonTop)
        return;
    // Color rendering array pointers used by draw_keepersprite()
    render_fade_tables = pixmap.fade_tables;
    render_ghost = pixmap.ghost;
    render_alpha = (unsigned char *)&alpha_sprite_table;
    // Scale factor
    int ps_units_per_px;
    {
        struct TbSprite *spr;
        spr = &gui_panel_sprites[164]; // Use dungeon special box as reference
        ps_units_per_px = calculate_relative_upp(46, units_per_pixel_ui, spr->SHeight);
    }
    // Now draw
    if (((game.operation_flags & GOF_ShowGui) != 0) && (game.small_map_state != 2)
      && mouse_is_over_pannel_map(player->minimap_pos_x, player->minimap_pos_y))
    {
        MapSubtlCoord stl_x;
        MapSubtlCoord stl_y;
        stl_x = game.hand_over_subtile_x;
        stl_y = game.hand_over_subtile_y;
        SYNCDBG(7,"Drawing over pannel map");
        room = subtile_room_get(stl_x,stl_y);
        if ((!room_is_invalid(room)) && (subtile_revealed(stl_x, stl_y, player->id_number)))
        {
            rdata = room_data_get_for_room(room);
            draw_gui_panel_sprite_centered(GetMouseX()+scale_ui_value(24), GetMouseY()+scale_ui_value(32), ps_units_per_px, rdata->medsym_sprite_idx);
        }
        if ((!power_hand_is_empty(player)) && (game.small_map_state == 1))
        {
            draw_mini_things_in_hand(GetMouseX()+scale_ui_value(10), GetMouseY()+scale_ui_value(10));
        }
        return;
    }
    if (game_is_busy_doing_gui())
    {
        SYNCDBG(7,"Drawing while GUI busy");
        draw_mini_things_in_hand(GetMouseX()+scale_ui_value(10), GetMouseY()+scale_ui_value(10));
        return;
    }
    thing = thing_get(player->hand_thing_idx);
    if (thing_is_invalid(thing))
        return;
    if (player->hand_busy_until_turn > game.play_gameturn)
    {
        SYNCDBG(7,"Drawing hand %s index %d, busy state", thing_model_name(thing), (int)thing->index);
        process_keeper_sprite(GetMouseX()+scale_ui_value(60), GetMouseY()+scale_ui_value(40),
          thing->anim_sprite, 0, thing->field_48, scale_ui_value(64));
        draw_mini_things_in_hand(GetMouseX()+scale_ui_value(60), GetMouseY());
        return;
    }
    SYNCDBG(7,"Drawing hand %s index %d", thing_model_name(thing), (int)thing->index);
    if ((player->additional_flags & PlaAF_ChosenSubTileIsHigh) != 0)
    {
        draw_mini_things_in_hand(GetMouseX()+scale_ui_value(18), GetMouseY());
        return;
    }
    if (player->work_state != PSt_HoldInHand)
    {
      if ( (player->work_state != PSt_CtrlDungeon)
        || ((player->secondary_cursor_state != CSt_PowerHand) && ((player->work_state != PSt_CtrlDungeon) || (player->secondary_cursor_state != CSt_DefaultArrow) || (player->primary_cursor_state != CSt_PowerHand))) )
      {
        if ((player->instance_num != PI_Grab) && (player->instance_num != PI_Drop))
        {
          if (player->work_state == PSt_Slap)
          {
            process_keeper_sprite(GetMouseX() + scale_ui_value(70), GetMouseY() + scale_ui_value(46),
                thing->anim_sprite, 0, thing->field_48, scale_ui_value(64));
          } else
          if (player->work_state == PSt_CtrlDungeon)
          {
            if ((player->secondary_cursor_state == CSt_DoorKey) || (player->primary_cursor_state == CSt_DoorKey))
            {
              draw_mini_things_in_hand(GetMouseX()+scale_ui_value(18), GetMouseY());
            }
          }
          return;
        }
      }
    }
    long inputpos_x;
    long inputpos_y;
    picktng = get_first_thing_in_power_hand(player);
    if ((!thing_is_invalid(picktng)) && ((picktng->field_4F & TF4F_Unknown01) == 0))
    {
        SYNCDBG(7,"Holding %s",thing_model_name(picktng));
        switch (picktng->class_id)
        {
        case TCls_Creature:
            if (!creature_affected_by_spell(picktng, SplK_Chicken))
            {
                pickoffs = get_creature_picked_up_offset(picktng);
                inputpos_x = GetMouseX() + scale_ui_value(pickoffs->delta_x);
                inputpos_y = GetMouseY() + scale_ui_value(pickoffs->delta_y);
                if (creatures[picktng->model].field_7)
                  EngineSpriteDrawUsingAlpha = 1;
                process_keeper_sprite(inputpos_x / pixel_size, inputpos_y / pixel_size,
                    picktng->anim_sprite, 0, picktng->field_48, scale_ui_value(64));
                EngineSpriteDrawUsingAlpha = 0;
            } else
            {
                inputpos_x = GetMouseX() + scale_ui_value(11);
                inputpos_y = GetMouseY() + scale_ui_value(56);
                process_keeper_sprite(inputpos_x / pixel_size, inputpos_y / pixel_size,
                    picktng->anim_sprite, 0, picktng->field_48, scale_ui_value(64));
            }
            break;
        case TCls_Object:
            if (object_is_mature_food(picktng))
            {
              inputpos_x = GetMouseX() + scale_ui_value(11);
              inputpos_y = GetMouseY() + scale_ui_value(56);
              process_keeper_sprite(inputpos_x / pixel_size, inputpos_y / pixel_size,
                  picktng->anim_sprite, 0, picktng->field_48, scale_ui_value(64));
              break;
            } else
            if ((picktng->class_id == TCls_Object) && object_is_gold_pile(picktng))
            {
                break;
            }
            // fall through
        default:
            inputpos_x = GetMouseX();
            inputpos_y = GetMouseY();
            process_keeper_sprite(inputpos_x / pixel_size, inputpos_y / pixel_size,
                  picktng->anim_sprite, 0, picktng->field_48, scale_ui_value(64));
            break;
        }
    }
    if (player->field_C == 784)
    {
        inputpos_x = GetMouseX() + scale_ui_value(58);
        inputpos_y = GetMouseY() +  scale_ui_value(6);
        process_keeper_sprite(inputpos_x / pixel_size, inputpos_y / pixel_size,
            thing->anim_sprite, 0, thing->field_48, scale_ui_value(64));
        draw_mini_things_in_hand(GetMouseX()+scale_ui_value(60), GetMouseY());
    } else
    {
        inputpos_x = GetMouseX() + scale_ui_value(60);
        inputpos_y = GetMouseY() + scale_ui_value(40);
        process_keeper_sprite(inputpos_x / pixel_size, inputpos_y / pixel_size,
            thing->anim_sprite, 0, thing->field_48, scale_ui_value(64));
        draw_mini_things_in_hand(GetMouseX()+scale_ui_value(60), GetMouseY());
    }
}

TbBool object_is_slappable(const struct Thing *thing, long plyr_idx)
{
    if (thing->owner == plyr_idx) {
        return (object_is_mature_food(thing));
    }
    return false;
}

/*void get_nearest_thing_for_hand_or_slap_on_map_block(long *near_distance, struct Thing **near_thing,struct Map *mapblk, long plyr_idx, long x, long y)
{
  struct Thing *thing;
  long i;
  unsigned long k;
  k = 0;
  i = get_mapwho_thing_index(mapblk);
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->field_2;
    // Begin per-loop code
    if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & TF1_Unkn02) == 0) && (thing->field_7 != 67))
    {
      if (can_thing_be_picked_up_by_player(thing, plyr_idx) || thing_slappable(thing, plyr_idx))
      {
        if (*near_distance > 2 * abs(y-thing->mappos.y.stl.pos))
        {
          *near_distance = 2 * abs(y-thing->mappos.y.stl.pos);
          *near_thing = thing;
        }
      }
    }
    // End of per-loop code
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break_mapwho_infinite_chain(mapblk);
      break;
    }
  }
}*/

long near_map_block_thing_filter_ready_for_hand_or_slap(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    long dist_x;
    long dist_y;
    if (!thing_is_picked_up(thing)
        && (thing->active_state != CrSt_CreatureUnconscious))
    {
      if (can_thing_be_picked_up_by_player(thing, param->plyr_idx) || thing_slappable(thing, param->plyr_idx))
      {
          // note that abs() is not required because we're computing square of the values
          dist_x = param->num1-(MapCoord)thing->mappos.x.val;
          dist_y = param->num2-(MapCoord)thing->mappos.y.val;
          // This function should return max value when the distance is minimal, so:
          return LONG_MAX-(dist_x*dist_x + dist_y*dist_y);
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

TbBool thing_slappable(const struct Thing *thing, long plyr_idx)
{
    switch (thing->class_id)
    {
    case TCls_Object:
        return object_is_slappable(thing, plyr_idx);
    case TCls_Shot:
        return shot_is_slappable(thing, plyr_idx);
    case TCls_Creature:
        return creature_is_slappable(thing, plyr_idx);
    case TCls_Trap:
        return trap_is_slappable(thing, plyr_idx);
    default:
        return false;
    }
}

struct Thing *get_nearest_thing_for_hand_or_slap(PlayerNumber plyr_idx, MapCoord pos_x, MapCoord pos_y)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    filter = near_map_block_thing_filter_ready_for_hand_or_slap;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

void drop_gold_coins(const struct Coord3d *pos, long value, long plyr_idx)
{
    struct Coord3d locpos;
    int i;
    locpos.z.val = get_ceiling_height_at(pos) - PLAYER_RANDOM(plyr_idx, 128);
    for (i = 0; i < 8; i++)
    {
        if (i > 0)
        {
            long angle;
            angle = PLAYER_RANDOM(plyr_idx, 2*LbFPMath_PI);
            locpos.x.val = pos->x.val + distance_with_angle_to_coord_x(127, angle);
            locpos.y.val = pos->y.val + distance_with_angle_to_coord_y(127, angle);
        } else
        {
            locpos.x.val = pos->x.val;
            locpos.y.val = pos->y.val;
        }
        struct Thing *thing;
        thing = create_object(&locpos, 128, plyr_idx, -1);
        if (thing_is_invalid(thing))
            break;
        if (i > 0)
        {
            thing->fall_acceleration += PLAYER_RANDOM(plyr_idx, thing->fall_acceleration) - thing->fall_acceleration / 2;
            thing->valuable.gold_stored = 0;
        } else
        {
            thing->valuable.gold_stored = value;
        }
    }
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    if (player_exists(player)) {
        set_power_hand_graphic(plyr_idx, 782, 256);
        player->hand_busy_until_turn = game.play_gameturn + 16;
    }
}

long gold_being_dropped_on_creature(long plyr_idx, struct Thing *goldtng, struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    struct Coord3d pos;
    TbBool taking_salary;
    taking_salary = false;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    pos.z.val = creatng->mappos.z.val;
    pos.z.val = get_ceiling_height_at(&pos);
    if (creature_is_taking_salary_activity(creatng))
    {
        cctrl = creature_control_get_from_thing(creatng);
        if (cctrl->paydays_owed > 0)
            cctrl->paydays_owed--;
        set_start_state(creatng);
        taking_salary = true;
    }
    GoldAmount salary;
    salary = calculate_correct_creature_pay(creatng);
    if (salary < 1) // we devide by this number later on
    {
        salary = 1;
    }
    long tribute;
    tribute = goldtng->valuable.gold_stored;
    drop_gold_coins(&pos, 0, plyr_idx);
    if (tribute >= salary)
    {
        thing_play_sample(creatng, 34, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    else if ((tribute * 2) >= salary)
    {
        thing_play_sample(creatng, 33, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    else
    {
        thing_play_sample(creatng, 32, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS/2);
    }
    if ( !taking_salary )
    {
        cctrl = creature_control_get_from_thing(creatng);
        if (cctrl->prepayments_received < 255) {
            cctrl->prepayments_received++;
        }
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    anger_apply_anger_to_creature_all_types(creatng, (crstat->annoy_got_wage * tribute / salary * 2));
    if (gameadd.classic_bugs_flags & ClscBug_FullyHappyWithGold)
    {
        anger_set_creature_anger_all_types(creatng, 0);
    }
    if (can_change_from_state_to(creatng, get_creature_state_besides_interruptions(creatng), CrSt_CreatureBeHappy))
    {
        if (external_set_thing_state(creatng, CrSt_CreatureBeHappy)) {
            cctrl = creature_control_get_from_thing(creatng);
            cctrl->countdown_282 = 50;
        }
    }
    return 1;
}

/**
 * Low level function which unconditionally drops creature held in hand.
 *
 * @param dungeon
 * @param droptng
 * @param dstpos
 */
void drop_held_thing_on_ground(struct Dungeon *dungeon, struct Thing *droptng, const struct Coord3d *dstpos)
{
    droptng->mappos.x.val = dstpos->x.val;
    droptng->mappos.y.val = dstpos->y.val;
    droptng->mappos.z.val = subtile_coord(8,0);
    long fall_dist;
    fall_dist = get_ceiling_height_at(&droptng->mappos) - get_floor_height_at(&droptng->mappos);
    struct CreatureStats* crstat;
    if (fall_dist < 0) {
        fall_dist = 0;
    } else
    if (fall_dist > subtile_coord(3,0)) {
        fall_dist = subtile_coord(3,0);
    }
    droptng->mappos.z.val = fall_dist + get_floor_height_at(&droptng->mappos);
    remove_thing_from_limbo(droptng);
    if (thing_is_creature(droptng))
    {
        initialise_thing_state(droptng, CrSt_CreatureBeingDropped);
        stop_creature_sound(droptng, 5);
        if (is_my_player_number(dungeon->owner)) {
            play_creature_sound(droptng, 6, 3, 0);
        }
        dungeon->last_creature_dropped_gameturn = game.play_gameturn;
        crstat = creature_stats_get(droptng->model);
        if ( (crstat->illuminated) || (creature_affected_by_spell(droptng, SplK_Light)) )
        {
            illuminate_creature(droptng);
        }
    } else
    if (thing_is_object(droptng))
    {
        if (object_is_mature_food(droptng)) {
            set_thing_draw(droptng, convert_td_iso(819), 256, -1, -1, 0, 2);
        }
        droptng->continue_state = droptng->active_state;
        droptng->active_state = ObSt_BeingDropped;
    }
}

short dump_first_held_thing_on_map(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool update_hand)
{
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    // If nothing in hand - nothing to do
    if (dungeon->num_things_in_hand < 1) {
        return 0;
    }
    // Check if drop position is allowed
    struct Thing *droptng;
    droptng = thing_get(dungeon->things_in_hand[0]);
    if (!can_drop_thing_here(stl_x, stl_y, plyr_idx, thing_is_creature_special_digger(droptng))) {
        return 0;
    }
    // Check if object will fit into that position
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_thing_height_at(droptng, &pos);
    if (thing_in_wall_at(droptng, &pos)) {
        return 0;
    }
    struct Thing *overtng;
    overtng = thing_get(player->thing_under_hand);
    if (thing_is_object(droptng) && object_is_gold_pile(droptng))
    {
        if (thing_is_creature(overtng) && creature_able_to_get_salary(overtng))
        {
            gold_being_dropped_on_creature(plyr_idx, droptng, overtng);
        } else
        {
            drop_gold_coins(&pos, droptng->valuable.gold_stored, plyr_idx);
            if (is_my_player_number(plyr_idx)) {
                play_non_3d_sample(88);
            }
        }
        delete_thing_structure(droptng, 0);
    } else
    if (thing_is_object(droptng) && object_is_mature_food(droptng))
    {
        if (thing_is_creature(overtng) && creature_able_to_eat(overtng))
        {
            food_eaten_by_creature(droptng, thing_get(player->thing_under_hand));
        } else
        {
            drop_held_thing_on_ground(dungeon, droptng, &pos);
        }
    } else
    {
        drop_held_thing_on_ground(dungeon, droptng, &pos);
    }
    if (dungeon->num_things_in_hand == 1) {
        set_player_instance(player, PI_Drop, 0);
    }
    remove_first_thing_from_power_hand_list(plyr_idx);
    if ( update_hand ) {
      set_power_hand_offset(player, get_first_thing_in_power_hand(player));
    }
    return 1;
}

void dump_thing_held_by_any_player(struct Thing *thing)
{
    int i;
    for (i=0; i<PLAYERS_COUNT; i++)
    {
        struct PlayerInfo *player;
        player = get_player(i);
        if (player_exists(player))
        {
            struct Dungeon *dungeon;
            dungeon = get_players_num_dungeon(i);
            if (dungeon_invalid(dungeon)) {
                continue;
            }
            const struct Coord3d *pos;
            pos = &dungeon->essential_pos;
            // Remove from human player hand
            drop_held_thing_on_ground(dungeon, thing, pos);
            remove_thing_from_power_hand_list(thing, dungeon->owner);
            // Remove from computer player hand
            struct Computer2 *comp;
            comp = get_computer_player(dungeon->owner);
            computer_force_dump_specific_held_thing(comp, thing, pos);
        }
    }
}

int dump_all_held_things_on_map(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    int k;
    // Dump all things
    k = 0;
    while (dump_first_held_thing_on_map(plyr_idx, stl_x, stl_y, 0) == 1) {
        k++;
    }
    return k;
}

void clear_things_in_hand(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  long i;
  dungeon = get_dungeon(player->id_number);
  for (i=0; i < MAX_THINGS_IN_HAND; i++)
    dungeon->things_in_hand[i] = 0;
}

/**
 * Processes a thing in players power hand.
 * @param dungeon The dungeon owned by target player.
 * @param thing Thing to be processed.
 * @return True if thing was processed, false if it was removed from hand.
 */
TbBool process_creature_in_dungeon_hand(struct Dungeon *dungeon, struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    //TODO CLEANUP would be nice to integrate this with process_armageddon_influencing_creature()
    if (game.armageddon_cast_turn != 0)
    {
        // If Armageddon is on, teleport creature to its position
        if ((cctrl->armageddon_teleport_turn != 0) && (cctrl->armageddon_teleport_turn <= game.play_gameturn))
        {
            cctrl->armageddon_teleport_turn = 0;
            if (remove_creature_from_power_hand(thing, dungeon->owner))
            {
                create_effect(&thing->mappos, imp_spangle_effects[thing->owner], thing->owner);
                move_thing_in_map(thing, &game.armageddon.mappos);
                //originally move was to get_player_soul_container(game.armageddon_caster_idx) mappos
                return false;
            }
        }
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    anger_apply_anger_to_creature(thing, crstat->annoy_in_hand, AngR_Other, 1);
    process_thing_spell_effects_while_blocked(thing);
    return true;
}

void process_things_in_dungeon_hand(void)
{
    //_DK_process_things_in_dungeon_hand();
    PlayerNumber plyr_idx;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo *player;
        player = get_player(plyr_idx);
        struct Dungeon *dungeon;
        dungeon = get_players_dungeon(player);
        if (player_exists(player) && (player->is_active == 1) && !dungeon_invalid(dungeon))
        {
            int i;
            for (i = 0; i < dungeon->num_things_in_hand; i++)
            {
                struct Thing *thing;
                thing = thing_get(dungeon->things_in_hand[i]);
                if (thing_is_creature(thing))
                {
                    if (!process_creature_in_dungeon_hand(dungeon, thing)) {
                        // If thing was removed from hand, process the index in hand again
                        i--;
                    }
                }
            }
        }
    }
}

void draw_mini_things_in_hand(long x, long y)
{
    SYNCDBG(7,"Starting");
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    int i;
    int expshift_x;
    // Scale factor
    int ps_units_per_px;
    {
        struct TbSprite *spr;
        spr = &gui_panel_sprites[164]; // Use dungeon special box as reference
        ps_units_per_px = calculate_relative_upp(46, units_per_pixel_ui, spr->SHeight);
    }
    int bs_units_per_px;
    {
        struct TbSprite *spr;
        spr = &button_sprite[184]; // Use creature flower level number as reference
        bs_units_per_px = calculate_relative_upp(17, units_per_pixel_ui, spr->SHeight);
    }
    unsigned long spr_idx;
    spr_idx = get_creature_model_graphics(get_players_special_digger_model(dungeon->owner), CGI_HandSymbol);
    if ((spr_idx > 0) && (spr_idx < GUI_PANEL_SPRITES_COUNT))
        i = gui_panel_sprites[spr_idx].SWidth - button_sprite[184].SWidth;
    else
        i = 0;
    long scrbase_x;
    long scrbase_y;
    scrbase_x = x;
    scrbase_y = y - scale_ui_value(58);
    expshift_x = scale_ui_value(abs(i)) / 2;
    for (i = dungeon->num_things_in_hand-1; i >= 0; i--)
    {
        int icol;
        int irow;
        icol = i % 4;
        irow = (i / 4);
        struct Thing *thing;
        thing = thing_get(dungeon->things_in_hand[i]);
        if (!thing_exists(thing)) {
            continue;
        }
        int scrpos_x;
        int scrpos_y;
        int shift_y;
        if (thing->class_id == TCls_Creature)
        {
            spr_idx = get_creature_model_graphics(thing->model, CGI_HandSymbol);
            if (spr_idx > 0)
            {
                struct CreatureControl *cctrl;
                cctrl = creature_control_get_from_thing(thing);
                int expspr_idx;
                expspr_idx = 184 + cctrl->explevel;
                if (irow > 0)
                    shift_y = 40;
                else
                    shift_y = 6;
                scrpos_x = scrbase_x + scale_ui_value(16) * icol;
                scrpos_y = scrbase_y + scale_ui_value(18) * irow;
                // Draw exp level
                draw_button_sprite_left(scrpos_x + expshift_x, scrpos_y + scale_ui_value(shift_y), bs_units_per_px, expspr_idx);
                // Draw creature symbol
                draw_gui_panel_sprite_left(scrpos_x, scrpos_y, ps_units_per_px, spr_idx);
            }
        } else
        if ((thing->class_id == TCls_Object) && object_is_gold_pile(thing))
        {
            spr_idx = 57;
            if (irow > 0)
                shift_y = 20;
            else
                shift_y = 0;
            scrpos_x = scrbase_x + scale_ui_value(16) * icol;
            scrpos_y = scrbase_y + scale_ui_value(14) * irow;
            draw_gui_panel_sprite_left(scrpos_x - 2, scrpos_y + scale_ui_value(shift_y), ps_units_per_px, spr_idx);
        } else
        {
            spr_idx = 59;
            if (irow > 0)
                shift_y = 20;
            else
                shift_y = 0;
            scrpos_x = scrbase_x + scale_ui_value(16) * icol;
            scrpos_y = scrbase_y + scale_ui_value(14) * irow;
            draw_gui_panel_sprite_left(scrpos_x - 2, scrpos_y + scale_ui_value(shift_y), ps_units_per_px, spr_idx);
        }
    }
}

struct Thing *create_power_hand(PlayerNumber owner)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    struct Thing *grabtng;
    struct Coord3d pos;
    pos.x.val = 0;
    pos.y.val = 0;
    pos.z.val = 0;
    thing = create_object(&pos, 37, owner, -1);
    player = get_player(owner);
    if (thing_is_invalid(thing)) {
        player->hand_thing_idx = 0;
        return INVALID_THING;
    }
    player->hand_thing_idx = thing->index;
    player->field_C = 0;
    grabtng = get_first_thing_in_power_hand(player);
    if (thing_is_invalid(thing))
    {
      set_power_hand_graphic(owner, 782, 256);
    } else
    if ((grabtng->class_id == TCls_Object) && object_is_gold_pile(grabtng))
    {
        set_power_hand_graphic(owner, 781, 256);
    } else
    {
        set_power_hand_graphic(owner, 784, 256);
    }
    place_thing_in_limbo(thing);
    return thing;
}

void delete_power_hand(PlayerNumber owner)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    long hand_idx;
    player = get_player(owner);
    hand_idx = player->hand_thing_idx;
    if (hand_idx == 0)
        return;
    player->hand_thing_idx = 0;
    thing = thing_get(hand_idx);
    delete_thing_structure(thing, 0);
}

long prepare_thing_for_power_hand(unsigned short tng_idx, PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    player = get_player(plyr_idx);
    dungeon = get_dungeon(player->id_number);
    if (player->hand_thing_idx == 0) {
        create_power_hand(plyr_idx);
    }
    if (dungeon->num_things_in_hand >= MAX_THINGS_IN_HAND) {
      return 0;
    }
    struct Thing *thing;
    thing = thing_get(tng_idx);
    player->influenced_thing_idx = thing->index;
    player->influenced_thing_creation = thing->creation_turn;
    set_player_instance(player, PI_Grab, 0);
    if (thing_is_creature(thing)) {
        clear_creature_instance(thing);
    }
    return 1;
}

void add_creature_to_sacrifice_list(PlayerNumber plyr_idx, long model, long explevel)
{
  struct Dungeon *dungeon;
  SYNCLOG("Player %d sacrificed %s exp level %d",(int)plyr_idx,thing_class_and_model_name(TCls_Creature, model),explevel);
  if ((plyr_idx < 0) || (plyr_idx >= DUNGEONS_COUNT))
  {
    ERRORLOG("Player %d cannot sacrifice %s",(int)plyr_idx,thing_class_and_model_name(TCls_Creature, model));
    return;
  }
  if ((model < 0) || (model >= CREATURE_TYPES_COUNT))
  {
    ERRORLOG("Tried to sacrifice invalid creature model %d",(int)model);
    return;
  }
  dungeon = get_dungeon(plyr_idx);
  dungeon->creature_sacrifice[model]++;
  dungeon->creature_sacrifice_exp[model] += explevel+1;
  dungeon->lvstats.creatures_sacrificed++;
}

TbBool place_thing_in_power_hand(struct Thing *thing, PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    long i;
    player = get_player(plyr_idx);
    if (!thing_is_pickable_by_hand(player, thing)) {
        ERRORLOG("The %s owned by player %d is not pickable by player %d",thing_model_name(thing),(int)thing->owner,(int)plyr_idx);
        return false;
    }
    if (thing_is_picked_up(thing)) {
        ERRORLOG("The %s is already picked up",thing_model_name(thing));
        return false;
    }
    if (thing_is_creature(thing))
    {
        reset_creature_if_affected_by_cta(thing);
        clear_creature_instance(thing);
        if (!external_set_thing_state(thing, CrSt_InPowerHand)) {
            return false;
        }
        //Removing combat is called in insert_thing_into_power_hand_list(), so we don't have to do it here
        if (creature_affected_by_spell(thing, SplK_Chicken))
            i = convert_td_iso(122);
        else
            i = get_creature_anim(thing, 9);
        if (thing->light_id != 0)
        {
            light_delete_light(thing->light_id);
            thing->light_id = 0;   
        }
        set_thing_draw(thing, i, 256, -1, -1, 0, 2);
    } else
    if (thing_is_object(thing))
    {
        thing = process_object_being_picked_up(thing, plyr_idx);
        if (thing_is_invalid(thing)) {
            return false;
        }
    }
    insert_thing_into_power_hand_list(thing, plyr_idx);
    place_thing_in_limbo(thing);
    return true;
}

TbBool remove_creature_from_power_hand(struct Thing *thing, PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    if (!thing_is_in_power_hand_list(thing, plyr_idx)) {
        return false;
    }
    if (thing_is_creature(thing))
    {
        remove_thing_from_limbo(thing);
        set_start_state(thing);
        remove_thing_from_power_hand_list(thing, plyr_idx);
        set_power_hand_offset(player, get_first_thing_in_power_hand(player));
        return true;
    }
    return false;
}

TbResult magic_use_power_hand(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short tng_idx)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    player = get_player(plyr_idx);
    if (power_hand_is_full(player)) {
        return Lb_FAIL;
    }
    thing = thing_get(tng_idx);
    if (thing_is_invalid(thing))
    {
        thing = INVALID_THING;
    } else
    if (!can_thing_be_picked_up_by_player(thing, plyr_idx))
    {
        thing = INVALID_THING;
    }
    if (thing_is_invalid(thing))
    {
        if (player->thing_under_hand > 0)
            thing = thing_get(player->thing_under_hand);
    }
    if (thing_is_invalid(thing)) {
        return Lb_FAIL;
    }
    if (!can_thing_be_picked_up_by_player(thing, plyr_idx))
    {
        ERRORLOG("The %s owned by player %d is not pickable by player %d",thing_model_name(thing),(int)thing->owner,(int)plyr_idx);
        return Lb_OK;
    }
    if (thing_is_special_box(thing))
    {
        activate_dungeon_special(thing, player);
        return Lb_OK;
    }
    if (!is_power_available(plyr_idx, PwrK_HAND)) {
        return Lb_FAIL;
    }
    prepare_thing_for_power_hand(thing->index, plyr_idx);
    return Lb_SUCCESS;
}

void stop_creatures_around_hand(char a1, unsigned short a2, unsigned short a3)
{
  _DK_stop_creatures_around_hand(a1, a2, a3);
}

TbBool slap_object(struct Thing *thing)
{
  if (object_is_mature_food(thing)) {
      destroy_object(thing);
      return true;
  }
  return false;
}

TbBool is_dangerous_drop_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (subtile_has_sacrificial_on_top(stl_x, stl_y)) {
        return true;
    }
    struct Room* droproom = (subtile_room_get(stl_x, stl_y));
    if (room_role_matches(droproom->kind, RoRoF_CrPoolLeave))
    {
        return true;
    }
    return false;
}

TbBool can_drop_thing_here(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, unsigned long allow_unclaimed)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    if (!map_block_revealed(mapblk, plyr_idx))
        return false;
    if (((mapblk->flags & SlbAtFlg_Blocking) != 0) || ((mapblk->flags & SlbAtFlg_IsDoor) != 0))
        return false;
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_owner(slb) == plyr_idx)
        return true;
    if (allow_unclaimed && slabmap_owner(slb) == game.neutral_player_num && slb->kind == SlbT_PATH)
        return true;
    return false;
}

short can_place_thing_here(struct Thing *thing, long stl_x, long stl_y, long dngn_idx)
{
    struct Coord3d pos;
    TbBool is_digger;
    is_digger = thing_is_creature_special_digger(thing);
    if (!can_drop_thing_here(stl_x, stl_y, dngn_idx, is_digger))
      return false;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_thing_height_at(thing, &pos);
    return !thing_in_wall_at(thing, &pos);
}
/******************************************************************************/
