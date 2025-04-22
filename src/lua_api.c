#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bflib_basics.h"
#include "bflib_sndlib.h"
#include "globals.h"
#include "thing_data.h"
#include "creature_states.h"
#include "creature_states_pray.h"
#include "gui_msgs.h"
#include "thing_navigate.h"
#include "map_data.h"
#include "game_legacy.h"
#include "player_utils.h"
#include "lvl_script_lib.h"
#include "room_library.h"
#include "room_util.h"
#include "keeperfx.hpp"
#include "power_specials.h"
#include "thing_creature.h"
#include "thing_effects.h"
#include "magic_powers.h"

#include "lua_base.h"
#include "lua_params.h"


#include "post_inc.h"

/**********************************************/



/***************************************************************************************************/
/************    Api Functions    ******************************************************************/
/***************************************************************************************************/

//Setup Commands

static int lua_Set_generate_speed(lua_State *L)
{
    GameTurnDelta interval   = luaL_checkinteger(L,1);

    game.generate_speed = saturate_set_unsigned(interval, 16);
    update_dungeon_generation_speeds();
    return 0;
}

static int lua_Computer_player(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L,1);
    if (lua_isnumber(L, 2))
    {
        long attitude       = luaL_checkint(L,2);
        script_support_setup_player_as_computer_keeper(player_idx, attitude);
        return 0;
    }

    const char* comp_model = luaL_checkstring(L,2);

    if(strcasecmp(comp_model,"ROAMING") == 0)
    {

        struct PlayerInfo* player = get_player(player_idx);
        player->player_type = PT_Roaming;
        player->allocflags |= PlaF_Allocated;
        player->allocflags |= PlaF_CompCtrl;
        player->id_number = player_idx;
        return 0;
        
    }
    else
    {
        luaL_error(L,"invalid Computer_player param '%s'",comp_model);
        return 0;
    }
}

static int lua_Ally_players(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    PlayerNumber player_idx = luaL_checkPlayerSingle(L,2);
    uchar state = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_ally_with_player(i, player_idx,   (state & 1) ? true : false);
        set_ally_with_player(player_idx, i,   (state & 1) ? true : false);
        set_player_ally_locked(i, player_idx, (state & 2) ? true : false);
        set_player_ally_locked(player_idx, i, (state & 2) ? true : false);
    }
    return 0;
}

static int lua_Start_money(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    GoldAmount gold_val = luaL_checkinteger(L, 2);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        if (gold_val > SENSIBLE_GOLD)
        {
            gold_val = SENSIBLE_GOLD;
            SCRPTWRNLOG("Gold added to player reduced to %d", SENSIBLE_GOLD);
        }
        player_add_offmap_gold(i, gold_val);
    }
    return 0;

}

static int lua_Max_creatures(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long max_amount                 = luaL_checkinteger(L, 2);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        SYNCDBG(4,"Setting player %d max attracted creatures to %d.",(int)i,(int)max_amount);
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        dungeon->max_creatures_attracted = max_amount;
    }
    return 0;
}

static int lua_Add_creature_to_pool(lua_State *L)
{
    long crtr_model = luaL_checkNamedCommand(L,1,creature_desc);
    long amount     = luaL_checkinteger(L, 2);

    add_creature_to_pool(crtr_model, amount);
    return 0;
}

static int lua_Creature_available(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long cr_kind                    = luaL_checkNamedCommand(L,2,creature_desc);
    TbBool can_be_attracted         = lua_toboolean(L, 3);
    long amount_forced              = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        if (!set_creature_available(i,cr_kind,can_be_attracted,amount_forced))
            WARNLOG("Setting creature %s availability for player %d failed.",creature_code_name(cr_kind),(int)i);
    }
    return 0;
}

static int lua_Dead_creatures_return_to_pool(lua_State *L)
{
    TbBool return_to_pool         = lua_toboolean(L, 3);
    set_flag_value(game.flags_cd, MFlg_DeadBackToPool, return_to_pool);
    return 0;
}

static int lua_Room_available(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long rkind                      = luaL_checkNamedCommand(L,2,room_desc);
    TbBool can_be_available         = lua_tointeger(L, 3);
    TbBool is_available             = lua_toboolean(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_room_available(i,rkind,can_be_available,is_available);
    }
    return 0;
}

static int lua_Magic_available(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long power                      = luaL_checkNamedCommand(L,2,power_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    TbBool is_available             = lua_toboolean(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_power_available(i,power,can_be_available,is_available);
    }
    return 0;
}

static int lua_Door_available(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long door_type                  = luaL_checkNamedCommand(L,2,door_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    long number_available           = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_door_buildable_and_add_to_amount(i, door_type, can_be_available, number_available);
    }
    return 0;
}

static int lua_Trap_available(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long trap_type                  = luaL_checkNamedCommand(L,2,trap_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    long number_available           = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_trap_buildable_and_add_to_amount(i, trap_type, can_be_available, number_available);
    }
    return 0;
}




//Script flow control

static int lua_Win_game(lua_State *L)
{
    struct PlayerRange player_range;
    if(lua_isnone(L,1))
    {
        player_range.start_idx = 0;
        player_range.end_idx = PLAYERS_COUNT;
    }
    else
        player_range = luaL_checkPlayerRange(L, 1);


    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct PlayerInfo *player = get_player(i);
        set_player_as_won_level(player);
    }
    return 0;
}

static int lua_Lose_game(lua_State *L)
{
    struct PlayerRange player_range;
    if(lua_isnone(L,1))
    {
        player_range.start_idx = 0;
        player_range.end_idx = PLAYERS_COUNT;
    }
    else
        player_range = luaL_checkPlayerRange(L, 1);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct PlayerInfo *player = get_player(i);
        set_player_as_lost_level(player);
    }
    return 0;
}

static int lua_Count_creatures_at_action_point(lua_State *L)
{
    ActionPointId ap_idx = luaL_checkActionPoint(L, 1);
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 2);
    long crtr_model = luaL_checkCreature_or_creature_wildcard(L,3);

    long sum = 0;
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        sum += count_player_creatures_of_model_in_action_point(i, crtr_model, ap_idx);
    }
    lua_pushinteger(L, sum);
    return 1;
}

static int lua_Set_timer(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long timr_id              = luaL_checkNamedCommand(L,2,timer_desc);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        restart_script_timer(i, timr_id);
    }
    return 0;
}

static int lua_Add_to_timer(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long timr_id              = luaL_checkNamedCommand(L,2,timer_desc);
    long amount               = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        add_to_script_timer(i, timr_id, amount);
    }
    return 0;
}
static int lua_Display_timer(lua_State *L)
{
    PlayerNumber player_id = luaL_checkPlayerSingle(L, 1);
    long timr_id              = luaL_checkNamedCommand(L,2,timer_desc);
    long display              = luaL_checkinteger(L, 3);

    gameadd.script_timer_player = player_id;
    gameadd.script_timer_id = timr_id;
    gameadd.script_timer_limit = 0;
    gameadd.timer_real = display;
    game.flags_gui |= GGUI_ScriptTimer;
    return 0;
}

