/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets_cheats.c
 *     Processing packets with cheat commands.
 * @par Purpose:
 *     Functions for creating and executing packets.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     20 Sep 2020 - 20 Sep 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "packets.h"

#include "bflib_keybrd.h"
#include "bflib_sound.h"
#include "config_terrain.h"
#include "config_effects.h"
#include "creature_states.h"
#include "frontend.h"
#include "game_legacy.h"
#include "kjm_input.h"
#include "magic.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "player_data.h"
#include "player_instances.h"
#include "player_states.h"
#include "player_utils.h"
#include "room_util.h"
#include "slab_data.h"
#include "thing_effects.h"
#include "thing_navigate.h"

extern void clear_input(struct Packet* packet);

TbBool packets_process_cheats(
          PlayerNumber plyr_idx,
          MapCoord x, MapCoord y,
          struct Packet* pckt,
          MapSubtlCoord stl_x, MapSubtlCoord stl_y,
          MapSlabCoord slb_x, MapSlabCoord slb_y,
          short *influence_own_creatures)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct SlabMap *slb;
    struct Room* room;
    PowerKind pwkind;
    struct Thing *thing;
    unsigned short control_flags = pckt->control_flags;
    long i;

    switch (player->work_state)
    {
    case PSt_MkGoodDigger:
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {
            create_owned_special_digger(x, y, get_selected_player_for_cheat(game.hero_player_num));
            clear_input(pckt);
        }
        break;
    case PSt_MkGoodCreatr:
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {
            create_random_hero_creature(x, y, get_selected_player_for_cheat(game.hero_player_num), CREATURE_MAX_LEVEL);
            clear_input(pckt);
        }
        break;
    case PSt_MkGoldPot:
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {
            create_gold_pot_at(x, y, player->id_number);
            clear_input(pckt);
        }
        break;
    case PSt_OrderCreatr:
        *influence_own_creatures = 1;
        thing = get_creature_near(x, y);
        if (!thing_is_creature(thing))
            player->thing_under_hand = 0;
        else
            player->thing_under_hand = thing->index;
        if ((control_flags & PCtr_LBtnRelease) != 0)
        {
          if ((player->controlled_thing_idx > 0) && (player->controlled_thing_idx < THINGS_COUNT))
          {
            if ((control_flags & PCtr_MapCoordsValid) != 0)
            {
              thing = thing_get(player->controlled_thing_idx);
              if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
                  WARNLOG("Move %s order failed",thing_model_name(thing));
              thing->continue_state = CrSt_ManualControl;
            }
          } else
          {
            thing = get_creature_near(x, y);
            if (!thing_is_invalid(thing))
            {
                set_selected_creature(player, thing);
                initialise_thing_state(thing, CrSt_ManualControl);
                if (creature_is_group_member(thing)) {
                    make_group_member_leader(thing);
                }
            }
          }
          clear_input(pckt);
        }
        if ((control_flags & PCtr_RBtnRelease) != 0)
        {
          if ((player->controlled_thing_idx > 0) && (player->controlled_thing_idx < THINGS_COUNT))
          {
            thing = thing_get(player->controlled_thing_idx);
            set_start_state(thing);
            clear_selected_thing(player);
          }
          clear_input(pckt);
        }
        break;
    case PSt_MkBadCreatr:
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {
            create_random_evil_creature(x, y, get_selected_player_for_cheat(plyr_idx), CREATURE_MAX_LEVEL);
            clear_input(pckt);
        }
        break;
    case PSt_FreeDestroyWalls:
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {
            i = get_power_overcharge_level(player);
            magic_use_power_destroy_walls(plyr_idx, stl_x, stl_y, i, PwMod_CastForFree);
            clear_input(pckt);
        }
        break;
    case PSt_FreeTurnChicken:
    case PSt_FreeCastDisease:
        pwkind = player_state_to_power_kind[player->work_state];
        thing = get_creature_near_to_be_keeper_power_target(x, y, pwkind, plyr_idx);
        if (thing_is_invalid(thing))
        {
            player->thing_under_hand = 0;
            break;
        }
        player->thing_under_hand = thing->index;
        if ((control_flags & PCtr_LBtnRelease) != 0)
        {
            i = get_power_overcharge_level(player);
            switch (pwkind)
            {
            case PwrK_DISEASE:
                magic_use_power_disease(plyr_idx, thing, stl_x, stl_y, i, PwMod_CastForFree);
                break;
            case PwrK_CHICKEN:
                magic_use_power_chicken(plyr_idx, thing, stl_x, stl_y, i, PwMod_CastForFree);
                break;
            }
            clear_input(pckt);
        }
        break;
    case PSt_StealRoom:
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {          
            slb = get_slabmap_block(slb_x, slb_y);
            if (slb->room_index)
                {
                    room = room_get(slb->room_index);
                    i = get_selected_player_for_cheat(plyr_idx);
                    if (is_key_pressed(KC_RALT, KMod_DONTCARE))
                    {
                        play_non_3d_sample(116);
                        create_effects_on_room_slabs(room, imp_spangle_effects[i], 0, i);
                    }
                    {
                        take_over_room(room, i);
                    }
                }
            clear_input(pckt);
        }
        break;
    case PSt_DestroyRoom:
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {          
            slb = get_slabmap_block(slb_x, slb_y);
            if (slb->room_index)
                {
                    room = room_get(slb->room_index);
                    destroy_room_leaving_unclaimed_ground(room);
                }
            clear_input(pckt);
        }
        break;
    case PSt_KillCreatr:
        thing = get_creature_near(x, y);
        if (!thing_is_creature(thing))
        {
            player->thing_under_hand = 0;
        }
        else
        {
            player->thing_under_hand = thing->index;
        }
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (player->thing_under_hand > 0)
            {
                kill_creature(thing, INVALID_THING, -1, CrDed_NoUnconscious);
            }
            clear_input(pckt);    
        }
        break;
    case PSt_ConvertCreatr:
        thing = get_creature_near(x, y);
        if (!thing_is_creature(thing))
        {
            player->thing_under_hand = 0;
        }
        else
        {
            player->thing_under_hand = thing->index;
        }
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (player->thing_under_hand > 0)
            {
                change_creature_owner(thing, get_selected_player_for_cheat(plyr_idx));
            }
            clear_input(pckt);    
        }
        break;
    case PSt_StealSlab:
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {          
            slb = get_slabmap_block(slb_x, slb_y);
            if (slb->kind >= SlbT_EARTH && slb->kind <= SlbT_CLAIMED)
            {
                short slbkind;
                switch(slb->kind)
                {
                    case SlbT_PATH:
                    {
                        slbkind = SlbT_CLAIMED;
                        break;
                    }
                    case SlbT_EARTH:
                    {
                        slbkind = rand() % (5) + 4;
                        break;
                    }
                    case SlbT_TORCHDIRT:
                    {
                        slbkind = SlbT_WALLTORCH;
                        break;
                    }
                    default:
                    {
                        slbkind = slb->kind;
                        break;
                    }
                }
                i = get_selected_player_for_cheat(plyr_idx);
                if ((slbkind == SlbT_CLAIMED) || ((slbkind >= SlbT_WALLDRAPE) && (slbkind <= SlbT_WALLPAIRSHR)))
                {
                    if (is_key_pressed(KC_RALT, KMod_DONTCARE))
                    {
                        struct Coord3d pos;                    
                        if (slbkind == SlbT_CLAIMED)
                        {
                            pos.x.val = subtile_coord_center(slab_subtile_center(subtile_slab(stl_x)));
                            pos.y.val = subtile_coord_center(slab_subtile_center(subtile_slab(stl_y))); 
                            pos.z.val = subtile_coord_center(1);
                            play_non_3d_sample(76);
                            create_effect(&pos, imp_spangle_effects[i], i);
                        }
                        else
                        {
                            play_non_3d_sample(41);
                            for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
                            {
                                pos.x.stl.pos = 128;
                                pos.y.stl.pos = 128;
                                pos.z.stl.pos = 128;
                                pos.x.stl.num = stl_x + 2 * small_around[n].delta_x;
                                pos.y.stl.num = stl_y + 2 * small_around[n].delta_y;
                                struct Map* mapblk = get_map_block_at(pos.x.stl.num, pos.y.stl.num);
                                if (map_block_revealed(mapblk, i) && ((mapblk->flags & SlbAtFlg_Blocking) == 0))
                                {
                                    pos.z.val = get_floor_height_at(&pos);
                                    create_effect(&pos, imp_spangle_effects[i], i);
                                    // thing = thing_get(player->hand_thing_idx);
                                    // pos.z.val = get_thing_height_at(thing, &pos);   
                                }
                            }
                        }
                    }
                }
                place_slab_type_on_map(slbkind, stl_x, stl_y, i, 0);
                do_slab_efficiency_alteration(subtile_slab(stl_x), subtile_slab(stl_y));
                slb = get_slabmap_block(slb_x, slb_y);
                for (i = 0; i < PLAYERS_COUNT; i++)
                {
                    if (i != slabmap_owner(slb))
                    {
                        untag_blocks_for_digging_in_area(stl_x, stl_y, i);
                    }
                }
            }
            clear_input(pckt);
        }
        break;
    case PSt_LevelCreatureUp:
        thing = get_creature_near(x, y);
        if (!thing_is_creature(thing))
        {
            player->thing_under_hand = 0;
        }
        else
        {
            player->thing_under_hand = thing->index;
        }
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {
             if (player->thing_under_hand > 0)
             {
                creature_increase_level(thing);
             }
        clear_input(pckt);    
        }
        break;
    case PSt_KillPlayer:
          i = get_selected_player_for_cheat(-1);
          struct PlayerInfo* PlayerToKill = get_player(i);
          if (player_exists(PlayerToKill))
          {
              thing = get_player_soul_container(PlayerToKill->id_number);
              if (thing_is_dungeon_heart(thing))
              {
                    thing->health = -1;
              }
          }
        break;
    case PSt_HeartHealth:
        thing = get_player_soul_container(plyr_idx);
        if (is_key_pressed(KC_ADD, KMod_ALT))
        {
            if (thing_is_dungeon_heart(thing))
            {
                thing->health++;
            }
        }
        else if (is_key_pressed(KC_EQUALS, KMod_SHIFT))
        {
            if (thing_is_dungeon_heart(thing))
            {
                thing->health++;
            }
        }
        else if (is_key_pressed(KC_PERIOD, KMod_SHIFT))
        {
            if (thing_is_dungeon_heart(thing))
            {
                thing->health = thing->health + 100;
            }
        }
        else if (is_key_pressed(KC_COMMA, KMod_SHIFT))
        {
            if (thing_is_dungeon_heart(thing))
            {
                thing->health = thing->health - 100;
            }
        }
        else if (is_key_pressed(KC_SUBTRACT, KMod_ALT))
        {
            if (thing_is_dungeon_heart(thing))
            {
                thing->health--;
            }
        }
        else if (is_key_pressed(KC_MINUS, KMod_NONE))
        {
            if (thing_is_dungeon_heart(thing))
            {
                thing->health--;
            }
        }
        else if (is_key_pressed(KC_SLASH, KMod_SHIFT))
        {
            if (thing_is_dungeon_heart(thing))
            {
                char hhealth[5];
                itoa(thing->health, hhealth, 10);
                message_add(plyr_idx, hhealth);
                clear_key_pressed(KC_SLASH);
            }
        }
        break;
    case PSt_CreatrQueryAll:
        *influence_own_creatures = 1;
        thing = get_creature_near(x, y);
        if (thing_is_invalid(thing))
        {
            player->thing_under_hand = 0;
        }
        else
        {
            player->thing_under_hand = thing->index;
        }
        if ((control_flags & PCtr_LBtnRelease) != 0)
        {
            if (player->thing_under_hand > 0)
            {
                if (player->controlled_thing_idx != player->thing_under_hand)
                {
                    turn_off_all_panel_menus();
                    initialise_tab_tags_and_menu(GMnu_CREATURE_QUERY1);
                    turn_on_menu(GMnu_CREATURE_QUERY1);
                    player->influenced_thing_idx = player->thing_under_hand;
                    set_player_instance(player, PI_QueryCrtr, 0);
                }
            clear_input(pckt);
            }
        }
        break;
    case PSt_MkHappy:
    case PSt_MkAngry:
        *influence_own_creatures = 1;
        thing = get_creature_near(x, y);
        if (!thing_is_creature(thing))
        {
            player->thing_under_hand = 0;
        }
        else
        {
            player->thing_under_hand = thing->index;
        }
        if ((control_flags & PCtr_LBtnRelease) != 0)
        {
            if (player->thing_under_hand > 0)
            {
                if (player->work_state == PSt_MkHappy)
                {
                    anger_set_creature_anger_all_types(thing, 0);
                }
                else if (player->work_state == PSt_MkAngry)
                {
                    anger_set_creature_anger_all_types(thing, 10000);
                }
                clear_input(pckt);
            }
        }
        break;
    case PSt_PlaceTerrain:
        if (((control_flags & PCtr_LBtnRelease) != 0) && ((control_flags & PCtr_MapCoordsValid) != 0))
        {          
            slb = get_slabmap_block(slb_x, slb_y);
            short slbkind;
            char s[3];
            if (is_key_pressed(KC_SLASH, KMod_NONE))
            {
                 itoa(slb->kind, s, 10);
                 message_add(plyr_idx, s);
                 clear_key_pressed(KC_SLASH);
            }
            else if (is_key_pressed(KC_SLASH, KMod_SHIFT))
            {
                 itoa(slabmap_owner(slb), s, 10);
                 message_add(plyr_idx, s);
                 clear_key_pressed(KC_SLASH);
            }
            else if (is_key_pressed(KC_X, KMod_NONE))
            {
                 itoa(stl_x, s, 10);
                 message_add(plyr_idx, s);
                 clear_key_pressed(KC_X);
            }
            else if (is_key_pressed(KC_Y, KMod_NONE))
            {
                 itoa(stl_y, s, 10);
                 message_add(plyr_idx, s);
                 clear_key_pressed(KC_Y);
            }
            else if (is_key_pressed(KC_X, KMod_SHIFT))
            {
                 itoa(slb_x, s, 10);
                 message_add(plyr_idx, s);
                 clear_key_pressed(KC_X);
            }
            else if (is_key_pressed(KC_Y, KMod_SHIFT))
            {
                 itoa(slb_y, s, 10);
                 message_add(plyr_idx, s);
                 clear_key_pressed(KC_Y);
            }
            else if (is_key_pressed(KC_N, KMod_NONE))
            {
                 itoa(get_slab_number(subtile_slab(stl_x), subtile_slab(stl_y)), s, 10);
                 message_add(plyr_idx, s);
                 clear_key_pressed(KC_N);
            }
            else
            {
                if (is_key_pressed(KC_NUMPAD0, KMod_NONE))
                {
                    slbkind = SlbT_ROCK;
                    clear_key_pressed(KC_NUMPAD0);
                }
                else if (is_key_pressed(KC_NUMPAD1, KMod_NONE))
                {
                    slbkind = SlbT_GOLD;
                    clear_key_pressed(KC_NUMPAD1);
                }
                else if (is_key_pressed(KC_NUMPAD2, KMod_NONE))
                {
                    slbkind = SlbT_GEMS;
                    clear_key_pressed(KC_NUMPAD2);
                }
                else if (is_key_pressed(KC_NUMPAD3, KMod_NONE))
                {
                    slbkind = SlbT_EARTH;
                    clear_key_pressed(KC_NUMPAD3);
                }
                else if (is_key_pressed(KC_NUMPAD4, KMod_NONE))
                {
                    slbkind = SlbT_TORCHDIRT;
                    clear_key_pressed(KC_NUMPAD4);
                }
                else if (is_key_pressed(KC_NUMPAD5, KMod_NONE))
                {
                    slbkind = SlbT_PATH;
                    clear_key_pressed(KC_NUMPAD5);
                }
                else if (is_key_pressed(KC_NUMPAD6, KMod_NONE))
                {
                    slbkind = SlbT_CLAIMED;
                    clear_key_pressed(KC_NUMPAD6);
                }
                else if (is_key_pressed(KC_NUMPAD7, KMod_NONE))
                {
                    slbkind = SlbT_LAVA;
                    clear_key_pressed(KC_NUMPAD7);
                }
                else if (is_key_pressed(KC_NUMPAD8, KMod_NONE))
                {
                    slbkind = SlbT_WATER;
                    clear_key_pressed(KC_NUMPAD8);
                }
                else if (is_key_pressed(KC_NUMPAD9, KMod_NONE))
                {
                    slbkind = rand() % (5) + 4;
                    clear_key_pressed(KC_NUMPAD9);
                }
                else
                {
                    slbkind = 0;
                }
                if (subtile_is_room(stl_x, stl_y)) 
                {
                    room = subtile_room_get(stl_x, stl_y);
                    delete_room_slab(slb_x, slb_y, true);
                }
                if (slab_kind_is_animated(slbkind))
                {
                    place_animating_slab_type_on_map(slbkind, 0, stl_x, stl_y, game.neutral_player_num);  
                }
                else
                {
                    place_slab_type_on_map(slbkind, stl_x, stl_y, game.neutral_player_num, 0);
                }
                do_slab_efficiency_alteration(subtile_slab(stl_x), subtile_slab(stl_y));
            }
        }
        clear_input(pckt);
        break;
    default:
        return false;
    }
    return true;
}