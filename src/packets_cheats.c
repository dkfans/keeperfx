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

extern void clear_input(struct Packet* packet);

/******************************************************************************/
short place_terrain = 0;
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

    switch (player->work_state)
    {
        case PSt_MkGoodDigger:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                create_owned_special_digger(x, y, get_selected_player_for_cheat(game.hero_player_num));
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_MkGoodCreatr:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                create_random_hero_creature(x, y, get_selected_player_for_cheat(game.hero_player_num), CREATURE_MAX_LEVEL);
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_MkGoldPot:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
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
                    if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
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
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
            {
                if ((player->controlled_thing_idx > 0) && (player->controlled_thing_idx < THINGS_COUNT))
                {
                    thing = thing_get(player->controlled_thing_idx);
                    set_start_state(thing);
                    clear_selected_thing(player);
                }
                unset_packet_control(pckt, PCtr_RBtnRelease);
            }
            break;
        case PSt_MkBadCreatr:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                create_random_evil_creature(x, y, get_selected_player_for_cheat(plyr_idx), CREATURE_MAX_LEVEL);
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
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
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
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_DestroyRoom:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                slb = get_slabmap_block(slb_x, slb_y);
                if (slb->room_index)
                {
                    room = room_get(slb->room_index);
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
                    change_creature_owner(thing, get_selected_player_for_cheat(plyr_idx));
                }
                unset_packet_control(pckt, PCtr_LBtnRelease);
            }
            break;
        case PSt_StealSlab:
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                slb = get_slabmap_block(slb_x, slb_y);
                if (slb->kind >= SlbT_EARTH && slb->kind <= SlbT_CLAIMED)
                {
                    short slbkind;
                    i = get_selected_player_for_cheat(plyr_idx);
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
                                slbkind = choose_pretty_type(i, slb_x, slb_y);
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
                                slbkind = choose_pretty_type(i, slb_x, slb_y);
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
                    do_slab_efficiency_alteration(slb_x, slb_y);
                    slb = get_slabmap_block(slb_x, slb_y);
                    for (i = 0; i < PLAYERS_COUNT; i++)
                    {
                        if (i != slabmap_owner(slb))
                        {
                            untag_blocks_for_digging_in_area(stl_x, stl_y, i);
                        }
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
            i = get_selected_player_for_cheat(-1);
            struct PlayerInfo* PlayerToKill = get_player(i);
            if (player_exists(PlayerToKill))
            {
                thing = get_player_soul_container(PlayerToKill->id_number);
                if (thing_is_dungeon_heart(thing))
                {
                    thing->health = 0;
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
            if (is_key_pressed(KC_NUMPAD0, KMod_NONE))
            {
                place_terrain = SlbT_ROCK;
                clear_key_pressed(KC_NUMPAD0);
            }
            else if (is_key_pressed(KC_NUMPAD1, KMod_NONE))
            {
                place_terrain = SlbT_GOLD;
                clear_key_pressed(KC_NUMPAD1);
            }
            else if (is_key_pressed(KC_NUMPAD2, KMod_NONE))
            {
                place_terrain = SlbT_GEMS;
                clear_key_pressed(KC_NUMPAD2);
            }
            else if (is_key_pressed(KC_NUMPAD3, KMod_NONE))
            {
                place_terrain = SlbT_EARTH;
                clear_key_pressed(KC_NUMPAD3);
            }
            else if (is_key_pressed(KC_NUMPAD4, KMod_NONE))
            {
                place_terrain = SlbT_TORCHDIRT;
                clear_key_pressed(KC_NUMPAD4);
            }
            else if (is_key_pressed(KC_NUMPAD5, KMod_NONE))
            {
                place_terrain = SlbT_PATH;
                clear_key_pressed(KC_NUMPAD5);
            }
            else if (is_key_pressed(KC_NUMPAD6, KMod_NONE))
            {
                place_terrain = SlbT_CLAIMED;
                clear_key_pressed(KC_NUMPAD6);
            }
            else if (is_key_pressed(KC_NUMPAD7, KMod_NONE))
            {
                place_terrain = SlbT_LAVA;
                clear_key_pressed(KC_NUMPAD7);
            }
            else if (is_key_pressed(KC_NUMPAD8, KMod_NONE))
            {
                place_terrain = SlbT_WATER;
                clear_key_pressed(KC_NUMPAD8);
            }
            else if (is_key_pressed(KC_NUMPAD9, KMod_NONE))
            {
                place_terrain = rand() % (5) + 4;
                clear_key_pressed(KC_NUMPAD9);
            }
            else if (is_key_pressed(KC_DIVIDE, KMod_NONE))
            {
                place_terrain = SlbT_DAMAGEDWALL;
                clear_key_pressed(KC_DIVIDE);
            }
            else if (is_key_pressed(KC_MULTIPLY, KMod_NONE))
            {
                place_terrain = SlbT_SLAB50;
                clear_key_pressed(KC_MULTIPLY);
            }
            struct SlabConfigStats* slab_cfgstats;
            clear_messages_from_player(-127);
            struct SlabAttr *slbattr = get_slab_kind_attrs(place_terrain);
            if (slbattr->tooltip_stridx <= GUI_STRINGS_COUNT)
            {
                const char* msg = get_string(slbattr->tooltip_stridx);
                char msg_buf[255];
                strcpy(msg_buf, msg);
                char* dis_msg = strtok(msg_buf, ":");
                message_add(-127, dis_msg);
            }
            else
            {
                slab_cfgstats = get_slab_kind_stats(place_terrain);
                message_add(-127, slab_cfgstats->code_name);
            }
            if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
            {
                slb = get_slabmap_block(slb_x, slb_y);
                char s[3];
                if (is_key_pressed(KC_SLASH, KMod_NONE))
                {
                    slab_cfgstats = get_slab_kind_stats(slb->kind);
                    clear_messages_from_player(plyr_idx);
                    message_add(plyr_idx, slab_cfgstats->code_name);
                    clear_key_pressed(KC_SLASH);
                }
                else if (is_key_pressed(KC_SLASH, KMod_SHIFT))
                {
                    itoa(slabmap_owner(slb), s, 10);
                    clear_messages_from_player(plyr_idx);
                    message_add(plyr_idx, s);
                    clear_key_pressed(KC_SLASH);
                }
                else if (is_key_pressed(KC_X, KMod_NONE))
                {
                    itoa(stl_x, s, 10);
                    clear_messages_from_player(plyr_idx);
                    message_add(plyr_idx, s);
                    clear_key_pressed(KC_X);
                }
                else if (is_key_pressed(KC_Y, KMod_NONE))
                {
                    itoa(stl_y, s, 10);
                    clear_messages_from_player(plyr_idx);
                    message_add(plyr_idx, s);
                    clear_key_pressed(KC_Y);
                }
                else if (is_key_pressed(KC_X, KMod_SHIFT))
                {
                    itoa(slb_x, s, 10);
                    clear_messages_from_player(plyr_idx);
                    message_add(plyr_idx, s);
                    clear_key_pressed(KC_X);
                }
                else if (is_key_pressed(KC_Y, KMod_SHIFT))
                {
                    itoa(slb_y, s, 10);
                    clear_messages_from_player(plyr_idx);
                    message_add(plyr_idx, s);
                    clear_key_pressed(KC_Y);
                }
                else if (is_key_pressed(KC_N, KMod_NONE))
                {
                    itoa(get_slab_number(subtile_slab(stl_x), subtile_slab(stl_y)), s, 10);
                    clear_messages_from_player(plyr_idx);
                    message_add(plyr_idx, s);
                    clear_key_pressed(KC_N);
                }
                else
                {
                    if (subtile_is_room(stl_x, stl_y))
                    {
                        room = subtile_room_get(stl_x, stl_y);
                        delete_room_slab(slb_x, slb_y, true);
                    }
                    PlayerNumber id;
                    if ( (place_terrain == SlbT_CLAIMED) || ( (place_terrain >= SlbT_WALLDRAPE) && (place_terrain <= SlbT_DAMAGEDWALL) ) )
                    {
                        id = slabmap_owner(slb);
                    }
                    else
                    {
                        id = game.neutral_player_num;
                    }
                    if (slab_kind_is_animated(place_terrain))
                    {
                        place_animating_slab_type_on_map(place_terrain, 0, stl_x, stl_y, id);
                    }
                    else
                    {
                        if ( (place_terrain >= SlbT_WALLDRAPE) && (place_terrain <= SlbT_WALLPAIRSHR) )
                        {
                            if (is_key_pressed(KC_RSHIFT, KMod_DONTCARE))
                            {
                                place_terrain = choose_pretty_type(id, slb_x, slb_y);
                            }
                        }
                        place_slab_type_on_map(place_terrain, stl_x, stl_y, id, 0);
                        if ( (place_terrain >= SlbT_WALLDRAPE) && (place_terrain <= SlbT_WALLPAIRSHR) )
                        {
                            place_terrain = rand() % (5) + 4;
                        }
                    }
                    do_slab_efficiency_alteration(slb_x, slb_y);
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