static int lua_Hide_timer(lua_State *L)
{
    game.flags_gui &= ~GGUI_ScriptTimer;
    return 0;
}
static int lua_Bonus_level_time(lua_State *L)
{
    GameTurn turns = luaL_checkinteger(L, 1);
    TbBool clocktime = lua_toboolean(L, 2);
    if (turns > 0)
    {
        game.bonus_time = game.play_gameturn + turns;
        set_flag(game.flags_gui, GGUI_CountdownTimer);
    }
    else
    {
        game.bonus_time = 0;
        clear_flag(game.flags_gui, GGUI_CountdownTimer);
    }
    gameadd.timer_real = clocktime;
    return 0;
}
static int lua_Add_bonus_time(lua_State *L)
{
    GameTurnDelta turns = luaL_checkinteger(L, 1);
    game.bonus_time += turns;
    return 0;
}


//Adding New Creatures and Parties to the Level

static int lua_Add_creature_to_level(lua_State *L)
{
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 1);
    long crtr_id           = luaL_checkNamedCommand(L,2,creature_desc);
    TbMapLocation location = luaL_checkLocation(L,  3);
    long crtr_level        = luaL_checkinteger(L, 4);
    long carried_gold      = luaL_checkinteger(L, 5);
    long spawn_type        = SpwnT_Default;

    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return 0;
    }

    // Recognize place where party is created
    if (location == 0)
        return 0;

    lua_pushThing(L,script_create_new_creature(plr_idx, crtr_id, location, carried_gold, crtr_level-1,spawn_type));
    
    return 1;
}

static int lua_Add_tunneller_to_level(lua_State *L)
{
    PlayerNumber plr_id          = luaL_checkPlayerSingle(L,1);
    TbMapLocation spawn_location = luaL_checkLocation(L,2);
    TbMapLocation head_for       = luaL_checkHeadingLocation(L,3); // checks 2 params
    long level                   = luaL_checkinteger(L,5);
    long gold_held               = luaL_checkinteger(L,6);

    struct Thing* thing = script_process_new_tunneler(plr_id, spawn_location, head_for, level-1, gold_held);

    lua_pushThing(L, thing);
    return 1;
}

static int lua_Create_party(lua_State *L)
{
    const char* party_name = luaL_checklstring(L, 1,NULL);
    create_party(party_name);
    return 0;
}
static int lua_Add_to_party(lua_State *L)
{
    long party_id          = luaL_checkParty(L,  1);
    long crtr_id           = luaL_checkNamedCommand(L,2,creature_desc);
    long experience        = luaL_checklong(L, 3);
    long gold              = luaL_checklong(L, 4);
    long objective_id      = luaL_checkNamedCommand(L, 5,hero_objective_desc);
    long countdown         = luaL_checklong (L, 6);


    if ((experience < 1) || (experience > CREATURE_MAX_LEVEL))
    {
      SCRPTERRLOG("Invalid Creature Level parameter; %ld not in range (%d,%d)",experience,1,CREATURE_MAX_LEVEL);
      return 0;
    }

    add_member_to_party(party_id, crtr_id, experience, gold, objective_id, countdown);
    return 0;
}

static int lua_Delete_from_party(lua_State *L)
{
    long party_id          = luaL_checkParty(L,  1);
    const char* creature   = lua_tostring(L,  2);
    long experience  = lua_tointeger(L, 3);

    long creature_id = get_rid(creature_desc, creature);
    if (creature_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", creature);
      return 0;
    }

    delete_member_from_party(party_id, creature_id, experience);
    return 0;
}

static int lua_Add_tunneller_party_to_level(lua_State *L)
{
    PlayerNumber owner           = luaL_checkPlayerSingle(L, 1);
    long prty_id                 = luaL_checkParty(L,  2);
    TbMapLocation spawn_location = luaL_checkLocation(L,  3);
    TbMapLocation head_for       = luaL_checkHeadingLocation(L,4); // checks 2 params
    long crtr_level              = luaL_checkCrtLevel(L, 6);
    GoldAmount carried_gold      = luaL_checkinteger(L, 7);


    struct Party* party = &gameadd.script.creature_partys[prty_id];
    if (party->members_num >= GROUP_MEMBERS_COUNT-1)
    {
        SCRPTERRLOG("Party too big for ADD_TUNNELLER (Max %d members)", GROUP_MEMBERS_COUNT-1);
        return 0;
    }

    struct Thing* leadtng = script_process_new_tunneller_party(owner, prty_id, spawn_location, head_for, crtr_level, carried_gold);
    if (thing_is_invalid(leadtng))
    { 
        return 0;
    }
    lua_pushPartyTable(L, leadtng);
    return 1;
}

static int lua_Add_party_to_level(lua_State *L)
{
    PlayerNumber owner     = luaL_checkPlayerSingle(L, 1);
    long prty_id           = luaL_checkParty(L,  2);
    TbMapLocation location = luaL_checkLocation(L,  3);

    // Recognize place where party is created
    if (location == 0)
        return 0;
    struct Party* party = &gameadd.script.creature_partys[prty_id];
    struct Thing* leadtng = script_process_new_party(party, owner, location, 1);

    if (thing_is_invalid(leadtng))
    { 
        return 0;
    }
    lua_pushPartyTable(L, leadtng);
    return 1;
}

//Displaying information and affecting interface

static int lua_Display_objective(lua_State *L)
{
    long msg_id    = luaL_checkinteger(L, 1);
    TbMapLocation zoom_location = luaL_optLocation(L,2);

    set_general_objective(msg_id,zoom_location,0,0);
    return 0;
}

static int lua_Display_objective_with_pos(lua_State *L)
{
    long msg_id   = luaL_checkinteger(L, 1);
    long stl_x    = luaL_checkstl_x(L, 2);
    long stl_y    = luaL_checkstl_y(L, 3);

    set_general_objective(msg_id,0,stl_x,stl_y);
    return 0;
}

static int lua_Display_information(lua_State *L)
{
    long msg_id    = luaL_checkinteger(L, 1);
    TbMapLocation zoom_location = luaL_optLocation(L,2);

    set_general_information(msg_id,zoom_location,0,0);
    return 0;
}

static int lua_Display_information_with_pos(lua_State *L)
{
    long msg_id    = luaL_checkinteger(L, 1);
    long stl_x    = luaL_checkstl_x(L, 2);
    long stl_y    = luaL_checkstl_y(L, 3);

    set_general_objective(msg_id,0,stl_x,stl_y);
    return 0;
}

static int lua_Quick_objective(lua_State *L)
{
    const char *msg_text = lua_tostring(L, 1);
    TbMapLocation target = luaL_checkLocation(L, 2);

    process_objective(msg_text, target, 0, 0);
    return 0;
}

static int lua_Quick_information(lua_State *L)
{
    long slot = luaL_checkIntMinMax(L, 1, 0,QUICK_MESSAGES_COUNT-1);
    const char *msg_text = lua_tostring(L, 2);
    TbMapLocation target = luaL_checkLocation(L, 3);
    snprintf(gameadd.quick_messages[slot], MESSAGE_TEXT_LEN, "%s", msg_text);

    set_quick_information(slot, target, 0, 0);
    return 0;
}

static int lua_Quick_objective_with_pos(lua_State *L)
{
    const char *msg_text = lua_tostring(L, 1);
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 3);

    process_objective(msg_text, 0, stl_x, stl_y);
    return 0;
}

static int lua_Quick_information_with_pos(lua_State *L)
{
    long slot = luaL_checkIntMinMax(L, 1, 0,QUICK_MESSAGES_COUNT-1);
    const char *msg_text = lua_tostring(L, 2);
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 3);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 4);
    snprintf(gameadd.quick_messages[slot], MESSAGE_TEXT_LEN, "%s", msg_text);

    set_quick_information(slot, 0, stl_x, stl_y);
    return 0;
}

