/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets_cheats.c
 *     Packet processing routines.
 * @par Purpose:
 *     Functions for creating and executing packets.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     30 Jan 2009 - 10 Mar 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "packets.h"
#include "player_data.h"
#include "player_states.h"
#include "thing_creature.h"
#include "player_utils.h"
#include "game_legacy.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "player_instances.h"
#include "creature_states.h"
#include "kjm_input.h"
#include "bflib_sound.h"
#include "thing_effects.h"
#include "config_effects.h"
#include "map_utils.h"
#include "map_blocks.h"
#include "magic.h"
#include "keeperfx.hpp"
#include "gui_frontmenu.h"
#include "frontend.h"
#include "room_util.h"
#include "room_workshop.h"
#include "cursor_tag.h"
#include "gui_boxmenu.h"
#include "front_input.h"
#include "bflib_math.h"
#include "gui_topmsg.h"

extern void clear_input(struct Packet* packet);

/******************************************************************************/
TbBool terrain_details = false;
/******************************************************************************/

TbBool packets_process_cheats(
        PlayerNumber plyr_idx,
        MapCoord x, MapCoord y,
        struct Packet* pckt,
        MapSubtlCoord stl_x, MapSubtlCoord stl_y,
        MapSlabCoord slb_x, MapSlabCoord slb_y,
        short *influence_own_creatures)
{
    struct Thing *thing;
    struct Room* room = NULL;
    int i;
    PowerKind pwkind;
    struct SlabMap *slb;
    struct PlayerInfo* player = get_player(plyr_idx);
    TbBool allowed;
    char str[255] = {'\0'};
    struct PlayerInfoAdd *playeradd = get_playeradd(plyr_idx);
    switch (player->work_state)
    {
        case PSt_MkDigger:
        allowed = tag_cursor_blocks_place_thing(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(playeradd->cheatselection.chosen_player);
        targeted_message_add(playeradd->cheatselection.chosen_player, plyr_idx, 1, "%d", playeradd->cheatselection.chosen_experience_level + 1);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                set_packet_action(pckt, PckA_CheatMakeDigger, playeradd->cheatselection.chosen_player, playeradd->cheatselection.chosen_experience_level, 0, 0);
            }
            else
            {
                if (is_my_player(player))
                {
                    play_non_3d_sample(119);
                }
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
        case PSt_MkGoodCreatr:
        allowed = tag_cursor_blocks_place_thing(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(playeradd->cheatselection.chosen_player);
        if (playeradd->cheatselection.chosen_hero_kind == 0)
        {
            sprintf(str, "?");
        }
        else
        {
            struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[playeradd->cheatselection.chosen_hero_kind];
            sprintf(str, "%s %d", get_string(crconf->namestr_idx), playeradd->cheatselection.chosen_experience_level + 1);
        }
        targeted_message_add(playeradd->cheatselection.chosen_player, plyr_idx, 1, "%s", str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                ThingModel crmodel;
                unsigned char exp;
                if (playeradd->cheatselection.chosen_hero_kind == 0)
                {
                    while (1) 
                    {
                        crmodel = GAME_RANDOM(gameadd.crtr_conf.model_count) + 1;
                        if (crmodel >= gameadd.crtr_conf.model_count)
                        {
                            continue;
                        }
                        struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[crmodel];
                        if ((crconf->model_flags & CMF_IsSpectator) != 0) 
                        {
                            continue;
                        }
                        if ((crconf->model_flags & CMF_IsEvil) == 0)
                        {
                            break;
                        }
                    }
                    exp = GAME_RANDOM(CREATURE_MAX_LEVEL);                    
                }
                else
                {
                    crmodel = playeradd->cheatselection.chosen_hero_kind;
                    exp = playeradd->cheatselection.chosen_experience_level;
                }
                unsigned short param2 = playeradd->cheatselection.chosen_player | (exp << 8);
                set_packet_action(pckt, PckA_CheatMakeCreature, crmodel, param2, 0, 0);
            }
            else
            {
                if (is_my_player(player))
                {
                    play_non_3d_sample(119);
                }
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
        case PSt_MkGoldPot:
        allowed = tag_cursor_blocks_place_thing(plyr_idx, stl_x, stl_y);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                thing = create_gold_pot_at(x, y, player->id_number);
                if (!thing_is_invalid(thing))
                {
                    if (thing_in_wall_at(thing, &thing->mappos))
                    {
                        move_creature_to_nearest_valid_position(thing);
                    }
                    room = subtile_room_get(stl_x, stl_y);
                    if (room_exists(room))
                    {
                        if (room->kind == RoK_TREASURE)
                        {
                            count_gold_hoardes_in_room(room);
                        }
                    }
                }
            }
            else
            {
                if (is_my_player(player))
                {
                    play_non_3d_sample(119);
                }
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
        case PSt_OrderCreatr:
        *influence_own_creatures = 1;
        thing = get_creature_near(x, y);
        if (!thing_is_creature(thing))
            player->thing_under_hand = 0;
        else
            player->thing_under_hand = thing->index;
        thing = thing_get(player->controlled_thing_idx);
        if (thing_is_creature(thing))
        {
            allowed = tag_cursor_blocks_order_creature(plyr_idx, stl_x, stl_y, thing);
        }
        else
        {
            allowed = false;
        }
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
          if (player->thing_under_hand > 0)
          {
            if (player->controlled_thing_idx != player->thing_under_hand)
            {
                player->influenced_thing_idx = player->thing_under_hand;
            }
          }
          if ((player->controlled_thing_idx > 0) && (player->controlled_thing_idx < THINGS_COUNT))
          {
            if ( (stl_x == thing->mappos.x.stl.num) && (stl_y == thing->mappos.y.stl.num) )
            {
                set_start_state(thing);
                clear_selected_thing(player);
            }
            else if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
            {
              if (allowed)
              {
                if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
                    WARNLOG("Move %s order failed",thing_model_name(thing));
                thing->continue_state = CrSt_ManualControl;
              }
              else
              {
                if (is_my_player(player))
                {
                    play_non_3d_sample(119);
                }
              }
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
          unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
        case PSt_MkBadCreatr:
        allowed = tag_cursor_blocks_place_thing(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(playeradd->cheatselection.chosen_player);
        if (playeradd->cheatselection.chosen_creature_kind == 0)
        {
            sprintf(str, "?");
        }
        else
        {
            struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[playeradd->cheatselection.chosen_creature_kind + 13];
            sprintf(str, "%s %d", get_string(crconf->namestr_idx), playeradd->cheatselection.chosen_experience_level + 1);
        }
        targeted_message_add(playeradd->cheatselection.chosen_player, plyr_idx, 1, "%s", str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                ThingModel crmodel;
                unsigned char exp;
                if (playeradd->cheatselection.chosen_creature_kind == 0)
                {
                    while (1)
                    {
                        crmodel = GAME_RANDOM(gameadd.crtr_conf.model_count) + 1;
                        struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[crmodel];
                        if ((crconf->model_flags & CMF_IsSpectator) != 0) {
                            continue;
                        }
                        if ((crconf->model_flags & CMF_IsEvil) != 0) {
                            break;
                        }
                    }
                    exp = GAME_RANDOM(CREATURE_MAX_LEVEL);                    
                }
                else
                {
                    crmodel = playeradd->cheatselection.chosen_creature_kind + 13;
                    exp = playeradd->cheatselection.chosen_experience_level;
                }
                unsigned short param2 = playeradd->cheatselection.chosen_player | (exp << 8);
                set_packet_action(pckt, PckA_CheatMakeCreature, crmodel, param2, 0, 0);
            }
            else
            {
                if (is_my_player(player))
                {
                    play_non_3d_sample(119);
                }
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
        case PSt_FreeDestroyWalls:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                i = get_power_overcharge_level(player);
                magic_use_power_destroy_walls(plyr_idx, stl_x, stl_y, i, PwMod_CastForFree);
                unset_packet_control(pckt, PCtr_LBtnRelease);
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
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
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
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_StealRoom:
        clear_messages_from_player(playeradd->cheatselection.chosen_player);
        slb = get_slabmap_block(slb_x, slb_y);
        room = room_get(slb->room_index);
        allowed = ( (room_exists(room)) && (room->owner != playeradd->cheatselection.chosen_player) );
        if (allowed)
        {
            sprintf(str, get_string(419));
        }
        targeted_message_add(playeradd->cheatselection.chosen_player, plyr_idx, 1, str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {    
            if (allowed)
            {
                TbBool effect = (is_key_pressed(KC_RALT, KMod_DONTCARE));
                set_packet_action(pckt, PckA_CheatStealRoom, playeradd->cheatselection.chosen_player, effect, 0, 0);
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
        case PSt_DestroyRoom:
        clear_messages_from_player(-127);
        slb = get_slabmap_block(slb_x, slb_y);
        room = room_get(slb->room_index);
        allowed = (room_exists(room));
        if (allowed)
        {
            targeted_message_add(-127, plyr_idx, 1, get_string(419));
        }
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {          
            if (allowed)
            {
                destroy_room_leaving_unclaimed_ground(room);
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
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
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                if (player->thing_under_hand > 0)
                {
                    kill_creature(thing, INVALID_THING, -1, CrDed_NoUnconscious);
                }
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_ConvertCreatr:
        clear_messages_from_player(playeradd->cheatselection.chosen_player);
        targeted_message_add(playeradd->cheatselection.chosen_player, plyr_idx, 1, str);
        thing = get_creature_near(x, y);
        if ((!thing_is_creature(thing)) || (thing->owner == playeradd->cheatselection.chosen_player))
        {
            player->thing_under_hand = 0;
        }
        else
        {
            player->thing_under_hand = thing->index;
        }
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            set_packet_action(pckt, PckA_CheatConvertCreature, playeradd->cheatselection.chosen_player, 0, 0, 0);
            unset_packet_control(pckt, PCtr_LBtnRelease);    
        }
        break;
        case PSt_StealSlab:
        allowed = tag_cursor_blocks_steal_slab(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(playeradd->cheatselection.chosen_player);
        targeted_message_add(playeradd->cheatselection.chosen_player, plyr_idx, 1, str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                slb = get_slabmap_block(slb_x, slb_y);
                if (slb->kind >= SlbT_EARTH && slb->kind <= SlbT_CLAIMED)
                {
                    SlabKind slbkind;
                    switch(slb->kind)
                    {
                        case SlbT_PATH:
                        {
                            slbkind = SlbT_CLAIMED;
                            break;
                        }
                        case SlbT_EARTH:
                        {
                            if (is_key_pressed(KC_RSHIFT, KMod_DONTCARE))
                            {
                                slbkind = choose_pretty_type(playeradd->cheatselection.chosen_player, slb_x, slb_y);
                            }
                            else
                            {
                                slbkind = SlbT_WALLDRAPE + GAME_RANDOM(5);
                            }
                            break;
                        }
                        case SlbT_TORCHDIRT:
                        {
                            if (is_key_pressed(KC_RSHIFT, KMod_DONTCARE))
                            {
                                slbkind = choose_pretty_type(playeradd->cheatselection.chosen_player, slb_x, slb_y);
                            }
                            else
                            {
                                slbkind = SlbT_WALLTORCH;
                            }
                            break;
                        }
                        default:
                        {
                            slbkind = slb->kind;
                            break;
                        }
                    }
                    TbBool effect;
                    if ((slbkind == SlbT_CLAIMED) || ((slbkind >= SlbT_WALLDRAPE) && (slbkind <= SlbT_WALLPAIRSHR)))
                    {
                        effect = (is_key_pressed(KC_RALT, KMod_DONTCARE));
                    }
                    else
                    {
                        effect = false;
                    }
                    unsigned short param2 = playeradd->cheatselection.chosen_player | (effect << 8);
                    set_packet_action(pckt, PckA_CheatStealSlab, slbkind, param2, 0, 0);
                }
            }
            else
            {
                if (is_my_player(player))
                {
                    play_non_3d_sample(119);
                }
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
        case PSt_LevelCreatureUp:
        case PSt_LevelCreatureDown:
            thing = get_creature_near(x, y);
            if (!thing_is_creature(thing))
            {
                player->thing_under_hand = 0;
            }
            else
            {
                player->thing_under_hand = thing->index;
            }
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                if (player->thing_under_hand > 0)
                {
                    switch (player->work_state)
                    {
                        case PSt_LevelCreatureUp:
                        {
                            creature_increase_level(thing);
                            break;
                        }
                        case PSt_LevelCreatureDown:
                        {
                            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
                            if (!creature_control_invalid(cctrl))
                            {
                                set_creature_level(thing, cctrl->explevel-1);
                            }
                            break;
                        }
                    }
                }
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_KillPlayer:
          clear_messages_from_player(playeradd->cheatselection.chosen_player);
          struct PlayerInfo* PlayerToKill = get_player(playeradd->cheatselection.chosen_player);
          if (player_exists(PlayerToKill))
          {
              targeted_message_add(playeradd->cheatselection.chosen_player, plyr_idx, 1, str);
              if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
              {
                set_packet_action(pckt, PckA_CheatKillPlayer, PlayerToKill->id_number, 0, 0, 0);
                unset_packet_control(pckt, PCtr_LBtnRelease);
              }
          }
        break;
        case PSt_HeartHealth:
        clear_messages_from_player(playeradd->cheatselection.chosen_player);
        thing = get_player_soul_container(playeradd->cheatselection.chosen_player);
        if (!thing_is_invalid(thing))
        {
            targeted_message_add(thing->owner, plyr_idx, 1, "%d/%d", thing->health, game.dungeon_heart_health);
        }
        else
        {
            break;
        }
        short new_health = thing->health;
        if (process_cheat_heart_health_inputs(&new_health))
        {
            set_packet_action(pckt, PckA_CheatHeartHealth, playeradd->cheatselection.chosen_player, new_health, 0, 0);
        }
        break;
        case PSt_CreatrQueryAll:
        case PSt_CreatrInfoAll:
            *influence_own_creatures = 1;
            thing = get_creature_near(x, y);
            TbBool CanQuery = false;
            if (thing_is_creature(thing))
            {
                CanQuery = true;
            }
            else
            {
                thing = get_nearest_thing_at_position(stl_x, stl_y);
                CanQuery = (!thing_is_invalid(thing));
                if (!CanQuery)
                {
                    room = subtile_room_get(stl_x, stl_y);
                }
            }
            if (!CanQuery)
            {
                player->thing_under_hand = 0;
            }
            else
            {
                player->thing_under_hand = thing->index;
            }
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                if (player->thing_under_hand > 0)
                {
                    if (thing->class_id == TCls_Creature)
                    {
                        if (player->controlled_thing_idx != player->thing_under_hand)
                        {
                            if (is_my_player(player))
                            {
                                turn_off_all_panel_menus();
                                initialise_tab_tags_and_menu(GMnu_CREATURE_QUERY1);
                                turn_on_menu(GMnu_CREATURE_QUERY1);
                            }
                            player->influenced_thing_idx = player->thing_under_hand;
                            set_player_instance(player, PI_QueryCrtr, 0);
                        }
                    }
                    else
                    {
                        query_thing(thing);
                    }
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
                else if (room_exists(room) )
                {
                    query_room(room);
                }
            }
            if ( player->work_state == PSt_CreatrInfoAll )
            {
                thing = thing_get(player->controlled_thing_idx);
                if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
                {
                    if (is_my_player(player))
                    {
                        turn_off_query_menus();
                        turn_on_main_panel_menu();
                    }
                    set_player_instance(player, PI_UnqueryCrtr, 0);
                    unset_packet_control(pckt, PCtr_RBtnRelease);
                } else
                if (creature_is_dying(thing))
                {
                    set_player_instance(player, PI_UnqueryCrtr, 0);
                    if (is_my_player(player))
                    {
                        turn_off_query_menus();
                        turn_on_main_panel_menu();
                    }
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
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
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
                    unset_packet_control(pckt, PCtr_LBtnRelease);
                }
            }
            break;
        case PSt_PlaceTerrain:
        {
            tag_cursor_blocks_place_terrain(plyr_idx, stl_x, stl_y);
            struct SlabConfigStats* slab_cfgstats;
            clear_messages_from_player(playeradd->cheatselection.chosen_player);
            struct SlabAttr *slbattr = get_slab_kind_attrs(playeradd->cheatselection.chosen_terrain_kind);
            if (slab_kind_has_no_ownership(playeradd->cheatselection.chosen_terrain_kind))
            {
                playeradd->cheatselection.chosen_player = game.neutral_player_num;
            }
            if (slbattr->tooltip_stridx <= GUI_STRINGS_COUNT)
            {
                const char* msg = get_string(slbattr->tooltip_stridx);
                strcpy(str, msg);
                char* dis_msg = strtok(str, ":");
                targeted_message_add(playeradd->cheatselection.chosen_player, plyr_idx, 1, dis_msg);
            }
            else
            {
                slab_cfgstats = get_slab_kind_stats(playeradd->cheatselection.chosen_terrain_kind);
                targeted_message_add(playeradd->cheatselection.chosen_player, plyr_idx, 1, slab_cfgstats->code_name);            
            }
            clear_messages_from_player(-127);
            if (is_key_pressed(KC_RSHIFT, KMod_DONTCARE))
            {
                terrain_details ^= 1;
                clear_key_pressed(KC_RSHIFT);
            }
            if (terrain_details)
            {
                slb = get_slabmap_block(slb_x, slb_y);
                slab_cfgstats = get_slab_kind_stats(slb->kind);
                targeted_message_add(-127, plyr_idx, 1, "%s (%d) %d %d (%d) %d %d (%d)", slab_cfgstats->code_name, slabmap_owner(slb), slb_x, slb_y, get_slab_number(slb_x, slb_y), stl_x, stl_y, get_subtile_number(stl_x, stl_y));
            }        
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {          
                if (subtile_is_room(stl_x, stl_y)) 
                {
                    room = subtile_room_get(stl_x, stl_y);
                    delete_room_slab(slb_x, slb_y, true);
                }
                PlayerNumber id;
                if ( (playeradd->cheatselection.chosen_terrain_kind == SlbT_CLAIMED) || ( (playeradd->cheatselection.chosen_terrain_kind >= SlbT_WALLDRAPE) && (playeradd->cheatselection.chosen_terrain_kind <= SlbT_DAMAGEDWALL) ) )
                {
                    slb = get_slabmap_block(slb_x, slb_y);
                    if ( (slb->kind == SlbT_CLAIMED) || ( (slb->kind >= SlbT_WALLDRAPE) && (slb->kind <= SlbT_DAMAGEDWALL) ) )
                    {
                        id = slabmap_owner(slb);
                    }
                    else
                    {
                        id = playeradd->cheatselection.chosen_player;
                    }
                }
                else
                {
                    id = game.neutral_player_num;
                }
                set_packet_action(pckt, PckA_CheatPlaceTerrain, playeradd->cheatselection.chosen_terrain_kind, id, 0, 0);
                if ( (playeradd->cheatselection.chosen_terrain_kind >= SlbT_WALLDRAPE) && (playeradd->cheatselection.chosen_terrain_kind <= SlbT_WALLPAIRSHR) )
                {
                    playeradd->cheatselection.chosen_terrain_kind = SlbT_WALLDRAPE + GAME_RANDOM(5);
                }
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
            break;
        }
        case PSt_DestroyThing:
            thing = get_nearest_thing_at_position(stl_x, stl_y);
            if (thing_is_invalid(thing))
            {
                player->thing_under_hand = 0;
            }
            else
            {
                player->thing_under_hand = thing->index;
            }
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                if (player->thing_under_hand > 0)
                {
                    room = get_room_thing_is_on(thing);
                    TbBool IsRoom = (!room_is_invalid(room));
                    switch(thing->class_id)
                    {
                        case TCls_Door:
                        {
                            destroy_door(thing);
                            break;
                        }
                        case TCls_Effect:
                        {
                            destroy_effect_thing(thing);
                            break;
                        }
                        default:
                        {
                            if (thing_is_spellbook(thing))
                            {
                                if (!is_neutral_thing(thing))
                                {
                                    remove_power_from_player(book_thing_to_power_kind(thing), thing->owner);
                                }
                            }
                            else if (thing_is_workshop_crate(thing))
                            {
                                if (!is_neutral_thing(thing))
                                {
                                    ThingClass tngclass = crate_thing_to_workshop_item_class(thing);
                                    ThingModel tngmodel = crate_thing_to_workshop_item_model(thing);
                                    if (IsRoom)
                                    {
                                        remove_workshop_item_from_amount_stored(thing->owner, tngclass, tngmodel, WrkCrtF_NoOffmap);
                                    }
                                    remove_workshop_item_from_amount_placeable(thing->owner, tngclass, tngmodel);
                                }
                            }
                            destroy_object(thing);
                            break;
                        }
                    }
                    if (IsRoom)
                    {
                        update_room_contents(room);
                    }
                }
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        default:
            return false;
    }
    return true;
}

TbBool process_players_global_cheats_packet_action(PlayerNumber plyr_idx, struct Packet* pckt)
{
  struct PlayerInfoAdd* playeradd;
  switch (pckt->action)
  {
      case PckA_CheatEnter:
    //      game.???[my_player_number].cheat_mode = 1;
          show_onscreen_msg(2*game.num_fps, "Cheat mode activated by player %d", plyr_idx);
          return true;
      case PckA_CheatAllFree:
          make_all_creatures_free();
          make_all_rooms_free();
          make_all_powers_cost_free();
          return true;
      case PckA_CheatCrtSpells:
          //TODO: remake from beta
          return false;
      case PckA_CheatRevealMap:
      {
          struct PlayerInfo* player = get_player(plyr_idx);
          reveal_whole_map(player);
          return false;
      }
      case PckA_CheatCrAllSpls:
          //TODO: remake from beta
          return false;
      case PckA_CheatAllMagic:
          make_available_all_researchable_powers(plyr_idx);
          return false;
      case PckA_CheatAllRooms:
          make_available_all_researchable_rooms(plyr_idx);
          return false;
      case PckA_CheatAllResrchbl:
          make_all_powers_researchable(plyr_idx);
          make_all_rooms_researchable(plyr_idx);
          return false;
      case PckA_CheatSwitchTerrain:
        {
            playeradd = get_playeradd(plyr_idx);
            playeradd->cheatselection.chosen_terrain_kind = pckt->actn_par1;
            if (slab_kind_has_no_ownership(playeradd->cheatselection.chosen_terrain_kind))
            {
               clear_messages_from_player(playeradd->cheatselection.chosen_player);
               playeradd->cheatselection.chosen_player = game.neutral_player_num; 
            }
            return false;
        }
      case PckA_CheatSwitchPlayer:
        {
            playeradd = get_playeradd(plyr_idx);
            clear_messages_from_player(playeradd->cheatselection.chosen_player);
            playeradd->cheatselection.chosen_player = pckt->actn_par1;
            return false;
        }
      case PckA_CheatSwitchCreature:
        {
            playeradd = get_playeradd(plyr_idx);
            playeradd->cheatselection.chosen_creature_kind = pckt->actn_par1;
            return false;
        }
      case PckA_CheatSwitchHero:
        {
            playeradd = get_playeradd(plyr_idx);
            playeradd->cheatselection.chosen_hero_kind = pckt->actn_par1;
            return false;
        }
      case PckA_CheatSwitchExperience:
        {
            playeradd = get_playeradd(plyr_idx);
            playeradd->cheatselection.chosen_experience_level = pckt->actn_par1;
            return false;
        }
        default:
          return false;
  }
}

TbBool process_players_dungeon_control_cheats_packet_action(PlayerNumber plyr_idx, struct Packet* pckt)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    MapCoord x, y;
    struct Thing* thing;
    MapSubtlCoord stl_x, stl_y;
    MapSlabCoord slb_x, slb_y;
    struct Coord3d pos;
    switch (pckt->action)
    {
        case PckA_CheatPlaceTerrain:
        {
            x = ((unsigned short)pckt->pos_x);
            y = ((unsigned short)pckt->pos_y);
            stl_x = coord_subtile(x);
            stl_y = coord_subtile(y);
            slb_x = subtile_slab_fast(stl_x);
            slb_y = subtile_slab_fast(stl_y);
            if (slab_kind_is_animated(pckt->actn_par1))
            {
                place_animating_slab_type_on_map(pckt->actn_par1, 0, stl_x, stl_y, pckt->actn_par2);
            }
            else
            {
                place_slab_type_on_map(pckt->actn_par1, stl_x, stl_y, pckt->actn_par2, 0);
            }
            do_slab_efficiency_alteration(slb_x, slb_y);
            break;
        }
        case PckA_CheatMakeCreature:
        {
            x = ((unsigned short)pckt->pos_x);
            y = ((unsigned short)pckt->pos_y);
            pos.x.val = x;
            pos.y.val = y;
            PlayerNumber id = pckt->actn_par2;
            unsigned char exp = pckt->actn_par2 >> 8;
            thing = create_creature(&pos, pckt->actn_par1, id);
            if (!thing_is_invalid(thing))
            {
                set_creature_level(thing, exp);
            }
            break;
        }
        case PckA_CheatMakeDigger:
        {
            x = ((unsigned short)pckt->pos_x);
            y = ((unsigned short)pckt->pos_y);
            thing = create_owned_special_digger(x, y, pckt->actn_par1);
            if (!thing_is_invalid(thing))
            {
                set_creature_level(thing, pckt->actn_par2);
            }
            break;
        }
        case PckA_CheatStealSlab:
        {
            x = ((unsigned short)pckt->pos_x);
            y = ((unsigned short)pckt->pos_y);
            stl_x = coord_subtile(x);
            stl_y = coord_subtile(y);
            slb_x = subtile_slab_fast(stl_x);
            slb_y = subtile_slab_fast(stl_y);
            PlayerNumber id = pckt->actn_par2;
            TbBool effect = pckt->actn_par2 >> 8;
            if (effect)
            {
                if (pckt->actn_par1 == SlbT_CLAIMED)
                {
                    pos.x.val = subtile_coord_center(slab_subtile_center(subtile_slab(stl_x)));
                    pos.y.val = subtile_coord_center(slab_subtile_center(subtile_slab(stl_y))); 
                    pos.z.val = subtile_coord_center(1);
                    if (is_my_player(player))
                    {
                        play_non_3d_sample(76);
                    }
                    create_effect(&pos, imp_spangle_effects[id], id);
                }
                else
                {
                    if (is_my_player(player))
                    {
                        play_non_3d_sample(41);
                    }
                    for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
                    {
                        pos.x.stl.pos = 128;
                        pos.y.stl.pos = 128;
                        pos.z.stl.pos = 128;
                        pos.x.stl.num = stl_x + 2 * small_around[n].delta_x;
                        pos.y.stl.num = stl_y + 2 * small_around[n].delta_y;
                        struct Map* mapblk = get_map_block_at(pos.x.stl.num, pos.y.stl.num);
                        if (map_block_revealed(mapblk, id) && ((mapblk->flags & SlbAtFlg_Blocking) == 0))
                        {
                            pos.z.val = get_floor_height_at(&pos);
                            create_effect(&pos, imp_spangle_effects[id], id);  
                        }
                    }
                }
            }
            place_slab_type_on_map(pckt->actn_par1, stl_x, stl_y, id, 0);
            do_slab_efficiency_alteration(slb_x, slb_y);
            struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
            for (int i = 0; i < PLAYERS_COUNT; i++)
            {
                if (i != slabmap_owner(slb))
                {
                    untag_blocks_for_digging_in_area(stl_x, stl_y, i);
                }
            }
            break;
        }
        case PckA_CheatStealRoom:
        {
            x = ((unsigned short)pckt->pos_x);
            y = ((unsigned short)pckt->pos_y);
            stl_x = coord_subtile(x);
            stl_y = coord_subtile(y);
            struct Room* room = subtile_room_get(stl_x, stl_y);
            if (room_exists(room))
            {
                if (pckt->actn_par2)
                {
                    if (is_my_player(player))
                    {
                        play_non_3d_sample(116);
                    }
                    create_effects_on_room_slabs(room, imp_spangle_effects[pckt->actn_par1], 0, pckt->actn_par1);
                }
                take_over_room(room, pckt->actn_par1);
            }
            break;
        }
        case PckA_CheatHeartHealth:
        {
            thing = get_player_soul_container(pckt->actn_par1);
            if (!thing_is_invalid(thing))
            {
                thing->health = (short)pckt->actn_par2;
            }
            break;
        }
        case PckA_CheatKillPlayer:
        {
            thing = get_player_soul_container(pckt->actn_par1);
            if (!thing_is_invalid(thing))
            {
                thing->health = 0;
            }
            break;
        }
        case PckA_CheatConvertCreature:
        {
            thing = thing_get(player->thing_under_hand);
            if (thing_is_creature(thing))
            {
                change_creature_owner(thing, pckt->actn_par1);
            }
            break;
        }
        default:
            return false;
        }
    return true;
}