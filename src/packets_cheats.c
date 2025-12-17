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
#include "pre_inc.h"
#include "packets.h"
#include "player_data.h"
#include "config_players.h"
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
#include "magic_powers.h"
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
#include "gui_msgs.h"
#include "frontmenu_ingame_tabs.h"
#include "room_treasure.h"
#include "post_inc.h"

extern void clear_input(struct Packet* packet);

/******************************************************************************/
TbBool terrain_details = false;
/******************************************************************************/

TbBool packets_process_cheats(
        PlayerNumber plyr_idx,
        MapCoord x, MapCoord y,
        struct Packet* pckt,
        MapSubtlCoord stl_x, MapSubtlCoord stl_y,
        MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Thing *thing;
    struct Room* room = NULL;
    int i;
    PowerKind pwkind;
    struct SlabMap *slb;
    struct PlayerInfo* player = get_player(plyr_idx);
    TbBool allowed;
    char str[255] = "";
    switch (player->work_state)
    {
        case PSt_MkDigger:
        player->render_roomspace = create_box_roomspace(player->render_roomspace, 1, 1, slb_x, slb_y);
        allowed = tag_cursor_blocks_place_thing(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
        targeted_message_add(MsgType_Player, player->cheatselection.chosen_player, plyr_idx, 1, "%d", player->cheatselection.chosen_experience_level + 1);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                set_packet_action(pckt, PckA_CheatMakeDigger, player->cheatselection.chosen_player, player->cheatselection.chosen_experience_level, 0, 0);
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
        player->render_roomspace = create_box_roomspace(player->render_roomspace, 1, 1, slb_x, slb_y);
        allowed = tag_cursor_blocks_place_thing(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
        if (player->cheatselection.chosen_hero_kind == 0)
        {
            snprintf(str, sizeof(str), "?");
        }
        else
        {
            struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[player->cheatselection.chosen_hero_kind];
            snprintf(str, sizeof(str), "%s %d", get_string(crconf->namestr_idx), player->cheatselection.chosen_experience_level + 1);
        }
        targeted_message_add(MsgType_Player, player->cheatselection.chosen_player, plyr_idx, 1, "%s", str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                ThingModel crmodel;
                unsigned char exp;
                if (player->cheatselection.chosen_hero_kind == 0)
                {
                    while (1)
                    {
                        crmodel = GAME_RANDOM(game.conf.crtr_conf.model_count) + 1;
                        if (crmodel >= game.conf.crtr_conf.model_count)
                        {
                            continue;
                        }
                        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[crmodel];
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
                    crmodel = player->cheatselection.chosen_hero_kind;
                    exp = player->cheatselection.chosen_experience_level;
                }
                unsigned short param2 = player->cheatselection.chosen_player | (exp << 8);
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
        player->render_roomspace = create_box_roomspace(player->render_roomspace, 1, 1, slb_x, slb_y);
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
                        if (room_role_matches(room->kind,RoRoF_GoldStorage))
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
        thing = get_creature_near(x, y);
        if (!thing_is_creature(thing))
            player->thing_under_hand = 0;
        else
            player->thing_under_hand = thing->index;
        thing = thing_get(player->controlled_thing_idx);
        if (thing_is_creature(thing))
        {
            player->render_roomspace = create_box_roomspace(player->render_roomspace, 1, 1, slb_x, slb_y);
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
                player->influenced_thing_creation = thing->creation_turn;
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
        player->render_roomspace = create_box_roomspace(player->render_roomspace, 1, 1, slb_x, slb_y);
        allowed = tag_cursor_blocks_place_thing(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
        if (player->cheatselection.chosen_creature_kind == 0)
        {
            snprintf(str, sizeof(str), "?");
        }
        else
        {
            struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[player->cheatselection.chosen_creature_kind];
            snprintf(str, sizeof(str), "%s %d", get_string(crconf->namestr_idx), player->cheatselection.chosen_experience_level + 1);
        }
        targeted_message_add(MsgType_Player, player->cheatselection.chosen_player, plyr_idx, 1, "%s", str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                ThingModel crmodel;
                unsigned char exp;
                if (player->cheatselection.chosen_creature_kind == 0)
                {
                    while (1)
                    {
                        crmodel = GAME_RANDOM(game.conf.crtr_conf.model_count) + 1;
                        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[crmodel];
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
                    crmodel = player->cheatselection.chosen_creature_kind;
                    exp = player->cheatselection.chosen_experience_level;
                }
                unsigned short param2 = player->cheatselection.chosen_player | (exp << 8);
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
                pwkind = player->chosen_power_kind;
                i = get_power_overcharge_level(player);
                magic_use_power_direct(plyr_idx,pwkind,i,stl_x, stl_y,INVALID_THING, PwMod_CastForFree);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_FreeTurnChicken:
        case PSt_FreeCastDisease:
        {
            pwkind = player->chosen_power_kind;
            thing = get_creature_near_to_be_keeper_power_target(x, y, pwkind, plyr_idx);
            if (thing_is_invalid(thing))
            {
                player->thing_under_hand = 0;
                break;
            }
            player->thing_under_hand = thing->index;
            if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
            {
                KeepPwrLevel power_level = get_power_overcharge_level(player);
                magic_use_power_direct(plyr_idx,pwkind,power_level,stl_x,stl_y,thing,PwMod_CastForFree);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        }
        case PSt_StealRoom:
        clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
        slb = get_slabmap_block(slb_x, slb_y);
        room = room_get(slb->room_index);
        allowed = ( (room_exists(room)) && (room->owner != player->cheatselection.chosen_player) );
        if (allowed)
        {
            snprintf(str, sizeof(str), "%s", get_string(GUIStr_MnuOk));
        }
        targeted_message_add(MsgType_Player, player->cheatselection.chosen_player, plyr_idx, 1, str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                TbBool effect = (is_key_pressed(KC_RALT, KMod_DONTCARE));
                set_packet_action(pckt, PckA_CheatStealRoom, player->cheatselection.chosen_player, effect, 0, 0);
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
        case PSt_DestroyRoom:
        clear_messages_from_player(MsgType_Blank, -1);
        slb = get_slabmap_block(slb_x, slb_y);
        room = room_get(slb->room_index);
        allowed = (room_exists(room));
        if (allowed)
        {
            targeted_message_add(MsgType_Blank, 0, plyr_idx, 1, get_string(GUIStr_MnuOk));
        }
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (allowed)
            {
                destroy_room_leaving_unclaimed_ground(room, false);
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
        clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
        targeted_message_add(MsgType_Player, player->cheatselection.chosen_player, plyr_idx, 1, str);
        thing = get_creature_near(x, y);
        if ((!thing_is_creature(thing)) || (thing->owner == player->cheatselection.chosen_player))
        {
            player->thing_under_hand = 0;
        }
        else
        {
            player->thing_under_hand = thing->index;
        }
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            set_packet_action(pckt, PckA_CheatConvertCreature, player->cheatselection.chosen_player, 0, 0, 0);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
        case PSt_StealSlab:
        player->render_roomspace = create_box_roomspace(player->render_roomspace, 1, 1, slb_x, slb_y);
        allowed = tag_cursor_blocks_steal_slab(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
        targeted_message_add(MsgType_Player, player->cheatselection.chosen_player, plyr_idx, 1, str);
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
                                slbkind = choose_pretty_type(player->cheatselection.chosen_player, slb_x, slb_y);
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
                                slbkind = choose_pretty_type(player->cheatselection.chosen_player, slb_x, slb_y);
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
                    unsigned short param2 = player->cheatselection.chosen_player | (effect << 8);
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
                                set_creature_level(thing, cctrl->exp_level-1);
                            }
                            break;
                        }
                    }
                }
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_KillPlayer:
          clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
          struct PlayerInfo* PlayerToKill = get_player(player->cheatselection.chosen_player);
          if (player_exists(PlayerToKill))
          {
              targeted_message_add(MsgType_Player, player->cheatselection.chosen_player, plyr_idx, 1, str);
              if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
              {
                set_packet_action(pckt, PckA_CheatKillPlayer, PlayerToKill->id_number, 0, 0, 0);
                unset_packet_control(pckt, PCtr_LBtnRelease);
              }
          }
        break;
        case PSt_HeartHealth:
        clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
        thing = get_player_soul_container(player->cheatselection.chosen_player);
        struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
        if (thing_exists(thing))
        {
            targeted_message_add(MsgType_Player, thing->owner, plyr_idx, 1, "%d/%d", thing->health, objst->health);
        }
        else
        {
            break;
        }
        HitPoints new_health = thing->health;
        if (process_cheat_heart_health_inputs(&new_health, objst->health))
        {
            set_packet_action(pckt, PckA_CheatHeartHealth, player->cheatselection.chosen_player, new_health, 0, 0);
        }
        break;
        case PSt_QueryAll:
        case PSt_CreatrInfoAll:
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
                            query_creature(player, player->thing_under_hand, true, false);
                        }
                    }
                    query_thing(thing);
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
            player->render_roomspace = create_box_roomspace(player->render_roomspace, 1, 1, slb_x, slb_y);
            tag_cursor_blocks_place_terrain(plyr_idx, stl_x, stl_y);
            struct SlabConfigStats* slab_cfgstats;
            clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
            struct SlabConfigStats *slabst = get_slab_kind_stats(player->cheatselection.chosen_terrain_kind);
            if (slab_kind_has_no_ownership(player->cheatselection.chosen_terrain_kind))
            {
                player->cheatselection.chosen_player = game.neutral_player_num;
            }
            if (slabst->tooltip_stridx <= GUI_STRINGS_COUNT)
            {
                const char* msg = get_string(slabst->tooltip_stridx);
                strcpy(str, msg);
                char* dis_msg = strtok(str, ":");
                if (dis_msg == NULL)
                {
                    dis_msg = malloc(strlen(str) + 1);
                    strcpy(dis_msg, str);
                }
                targeted_message_add(MsgType_Player, player->cheatselection.chosen_player, plyr_idx, 1, dis_msg);
                free(dis_msg);
            }
            else
            {
                slab_cfgstats = get_slab_kind_stats(player->cheatselection.chosen_terrain_kind);
                targeted_message_add(MsgType_Player, player->cheatselection.chosen_player, plyr_idx, 1, slab_cfgstats->code_name);
            }
            clear_messages_from_player(MsgType_Blank, -1);
            if (is_key_pressed(KC_RSHIFT, KMod_DONTCARE))
            {
                terrain_details ^= 1;
                clear_key_pressed(KC_RSHIFT);
            }
            if (terrain_details)
            {
                slb = get_slabmap_block(slb_x, slb_y);
                slab_cfgstats = get_slab_kind_stats(slb->kind);
                targeted_message_add(MsgType_Blank, 0, plyr_idx, 1, "%s (%d) %d %d (%d) %d %d (%d)", slab_cfgstats->code_name, slabmap_owner(slb), slb_x, slb_y, get_slab_number(slb_x, slb_y), stl_x, stl_y, get_subtile_number(stl_x, stl_y));
            }
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                if (subtile_is_room(stl_x, stl_y))
                {
                    room = subtile_room_get(stl_x, stl_y);
                    delete_room_slab(slb_x, slb_y, true);
                }
                PlayerNumber id = (slab_kind_has_no_ownership(player->cheatselection.chosen_terrain_kind)) ? game.neutral_player_num : player->cheatselection.chosen_player;
                set_packet_action(pckt, PckA_CheatPlaceTerrain, player->cheatselection.chosen_terrain_kind, id, 0, 0);
                if ( (player->cheatselection.chosen_terrain_kind >= SlbT_WALLDRAPE) && (player->cheatselection.chosen_terrain_kind <= SlbT_WALLPAIRSHR) )
                {
                    player->cheatselection.chosen_terrain_kind = SlbT_WALLDRAPE + GAME_RANDOM(5);
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
  switch (pckt->action)
  {
      case PckA_CheatEnter:
    //      game.???[my_player_number].cheat_mode = 1;
          show_onscreen_msg(2*game_num_fps, "Cheat mode activated by player %d", plyr_idx);
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
            struct PlayerInfo* player = get_player(plyr_idx);
            player->cheatselection.chosen_terrain_kind = pckt->actn_par1;
            if (slab_kind_has_no_ownership(player->cheatselection.chosen_terrain_kind))
            {
               clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
               player->cheatselection.chosen_player = game.neutral_player_num;
            }
            return false;
        }
      case PckA_CheatSwitchPlayer:
        {
            struct PlayerInfo* player = get_player(plyr_idx);
            clear_messages_from_player(MsgType_Player, player->cheatselection.chosen_player);
            player->cheatselection.chosen_player = pckt->actn_par1;
            return false;
        }
      case PckA_CheatSwitchCreature:
        {
            struct PlayerInfo* player = get_player(plyr_idx);
            player->cheatselection.chosen_creature_kind = pckt->actn_par1;
            return false;
        }
      case PckA_CheatSwitchHero:
        {
            struct PlayerInfo* player = get_player(plyr_idx);
            player->cheatselection.chosen_hero_kind = pckt->actn_par1;
            return false;
        }
      case PckA_CheatSwitchExperience:
        {
            struct PlayerInfo* player = get_player(plyr_idx);
            player->cheatselection.chosen_experience_level = pckt->actn_par1;
            return false;
        }
        case PckA_CheatAllDoors:
        {
            make_available_all_doors(plyr_idx);
            return false;
        }
        case PckA_CheatAllTraps:
        {
            make_available_all_traps(plyr_idx);
            return false;
        }
        case PckA_CheatGiveDoorTrap:
        {
            long model;
            for (model = 1; model < game.conf.trapdoor_conf.door_types_count; model++)
            {
                if (is_door_buildable(plyr_idx, model))
                {
                    set_door_buildable_and_add_to_amount(plyr_idx, model, 1, 1);
                }
            }
            for (model = 1; model < game.conf.trapdoor_conf.trap_types_count; model++)
            {
                if (is_trap_buildable(plyr_idx, model))
                {
                    set_trap_buildable_and_add_to_amount(plyr_idx, model, 1, 1);
                }
            }
            update_trap_tab_to_config();
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
            x = (pckt->pos_x);
            y = (pckt->pos_y);
            stl_x = coord_subtile(x);
            stl_y = coord_subtile(y);
            slb_x = subtile_slab(stl_x);
            slb_y = subtile_slab(stl_y);
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
            x = (pckt->pos_x);
            y = (pckt->pos_y);
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
            x = (pckt->pos_x);
            y = (pckt->pos_y);
            thing = create_owned_special_digger(x, y, pckt->actn_par1);
            if (!thing_is_invalid(thing))
            {
                set_creature_level(thing, pckt->actn_par2);
            }
            break;
        }
        case PckA_CheatStealSlab:
        {
            x = (pckt->pos_x);
            y = (pckt->pos_y);
            stl_x = coord_subtile(x);
            stl_y = coord_subtile(y);
            slb_x = subtile_slab(stl_x);
            slb_y = subtile_slab(stl_y);
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
                    create_effect(&pos, imp_spangle_effects[get_player_color_idx(id)], id);
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
                            create_effect(&pos, imp_spangle_effects[get_player_color_idx(id)], id);
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
            x = (pckt->pos_x);
            y = (pckt->pos_y);
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
                    create_effects_on_room_slabs(room, imp_spangle_effects[get_player_color_idx(pckt->actn_par1)], 0, pckt->actn_par1);
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
            if (thing->health <= 0)
            {
                    struct Dungeon* dungeon = get_dungeon(plyr_idx);
                    dungeon->lvstats.keeper_destroyed[pckt->actn_par1]++;
                    dungeon->lvstats.keepers_destroyed++;
            }
            break;
        }
        case PckA_CheatKillPlayer:
        {
            thing = get_player_soul_container(pckt->actn_par1);
            struct Dungeon* dungeon = get_dungeon(plyr_idx);
            if (!thing_is_invalid(thing))
            {
                thing->health = 0;
                dungeon->lvstats.keeper_destroyed[pckt->actn_par1]++;
                dungeon->lvstats.keepers_destroyed++;
            }
            struct Thing* heartng = find_players_backup_dungeon_heart(pckt->actn_par1);
            if (!thing_is_invalid(heartng))
            {
                heartng->health = 0;
                dungeon->lvstats.keeper_destroyed[pckt->actn_par1]++;
                dungeon->lvstats.keepers_destroyed++;
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
        {
           return false;
        }
    }
    return true;
}