static int lua_DisPlay_message(lua_State *L)
{
    int msg_id = luaL_checkinteger(L, 1);
    const char *msg =  get_string(msg_id);
    char id;
    char type;
    luaL_checkMessageIcon(L, 1, &type, &id);

    message_add(type,id, msg);

    return 0;
}

static int lua_Quick_message(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);
    char id;
    char type;
    luaL_checkMessageIcon(L, 2, &type, &id);

    message_add(type,id, msg);

    return 0;
}

static int lua_Heart_lost_objective(lua_State *L)
{
    long message_id = luaL_checkinteger(L, 1);
    TbMapLocation target = luaL_checkLocation(L, 2);

    gameadd.heart_lost_display_message = true;
    gameadd.heart_lost_quick_message = false;
    gameadd.heart_lost_message_id = message_id;
    gameadd.heart_lost_message_target = target; 
    return 0;
}
static int lua_Heart_lost_quick_objective(lua_State *L)
{
    long slot = luaL_checkIntMinMax(L, 1, 0,QUICK_MESSAGES_COUNT-1);
    const char *msg = lua_tostring(L, 2);
    TbMapLocation target = luaL_checkLocation(L, 3);

    snprintf(gameadd.quick_messages[slot], MESSAGE_TEXT_LEN, "%s", msg);

    gameadd.heart_lost_display_message = true;
    gameadd.heart_lost_quick_message = true;
    gameadd.heart_lost_message_id = slot;
    gameadd.heart_lost_message_target = target; 
    return 0;
}
//static int lua_Play_message(lua_State *L)

static int lua_Tutorial_flash_button(lua_State *L)
{
    long          button    = luaL_checkinteger(L, 1);
    GameTurnDelta gameturns = luaL_checkinteger(L, 2);

    gui_set_button_flashing(button,gameturns);
    return 0;
}

static int lua_Display_countdown(lua_State *L)
{
    PlayerNumber player   = luaL_checkPlayerSingle(L, 1);
    int timer = luaL_checkNamedCommand(L,2,timer_desc);
    int target = luaL_checkinteger(L,3);
    int clocktime = lua_toboolean(L,4);

    gameadd.script_timer_player = player;
    gameadd.script_timer_id = timer;
    gameadd.script_timer_limit = target;
    gameadd.timer_real = clocktime;
    game.flags_gui |= GGUI_ScriptTimer;
    return 0;    
}

static int lua_Display_variable(lua_State *L)
{
    PlayerNumber player   = luaL_checkPlayerSingle(L, 1);
    int variable = luaL_checkinteger(L,2);
    int target = luaL_checkinteger(L,3);

    gameadd.script_variable_player = player;
    gameadd.script_value_type = variable;
    gameadd.script_value_id = target;
    game.flags_gui |= GGUI_Variable;
    return 0;
}

static int lua_Hide_variable(lua_State *L)
{
    game.flags_gui &= ~GGUI_Variable;
    return 0;
}

//Manipulating Map

static int lua_Reveal_map_location(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    TbMapLocation target = luaL_checkLocation(L,  2);
    MapSubtlDelta range = luaL_checkinteger(L, 3);


    SYNCDBG(0, "Revealing location type %lu", target);
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        long x = 0;
        long y = 0;
        find_map_location_coords(target, &x, &y, i, __func__);
        if ((x == 0) && (y == 0))
        {
            WARNLOG("Can't decode location %lu", target);
            return 0;
        }
        if (range == -1)
        {
            struct CompoundCoordFilterParam iter_param;
            iter_param.plyr_idx = i;
            slabs_fill_iterate_from_slab(subtile_slab(x), subtile_slab(y), slabs_reveal_slab_and_corners, &iter_param);
        } else
            player_reveal_map_area(i, x, y,range,range);
    }
    return 0;

}
static int lua_Reveal_map_rect(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    MapSubtlCoord stl_x    = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y    = luaL_checkstl_y(L, 3);
    MapSubtlDelta width    = lua_tointeger(L,4);
    MapSubtlDelta height   = lua_tointeger(L,5);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        player_reveal_map_area(i,stl_x,stl_y,width,height);
    }
    return 0;
}
static int lua_Conceal_map_rect(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    MapSubtlCoord stl_x    = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y    = luaL_checkstl_y(L, 3);
    MapSubtlDelta width    = lua_tointeger(L,4);
    MapSubtlDelta height   = lua_tointeger(L,5);
    TbBool conceal_all     = lua_toboolean(L,6);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        player_conceal_map_area(i,stl_x,stl_y,width,height,conceal_all);
    }
    return 0;
}

static int lua_Set_door(lua_State *L)
{
    long doorAction = luaL_checkNamedCommand(L,1,locked_desc);
    MapSlabCoord slb_x = luaL_checkslb_x(L,2);
    MapSlabCoord slb_y = luaL_checkslb_y(L,3);

    struct Thing* doortng = get_door_for_position(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (!thing_is_invalid(doortng))
    {
        switch (doorAction)
        {
        case 0:
            unlock_door(doortng);
            break;
        case 1:
            lock_door(doortng);
            break;
        }
    }
    return 0;
}

static int lua_Add_heart_health(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L,1);
    HitPoints healthdelta = lua_tointeger(L,2);
    TbBool warn_on_damage = lua_toboolean(L,3);
    
    add_heart_health(plyr_idx,healthdelta,warn_on_damage);
    return 0;
}


static int lua_Add_object_to_level(lua_State *L)
{
    long obj_id            = luaL_checkNamedCommand(L,1,object_desc);
    TbMapLocation location = luaL_checkLocation(L,  2);
    long arg               = lua_tointeger(L,3);
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 4);
    short angle            = lua_tointeger(L, 5);

    struct Coord3d pos;
    if (!get_coords_at_location(&pos, location,true))
    {
        return 0;
    }
    lua_pushThing(L,script_process_new_object(obj_id, pos.x.stl.num, pos.y.stl.num, arg, plr_idx,angle));
    return 1;
}

static int lua_Add_object_to_level_at_pos(lua_State *L)
{
    long obj_id            = luaL_checkNamedCommand(L,1,object_desc);
    MapSubtlCoord stl_x    = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y    = luaL_checkstl_y(L, 3);
    long arg               = lua_tointeger(L,4);
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 5);
    short angle            = lua_tointeger(L, 6);

    lua_pushThing(L,script_process_new_object(obj_id, stl_x, stl_y, arg, plr_idx,angle));
    return 1;
}

static int lua_Add_effect_generator_to_level(lua_State *L)
{
    ThingModel gen_id      = luaL_checkNamedCommand(L,1,effectgen_desc);
    TbMapLocation location = luaL_checkLocation(L,  2);
    long range             = luaL_checkinteger(L, 3);

    lua_pushThing(L,script_process_new_effectgen(gen_id, location, range));
    return 1;
}

static int lua_Place_door(lua_State *L)
{
    PlayerNumber plyridx = luaL_checkPlayerSingle(L,1);
    long doorkind = luaL_checkNamedCommand(L,2,door_desc);
    MapSlabCoord slb_x = luaL_checkslb_x(L,3);
    MapSlabCoord slb_y = luaL_checkslb_y(L,4);
    TbBool locked = lua_toboolean(L,5);
    TbBool free = lua_toboolean(L,6);

    script_place_door(plyridx, doorkind, slb_x, slb_y, locked, free);
    return 0;
}

