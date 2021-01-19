/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file console_cmd.c
 *     
 * @par Purpose:
 *     Define various console commans
 * @par Comment:
 *     None
 * @author   Sim
 * @date     07 Jul 2020 - 07 Jul 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "console_cmd.h"
#include "globals.h"

#include "actionpt.h"
#include "bflib_datetm.h"
#include "bflib_sound.h"
#include "config.h"
#include "config_campaigns.h"
#include "config_magic.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "creature_instances.h"
#include "creature_jobs.h"
#include "creature_states.h"
#include "creature_states_hero.h"
#include "dungeon_data.h"
#include "frontend.h"
#include "frontmenu_ingame_evnt.h"
#include "frontmenu_ingame_tabs.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "gui_boxmenu.h"
#include "gui_msgs.h"
#include "gui_soundmsgs.h"
#include "keeperfx.hpp"
#include "map_blocks.h"
#include "map_columns.h"
#include "map_utils.h"
#include "math.h"
#include "music_player.h"
#include "packets.h"
#include "player_computer.h"
#include "player_instances.h"
#include "player_states.h"
#include "player_utils.h"
#include "room_data.h"
#include "room_util.h"
#include "slab_data.h"
#include "thing_factory.h"
#include "thing_list.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_physics.h"
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

static struct GuiBoxOption cmd_comp_procs_data[COMPUTER_PROCESSES_COUNT + 3] = {
  {"!", 1, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0 }
  };

static char cmd_comp_procs_label[COMPUTER_PROCESSES_COUNT + 2][COMMAND_WORD_LEN + 8];

struct GuiBoxOption cmd_comp_checks_data[COMPUTER_CHECKS_COUNT + 1] = { 0 };
static char cmd_comp_checks_label[COMPUTER_CHECKS_COUNT][COMMAND_WORD_LEN + 8];

struct GuiBoxOption cmd_comp_events_data[COMPUTER_EVENTS_COUNT + 1] = { 0 };

static TbBool script_set_pool(PlayerNumber player_idx, const char *creature, const char *num);

static char cmd_comp_events_label[COMPUTER_EVENTS_COUNT][COMMAND_WORD_LEN + 8];

static long cmd_comp_procs_click(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *args)
{
    struct Computer2 *comp;
    comp = get_computer_player(args[0]);
    struct ComputerProcess* cproc = &comp->processes[args[1]];

    if (cproc->flags & ComProc_Unkn0020)
        message_add_fmt(args[0], "resuming %s", cproc->name?cproc->name:"(null)");
    else
        message_add_fmt(args[0], "suspending %s", cproc->name?cproc->name:"(null)");
    
    cproc->flags ^= ComProc_Unkn0020; // Suspend, but do not update running time
    return 1;
}

static long cmd_comp_procs_update(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *args)
{
    struct Computer2 *comp = get_computer_player(args[0]);
    int i = 0;

    for (; i < args[1]; i++)
    {
        struct ComputerProcess* cproc = &comp->processes[i];
        if (cproc != NULL)
        {
            char *label = (char*)goptn[i].label;
            sprintf(label, "%02lx", cproc->flags);
            label[2] = ' ';
        }
    }

    sprintf(cmd_comp_procs_label[i], "comp=%d, wait=%ld", 0, comp->gameturn_wait);
    return 1;
}

static long cmd_comp_checks_update(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *args)
{
    struct Computer2 *comp = get_computer_player(args[0]);
    int i = 0;

    for (; i < args[1]; i++)
    {
        struct ComputerCheck* check = &comp->checks[i];
        if (check != NULL)
        {
            char *label = (char*)goptn[i].label;
            sprintf(label, "%02lx", check->flags);
            label[2] = ' ';
        }
    }
    return 1;
}

