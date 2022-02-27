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

extern void clear_input(struct Packet* packet);

/******************************************************************************/
SlabKind place_terrain = 0;
PlayerNumber selected_player = 0;
ThingModel selected_creature = 0;
ThingModel selected_hero = 0;
unsigned char selected_experience = 1;
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
    struct Coord3d pos;
    char str[255] = {'\0'};
    switch (player->work_state)
    {
        case PSt_MkDigger:
        allowed = tag_cursor_blocks_place_thing(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(selected_player);
        get_selected_player_for_cheat(&selected_player);
        process_cheat_mode_selection_inputs(&selected_experience);
        message_add_timeout(selected_player, 1, "%d", selected_experience);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }
            if (allowed)
            {
                set_players_packet_action(player, PckA_CheatMakeDigger, selected_player, selected_experience, 0, 0);
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
        clear_messages_from_player(selected_player);
        get_selected_player_for_cheat(&selected_player);
        if (is_key_pressed(KC_LSHIFT, KMod_DONTCARE))
        {
            selected_hero++;
            if (selected_hero > 13)
            {
                selected_hero = 0;
            }
            clear_key_pressed(KC_LSHIFT);
        }
        process_cheat_mode_selection_inputs(&selected_experience);
        if (selected_hero == 0)
        {
            sprintf(str, "?");
        }
        else
        {
            struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[selected_hero];
            sprintf(str, "%s %d", get_string(crconf->namestr_idx), selected_experience);
        }
        message_add_timeout(selected_player, 1, "%s", str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }
            if (allowed)
            {
                ThingModel crmodel;
                unsigned char exp;
                if (selected_hero == 0)
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
                    crmodel = selected_hero;
                    exp = selected_experience;
                }
                unsigned short param2 = selected_player | (exp << 8);
                set_players_packet_action(player, PckA_CheatMakeCreature, crmodel, param2, 0, 0);
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
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }
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
        influence_own_creatures = 1;
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
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }
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
        clear_messages_from_player(selected_player);
        get_selected_player_for_cheat(&selected_player);
        if (is_key_pressed(KC_LSHIFT, KMod_DONTCARE))
        {
            selected_creature++;
            if (selected_creature > 17)
            {
                selected_creature = 0;
            }
            clear_key_pressed(KC_LSHIFT);
        }
        process_cheat_mode_selection_inputs(&selected_experience);
        if (selected_creature == 0)
        {
            sprintf(str, "?");
        }
        else
        {
            struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[selected_creature + 13];
            sprintf(str, "%s %d", get_string(crconf->namestr_idx), selected_experience);
        }
        message_add_timeout(selected_player, 1, "%s", str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }
            if (allowed)
            {
                ThingModel crmodel;
                unsigned char exp;
                if (selected_creature == 0)
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
                    crmodel = selected_creature + 13;
                    exp = selected_experience;
                }
                unsigned short param2 = selected_player | (exp << 8);
                set_players_packet_action(player, PckA_CheatMakeCreature, crmodel, param2, 0, 0);
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
                if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
                {
                    return false;
                }
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
                if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
                {
                    return false;
                }
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
        clear_messages_from_player(selected_player);
        get_selected_player_for_cheat(&selected_player);
        slb = get_slabmap_block(slb_x, slb_y);
        room = room_get(slb->room_index);
        allowed = ( (room_exists(room)) && (room->owner != selected_player) );
        if (allowed)
        {
            sprintf(str, get_string(419));
        }
        message_add_timeout(selected_player, 1, str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {    
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }   
            if (allowed)
            {
                TbBool effect = (is_key_pressed(KC_RALT, KMod_DONTCARE));
                set_players_packet_action(player, PckA_CheatStealRoom, selected_player, effect, 0, 0);
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
            message_add_timeout(-127, 1, get_string(419));
        }
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }            
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
                if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
                {
                    return false;
                }
                if (player->thing_under_hand > 0)
                {
                    kill_creature(thing, INVALID_THING, -1, CrDed_NoUnconscious);
                }
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_ConvertCreatr:
        clear_messages_from_player(selected_player);
        get_selected_player_for_cheat(&selected_player);
        message_add_timeout(selected_player, 1, str);
        thing = get_creature_near(x, y);
        if ((!thing_is_creature(thing)) || (thing->owner == selected_player))
        {
            player->thing_under_hand = 0;
        }
        else
        {
            player->thing_under_hand = thing->index;
        }
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }
            set_players_packet_action(player, PckA_CheatConvertCreature, selected_player, 0, 0, 0);
            unset_packet_control(pckt, PCtr_LBtnRelease);    
        }
        break;
        case PSt_StealSlab:
        allowed = tag_cursor_blocks_steal_slab(plyr_idx, stl_x, stl_y);
        clear_messages_from_player(selected_player);
        get_selected_player_for_cheat(&selected_player);
        message_add_timeout(selected_player, 1, str);
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }   
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
                                slbkind = choose_pretty_type(selected_player, slb_x, slb_y);
                            }
                            else
                            {
                                slbkind = rand() % (5) + 4;
                            }
                            break;
                        }
                        case SlbT_TORCHDIRT:
                        {
                            if (is_key_pressed(KC_RSHIFT, KMod_DONTCARE))
                            {
                                slbkind = choose_pretty_type(selected_player, slb_x, slb_y);
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
                    unsigned short param2 = selected_player | (effect << 8);
                    set_players_packet_action(player, PckA_CheatStealSlab, slbkind, param2, 0, 0);
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
                if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
                {
                    return false;
                }
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
          clear_messages_from_player(selected_player);
          get_selected_player_for_cheat(&selected_player);
          struct PlayerInfo* PlayerToKill = get_player(selected_player);
          if (player_exists(PlayerToKill))
          {
              message_add_timeout(selected_player, 1, str);
              if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
              {
                if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
                {
                    return false;
                }
                set_players_packet_action(player, PckA_CheatKillPlayer, PlayerToKill->id_number, 0, 0, 0);
              }
          }
        break;
        case PSt_HeartHealth:
        clear_messages_from_player(selected_player);
        get_selected_player_for_cheat(&selected_player);
        thing = get_player_soul_container(selected_player);
        if (!thing_is_invalid(thing))
        {
            message_add_timeout(thing->owner, 1, "%d/%d", thing->health, game.dungeon_heart_health);
        }
        else
        {
            break;
        }
        short new_health = thing->health;
        process_cheat_heart_health_inputs(&new_health);
        set_players_packet_action(player, PckA_CheatHeartHealth, selected_player, new_health, 0, 0);
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
                if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
                {
                    return false;
                }
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
                if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
                {
                    return false;
                }
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
        process_cheat_mode_selection_inputs(&place_terrain);
        struct SlabConfigStats* slab_cfgstats;
        clear_messages_from_player(selected_player);
        get_selected_player_for_cheat(&selected_player);
        struct SlabAttr *slbattr = get_slab_kind_attrs(place_terrain);
        if (slbattr->tooltip_stridx <= GUI_STRINGS_COUNT)
        {
            const char* msg = get_string(slbattr->tooltip_stridx);
            strcpy(str, msg);
            char* dis_msg = strtok(str, ":");
            message_add_timeout(selected_player, 1, dis_msg);
        }
        else
        {
            slab_cfgstats = get_slab_kind_stats(place_terrain);
            message_add_timeout(selected_player, 1, slab_cfgstats->code_name);            
        }
        clear_messages_from_player(-127);
        if (is_key_pressed(KC_LSHIFT, KMod_DONTCARE))
        {
            terrain_details ^= 1;
            clear_key_pressed(KC_LSHIFT);
        }
        if (terrain_details)
        {
            slb = get_slabmap_block(slb_x, slb_y);
            slab_cfgstats = get_slab_kind_stats(slb->kind);
            message_add_timeout(-127, 1, "%s (%d) %d %d (%d) %d %d (%d)", slab_cfgstats->code_name, slabmap_owner(slb), slb_x, slb_y, get_slab_number(slb_x, slb_y), stl_x, stl_y, get_subtile_number(stl_x, stl_y));
        }        
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
            {
                return false;
            }             
            if (subtile_is_room(stl_x, stl_y)) 
            {
                room = subtile_room_get(stl_x, stl_y);
                delete_room_slab(slb_x, slb_y, true);
            }
            PlayerNumber id;
            if ( (place_terrain == SlbT_CLAIMED) || ( (place_terrain >= SlbT_WALLDRAPE) && (place_terrain <= SlbT_DAMAGEDWALL) ) )
            {
                slb = get_slabmap_block(slb_x, slb_y);
                if ( (slb->kind == SlbT_CLAIMED) || ( (slb->kind >= SlbT_WALLDRAPE) && (slb->kind <= SlbT_DAMAGEDWALL) ) )
                {
                    id = slabmap_owner(slb);
                }
                else
                {
                    id = selected_player;
                }
            }
            else
            {
                id = game.neutral_player_num;
            }
            if ( (place_terrain >= SlbT_WALLDRAPE) && (place_terrain <= SlbT_WALLPAIRSHR) )
            {
                if (is_key_pressed(KC_RSHIFT, KMod_DONTCARE))
                {
                    place_terrain = choose_pretty_type(id, slb_x, slb_y);
                }
            }
            set_players_packet_action(player, PckA_CheatPlaceTerrain, place_terrain, id, 0, 0);
            if ( (place_terrain >= SlbT_WALLDRAPE) && (place_terrain <= SlbT_WALLPAIRSHR) )
            {
                place_terrain = rand() % (5) + 4;
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
                if (pos_is_on_gui_box(left_button_clicked_x, left_button_clicked_y))
                {
                    return false;
                }
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