static int lua_Place_trap(lua_State *L)
{
    PlayerNumber plyridx = luaL_checkPlayerSingle(L,1);
    long trapkind = luaL_checkNamedCommand(L,2,trap_desc);
    MapSubtlCoord stl_x = luaL_checkstl_x(L,3);
    MapSubtlCoord stl_y = luaL_checkstl_y(L,4);
    TbBool free = lua_toboolean(L,5);

    script_place_trap(plyridx, trapkind, stl_x, stl_y, free);
    return 0;
}



//Manipulating Configs

static void set_configuration(lua_State *L, const struct NamedFieldSet* named_fields_set, const char* function_name)
{
    if (lua_gettop(L) < 2)
    {
        luaL_error(L, "Not enough arguments for %s", function_name);
        return;
    }
    if (!lua_isstring(L, 1))
    {
        luaL_error(L, "First argument must be a string for %s", function_name);
        return;
    }
    if (!lua_isstring(L, 2))
    {
        luaL_error(L, "Second argument must be a string for %s", function_name);
        return;
    }
    const char* id_str      = lua_tostring(L, 1);
    const char* property    = lua_tostring(L, 2);
    
    short id = get_id(named_fields_set->names, id_str);
    if (id == -1)
    {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Unknown %s, '%s'", named_fields_set->block_basename, id_str);
        luaL_argerror(L, 1, error_msg);
        return;
    }
    if (id > named_fields_set->max_count)
    {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "'%s%d' is out of range",named_fields_set->block_basename, id);
        luaL_argerror(L, 1, error_msg);
        return;
    }
    
    long property_id = get_named_field_id(named_fields_set->named_fields, property);
    if (property_id == -1)
    {    
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Expected a valid property name, got '%s'",property);
        luaL_argerror(L, 1, error_msg);
        return;
    }
    
    const struct NamedField* field = &named_fields_set->named_fields[property_id];
    
    char concatenated_values[MAX_TEXT_LENGTH];
    if (field->argnum == -1)
    {
        concatenated_values[0] = '\0';
        for (int i = 3; lua_tostring(L, i) != NULL; i++) {
            strncat(concatenated_values, lua_tostring(L, i), sizeof(concatenated_values) - strlen(concatenated_values) - 1);
            if (lua_tostring(L, i + 1) != NULL) {
            strncat(concatenated_values, " ", sizeof(concatenated_values) - strlen(concatenated_values) - 1);
            }
        }
        int64_t value = parse_named_field_value(field, concatenated_values,named_fields_set,id,function_name,ccf_DuringLevel);
        assign_named_field_value(&named_fields_set->named_fields[property_id],value,named_fields_set,id,function_name,ccf_DuringLevel);
        
    }
    else
    {
        int i = 0;
        while (lua_tostring(L, i + 3) != NULL)
        {    
            if( named_fields_set->named_fields[property_id + i].name == NULL || 
                (strcmp(named_fields_set->named_fields[property_id + i].name, named_fields_set->named_fields[property_id].name) != 0))
            {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "more values then expected for property: '%s' '%s'", property, lua_tostring(L, i + 3));
                luaL_argerror(L, 1, error_msg);
                return;
            }
            int64_t value = parse_named_field_value(&named_fields_set->named_fields[property_id + i], lua_tostring(L, i + 3),named_fields_set,id,function_name,ccf_DuringLevel);
            assign_named_field_value(&named_fields_set->named_fields[property_id + i],value,named_fields_set,id,function_name,ccf_DuringLevel);
            i++;
        }
    }
}

static int lua_Set_door_configuration(lua_State *L)
{
    set_configuration(L, &trapdoor_door_named_fields_set, "SET_DOOR_CONFIGURATION");
    return 0;
}

static int lua_Set_object_configuration(lua_State *L)
{
    set_configuration(L, &objects_named_fields_set, "SET_OBJECT_CONFIGURATION");
    return 0;
}

static int lua_Set_trap_configuration(lua_State *L)
{
    set_configuration(L, &trapdoor_trap_named_fields_set, "SET_TRAP_CONFIGURATION");
    return 0;
}

//static int lua_Set_creature_configuration(lua_State *L)
static int lua_Set_effect_generator_configuration(lua_State *L)
{
    set_configuration(L, &effects_effectgenerator_named_fields_set, "SET_EFFECT_GENERATOR_CONFIGURATION");
    return 0;
}

//static int lua_Set_power_configuration(lua_State *L)
//{
//    set_configuration(L, &terrain_room_named_fields_set, "SET_POWER_CONFIGURATION");
//    return 0;
//}

static int lua_Set_room_configuration(lua_State *L)
{
    set_configuration(L, &terrain_room_named_fields_set, "SET_ROOM_CONFIGURATION");
    return 0;
}

static int lua_Set_game_rule(lua_State *L)
{

    //TODO implement
    return 0;
}

static int lua_Set_hand_rule(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    long crtr_id = luaL_checkNamedCommand(L,2,creature_desc);
    long rule_slot = luaL_checkinteger(L, 3);
    long rule_action = luaL_checkNamedCommand(L,4,rule_action_desc);
    long rule = luaL_checkNamedCommand(L,4,rule_action_desc);
    long param = luaL_checkinteger(L, 5);
    
    script_set_hand_rule(player_idx, crtr_id, rule_action, rule_slot, rule, param);
    return 0;
}

static int lua_Swap_creature(lua_State *L)
{
    long crtr_id1 = luaL_checkNamedCommand(L,1,creature_desc);
    long crtr_id2 = luaL_checkNamedCommand(L,2,creature_desc);

    swap_creature(crtr_id1, crtr_id2);
    return 0;
}

static int lua_Set_sacrifice_recipe(lua_State *L)
{

    int command = luaL_checkNamedCommand(L,1,rules_sacrifices_commands);
    const char * reward_str = luaL_checkstring(L,1);
    int reward =  0;
    ThingModel victims[MAX_SACRIFICE_VICTIMS];
    for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
        long crtr_model  = luaL_optNamedCommand(L,i + 1,creature_desc);
        victims[i] = crtr_model;
    }


    if ((command == SacA_CustomPunish) || (command == SacA_CustomReward))
    {
        reward = get_id(flag_desc, reward_str) + 1;
    }
    else
    {
        reward = get_id(creature_desc, reward_str);
        if (reward == -1)
        {
            reward = get_id(sacrifice_unique_desc, reward_str);
        }
        if (reward == -1)
        {
            reward = get_id(spell_desc, reward_str);
        }
    }
    if (reward == -1 && (strcmp(reward_str, "NONE") == 0))
    {
        reward = 0;
    }

    if (reward < 0)
    {
        reward = 0;
        command = SacA_None;
        luaL_error(L,"Unexpected parameter:%s", reward_str);
    }

    script_set_sacrifice_recipe(command, reward, victims);
    return 0;

}

static int lua_Remove_sacrifice_recipe(lua_State *L)
{
    int command = SacA_None;
    int reward =  0;
    ThingModel victims[MAX_SACRIFICE_VICTIMS];

    for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
        long crtr_model  = luaL_optNamedCommand(L,i + 1,creature_desc);
        victims[i] = crtr_model;
    }

    script_set_sacrifice_recipe(command, reward, victims);
    return 0;

}

//Manipulating Creature stats


static int lua_Set_creature_instance(lua_State *L)
{
    ThingModel crmodel = luaL_checkNamedCommand(L,1,creature_desc);
    int slot = luaL_checkinteger(L, 2);
    int instance = luaL_checkNamedCommand(L, 3,instance_desc);
    unsigned char level = luaL_checkinteger(L, 4);

    script_set_creature_instance(crmodel, slot, instance, level);
    return 0;
}