int cmd_comp_list(PlayerNumber plyr_idx, int max_count,
    struct GuiBoxOption *data_list, char label_list[][COMMAND_WORD_LEN + 8],
    const char *(*get_name)(struct Computer2 *, int),
    unsigned long (*get_flags)(struct Computer2 *, int),
    Gf_OptnBox_4Callback click_fn
    )
{
    close_creature_cheat_menu();
    //gui_cheat_box
    int i = 0;
    struct Computer2 *comp;
    comp = get_computer_player(plyr_idx);
    for (; i < max_count; i++)
    {
        unsigned long flags = get_flags(comp, i); 
        const char *name = get_name(comp, i);
        if (name == NULL)
            sprintf(label_list[i], "%02lx %s", flags, "(null2)");  
        else
          sprintf(label_list[i], "%02lx %s", flags, name);
        data_list[i].label = label_list[i];

        data_list[i].numfield_4 = 1;
        data_list[i].callback = click_fn;
        data_list[i].field_D = plyr_idx;
        data_list[i].field_11 = max_count;
        data_list[i].field_19 = plyr_idx;
        data_list[i].field_1D = i;
    }
    data_list[i].label = "!";
    data_list[i].numfield_4 = 0;
    return i;
}

static const char *get_process_name(struct Computer2 *comp, int i)
{
    return comp->processes[i].name;
}
static unsigned long  get_process_flags(struct Computer2 *comp, int i)
{
    return comp->processes[i].flags;
}
static void cmd_comp_procs(PlayerNumber plyr_idx)
{
    int i = cmd_comp_list(plyr_idx, COMPUTER_PROCESSES_COUNT, 
        cmd_comp_procs_data, cmd_comp_procs_label,
        &get_process_name, &get_process_flags,
        &cmd_comp_procs_click);

    cmd_comp_procs_data[0].active_cb = &cmd_comp_procs_update;
    
    cmd_comp_procs_data[i].label = "======";
    cmd_comp_procs_data[i].numfield_4 = 1;
    i++;
    cmd_comp_procs_data[i].label = cmd_comp_procs_label[COMPUTER_PROCESSES_COUNT];
    cmd_comp_procs_data[i].numfield_4 = 1;
    i++;
    cmd_comp_procs_data[i].label = "!";
    cmd_comp_procs_data[i].numfield_4 = 0;

    gui_cheat_box = gui_create_box(my_mouse_x, 20, cmd_comp_procs_data);
}

static const char *get_event_name(struct Computer2 *comp, int i)
{
    return comp->events[i].name;
}
static unsigned long  get_event_flags(struct Computer2 *comp, int i)
{
    return 0;
}
static void cmd_comp_events(PlayerNumber plyr_idx)
{
    cmd_comp_list(plyr_idx, COMPUTER_EVENTS_COUNT, 
        cmd_comp_events_data, cmd_comp_events_label,
        &get_event_name, &get_event_flags, NULL);
    cmd_comp_events_data[0].active_cb = NULL;

    gui_cheat_box = gui_create_box(my_mouse_x, 20, cmd_comp_events_data);
}


static long cmd_comp_checks_click(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *args)
{
    struct Computer2 *comp;
    comp = get_computer_player(args[0]);
    struct ComputerCheck* cproc = &comp->checks[args[1]];

    if (cproc->flags & ComChk_Unkn0001)
        message_add_fmt(args[0], "resuming %s", cproc->name?cproc->name:"(null)");
    else
        message_add_fmt(args[0], "suspending %s", cproc->name?cproc->name:"(null)");
    
    cproc->flags ^= ComChk_Unkn0001;
    return 1;
}
static const char *get_check_name(struct Computer2 *comp, int i)
{
    return comp->checks[i].name;
}
static unsigned long  get_check_flags(struct Computer2 *comp, int i)
{
    return comp->checks[i].flags;
}

static void cmd_comp_checks(PlayerNumber plyr_idx)
{
    cmd_comp_list(plyr_idx, COMPUTER_CHECKS_COUNT, 
        cmd_comp_checks_data, cmd_comp_checks_label,
        &get_check_name, &get_check_flags, &cmd_comp_checks_click);
    cmd_comp_checks_data[0].active_cb = NULL;

    gui_cheat_box = gui_create_box(my_mouse_x, 20, cmd_comp_checks_data);
}

static char *cmd_strtok(char *tail)
{
    if (tail == NULL)
    {
        return NULL;
    }
    char* next = strchr(tail,' ');
    if (next == NULL)
    {
        return NULL;
    }
    next[0] = '\0';
    next++; // it was space
    while (*next == ' ')
      next++;
    return next;
}

