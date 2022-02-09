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
#include "bflib_sndlib.h"
#include "config.h"
#include "config_campaigns.h"
#include "config_effects.h"
#include "config_magic.h"
#include "config_rules.h"
#include "config_settings.h"
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

long cmd_comp_checks_update(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *args)
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
    struct ScriptValue tmp_value = {0};
    if (strcasecmp(parstr, "stats") == 0)
    {
      message_add_fmt(plyr_idx, "Now time is %d, last loop time was %d",LbTimerClock(),last_loop_time);
      message_add_fmt(plyr_idx, "clock is %d, requested fps is %d",clock(),game.num_fps);
      return true;
    } 
    else if (strcasecmp(parstr, "fps") == 0)
    {
        if (pr2str == NULL)
        {
            message_add_fmt(plyr_idx, "Framerate is %d fps", game.num_fps);
        }
        else
        {
            game.num_fps = atoi(pr2str);
        }
        return true;
    }
    else if (strcasecmp(parstr, "quit") == 0)
    {
        quit_game = 1;
        exit_keeper = 1;
        return true;
    } 
    else if (strcasecmp(parstr, "time") == 0)
    {
        unsigned long turn = (pr2str != NULL) ? atoi(pr2str) : game.play_gameturn;
        unsigned char frames = (pr3str != NULL) ? atoi(pr3str) : game.num_fps;
        show_game_time_taken(frames, turn);
        return true;
    }
    else if (strcasecmp(parstr, "timer.toggle") == 0)
    {
        game_flags2 ^= GF2_Timer;
        player = get_player(plyr_idx);
        if ( (player->victory_state == VicS_WonLevel) && (timer_enabled()) && (TimerGame) )
        {
            dungeon = get_my_dungeon();
            TimerTurns = dungeon->lvstats.hopes_dashed;
        }
        return true;
    }
    else if (strcasecmp(parstr, "timer.switch") == 0)
    {
        TimerGame ^= 1;
        player = get_player(plyr_idx);
        if ( (player->victory_state == VicS_WonLevel) && (timer_enabled()) && (TimerGame) )
        {
            dungeon = get_my_dungeon();
            TimerTurns = dungeon->lvstats.hopes_dashed;
        }
        return true;
    }
    else if (strcasecmp(parstr, "turn") == 0)
    {
        message_add_fmt(plyr_idx, "turn %ld", game.play_gameturn);
        return true;
    } 
    else if (strcasecmp(parstr, "game.save") == 0)
    {
        long slot_num = atoi(pr2str);
        if ( (pr3str != NULL) && (slot_num <= TOTAL_SAVE_SLOTS_COUNT) )
        {
            fill_game_catalogue_slot(slot_num, pr3str);
        }
        player = get_player(plyr_idx);
        set_flag_byte(&game.operation_flags,GOF_Paused,true); // games are saved in a paused state
        TbBool result = save_game(slot_num);
        if (result)
        {
            output_message(SMsg_GameSaved, 0, true);
        }
        else
        {
          ERRORLOG("Error in save!");
          create_error_box(GUIStr_ErrorSaving);
        }
        set_flag_byte(&game.operation_flags,GOF_Paused,false); // unpause after save attempt
        return result;
    }
    else if (strcasecmp(parstr, "game.load") == 0)
    {
        long slot_num = atoi(pr2str);
        TbBool Pause = atoi(pr3str);
        if (is_save_game_loadable(slot_num))
        {
            if (load_game(slot_num))
            {
                set_flag_byte(&game.operation_flags,GOF_Paused,Pause); // unpause, because games are saved whilst paused
                return true;
            }
            else
            {
                message_add_fmt(plyr_idx, "Unable to load game %d", slot_num);
            }
        }
        else
        {
            message_add_fmt(plyr_idx, "Unable to load game %d", slot_num);
        }
    }
    else if (strcasecmp(parstr, "cls") == 0)
    {
        zero_messages();
        return true;
    }
    else if (strcasecmp(parstr, "ver") == 0)
    {
        message_add_fmt(plyr_idx, "%s", PRODUCT_VERSION);
        return true;
    }
    else if (strcasecmp(parstr, "volume") == 0)
    {
        message_add_fmt(plyr_idx, "%s: %d %s: %d", get_string(340), settings.sound_volume, get_string(341), settings.redbook_volume);
        return true;        
    }
    else if ( (strcasecmp(parstr, "volume.sound") == 0) || (strcasecmp(parstr, "volume.sfx") == 0) )
    {
        if (pr2str != NULL)
        {
            if (parameter_is_number(pr2str))
            {
                settings.sound_volume = atoi(pr2str);
                if (settings.sound_volume > 127)
                {
                    settings.sound_volume = 127;
                }
                save_settings();
                SetSoundMasterVolume(settings.sound_volume);
                return true;
            }
        }
    }
    else if ( (strcasecmp(parstr, "volume.music") == 0) || (strcasecmp(parstr, "volume.soundtrack") == 0) )
    {
        if (pr2str != NULL)
        {
            if (parameter_is_number(pr2str))
            {
                settings.redbook_volume = atoi(pr2str);
                if (settings.redbook_volume > 127)
                {
                    settings.redbook_volume = 127;
                }
                save_settings();
                SetMusicPlayerVolume(settings.redbook_volume);
                return true;
            }
        }
    }
    else if ((game.flags_font & FFlg_AlexCheat) != 0)
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
            player = get_my_player();
            reveal_whole_map(player);
            return true;
        } else if (strcasecmp(parstr, "comp.kill") == 0)
        {
            if (pr2str == NULL)
                return false;
            int id = atoi(pr2str);
            if (id < 0 || id > PLAYERS_COUNT)
                return false;
            thing = get_player_soul_container(id);
            thing->health = 0;
            
        }
        else if (strcasecmp(parstr, "player.score") == 0)
        {
            PlayerNumber id = get_player_number_for_command(pr2str);
            dungeon = get_dungeon(id);
            if (!dungeon_invalid(dungeon))
            {
                message_add_fmt(plyr_idx, "Player %d score: %ld", id, dungeon->total_score);
                return true;                
            }
        }
        else if (strcasecmp(parstr, "player.flag") == 0)
        {
            PlayerNumber id = get_player_number_for_command(pr2str);
            unsigned char flg_id = atoi(pr3str);
            if (flg_id < SCRIPT_FLAGS_COUNT)
            {
                dungeon = get_dungeon(id);
                if (!dungeon_invalid(dungeon))
                {
                    if (pr4str == NULL)
                    {
                        message_add_fmt(plyr_idx, "Player %d flag %d value: %d", id, flg_id, dungeon->script_flags[flg_id]);
                    }
                    else
                    {
                        dungeon->script_flags[flg_id] = atoi(pr4str);
                    }
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "comp.me") == 0)
        {
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
        } else if ( (strcasecmp(parstr, "give.trap") == 0) || (strcasecmp(parstr, "trap.give") == 0) )
        {
            long id = get_trap_number_for_command(pr2str);
            if (id <= 0 || id > gameadd.trapdoor_conf.trap_types_count)
            {
                return false;
            }
            unsigned char num = (pr3str != NULL) ? atoi(pr3str) : 1;
            set_trap_buildable_and_add_to_amount(plyr_idx, id, 1, num);
            update_trap_tab_to_config();
            message_add(plyr_idx, "done!");
            return true;
        } else if ( (strcasecmp(parstr, "give.door") == 0) || (strcasecmp(parstr, "door.give") == 0) )
        {
            long id = get_door_number_for_command(pr2str);
            if (id <= 0 || id > gameadd.trapdoor_conf.door_types_count)
            {
                return false;
            }
            unsigned char num = (pr3str != NULL) ? atoi(pr3str) : 1;
            set_door_buildable_and_add_to_amount(plyr_idx, id, 1, num);
            update_trap_tab_to_config();
            message_add(plyr_idx, "done!");
            return true;
        } else if ( (strcasecmp(parstr, "map.pool") == 0) || (strcasecmp(parstr, "creature.pool") == 0) )
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
                    if (ObjModel >= 0)
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
                short tngclass = -1;
                short tngmodel = -1;
                if (strcasecmp(pr2str, "object") == 0)
                {
                    tngclass = TCls_Object;
                    tngmodel = get_rid(object_desc, pr3str);
                }
                else if (strcasecmp(pr2str, "corpse") == 0)
                {
                    tngclass = TCls_DeadCreature;
                    tngmodel = get_creature_model_for_command(pr3str);
                }
                else if (strcasecmp(pr2str, "creature") == 0)
                {
                    tngclass = TCls_Creature;
                    tngmodel = get_creature_model_for_command(pr3str);
                }
                else if (strcasecmp(pr2str, "trap") == 0)
                {
                    tngclass = TCls_Trap;
                    tngmodel = get_trap_number_for_command(pr3str);
                }
                else if (strcasecmp(pr2str, "door") == 0)
                {
                    tngclass = TCls_Door;
                    tngmodel = get_door_number_for_command(pr3str);
                }
                else if (strcasecmp(pr2str, "effect") == 0)
                {
                    tngclass = TCls_Effect;
                    tngmodel = get_rid(effect_desc, pr3str);
                }
                else if (strcasecmp(pr2str, "shot") == 0)
                {
                    tngclass = TCls_Shot;
                    tngmodel = get_rid(shot_desc, pr3str);
                }
                if (tngclass < 0)
                {
                    if (parameter_is_number(pr2str))
                    {
                        tngclass = atoi(pr2str);
                    }
                }
                if (tngmodel < 0)
                {
                    if (parameter_is_number(pr3str))
                    {
                        tngmodel = atoi(pr3str);
                    }
                }
                if ( (tngclass >= 0) && (tngmodel >= 0) )
                {
                    player = get_player(plyr_idx);
                    pckt = get_packet_direct(player->packet_num);
                    pos.x.stl.num = coord_subtile(((unsigned short)pckt->pos_x));
                    pos.y.stl.num = coord_subtile(((unsigned short)pckt->pos_y));
                    if (!subtile_coords_invalid(pos.x.stl.num, pos.y.stl.num))
                    {
                        pos.z.val = get_floor_height(pos.x.stl.num, pos.y.stl.num);
                        PlayerNumber id = get_player_number_for_command(pr4str);
                        thing = create_thing(&pos, tngclass, tngmodel, id, -1);
                        if (!thing_is_invalid(thing))
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
        else if (strcasecmp(parstr, "room.available") == 0)
        {
            TbBool available = (pr3str == NULL) ? 1 : atoi(pr3str);
            PlayerNumber id = get_player_number_for_command(pr4str);
            long roomid;
            if (strcasecmp(pr2str, "all") == 0)
            {
                for (roomid = RoK_TREASURE; roomid <= RoK_GUARDPOST; roomid++)
                {
                    if (roomid != RoK_DUNGHEART)
                    {
                        set_room_available(id, roomid, available, available);
                    }                   
                }
            }
            else
            {
                roomid = get_rid(room_desc, pr2str);                
                if (roomid <= 0)
                {
                    if (strcasecmp(pr2str, "Hatchery" ) == 0)
                    {
                        roomid = RoK_GARDEN;
                    }
                    else if ( (strcasecmp(pr2str, "Guard" ) == 0) || (strcasecmp(pr2str, "GuardPost" ) == 0) )
                    {
                        roomid = RoK_GUARDPOST;
                    }
                    else
                    {
                        roomid = atoi(pr2str);
                    }
                }
                set_room_available(id, roomid, available, available);
            }
            update_room_tab_to_config();
            return true;
        }
        else if ( (strcasecmp(parstr, "power.give") == 0) || (strcasecmp(parstr, "spell.give") == 0) )
        {
            if (strcasecmp(pr2str, "all") == 0)
            {
                for (PowerKind pw = PwrK_ARMAGEDDON; pw > PwrK_HAND; pw--)
                {
                    if (!set_power_available(plyr_idx, pw, 1, 1))
                        WARNLOG("Setting power %s availability for player %d failed.",power_code_name(pw),1);
                }
                update_powers_tab_to_config();
                return true; 
            }
            else
            {
                long power = get_rid(power_desc, pr2str);
                if (power < 0)
                {
                    power = atoi(pr2str);
                }
                if (!set_power_available(plyr_idx, power, 1, 1))
                        WARNLOG("Setting power %s availability for player %d failed.",power_code_name(power),1);
                update_powers_tab_to_config();
                return true;
            }                
        }
        else if (strcasecmp(parstr, "player.heart.health") == 0)
        {
            PlayerNumber id = get_player_number_for_command(pr2str);
            thing = get_player_soul_container(id);
            if (thing_is_dungeon_heart(thing))
            {
                if (pr3str == NULL)
                {
                    float percent = ((float)thing->health / (float)game.dungeon_heart_health) * 100;
                    message_add_fmt(plyr_idx, "Player %d heart health: %ld (%.2f per cent)", id, thing->health, percent);
                    return true;
                }
                else
                {
                    short Health = atoi(pr3str);
                    thing->health = Health;
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "creature.available") == 0)
        {
            long crmodel = get_creature_model_for_command(pr2str);
            TbBool available = (pr3str == NULL) ? 1 : atoi(pr3str);
            PlayerNumber id = get_player_number_for_command(pr4str);
            if (!set_creature_available(id, crmodel, available, available))
              WARNLOG("Setting creature %s availability for player %d failed.",creature_code_name(crmodel),(int)id);
          
            return true;
        }
        else if ( (strcasecmp(parstr, "creature.addhealth") == 0) || (strcasecmp(parstr, "creature.health.add") == 0) )
        {
            player = get_my_player();
            thing = thing_get(player->influenced_thing_idx);
            if (thing_is_creature(thing))
            {
                thing->health += atoi(pr2str);
                return true;
            }
        }
        else if ( (strcasecmp(parstr, "creature.subhealth") == 0) || (strcasecmp(parstr, "creature.health.sub") == 0) )
        {
            player = get_my_player();
            thing = thing_get(player->influenced_thing_idx);
            if (thing_is_creature(thing))
            {
                thing->health -= atoi(pr2str);
                return true;
            }
        }
        else if (strcasecmp(parstr, "digger.sendto") == 0)
        {
            PlayerNumber id = get_player_number_for_command(pr2str);
            player = get_my_player();
            thing = thing_get(player->influenced_thing_idx);
            if (thing_is_creature(thing))
            {
                if (thing->model == get_players_special_digger_model(thing->owner))
                {
                    player = get_player(id);
                    if (player_exists(player))
                    {
                        get_random_position_in_dungeon_for_creature(id, CrWaS_WithinDungeon, thing, &pos);
                        return send_tunneller_to_point_in_dungeon(thing, id, &pos);
                    }
                }
            }
        }
        else if (strcasecmp(parstr, "creature.instance.set") == 0)
        {
            if (pr2str != NULL)
            {
                player = get_player(plyr_idx);
                thing = thing_get(player->influenced_thing_idx);
                unsigned char inst = atoi(pr2str);
                if (thing_is_creature(thing))
                {
                    set_creature_instance(thing, inst, 0, 0, 0);
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "creature.state.set") == 0)
        {
            if (pr2str != NULL)
            {
                player = get_player(plyr_idx);
                thing = thing_get(player->influenced_thing_idx);
                if (thing_is_creature(thing))
                {
                        unsigned char state = atoi(pr2str);
                        if (can_change_from_state_to(thing, thing->active_state, state))
                        {
                            return internal_set_thing_state(thing, state);
                        }
                }
            }
        }
        else if (strcasecmp(parstr, "creature.job.set") == 0)
        {
            if (pr2str != NULL)
            {
                player = get_player(plyr_idx);
                thing = thing_get(player->influenced_thing_idx);
                if (thing_is_creature(thing))
                {
                    unsigned char new_job = atoi(pr2str);
                    if (creature_can_do_job_for_player(thing, thing->owner, 1LL<<new_job, JobChk_None))
                    {
                        return send_creature_to_job_for_player(thing, thing->owner, 1LL<<new_job);
                    }
                    else
                    {
                        message_add_fmt(plyr_idx, "Cannot do job %d.", new_job);
                        return true;
                    }
                }
            }
        }
        else if (strcasecmp(parstr, "creature.attackheart") == 0)
        {
            if (pr2str != NULL)
            {
                player = get_player(plyr_idx);
                thing = thing_get(player->influenced_thing_idx);
                if (thing_is_creature(thing))
                {
                    PlayerNumber id = get_player_number_for_command(pr2str);
                    struct Thing* heartng = get_player_soul_container(id);
                    if (thing_is_dungeon_heart(heartng))
                    {
                        TRACE_THING(heartng);
                        set_creature_object_combat(thing, heartng);
                        return true;
                    }
                }
            }
        }
        else if ( (strcasecmp(parstr, "player.addgold") == 0) || (strcasecmp(parstr, "player.gold.add") == 0) )
        {
            PlayerNumber id = get_player_number_for_command(pr2str);
            player = get_player(id);
            if (player_exists(player))
            {
                if (pr3str == NULL)
                {
                    return false;
                }
                else
                {
                    player_add_offmap_gold(id, atoi(pr3str));
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "thing.get") == 0)
        {
            player = get_player(plyr_idx);
            pckt = get_packet_direct(player->packet_num);
            MapSubtlCoord stl_x = coord_subtile(((unsigned short)pckt->pos_x));
            MapSubtlCoord stl_y = coord_subtile(((unsigned short)pckt->pos_y));
            thing = (pr2str != NULL) ? thing_get(atoi(pr2str)) : get_nearest_object_at_position(stl_x, stl_y);
            if (!thing_is_invalid(thing))
            {
                message_add_fmt(plyr_idx, "Got thing ID %d", thing->index);
                player->influenced_thing_idx = thing->index;
                return true;
            }
        }
        else if (strcasecmp(parstr, "thing.health") == 0)
        {
            player = get_player(plyr_idx);
            thing = thing_get(player->influenced_thing_idx);
            if (!thing_is_invalid(thing))
            {
                if (pr2str != NULL)
                {
                    thing->health = atoi(pr2str);
                }
                else
                {
                    message_add_fmt(plyr_idx, "Thing ID: %d health: %d", thing->index, thing->health);
                }
                return true;
            }
        }
        else if (strcasecmp(parstr, "thing.pos") == 0)
        {
            player = get_player(plyr_idx);
            thing = thing_get(player->influenced_thing_idx);
            if (pr4str != NULL)
            {
                pos.x.stl.num = atoi(pr2str);
                pos.y.stl.num = atoi(pr3str);
                pos.z.stl.num = atoi(pr4str);
                move_thing_in_map(thing, &pos);                
            }
            else if (!thing_is_invalid(thing))
            {
                message_add_fmt(plyr_idx, "Thing ID %d X: %d Y: %d Z: %d", thing->index, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->mappos.z.stl.num);
                return true;
            }
        }
        else if (strcasecmp(parstr, "thing.destroy") == 0)
        {
            player = get_player(plyr_idx);
            thing = thing_get(player->influenced_thing_idx);
            if (!thing_is_invalid(thing))
            {
                destroy_object(thing);
                return true;
            }
        }
        else if (strcasecmp(parstr, "room.get") == 0 )
        {
            player = get_player(plyr_idx);
            pckt = get_packet_direct(player->packet_num);
            MapSubtlCoord stl_x = coord_subtile(((unsigned short)pckt->pos_x));
            MapSubtlCoord stl_y = coord_subtile(((unsigned short)pckt->pos_y));
            room = (pr2str != NULL) ? room_get(atoi(pr2str)) : subtile_room_get(stl_x, stl_y);
            if (room_exists(room))
            {
                message_add_fmt(plyr_idx, "Got room ID %d", room->index);
                player->influenced_thing_idx = room->index;
                return true;
            }
        }
        else if (strcasecmp(parstr, "room.health") == 0)
        {
            player = get_player(plyr_idx);
            room = room_get(player->influenced_thing_idx);
            if (!room_is_invalid(room))
            {
                if (pr2str == NULL)
                {
                    message_add_fmt(plyr_idx, "Room ID %d health: %d", room->index, room->health);
                    return true;
                }
                else
                {
                    room-> health = atoi(pr2str);
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "slab.health") == 0)
        {
            player = get_player(plyr_idx);
            pckt = get_packet_direct(player->packet_num);
            MapSubtlCoord stl_x = coord_subtile(((unsigned short)pckt->pos_x));
            MapSubtlCoord stl_y = coord_subtile(((unsigned short)pckt->pos_y));
            slb = get_slabmap_for_subtile(stl_x, stl_y);
            if (!slabmap_block_invalid(slb))
            {
                if (pr2str == NULL)
                {                    
                    message_add_fmt(plyr_idx, "Slab health: %d", slb->health);
                    return true;
                }
                else
                {
                    slb->health = atoi(pr2str);
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "creature.pool.add") == 0)
        {
            if (pr3str != NULL)
            {
                long crmodel = get_creature_model_for_command(pr2str);
                if (crmodel == -1)
                {
                    crmodel = atoi(pr2str);
                }
                game.pool.crtr_kind[crmodel] += atoi(pr3str);
                return true;
            }
        }
        else if ( (strcasecmp(parstr, "creature.pool.sub") == 0) || (strcasecmp(parstr, "creature.pool.remove") == 0) )
        {
            if (pr3str != NULL)
            {
                long crmodel = get_creature_model_for_command(pr2str);
                if (crmodel == -1)
                {
                    crmodel = atoi(pr2str);
                }
                game.pool.crtr_kind[crmodel] -= atoi(pr3str);
                return true;
            }
        }
        else if (strcasecmp(parstr, "creature.level") == 0)
        {
            if (pr2str != NULL)
            {
                player = get_player(plyr_idx);
                thing = thing_get(player->influenced_thing_idx);
                if (thing_is_creature(thing))
                {
                    set_creature_level(thing, (atoi(pr2str)-1));
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "creature.freeze") == 0)
        {
            player = get_player(plyr_idx);
            thing = thing_get(player->influenced_thing_idx);
            if (thing_is_creature(thing))
            {
                thing_play_sample(thing, 50, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
                apply_spell_effect_to_thing(thing, SplK_Freeze, 8);
                return true;
            }
        }
        else if (strcasecmp(parstr, "creature.slow") == 0)
        {
            player = get_player(plyr_idx);
            thing = thing_get(player->influenced_thing_idx);
            if (thing_is_creature(thing))
            {
                thing_play_sample(thing, 50, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
                apply_spell_effect_to_thing(thing, SplK_Slow, 8);
                return true;
            }
        }
        else if (strcasecmp(parstr, "music.set") == 0)
        {
            if (pr2str != NULL)
            {
                int track = atoi(pr2str);
                if (track >= FIRST_TRACK && track <= max_track)
                {
                    StopMusicPlayer();
                    game.audiotrack = track;
                    PlayMusicPlayer(track);
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "zoomto") == 0)
        {
            if ( (pr2str != NULL) && (pr3str != NULL) )
            {
                MapSubtlCoord stl_x = atoi(pr2str);
                MapSubtlCoord stl_y = atoi(pr3str);
                if (!subtile_coords_invalid(stl_x, stl_y))
                {
                    player = get_player(plyr_idx);
                    player->zoom_to_pos_x = subtile_coord_center(stl_x);
                    player->zoom_to_pos_y = subtile_coord_center(stl_y);
                    set_player_instance(player, PI_ZoomToPos, 0);
                    return true;
                }
                else
                {
                    message_add_fmt(plyr_idx, "Co-ordinates specified are invalid");
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "bug.toggle") == 0)
        {
            if (pr2str != NULL)
            {
                char bug = get_rid(rules_game_classicbugs_commands, pr2str);
                if (bug == -1)
                {
                    bug = atoi(pr2str);
                }
                unsigned long flg = (bug > 2) ? (1 << (bug - 1)) : bug;
                toggle_flag_dword(&gameadd.classic_bugs_flags, flg);
                message_add_fmt(plyr_idx, "%s %s", get_conf_parameter_text(rules_game_classicbugs_commands, bug), ((gameadd.classic_bugs_flags & flg) != 0) ? "enabled" : "disabled");
                return true;
            }
        }
        else if (strcasecmp(parstr, "actionpoint.pos") == 0)
        {
            if (pr2str != NULL)
            {
                unsigned char ap = atoi(pr2str);
                if (action_point_exists_idx(ap))
                {
                    struct ActionPoint* actionpt = action_point_get(ap);
                    message_add_fmt(plyr_idx, "Action Point %d X: %d Y: %d", ap, actionpt->mappos.x.stl.num, actionpt->mappos.y.stl.num);
                    return true;
                }
            }            
        }
        else if (strcasecmp(parstr, "actionpoint.zoomto") == 0)
        {
            if (pr2str != NULL)
            {
                unsigned char ap = atoi(pr2str);
                if (action_point_exists_idx(ap))
                {
                    struct ActionPoint* actionpt = action_point_get(ap);
                    player = get_player(plyr_idx);
                    player->zoom_to_pos_x = subtile_coord_center(actionpt->mappos.x.stl.num);
                    player->zoom_to_pos_y = subtile_coord_center(actionpt->mappos.y.stl.num);
                    set_player_instance(player, PI_ZoomToPos, 0);
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "actionpoint.reset") == 0)
        {
            if (pr2str != NULL)
            {
                unsigned char ap = atoi(pr2str);
                if (action_point_exists_idx(ap))
                {
                    return action_point_reset_idx(ap);
                }
            }
        }
        else if (strcasecmp(parstr, "herogate.zoomto") == 0)
        {
            if (pr2str != NULL)
            {
                unsigned char hg = atoi(pr2str);
                thing = find_hero_gate_of_number(hg);
                if ( (thing_is_object(thing)) && (object_is_hero_gate(thing)) )
                {
                    player = get_player(plyr_idx);
                    player->zoom_to_pos_x = subtile_coord_center(thing->mappos.x.stl.num);
                    player->zoom_to_pos_y = subtile_coord_center(thing->mappos.y.stl.num);
                    set_player_instance(player, PI_ZoomToPos, 0);
                    return true;
                }
            }
        }
        else if (strcasecmp(parstr, "sound.test") == 0)
        {
            if (pr2str != NULL)
            {
                play_non_3d_sample(atoi(pr2str));
                return true;
            }
        }
        else if (strcasecmp(parstr, "speech.test") == 0)
        {
            if (pr2str != NULL)
            {
                play_speech_sample(atoi(pr2str));
                return true;
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
        id = get_rid(player_desc, msg);
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
    }
    return id;
}

TbBool parameter_is_number(const char* parstr)
{
    for (int i = 0; parstr[i] != '\0'; i++)
    {
        TbBool digit = (i == 0) ? ( (parstr[i] == 0x2D) || (isdigit(parstr[i])) ) : (isdigit(parstr[i]));
        if (!digit)
        {
            return false;
        }
    }
    return true;
}

char get_trap_number_for_command(char* msg)
{
    char id = get_rid(trap_desc, msg);
    if (id < 0)
    {
        if ( (strcasecmp(msg, "gas") == 0) || (strcasecmp(msg, "poison") == 0) || (strcasecmp(msg, "poisongas") == 0) )
        {
            id = 3;
        }
        else if ( (strcasecmp(msg, "word") == 0) || (strcasecmp(msg, "wordofpower") == 0) )
        {
            id = 5;
        }
        else
        {
            if (parameter_is_number(msg))
            {
                id = atoi(msg);
            }
        }
    }
    return id;
}

char get_door_number_for_command(char* msg)
{
    long id = get_rid(door_desc, msg);
    if (id < 0)
    {
        if (strcasecmp(msg, "wooden") == 0)
        {
           id = 1;
        }
        else if (strcasecmp(msg, "iron") == 0)
        {
            id = 3;
        }
        else if (strcasecmp(msg, "magical") == 0)
        {
            id = 4;
        }
        else
        {
            if (parameter_is_number(msg))
            {
                id = atoi(msg);
            }
        }
    }
    return id;
}

#ifdef __cplusplus
}
#endif