static int lua_Set_creature_max_level(lua_State *L)
{

    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long crtr_model                 = luaL_checkNamedCommand(L,2,creature_desc);
    long max_level                  = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        if (max_level == -1)
            max_level = CREATURE_MAX_LEVEL + 1;
        dungeon->creature_max_level[crtr_model%game.conf.crtr_conf.model_count] = max_level;
    }
    return 0;
}

//static int lua_Set_creature_property(lua_State *L)

static int lua_Set_creature_tendencies(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    int tendancy = luaL_checkNamedCommand(L, 2, tendency_desc);
    int val = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct PlayerInfo *player = get_player(i);
        set_creature_tendencies(player, tendancy, val);
        if (is_my_player(player))
        {
            struct Dungeon* dungeon = get_players_dungeon(player);
            game.creatures_tend_imprison = ((dungeon->creature_tendencies & 0x01) != 0);
            game.creatures_tend_flee = ((dungeon->creature_tendencies & 0x02) != 0);
        }
    }
    return 0;
}

static int lua_Creature_entrance_level(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    unsigned char level = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        dungeon->creature_entrance_level = level - 1;
    }
    return 0;
}



//Manipulating Research

static int lua_Research(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long research_type              = luaL_checkNamedCommand(L,2,research_desc);
    int room_or_spell;
    switch (research_type)
    {
        case 1:
            room_or_spell = luaL_checkNamedCommand(L,3,power_desc);
            break;
        case 2:
            room_or_spell = luaL_checkNamedCommand(L,3,room_desc);
            break;
        default:
            luaL_error (L,"invalid research_type %d",research_type);
            return 0;
    }
    long research_value         = luaL_checkint(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        update_or_add_players_research_amount(i, research_type, room_or_spell, research_value);
    }
    return 0;
}

static int lua_Research_order(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long research_type              = luaL_checkNamedCommand(L,2,research_desc);
    int room_or_spell;
    switch (research_type)
    {
        case 1:
            room_or_spell = luaL_checkNamedCommand(L,3,power_desc);
            break;
        case 2:
            room_or_spell = luaL_checkNamedCommand(L,3,room_desc);
            break;
        default:
            luaL_error (L,"invalid research_type %d",research_type);
            return 0;
    }
    long research_value         = luaL_checkint(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        if (!research_overriden_for_player(i))
            remove_all_research_from_player(i);
        add_research_to_player(i, research_type, room_or_spell, research_value);
    }
    return 0;
}


//Tweaking computer players

static int lua_Computer_dig_to_location(lua_State *L)
{
    PlayerNumber plr_idx      = luaL_checkPlayerSingle(L, 1);
    TbMapLocation origin      = luaL_checkLocation(L,  2);
    TbMapLocation destination = luaL_checkLocation(L,  3);
    script_computer_dig_to_location(plr_idx, origin, destination);
    return 0;
}

static int lua_Set_computer_process(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    const char* procname = luaL_checkstring(L, 2);
    long val1 = luaL_checkinteger(L,3);
    long val2 = luaL_checkinteger(L,4);
    long val3 = luaL_checkinteger(L,5);  
    long val4 = luaL_checkinteger(L,6);
    long val5 = luaL_checkinteger(L,7);
    long n = 0;
    for (long i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Computer2* comp = get_computer_player(i);
        if (computer_player_invalid(comp)) {
            continue;
        }
        for (long k = 0; k < COMPUTER_PROCESSES_COUNT; k++)
        {
            struct ComputerProcess* cproc = &comp->processes[k];
            if (flag_is_set(cproc->flags, ComProc_ListEnd))
                break;
            if (strcasecmp(procname, cproc->name) == 0)
            {
                cproc->priority = val1;
                cproc->confval_2 = val2;
                cproc->confval_3 = val3;
                cproc->confval_4 = val4;
                cproc->confval_5 = val5;
                n++;
            }
        }
    }
    return 0;
}

static int lua_Set_computer_checks(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    const char* chkname = luaL_checkstring(L, 2);
    long val1 = luaL_checkinteger(L,3);
    long val2 = luaL_checkinteger(L,4);
    long val3 = luaL_checkinteger(L,5);  
    long val4 = luaL_checkinteger(L,6);
    long val5 = luaL_checkinteger(L,7);

    long n = 0;
    for (long i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Computer2* comp = get_computer_player(i);
        if (computer_player_invalid(comp)) {
            continue;
        }
        for (long k = 0; k < COMPUTER_CHECKS_COUNT; k++)
        {
            struct ComputerCheck* ccheck = &comp->checks[k];
            if ((ccheck->flags & ComChk_Unkn0002) != 0)
                break;
            if (strcasecmp(chkname, ccheck->name) == 0)
            {
                ccheck->turns_interval = val1;
                ccheck->param1 = val2;
                ccheck->param2 = val3;
                ccheck->param3 = val4;
                ccheck->last_run_turn = val5;
                n++;
            }
        }
    }
    return 0;
}

static int lua_Set_computer_globals(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    int dig_stack_size       = luaL_checkinteger(L,2);
    int processes_time       = luaL_checkinteger(L,3);
    int click_rate           = luaL_checkinteger(L,4);
    int max_room_build_tasks = luaL_checkinteger(L,5);
    int turn_begin           = luaL_checkinteger(L,6);
    int sim_before_dig       = luaL_checkinteger(L,7);
    int min_drop_delay       = luaL_checkinteger(L,8);

    for (long i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Computer2* comp = get_computer_player(i);
        if (computer_player_invalid(comp))
        {
            continue;
        }
        comp->dig_stack_size = dig_stack_size;
        comp->processes_time = processes_time;
        comp->click_rate = click_rate;
        comp->max_room_build_tasks = max_room_build_tasks;
        comp->turn_begin = turn_begin;
        comp->sim_before_dig = sim_before_dig;
        if (min_drop_delay != -1)
        {
            comp->task_delay = min_drop_delay;
        }
    }
    return 0;
}

static int lua_Set_computer_event(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    const char* evntname = luaL_checkstring(L, 2);
    long val1 = luaL_checkinteger(L,3);
    long val2 = luaL_checkinteger(L,4);
    long val3 = luaL_checkinteger(L,5);  
    long val4 = luaL_checkinteger(L,6);
    long val5 = luaL_checkinteger(L,7);

    long n = 0;
    for (long i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Computer2* comp = get_computer_player(i);
        if (computer_player_invalid(comp)) {
            continue;
        }
        for (long k = 0; k < COMPUTER_EVENTS_COUNT; k++)
        {
            struct ComputerEvent* event = &comp->events[k];
            if (strcasecmp(evntname, event->name) == 0)
            {
                event->test_interval = val1;
                event->param1 = val2;
                event->param2 = val3;
                event->param3 = val4;
                event->last_test_gameturn = val5;
                n++;
            }
        }
    }
    return 0;
}


//Specials
static int lua_Use_special_increase_level(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    char count;
    if(lua_isnone(L,2))
    {
        count = 1;
    }
    else
    {
        count = luaL_checkinteger(L, 2);
    }

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_use_special_increase_level(i,count);
    }
    return 0;
}

static int lua_Use_special_multiply_creatures(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_use_special_multiply_creatures(i);
    }
    return 0;
}

static int lua_Make_safe(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_make_safe(i);
    }
    return 0;
}

static int lua_Make_unsafe(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_make_unsafe(i);
    }
    return 0;
}