static void str_replace(char *str, int from, int to)
{
    for (char *p = strchr(str, from); p != NULL; p = strchr(p+1, from))
    {
        *p = to;
    }
}

static TbBool cmd_magic_instance(char* creature_str, const char*  slot_str, char* instance_str)
{
    if (creature_str == NULL || slot_str == NULL || instance_str == NULL)
        return false;
    str_replace(creature_str, '.', '_');
    str_replace(instance_str, '.', '_');
    int creature = get_id(creature_desc, creature_str);
    if (creature == -1)
    {
        message_add(10, "Invalid creature");
        return false;
    }
    int slot = atoi(slot_str);
    if (slot < 0 || slot > 9)
    {
        message_add(10, "Invalid slot");
        return false;
    }
    int instance = get_id(instance_desc, instance_str);
    if (instance == -1)
    {
        instance = atoi(instance_str);
    }
    if (instance <= 0)
    {
        message_add(10, "Invalid instance");
        return false;
    }
    struct CreatureStats* crstat = creature_stats_get(creature);
    crstat->learned_instance_id[slot] = instance;
    for (long i = 0; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if ((thing->alloc_flags & TAlF_Exists) != 0)
        {
            if (thing->class_id == TCls_Creature)
            {
                creature_increase_available_instances(thing);
            }
        }
    }

    return true;
}