static int lua_Set_box_tooltip(lua_State *L)
{
    long box_id = luaL_checkinteger(L, 1);
    const char* tooltip = luaL_checkstring(L, 2);

    snprintf(gameadd.box_tooltip[box_id], MESSAGE_TEXT_LEN, "%s", tooltip);
    return 0;
}
static int lua_Set_box_tooltip_id(lua_State *L)
{
    long box_id = luaL_checkinteger(L, 1);
    long tooltip_id = luaL_checkinteger(L, 2);

    snprintf(gameadd.box_tooltip[box_id], MESSAGE_TEXT_LEN, "%s", get_string(tooltip_id));
    return 0;
}

static int lua_Locate_hidden_world(lua_State *L)
{
    script_locate_hidden_world();
    return 0;
}

//Effects
static int lua_Create_effect(lua_State *L)
{
    EffectOrEffElModel effect_id = luaL_checkEffectOrEffElModel(L,1);
    TbMapLocation location = luaL_checkLocation(L,  2);
    long height = luaL_checkinteger(L, 3);

    struct Coord3d pos;
    if (!get_coords_at_location(&pos, location,true))
    {
        return 0;
    }

    lua_pushThing(L,script_create_effect(&pos, effect_id, height));
    return 1;
}

static int lua_Create_effect_at_pos(lua_State *L)
{
    EffectOrEffElModel effect_id = luaL_checkEffectOrEffElModel(L,1);
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 3);
    long height = luaL_checkinteger(L, 4);

    struct Coord3d pos;
    set_coords_to_subtile_center(&pos, stl_x, stl_y, 0);
    pos.z.val += get_floor_height(pos.x.stl.num, pos.y.stl.num);
    lua_pushThing(L,script_create_effect(&pos, effect_id, height));
    return 1;
}

static int lua_Create_effects_line(lua_State *L)
{
    TbMapLocation from = luaL_checkLocation(L,  1);
    TbMapLocation to   = luaL_checkLocation(L,  2);
    char curvature = luaL_checkinteger(L, 3);
    unsigned char spatial_stepping = luaL_checkinteger(L, 4);
    unsigned char temporal_stepping = luaL_checkinteger(L, 5);
    EffectOrEffElModel effct_id = luaL_checkEffectOrEffElModel(L,6);

    create_effects_line(from, to, curvature, spatial_stepping, temporal_stepping, effct_id);
    return 0;
}



static int lua_Set_music(lua_State *L)
{
    if (lua_isnumber(L, 1)) {
        
        long track_number = luaL_checkinteger(L, 1);
        if (track_number == 0) {
            stop_music();
        }
        else {
            play_music_track(track_number);
        }
    } else {
        const char *track_name = luaL_checkstring(L, 1);
        play_music(prepare_file_fmtpath(FGrp_CmpgMedia, "%s", track_name));
    }
    return 0;
}

static int lua_Set_hand_graphic(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    long hand_idx = luaL_checkNamedCommand(L,1,powerhand_desc);

    struct PlayerInfo * player = get_player(player_idx);
    player->hand_idx = hand_idx;
    return 0;
}

static int lua_Zoom_to_location(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    TbMapLocation location = luaL_checkLocation(L,  2);
    struct Coord3d pos;
    
    find_location_pos(location, player_idx, &pos, __func__);
    set_player_zoom_to_position(get_player(player_idx),&pos);

    return 0;
}

static int lua_Lock_possession(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    TbBool locked = lua_toboolean(L, 2);

    struct PlayerInfo *player;
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        player = get_player(i);
        if (player_exists(player))
        {
            player->possession_lock = locked;
        }
    }

    return 0;
}

static int lua_Set_digger(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long new_dig_model = luaL_checkNamedCommand(L,2,creature_desc);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        update_players_special_digger_model(i, new_dig_model);
    }
    return 0;
}

static int lua_Use_power_on_creature(lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);
    long pwkind = luaL_checkNamedCommand(L,2,power_desc);
    long power_level = luaL_checkinteger(L, 3);
    PlayerNumber caster = luaL_checkPlayerSingle(L, 4);
    TbBool is_free = lua_toboolean(L, 5);

    script_use_power_on_creature(thing, pwkind, power_level, caster, is_free);
    return 0;
}

static int lua_Use_power_at_pos(lua_State *L)
{
    PlayerNumber caster = luaL_checkPlayerSingle(L, 1);
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 3);
    PowerKind pwkind = luaL_checkNamedCommand(L,4,power_desc);
    KeepPwrLevel power_level = luaL_checkinteger(L, 5);
    TbBool is_free = lua_toboolean(L, 6);

    unsigned long allow_flags = PwCast_AllGround | PwCast_Unrevealed;
    unsigned long mod_flags = 0;
    if (is_free)
        set_flag(mod_flags,PwMod_CastForFree);

    magic_use_power_on_subtile(caster, pwkind, power_level, stl_x, stl_y, allow_flags, mod_flags);
    return 0;
}

static int lua_Use_power_at_location(lua_State *L)
{
    PlayerNumber caster = luaL_checkPlayerSingle(L, 1);
    TbMapLocation location = luaL_checkLocation(L, 2);
    PowerKind pwkind = luaL_checkNamedCommand(L,3,power_desc);
    KeepPwrLevel power_level = luaL_checkinteger(L, 4);
    TbBool is_free = lua_toboolean(L, 5);

    MapSubtlCoord stl_x = 0;
    MapSubtlCoord stl_y = 0;
    find_map_location_coords(location, &stl_x, &stl_y, caster, __func__);

    unsigned long allow_flags = PwCast_AllGround | PwCast_Unrevealed;
    unsigned long mod_flags = 0;
    if (is_free)
        set_flag(mod_flags,PwMod_CastForFree);

    magic_use_power_on_subtile(caster, pwkind, power_level, stl_x, stl_y, allow_flags, mod_flags);
    return 0;
}

static int lua_Use_power(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    PowerKind power_kind = luaL_checkNamedCommand(L,2,power_desc);
    TbBool free = lua_toboolean(L, 3);

    script_use_power(plyr_idx,power_kind,free);
    return 0;
}

static int lua_Use_special_Transfer_creature(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    if (plyr_idx == my_player_number)
    {
        struct Thing *heartng = get_player_soul_container(plyr_idx);
        struct PlayerInfo* player = get_my_player();
        start_transfer_creature(player, heartng);
    }
    return 0;
}

static int lua_Change_slab_owner(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    MapSlabCoord slb_x = luaL_checkslb_x(L, 2);
    MapSlabCoord slb_y = luaL_checkslb_y(L, 3);

    change_slab_owner_from_script(plyr_idx, slb_x, slb_y);
    return 0;
}

static int lua_Change_slab_type(lua_State *L)
{
    MapSlabCoord slb_x = luaL_checkslb_x(L, 1);
    MapSlabCoord slb_y = luaL_checkslb_y(L, 2);
    SlabKind slb_kind = luaL_checkNamedCommand(L, 3,slab_desc);
    int fill_type = luaL_optNamedCommand(L, 4,fill_desc);

    if (fill_type > 0)
    {
        struct CompoundCoordFilterParam iter_param;
        iter_param.num1 = slb_kind;
        iter_param.num2 = fill_type;
        iter_param.num3 = get_slabmap_block(slb_x, slb_y)->kind;
        slabs_fill_iterate_from_slab(slb_x, slb_y, slabs_change_type, &iter_param);
    }
    else
    {
        replace_slab_from_script(slb_x, slb_y,slb_kind);
    }

    return 0;
}

static int lua_Use_spell_on_creature(lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);
    long spell_id = luaL_checkNamedCommand(L,2,spell_desc);
    int spell_level = luaL_checkinteger(L, 3);

    unsigned long fmcl_bytes = (spell_id << 8) | spell_level;

    script_use_spell_on_creature(thing->owner, thing, fmcl_bytes);
    return 0;
}

static int lua_Hide_hero_gate(lua_State *L)
{
    long gate_number = luaL_checkinteger(L, 1);
    TbBool hide = lua_toboolean(L, 2);

    struct Thing* thing = find_hero_gate_of_number(gate_number);
    if (hide)
    {
        light_turn_light_off(thing->light_id);
        create_effect(&thing->mappos, TngEff_BallPuffWhite, thing->owner);
        place_thing_in_creature_controlled_limbo(thing);
    }
    else
    {
        create_effect(&thing->mappos, TngEff_BallPuffWhite, thing->owner);
        remove_thing_from_creature_controlled_limbo(thing);
        light_turn_light_on(thing->light_id);
    }
    return 0;
}

static int lua_Change_creatures_annoyance(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    ThingModel crmodel = luaL_checkNamedCommand(L,2,creature_desc);
    long operation = luaL_checkNamedCommand(L,3,script_operator_desc);
    long anger = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_change_creatures_annoyance(i, crmodel, operation, anger);
    }
    return 0;
}

/*
static int lua_SET_PLAYER_MODIFIER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
    }
    return 0;
}

static int lua_ADD_TO_PLAYER_MODIFIER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
    }
    return 0;
}
*/

/**********************************************/
// game
/**********************************************/


static int lua_get_creature_near(lua_State *L)
{
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 1);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 2);

    struct Thing *thing = get_creature_near(stl_x * COORD_PER_STL, stl_y * COORD_PER_STL);

    lua_pushThing(L, thing);
    return 1;
}

static int lua_get_thing_by_idx(lua_State *L)
{
    // the arguments lua passes to the C code
    ThingIndex tng_idx = lua_tointeger(L, 1);

    struct Thing *thing = thing_get(tng_idx);

    // arguments you push back to lua
    lua_pushThing(L, thing);
    return 1;
}

static int lua_get_creature_by_criterion(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerRangeId(L, 1);
    ThingModel crmodel = luaL_checkCreature_or_creature_wildcard(L,2);
    long criteria = luaL_checkNamedCommand(L,3,creature_select_criteria_desc);

    struct Thing* thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    lua_pushThing(L, thing);
    return 1;
}


static int lua_print(lua_State *L)
{
    //const char* msg = lua_tostring(L, 1);
    //JUSTLOG("%s",msg);

    int nargs = lua_gettop(L);
    char buffer[16384]; // Adjust size as needed
    int offset = 0;

    for (int i = 1; i <= nargs; i++) {
        size_t len;
        const char *str;

        if (lua_isstring(L, i) || lua_isnumber(L, i)) {
            str = lua_tolstring(L, i, &len);
        } else if (lua_isboolean(L, i)) {
            str = lua_toboolean(L, i) ? "true" : "false";
            len = strlen(str);
        } else if (lua_isnil(L, i)) {
            str = "nil";
            len = 3;
        } else {
            str = luaL_typename(L, i); // Prints type name (e.g., "table", "function")
            len = strlen(str);
        }

        // Ensure we don't overflow the buffer
        if (offset + len + 2 >= sizeof(buffer)) // +2 for "\t" + null terminator
            break;

        memcpy(buffer + offset, str, len);
        offset += len;

        if (i < nargs) {
            buffer[offset++] = '\t'; // Separate arguments with tabs
        }
    }

    buffer[offset] = '\0'; // Null-terminate

    JUSTLOG("%s", buffer); // Print using your logging function

    return 0;
}


static int lua_get_things_of_class(lua_State *L)
{
    ThingClass class_id = luaL_checkNamedCommand(L,1,class_commands);

    const struct StructureList* slist = get_list_for_thing_class(class_id);
    ThingIndex i = slist->index;
    long k = 0;

    lua_newtable(L);

    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code

        lua_pushThing(L, thing);  // Push the value onto the stack
        lua_rawseti(L, -2, k + 1);      // Set table[-2][i + 1] = value
        
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return 1; // return value is the amount of args you push back
}


static int lua_is_action_point_activated_by_player(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    ActionPointId apt_idx = luaL_checkActionPoint(L, 2);

    lua_pushboolean(L, action_point_activated_by_player(apt_idx, plyr_idx));
    return 1;
}

static int lua_get_slab(lua_State *L)
{
    MapSlabCoord slb_x = luaL_checkslb_x(L, 1);
    MapSlabCoord slb_y = luaL_checkslb_y(L, 2);

    lua_pushSlab(L, slb_x, slb_y);

    return 1;
}

static int lua_run_dkscript_command(lua_State *L)
{
    const char* map_command_const = lua_tostring(L, 1);
    //lua strings are constant, script_scan_line doesn't like const, so copy it to a temp buffer
    char map_command[256];
    strncpy(map_command, map_command_const, sizeof(map_command) - 1);
    map_command[sizeof(map_command) - 1] = '\0'; // Ensure null termination

    script_scan_line(map_command, false, 99);
    return 0;
}