TbBool cmd_exec(PlayerNumber plyr_idx, char *msg)
{
    SYNCDBG(2,"Command %d: %s",(int)plyr_idx, msg);
    char * parstr = msg + 1;
    char * pr2str = cmd_strtok(msg + 1);
    char * pr3str = (pr2str != NULL) ? cmd_strtok(pr2str + 1) : NULL;
    char * pr4str = (pr3str != NULL) ? cmd_strtok(pr3str + 1) : NULL;
    char * pr5str = (pr4str != NULL) ? cmd_strtok(pr4str + 1) : NULL;
    struct PlayerInfo* player;
    struct Thing* thing;
    struct Dungeon* dungeon;
    struct Room* room;
    struct Packet* pckt;
    struct SlabMap *slb;
    struct Coord3d pos;
    if (strcasecmp(parstr, "stats") == 0)
    {
      message_add_fmt(plyr_idx, "Now time is %d, last loop time was %d",LbTimerClock(),last_loop_time);
      message_add_fmt(plyr_idx, "clock is %d, requested fps is %d",clock(),game.num_fps);
      return true;
    } else if (strcasecmp(parstr, "quit") == 0)
    {
        quit_game = 1;
        exit_keeper = 1;
        return true;
    } else if (strcasecmp(parstr, "turn") == 0)
    {
        message_add_fmt(plyr_idx, "turn %ld", game.play_gameturn);
        return true;
    } else if ((game.flags_font & FFlg_AlexCheat) != 0)
    {
        if (strcasecmp(parstr, "compuchat") == 0)
        {
            if (pr2str == NULL)
                return false;

            if ((strcasecmp(pr2str,"scarce") == 0) || (strcasecmp(pr2str,"1") == 0))
            {
                for (int i = 0; i < PLAYERS_COUNT; i++)
                {
                    if ((i == game.hero_player_num)
                        || (plyr_idx == game.neutral_player_num))
                        continue;
                    struct Computer2* comp = get_computer_player(i);
                    if (player_exists(get_player(i)) && (!computer_player_invalid(comp)))
                        message_add_fmt(i, "Ai model %d", (int)comp->model);
                }
                gameadd.computer_chat_flags = CChat_TasksScarce;
            } else
            if ((strcasecmp(pr2str,"frequent") == 0) || (strcasecmp(pr2str,"2") == 0))
            {
                message_add_fmt(plyr_idx, "%s", pr2str);
                gameadd.computer_chat_flags = CChat_TasksScarce|CChat_TasksFrequent;
            } else
            {
                message_add(plyr_idx, "none");
                gameadd.computer_chat_flags = CChat_None;
            }
            return true;
        } else if (strcasecmp(parstr, "comp.procs") == 0)
        {
            if (pr2str == NULL)
                return false;
            int id = atoi(pr2str);
            if (id < 0 || id > PLAYERS_COUNT)
                return false;
            cmd_comp_procs(id);
            return true;
        } else if (strcasecmp(parstr, "comp.events") == 0)
        {
            if (pr2str == NULL)
                return false;
            int id = atoi(pr2str);
            if (id < 0 || id > PLAYERS_COUNT)
                return false;
            cmd_comp_events(id);
            return true;
        } else if (strcasecmp(parstr, "comp.checks") == 0)
        {
            if (pr2str == NULL)
                return false;
            int id = atoi(pr2str);
            if (id < 0 || id > PLAYERS_COUNT)
                return false;
            cmd_comp_checks(id);
            return true;
        } else if (strcasecmp(parstr, "reveal") == 0)
        {
            struct PlayerInfo* player = get_my_player();
            reveal_whole_map(player);
            return true;
        } else if (strcasecmp(parstr, "comp.kill") == 0)
        {
            if (pr2str == NULL)
                return false;
            int id = atoi(pr2str);
            if (id < 0 || id > PLAYERS_COUNT)
                return false;
            struct Thing* thing = get_player_soul_container(id);
            thing->health = 0;
        } else if (strcasecmp(parstr, "comp.me") == 0)
        {
            struct PlayerInfo* player = get_player(plyr_idx);
            if (pr2str == NULL)
                return false;
            if (!setup_a_computer_player(plyr_idx, atoi(pr2str))) {
                message_add(plyr_idx, "unable to set assistant");
            } else
                message_add_fmt(plyr_idx, "computer assistant is %d", atoi(pr2str));
            return true;
        } else if (strcasecmp(parstr, "magic.instance") == 0)
        {
            return cmd_magic_instance((char*)pr2str, pr3str, (char*)pr4str);
        } else if (strcasecmp(parstr, "give.trap") == 0)
        {
            int id = atoi(pr2str);
            if (id <= 0 || id > trapdoor_conf.trap_types_count)
                return false;
            command_add_value(Cmd_TRAP_AVAILABLE, plyr_idx, id, 1, 1);
            update_trap_tab_to_config();
            message_add(plyr_idx, "done!");
            return true;
        } else if (strcasecmp(parstr, "give.door") == 0)
        {
            int id = atoi(pr2str);
            if (id <= 0 || id > trapdoor_conf.door_types_count)
                return false;

            struct ScriptValue tmp_value = {0};
            script_process_value(Cmd_DOOR_AVAILABLE, plyr_idx, id, 1, 1, &tmp_value);
            update_trap_tab_to_config();
            message_add(plyr_idx, "done!");
            return true;
        } else if (strcasecmp(parstr, "map.pool") == 0)
        {
            return script_set_pool(plyr_idx, pr2str, pr3str);
        }
        else if ( (strcasecmp(parstr, "gold.create") == 0) || (strcasecmp(parstr, "create.gold") == 0) )
        {
            if ( (pr2str != NULL) && (parameter_is_number(pr2str)) )
            {
                player = get_player(plyr_idx);
                pckt = get_packet_direct(player->packet_num);
                thing = create_gold_pot_at(pckt->pos_x, pckt->pos_y, plyr_idx);
                if (!thing_is_invalid(thing))
                {
                    if (thing_in_wall_at(thing, &thing->mappos))
                    {
                        move_creature_to_nearest_valid_position(thing);
                    }
                    thing->valuable.gold_stored = atoi(pr2str);
                    add_gold_to_pile(thing, 0);
                    MapSubtlCoord stl_x = coord_subtile(((unsigned short)pckt->pos_x));
                    MapSubtlCoord stl_y = coord_subtile(((unsigned short)pckt->pos_y));
                    room = subtile_room_get(stl_x, stl_y);
                    if (room_exists(room))
                    {
                        if (room->kind == RoK_TREASURE)
                        {
                            count_gold_hoardes_in_room(room);
                        }
                    }
                    return true;
                }
            }           
        }
        else if ( (strcasecmp(parstr, "object.create") == 0) || (strcasecmp(parstr, "create.object") == 0) )
        {
            if (pr2str != NULL)
            {
                player = get_player(plyr_idx);
                pckt = get_packet_direct(player->packet_num);
                pos.x.stl.num = coord_subtile(((unsigned short)pckt->pos_x));
                pos.y.stl.num = coord_subtile(((unsigned short)pckt->pos_y));
                if (!subtile_coords_invalid(pos.x.stl.num, pos.y.stl.num))
                {
                    long ObjModel = get_rid(object_desc, pr2str);
                    if (ObjModel == -1)
                    {
                        if (parameter_is_number(pr2str))
                        {
                            ObjModel = atoi(pr2str);
                        }
                    }
                    if (ObjNumber >= 0)
                    {
                        PlayerNumber id = get_player_number_for_command(pr3str);
                        thing = create_object(&pos, ObjModel, id, -1);
                        if (thing_is_object(thing))
                        {
                            if (thing_in_wall_at(thing, &thing->mappos))
                            {
                                move_creature_to_nearest_valid_position(thing);
                            }
                        return true;
                        }
                    }
                }
            }
        }
        else if ( (strcasecmp(parstr, "creature.create") == 0) || (strcasecmp(parstr, "create.creature") == 0) )
        {
            if (pr2str != NULL)
            {
                long crmodel = get_creature_model_for_command(pr2str);
                if (crmodel == -1)
                {
                    if (parameter_is_number(pr2str))
                    {
                        crmodel = atoi(pr2str);
                    }
                }
                if ( (crmodel > 0) && (crmodel <= 31) )
                {
                    player = get_player(plyr_idx);
                    pckt = get_packet_direct(player->packet_num);
                    MapSubtlCoord stl_x = coord_subtile(((unsigned short)pckt->pos_x));
                    MapSubtlCoord stl_y = coord_subtile(((unsigned short)pckt->pos_y));
                    PlayerNumber id = get_player_number_for_command(pr5str);
                    if (!subtile_coords_invalid(stl_x, stl_y))
                    {
                        pos.x.stl.num = stl_x;
                        pos.y.stl.num = stl_y;                 
                        unsigned int count = (pr4str != NULL) ? atoi(pr4str) : 1;
                        unsigned int i;
                        for (i = 0; i < count; i++)
                        {                            
                            struct Thing* creatng = create_creature(&pos, crmodel, id);
                            if (thing_is_creature(creatng))
                            {
                                if (thing_in_wall_at(creatng, &creatng->mappos))
                                {
                                    move_creature_to_nearest_valid_position(creatng);
                                }
                                set_creature_level(creatng, (atoi(pr3str) - (unsigned char)(pr3str != NULL)));
                            }
                        }
                        return true;
                    }
                }
            }
        }
        else if ( (strcasecmp(parstr, "thing.create") == 0) || (strcasecmp(parstr, "create.thing") == 0) )
        {
            if ( (pr2str != NULL) && (pr3str != NULL) )
            {
                player = get_player(plyr_idx);
                pckt = get_packet_direct(player->packet_num);
                pos.x.stl.num = coord_subtile(((unsigned short)pckt->pos_x));
                pos.y.stl.num = coord_subtile(((unsigned short)pckt->pos_y));
                unsigned short tngclass = atoi(pr2str);
                unsigned short tngmodel = atoi(pr3str);
                PlayerNumber id = get_player_number_for_command(pr4str);
                thing = create_thing(&pos, tngclass, tngmodel, id, -1);
                return (!thing_is_invalid(thing));
            }
        }
        else if ( (strcasecmp(parstr, "slab.place") == 0) || (strcasecmp(parstr, "place.slab") == 0) )
        {
            if (pr2str != NULL)
            {
                player = get_player(plyr_idx);
                pckt = get_packet_direct(player->packet_num);
                MapSubtlCoord stl_x = coord_subtile(((unsigned short)pckt->pos_x));
                MapSubtlCoord stl_y = coord_subtile(((unsigned short)pckt->pos_y));
                MapSlabCoord slb_x = subtile_slab(stl_x);
                MapSlabCoord slb_y = subtile_slab(stl_y);
                slb = get_slabmap_block(slb_x, slb_y);
                if (!slabmap_block_invalid(slb))
                {
                    PlayerNumber id = (pr3str == NULL) ? slabmap_owner(slb) : get_player_number_for_command(pr3str);
                    short slbkind = get_rid(slab_desc, pr2str);
                    if (slbkind < 0)
                    {
                        long rid = get_rid(room_desc, pr2str);
                        if (rid > 0)
                        {
                            struct RoomConfigStats *roomst = get_room_kind_stats(rid);
                            slbkind = roomst->assigned_slab;
                        }
                        else
                        {
                            if (strcasecmp(pr2str, "Earth") == 0)
                            {
                                slbkind = rand() % (4 - 2) + 2;
                            }
                            else if ( (strcasecmp(pr2str, "Reinforced") == 0) || (strcasecmp(pr2str, "Fortified") == 0) )
                            {
                                slbkind = rand() % (9 - 4) + 4;
                            }
                            else if (strcasecmp(pr2str, "Claimed") == 0)
                            {
                                slbkind = SlbT_CLAIMED;
                            }
                            else if ( (strcasecmp(pr2str, "Rock") == 0) || (strcasecmp(pr2str, "Impenetrable") == 0) )
                            {
                                slbkind = SlbT_ROCK;
                            }
                            else
                            {
                                if (parameter_is_number(pr2str))
                                {
                                    slbkind = atoi(pr2str);
                                }
                            }
                        }
                    }
                    if ( (slbkind >= 0) && (slbkind <= slab_conf.slab_types_count) )
                    {
                        if (subtile_is_room(stl_x, stl_y)) 
                        {
                            delete_room_slab(slb_x, slb_y, true);
                        }
                        if (slab_kind_is_animated(slbkind))
                        {
                            place_animating_slab_type_on_map(slbkind, 0, stl_x, stl_y, id);  
                        }
                        else
                        {
                            place_slab_type_on_map(slbkind, stl_x, stl_y, id, 0);
                        }
                        do_slab_efficiency_alteration(slb_x, slb_y);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static TbBool script_set_pool(PlayerNumber plyr_idx, const char *creature, const char *str_num)
{
  if (creature == NULL)
    return false;
  long kind = get_id(creature_desc, creature);
  if (kind == -1)
  {
    if (0 == strcasecmp(creature, "EMPTY"))
    {
      clear_creature_pool();
      return true;
    }
    message_add_fmt(10, "Invalid creature");
    return false;
  }
  int num = atoi(str_num);
  if (num < 0)
    return false;
  game.pool.crtr_kind[kind] = num;
  return true;
}

long get_creature_model_for_command(char *msg)
{
    long rid = get_rid(creature_desc, msg);
    if (rid >= 1)
    {
        return rid;
    }
    else
    {
        if (strcasecmp(msg, "beetle") == 0)
        {
            return 24;
        }
        else if (strcasecmp(msg, "mistress") == 0)
        {
            return 20;
        }
        else if (strcasecmp(msg, "biledemon") == 0)
        {
            return 22;
        }
        else if (strcasecmp(msg, "hound") == 0)
        {
            return 27;
        }
        else if (strcasecmp(msg, "priestess") == 0)
        {
            return 9;
        }
        else if ( (strcasecmp(msg, "warlock") == 0) || (strcasecmp(msg, "sorcerer") == 0) )
        {
            return 21;
        }
        else if ( (strcasecmp(msg, "reaper") == 0) || (strcasecmp(msg, "hornedreaper") == 0) )
        {
            return 14;
        }
        else if ( (strcasecmp(msg, "dwarf") == 0) || (strcasecmp(msg, "mountaindwarf") == 0) )
        {
            return 5;
        }
        else if ( (strcasecmp(msg, "spirit") == 0) || (strcasecmp(msg, "floatingspirit") == 0) )
        {
            return 31;
        }
        else
        {
            return -1;
        }    
    }
}

PlayerNumber get_player_number_for_command(char *msg)
{
    PlayerNumber id = (msg == NULL) ? my_player_number : get_rid(cmpgn_human_player_options, msg);
    if (id == -1)
    {
        if (strcasecmp(msg, "neutral") == 0)
        {
            id = game.neutral_player_num;
        }
        else
        {
            id = atoi(msg);
        }                            
    }
    return id;
}

TbBool parameter_is_number(char* parstr)
{
    for (int i = 0; parstr[i] != '\0'; i++)
    {
        if (!isdigit(parstr[i]))
        {
            return false;
        }
    }
    return true;
}

#ifdef __cplusplus
}
#endif