static const luaL_Reg global_methods[] = {
//Setup Commands
   {"Set_generate_speed"            ,lua_Set_generate_speed           },
   {"Computer_player"               ,lua_Computer_player              },
   {"Ally_players"                  ,lua_Ally_players                 },
   {"Start_money"                   ,lua_Start_money                  },
   {"Max_creatures"                 ,lua_Max_creatures                },
   {"Add_creature_to_pool"          ,lua_Add_creature_to_pool         },
   {"Creature_available"            ,lua_Creature_available           },
   {"Dead_creatures_return_to_pool" ,lua_Dead_creatures_return_to_pool},
   {"Room_available"                ,lua_Room_available               },
   {"Magic_available"               ,lua_Magic_available              },
   {"Door_available"                ,lua_Door_available               },
   {"Trap_available"                ,lua_Trap_available               },

//Script flow control
   {"Win_game"                             ,lua_Win_game                        },
   {"Lose_game"                            ,lua_Lose_game                       },
   {"Count_creatures_at_action_point"      ,lua_Count_creatures_at_action_point },
   {"Set_timer"                            ,lua_Set_timer                       },
   {"Add_to_timer"                         ,lua_Add_to_timer                    },
   {"Display_timer"                        ,lua_Display_timer                   },
   {"Hide_timer"                           ,lua_Hide_timer                      },
   {"Bonus_level_time"                     ,lua_Bonus_level_time                },
   {"Add_bonus_time"                       ,lua_Add_bonus_time                  },

//Adding New Creatures and Parties to the Level
   {"Add_creature_to_level"                ,lua_Add_creature_to_level           },
   {"Add_tunneller_to_level"               ,lua_Add_tunneller_to_level          },
   {"Create_party"                         ,lua_Create_party                    },
   {"Add_to_party"                         ,lua_Add_to_party                    },
   {"Delete_from_party"                    ,lua_Delete_from_party               },
   {"Add_tunneller_party_to_level"         ,lua_Add_tunneller_party_to_level    },
   {"Add_party_to_level"                   ,lua_Add_party_to_level              },

//Displaying information and affecting interface
   {"Display_objective"                    ,lua_Display_objective               },
   {"Display_objective_with_pos"           ,lua_Display_objective_with_pos      },
   {"Display_information"                  ,lua_Display_information             },
   {"Display_information_with_pos"         ,lua_Display_information_with_pos    },
   {"Quick_objective"                      ,lua_Quick_objective                 },
   {"Quick_objective_with_pos"             ,lua_Quick_objective_with_pos        },
   {"Quick_information"                    ,lua_Quick_information               },
   {"Quick_information_with_pos"           ,lua_Quick_information_with_pos      },
   {"DisPlay_message"                      ,lua_DisPlay_message                 },
   {"Quick_message"                        ,lua_Quick_message                   },
   {"Heart_lost_objective"                 ,lua_Heart_lost_objective            },
   {"Heart_lost_quick_objective"           ,lua_Heart_lost_quick_objective      },
   //{"Play_message"                         ,lua_Play_message                    },
   {"Tutorial_flash_button"                ,lua_Tutorial_flash_button           },
   {"Display_countdown"                    ,lua_Display_countdown               },
   {"Display_variable"                     ,lua_Display_variable                },
   {"Hide_variable"                        ,lua_Hide_variable                   },

//Manipulating Map
   {"Reveal_map_location"                  ,lua_Reveal_map_location             },
   {"Reveal_map_rect"                      ,lua_Reveal_map_rect                 },
   {"Conceal_map_rect"                     ,lua_Conceal_map_rect                },
   {"Set_door"                             ,lua_Set_door                        },
   {"Add_heart_health"                     ,lua_Add_heart_health                },
   {"Add_object_to_level"                  ,lua_Add_object_to_level             },
   {"Add_object_to_level_at_pos"           ,lua_Add_object_to_level_at_pos      },
   {"Add_effect_generator_to_level"        ,lua_Add_effect_generator_to_level   },
   {"Place_door"                           ,lua_Place_door                      },
   {"Place_trap"                           ,lua_Place_trap                      },
   {"Change_slab_owner"                    ,lua_Change_slab_owner               },
   {"Change_slab_type"                     ,lua_Change_slab_type                },
   {"Hide_hero_gate"                       ,lua_Hide_hero_gate                  },
   
//Manipulating Configs
    //{"New_creature_type"                    ,lua_New_creature_type               },
    //{"New_object_type"                      ,lua_New_object_type                 },
    //{"New_trap_type"                        ,lua_New_trap_type                   },
    //{"New_room_type"                        ,lua_New_room_type                   },
    {"Set_door_configuration"              ,lua_Set_door_configuration          },
    {"Set_object_configuration"            ,lua_Set_object_configuration        },
    {"Set_trap_configuration"              ,lua_Set_trap_configuration          },
    //{"Set_creature_configuration"           ,lua_Set_creature_configuration      },
    {"Set_effect_generator_configuration"   ,lua_Set_effect_generator_configuration},
    //{"Set_power_configuration"              ,lua_Set_power_configuration         },
    {"Set_room_configuration"              ,lua_Set_room_configuration          },
    {"Set_game_rule"                       ,lua_Set_game_rule                   },
    {"Set_hand_rule"                       ,lua_Set_hand_rule                   },
    {"Swap_creature"                       ,lua_Swap_creature                   },
    {"Set_sacrifice_recipe"                ,lua_Set_sacrifice_recipe            },
    {"Remove_sacrifice_recipe"             ,lua_Remove_sacrifice_recipe         },

//Manipulating Creature stats
   {"Set_creature_instance"                ,lua_Set_creature_instance           },
   {"Set_creature_max_level"               ,lua_Set_creature_max_level          },
   //{"Set_creature_property"                ,lua_Set_creature_property           },
   {"Set_creature_tendencies"              ,lua_Set_creature_tendencies         },
   {"Creature_entrance_level"              ,lua_Creature_entrance_level         },
   {"Change_creatures_annoyance"           ,lua_Change_creatures_annoyance      },

//Manipulating Research
   {"Research"                             ,lua_Research                        },
   {"Research_order"                       ,lua_Research_order                  },

//Tweaking computer players
   {"Computer_dig_to_location"             ,lua_Computer_dig_to_location        },
   {"Set_computer_process"                 ,lua_Set_computer_process            },
   {"Set_computer_checks"                  ,lua_Set_computer_checks             },
   {"Set_computer_globals"                 ,lua_Set_computer_globals            },
   {"Set_computer_event"                   ,lua_Set_computer_event              },

//Specials
   {"Use_special_increase_level"           ,lua_Use_special_increase_level      },
   {"Use_special_multiply_creatures"       ,lua_Use_special_multiply_creatures  },
   {"Use_special_Transfer_creature"        ,lua_Use_special_Transfer_creature   },
   {"Set_box_tooltip"                      ,lua_Set_box_tooltip                 },
   {"Set_box_tooltip_id"                   ,lua_Set_box_tooltip_id              },
   {"Make_safe"                            ,lua_Make_safe                       },
   {"Make_unsafe"                          ,lua_Make_unsafe                     },
   {"Locate_hidden_world"                  ,lua_Locate_hidden_world             },

//Effects
    {"Create_effect"                        ,lua_Create_effect                   },
    {"Create_effect_at_pos"                 ,lua_Create_effect_at_pos            },
    {"Create_effects_line"                  ,lua_Create_effects_line             },

//Other
    {"Use_power"                            ,lua_Use_power                       },
    {"Use_power_at_location"                ,lua_Use_power_at_location           },
    {"Use_power_at_pos"                     ,lua_Use_power_at_pos                },
    {"Use_power_on_creature"                ,lua_Use_power_on_creature           },
    {"Use_spell_on_creature"                ,lua_Use_spell_on_creature           },
    //{"Use_spell_on_players_creatures"       ,lua_Use_spell_on_players_creatures  },
    //{"Use_power_on_players_creatures"       ,lua_Use_power_on_players_creatures  },
    {"Set_hand_graphic"                     ,lua_Set_hand_graphic                },
    //{"Set_increase_on_experience"           ,lua_Set_increase_on_experience      },
    {"Set_music"                            ,lua_Set_music                       },
    {"Zoom_to_location"                     ,lua_Zoom_to_location                },
    {"Lock_possession"                      ,lua_Lock_possession                 },
    {"Set_digger"                           ,lua_Set_digger                      },

//debug stuff
    {"print"                        ,lua_print                     },
    {"Run_DKScript_command"           ,lua_run_dkscript_command      },
    
//retrieving lua vars
    {"Get_creature_near",               lua_get_creature_near},
    {"Get_creature_by_criterion",        lua_get_creature_by_criterion},
    {"Get_thing_by_idx",                 lua_get_thing_by_idx},
    {"Get_things_of_class",              lua_get_things_of_class},
    {"Is_actionpoint_activated_by_player",lua_is_action_point_activated_by_player},
    {"Get_slab",                       lua_get_slab},
};
/*
static const luaL_Reg game_meta[] = {
    {NULL, NULL}
};
*/
static void global_register(lua_State *L)
{
    //luaL_newlib(L, global_methods);
    for (size_t i = 0; i < (sizeof(global_methods)/sizeof(global_methods[0])); i++)
    {
        lua_register(L, global_methods[i].name, global_methods[i].func);
    }
    lua_setglobal(L, "thing");
}


void Player_register(lua_State *L);
void Thing_register(lua_State *L);
void Slab_register(lua_State *L);

void reg_host_functions(lua_State *L)
{
    Player_register(L);
    global_register(L);
    Thing_register(L);
    Slab_register(L);
}
