/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_rules.c
 *     Various game configuration options support.
 * @par Purpose:
 *     Support of configuration files for game rules.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 31 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_rules.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_terrain.h"
#include "config_lenses.h"
#include "config_magic.h"
#include "config_creature.h"
#include "game_merge.h"
#include "room_library.h"
#include "game_legacy.h"
#include "frontmenu_ingame_map.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

static int64_t value_x10(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx);

static void assign_MapCreatureLimit_script(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx);
static void assign_AlliesShareVision_script(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx);

/******************************************************************************/


const char keeper_rules_file[]="rules.cfg";

const struct NamedCommand rules_game_classicbugs_commands[] = {
  {"RESURRECT_FOREVER",              1},
  {"OVERFLOW_8BIT",                  2},
  {"CLAIM_ROOM_ALL_THINGS",          3},
  {"RESURRECT_REMOVED",              4},
  {"NO_HAND_PURGE_ON_DEFEAT",        5},
  {"MUST_OBEY_KEEPS_NOT_DO_JOBS",    6},
  {"BREAK_NEUTRAL_WALLS",            7},
  {"ALWAYS_TUNNEL_TO_RED",           8},
  {"FULLY_HAPPY_WITH_GOLD",          9},
  {"FAINTED_IMMUNE_TO_BOULDER",     10},
  {"REBIRTH_KEEPS_SPELLS",          11},
  {"STUN_FRIENDLY_UNITS",           12},
  {"PASSIVE_NEUTRALS",              13},
  {"NEUTRAL_TORTURE_CONVERTS",      14},
  {NULL,                             0},
};



static const struct NamedField rules_game_named_fields[] = {
    //name                    //param  //field                                             //default  //min               //max   //namedCommand                    //valueFunc
  {"GOLDPERGOLDBLOCK",          0, field(game.conf.rules.game.gold_per_gold_block       ),        1000, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"POTOFGOLDHOLDS",            0, field(game.conf.rules.game.pot_of_gold_holds         ),        1000, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"CHESTGOLDHOLD",             0, field(game.conf.rules.game.chest_gold_hold           ),        1000, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"GOLDPILEVALUE",             0, field(game.conf.rules.game.gold_pile_value           ),         500, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"GOLDPILEMAXIMUM",           0, field(game.conf.rules.game.gold_pile_maximum         ),        5000, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"GOLDPERHOARD",              0, field(game.conf.rules.game.gold_per_hoard            ),        2000, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"FOODLIFEOUTOFHATCHERY",     0, field(game.conf.rules.game.food_life_out_of_hatchery ),         100,        0,          USHRT_MAX,NULL,                           value_default, NULL},
  {"BOULDERREDUCEHEALTHSLAP",   0, field(game.conf.rules.game.boulder_reduce_health_wall),          10, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"BOULDERREDUCEHEALTHWALL",   0, field(game.conf.rules.game.boulder_reduce_health_slap),          10, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"BOULDERREDUCEHEALTHROOM",   0, field(game.conf.rules.game.boulder_reduce_health_room),          10, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"PAYDAYGAP",                 0, field(game.conf.rules.game.pay_day_gap               ),        5000, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"PAYDAYSPEED",               0, field(game.conf.rules.game.pay_day_speed             ),         100,        0,          ULONG_MAX,NULL,                           value_default, NULL},
  {"DUNGEONHEARTHEALTIME",      0, field(game.conf.rules.game.dungeon_heart_heal_time   ),          10,        0,          ULONG_MAX,NULL,                           value_default, NULL},
  {"DUNGEONHEARTHEALHEALTH",    0, field(game.conf.rules.game.dungeon_heart_heal_health ),           1, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"HERODOORWAITTIME",          0, field(game.conf.rules.game.hero_door_wait_time       ),         100,        0,          ULONG_MAX,NULL,                           value_default, NULL},
  {"GEMEFFECTIVENESS",          0, field(game.conf.rules.game.gem_effectiveness         ),          17,        0,          ULONG_MAX,NULL,                           value_default, NULL},
  {"ROOMSELLGOLDBACKPERCENT",   0, field(game.conf.rules.game.room_sale_percent         ),          50,        0,           LONG_MAX,NULL,                           value_default, NULL},
  {"DOORSELLVALUEPERCENT",      0, field(game.conf.rules.game.door_sale_percent         ),         100,        0,           LONG_MAX,NULL,                           value_default, NULL},
  {"TRAPSELLVALUEPERCENT",      0, field(game.conf.rules.game.trap_sale_percent         ),         100,        0,           LONG_MAX,NULL,                           value_default, NULL},
  {"BAGGOLDHOLD",               0, field(game.conf.rules.game.bag_gold_hold             ),         200, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"ALLIESSHAREVISION",         0, field(game.conf.rules.game.allies_share_vision       ),           0,        0,                  1,NULL,                           value_default, assign_AlliesShareVision_script},
  {"ALLIESSHAREDROP",           0, field(game.conf.rules.game.allies_share_drop         ),           0,        0,                  1,NULL,                           value_default, NULL},
  {"ALLIESSHARECTA",            0, field(game.conf.rules.game.allies_share_cta          ),           0,        0,                  1,NULL,                           value_default, NULL},
  {"DISPLAYPORTALLIMIT",        0, field(game.conf.rules.game.display_portal_limit      ),           0,        0,                  1,NULL,                           value_default, NULL},
  {"MAXTHINGSINHAND",           0, field(game.conf.rules.game.max_things_in_hand        ),           8,        0, MAX_THINGS_IN_HAND,NULL,                           value_default, NULL},
  {"TORTUREPAYDAY",             0, field(game.conf.rules.game.torture_payday            ),          50,        0,          USHRT_MAX,NULL,                           value_default, NULL},
  {"TORTURETRAININGCOST",       0, field(game.conf.rules.game.torture_training_cost     ),         100, SHRT_MIN,           SHRT_MAX,NULL,                           value_default, NULL},
  {"TORTURESCAVENGINGCOST",     0, field(game.conf.rules.game.torture_scavenging_cost   ),         100, SHRT_MIN,           SHRT_MAX,NULL,                           value_default, NULL},
  {"EASTEREGGSPEECHCHANCE",     0, field(game.conf.rules.game.easter_egg_speech_chance  ),        2000,        0,           LONG_MAX,NULL,                           value_default, NULL},
  {"EASTEREGGSPEECHINTERVAL",   0, field(game.conf.rules.game.easter_egg_speech_interval),       20000,        0,           LONG_MAX,NULL,                           value_default, NULL},
  {"GLOBALAMBIENTLIGHT",        0, field(game.conf.rules.game.global_ambient_light      ),          10, LONG_MIN,           LONG_MAX,NULL,                           value_default, NULL},
  {"LIGHTENABLED",              0, field(game.conf.rules.game.light_enabled             ),           1,        0,                  1,NULL,                           value_default, NULL},
  {"MAPCREATURELIMIT",          0, field(game.conf.rules.game.creatures_count           ),         255,        0,  CREATURES_COUNT-2,NULL,                           value_default, assign_MapCreatureLimit_script},
  {"PRESERVECLASSICBUGS",      -1, field(game.conf.rules.game.classic_bugs_flags        ),ClscBug_None,ClscBug_None, ClscBug_ListEnd,rules_game_classicbugs_commands,value_flagsfieldshift, NULL},
  {NULL},
};


static const struct NamedField rules_computer_named_fields[] = {
    //name                    //param  //field                                           //default    //min     //max
  {"DISEASEHPTEMPLEPERCENTAGE",  0, field(game.conf.rules.computer.disease_to_temple_pct),500,        0, USHRT_MAX,NULL,value_default,NULL},
  {NULL},
};

static const struct NamedField rules_creatures_named_fields[] = {
    //name                    //param  //field                                           //default   //min     //max  //namedCommand //valueFunc
  {"RECOVERYFREQUENCY",          0, field(game.conf.rules.creature.recovery_frequency    ),  10,        0, UCHAR_MAX,NULL,value_default, NULL},
  {"FIGHTMAXHATE",               0, field(game.conf.rules.creature.fight_max_hate        ), 200, SHRT_MIN,  SHRT_MAX,NULL,value_default, NULL},
  {"FIGHTBORDERLINE",            0, field(game.conf.rules.creature.fight_borderline      ),   0, SHRT_MIN,  SHRT_MAX,NULL,value_default, NULL},
  {"FIGHTMAXLOVE",               0, field(game.conf.rules.creature.fight_max_love        ),-100, SHRT_MIN,  SHRT_MAX,NULL,value_default, NULL},
  {"BODYREMAINSFOR",             0, field(game.conf.rules.creature.body_remains_for      ),1000,        0, USHRT_MAX,NULL,value_default, NULL},
  {"FIGHTHATEKILLVALUE",         0, field(game.conf.rules.creature.fight_hate_kill_value ),  -5, SHRT_MIN,  SHRT_MAX,NULL,value_default, NULL},
  {"FLEEZONERADIUS",             0, field(game.conf.rules.creature.flee_zone_radius      ),2048,        0, ULONG_MAX,NULL,value_default, NULL},
  {"GAMETURNSINFLEE",            0, field(game.conf.rules.creature.game_turns_in_flee    ), 200,        0,  LONG_MAX,NULL,value_default, NULL},
  {"GAMETURNSUNCONSCIOUS",       0, field(game.conf.rules.creature.game_turns_unconscious),2000,        0, USHRT_MAX,NULL,value_default, NULL},
  {"CRITICALHEALTHPERCENTAGE",   0, field(game.conf.rules.creature.critical_health_permil), 125,        0,       100,NULL,value_x10    , NULL},
  {"STUNEVILENEMYCHANCE",        0, field(game.conf.rules.creature.stun_enemy_chance_evil), 100,        0,       100,NULL,value_default, NULL},
  {"STUNGOODENEMYCHANCE",        0, field(game.conf.rules.creature.stun_enemy_chance_good), 100,        0,       100,NULL,value_default, NULL},
  {NULL},
};

static const struct NamedField rules_magic_named_fields[] = {
    //name                        //param  //field                                                     //default //min  //max //namedCommand //valueFunc
  {"HOLDAUDIENCETIME",               0, field(game.conf.rules.magic.hold_audience_time                ), 500,        0, LONG_MAX,NULL,value_default, NULL},
  {"ARMAGEDDONTELEPORTYOURTIMEGAP",  0, field(game.conf.rules.magic.armageddon_teleport_your_time_gap ),  10, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"ARMAGEDDONTELEPORTENEMYTIMEGAP", 0, field(game.conf.rules.magic.armageddon_teleport_enemy_time_gap),  10,        0, LONG_MAX,NULL,value_default, NULL},
  {"ARMAGEDDONTELEPORTNEUTRALS",     0, field(game.conf.rules.magic.armageddon_teleport_neutrals      ),   0,        0,        1,NULL,value_default, NULL},
  {"ARMAGEDDONCOUNTDOWN",            0, field(game.armageddon.count_down                              ), 500, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"ARMAGEDDONDURATION",             0, field(game.armageddon.duration                                ),4000, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"DISEASETRANSFERPERCENTAGE",      0, field(game.conf.rules.magic.disease_transfer_percentage       ),  15,        0, CHAR_MAX,NULL,value_default, NULL},
  {"DISEASELOSEPERCENTAGEHEALTH",    0, field(game.conf.rules.magic.disease_lose_percentage_health    ),   8, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"DISEASELOSEHEALTHTIME",          0, field(game.conf.rules.magic.disease_lose_health_time          ), 200, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"MINDISTANCEFORTELEPORT",         0, field(game.conf.rules.magic.min_distance_for_teleport         ),  20, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"COLLAPSEDUNGEONDAMAGE",          0, field(game.conf.rules.magic.collapse_dungeon_damage           ),  10, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"TURNSPERCOLLAPSEDUNGEONDAMAGE",  0, field(game.conf.rules.magic.turns_per_collapse_dngn_dmg       ),   4, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"FRIENDLYFIGHTAREARANGEPERCENT",  0, field(game.conf.rules.magic.friendly_fight_area_range_percent ),   0, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"FRIENDLYFIGHTAREADAMAGEPERCENT", 0, field(game.conf.rules.magic.friendly_fight_area_damage_percent),   0, LONG_MIN, LONG_MAX,NULL,value_default, NULL},
  {"WEIGHTCALCULATEPUSH",            0, field(game.conf.rules.magic.weight_calculate_push             ),   0,        0, SHRT_MAX,NULL,value_default, NULL},
  {NULL},
};

static const struct NamedField rules_rooms_named_fields[] = {
    //name                             //param  //field                                                  //default //min                //max  //namedCommand //valueFunc
  {"SCAVENGECOSTFREQUENCY",               0, field(game.conf.rules.rooms.scavenge_cost_frequency         ),  64, LONG_MIN,            LONG_MAX,NULL,value_default, NULL},
  {"TEMPLESCAVENGEPROTECTIONTIME",        0, field(game.conf.rules.rooms.temple_scavenge_protection_turns),1000,        0,           ULONG_MAX,NULL,value_default, NULL},
  {"TRAINCOSTFREQUENCY",                  0, field(game.conf.rules.rooms.train_cost_frequency            ),  64, LONG_MIN,            LONG_MAX,NULL,value_default, NULL},
  {"TORTURECONVERTCHANCE",                0, field(game.conf.rules.rooms.torture_convert_chance          ),  33,        0,                 100,NULL,value_default, NULL},
  {"TIMESPENTINPRISONWITHOUTBREAK",       0, field(game.conf.rules.rooms.time_in_prison_without_break    ),2400,        0,           ULONG_MAX,NULL,value_default, NULL},
  {"GHOSTCONVERTCHANCE",                  0, field(game.conf.rules.rooms.ghost_convert_chance            ),  10,        0,                 100,NULL,value_default, NULL},
  {"DEFAULTGENERATESPEED",                0, field(game.conf.rules.rooms.default_generate_speed          ), 350,        0,           USHRT_MAX,NULL,value_default, NULL},
  {"DEFAULTMAXCREATURESGENERATEENTRANCE", 0, field(game.conf.rules.rooms.default_max_crtrs_gen_entrance  ), 200,        0,           ULONG_MAX,NULL,value_default, NULL},
  {"FOODGENERATIONSPEED",                 0, field(game.conf.rules.rooms.food_generation_speed           ),2000, LONG_MIN,            LONG_MAX,NULL,value_default, NULL},
  {"PRISONSKELETONCHANCE",                0, field(game.conf.rules.rooms.prison_skeleton_chance          ), 100,        0,                 100,NULL,value_default, NULL},
  {"BODIESFORVAMPIRE",                    0, field(game.conf.rules.rooms.bodies_for_vampire              ),   6,        0,           UCHAR_MAX,NULL,value_default, NULL},
  {"GRAVEYARDCONVERTTIME",                0, field(game.conf.rules.rooms.graveyard_convert_time          ), 300,        0,           USHRT_MAX,NULL,value_default, NULL},
  {"SCAVENGEGOODALLOWED",                 0, field(game.conf.rules.rooms.scavenge_good_allowed           ),   1,        0,                   1,NULL,value_default, NULL},
  {"SCAVENGENEUTRALALLOWED",              0, field(game.conf.rules.rooms.scavenge_neutral_allowed        ),   1,        0,                   1,NULL,value_default, NULL},
  {"TIMEBETWEENPRISONBREAK",              0, field(game.conf.rules.rooms.time_between_prison_break       ),  64,        0,           ULONG_MAX,NULL,value_default, NULL},
  {"PRISONBREAKCHANCE",                   0, field(game.conf.rules.rooms.prison_break_chance             ),  50,        0,           ULONG_MAX,NULL,value_default, NULL},
  {"TORTUREDEATHCHANCE",                  0, field(game.conf.rules.rooms.torture_death_chance            ),   0,        0,                 100,NULL,value_default, NULL},
  {"BARRACKMAXPARTYSIZE",                 0, field(game.conf.rules.rooms.barrack_max_party_size          ),  10,        0, GROUP_MEMBERS_COUNT,NULL,value_default, NULL},
  {"TRAININGROOMMAXLEVEL",                0, field(game.conf.rules.rooms.training_room_max_level         ),   0,        0,CREATURE_MAX_LEVEL+1,NULL,value_default, NULL},
  {NULL},
};

static const struct NamedField rules_workers_named_fields[] = {
    //name                    //param  //field                                              //default  //min   //max  //namedCommand //valueFunc
  {"HITSPERSLAB",                0, field(game.conf.rules.workers.hits_per_slab              ),  2,       0, UCHAR_MAX,NULL,value_default, NULL},
  {"DEFAULTIMPDIGDAMAGE",        0, field(game.conf.rules.workers.default_imp_dig_damage     ),  1,       0, ULONG_MAX,NULL,value_default, NULL},
  {"DEFAULTIMPDIGOWNDAMAGE",     0, field(game.conf.rules.workers.default_imp_dig_own_damage ),  2,       0, ULONG_MAX,NULL,value_default, NULL},
  {"IMPWORKEXPERIENCE",          0, field(game.conf.rules.workers.digger_work_experience     ),  0,       0,  LONG_MAX,NULL,value_default, NULL},
  {"DRAGUNCONSCIOUSTOLAIR",      0, field(game.conf.rules.workers.drag_to_lair               ),  0,       0,         2,NULL,value_default, NULL},
  {NULL},
};

static const struct NamedField rules_health_named_fields[] = {
    //name                       //param  //field                                             //default  //min   //max  //namedCommand //valueFunc
  {"HUNGERHEALTHLOSS",              0, field(game.conf.rules.health.hunger_health_loss           ),  1, LONG_MIN,  LONG_MAX,NULL,value_default, NULL},
  {"GAMETURNSPERHUNGERHEALTHLOSS",  0, field(game.conf.rules.health.turns_per_hunger_health_loss ),100,        0, USHRT_MAX,NULL,value_default, NULL},
  {"FOODHEALTHGAIN",                0, field(game.conf.rules.health.food_health_gain             ), 10, LONG_MIN,  LONG_MAX,NULL,value_default, NULL},
  {"TORTUREHEALTHLOSS",             0, field(game.conf.rules.health.torture_health_loss          ),  5, LONG_MIN,  LONG_MAX,NULL,value_default, NULL},
  {"GAMETURNSPERTORTUREHEALTHLOSS", 0, field(game.conf.rules.health.turns_per_torture_health_loss),100,        0, USHRT_MAX,NULL,value_default, NULL},
  {NULL},
};

static const struct NamedField rules_script_only_named_fields[] = {
  //name            //field                   //min //max
{"PayDayProgress",0,field(game.pay_day_progress),0,0,LONG_MAX,NULL,value_default,NULL},
{NULL},
};

const struct NamedField* ruleblocks[] = {rules_game_named_fields,rules_rooms_named_fields,rules_magic_named_fields,
rules_creatures_named_fields,rules_computer_named_fields,rules_workers_named_fields,rules_health_named_fields,rules_script_only_named_fields};

//rules do need one for technical reasons, but isn't really relevant
const struct NamedFieldSet rules_named_fields_set = {
  NULL,
  "",
  NULL,
  NULL,
  1,
  0,
  &game.conf.rules
};

const struct NamedCommand rules_research_commands[] = {
  {"RESEARCH",                     1},
  {NULL,                           0},
};

const struct NamedCommand research_desc[] = {
  {"MAGIC",                        1},
  {"ROOM",                         2},
  {"CREATURE",                     3},
  {NULL,                           0},
};

const struct NamedCommand rules_sacrifices_commands[] = {
  {"MKCREATURE",                   SacA_MkCreature},
  {"MKGOODHERO",                   SacA_MkGoodHero},
  {"NEGSPELLALL",                  SacA_NegSpellAll},
  {"POSSPELLALL",                  SacA_PosSpellAll},
  {"NEGUNIQFUNC",                  SacA_NegUniqFunc},
  {"POSUNIQFUNC",                  SacA_PosUniqFunc},
  {"CUSTOMREWARD",                 SacA_CustomReward},
  {"CUSTOMPUNISH",                 SacA_CustomPunish},
  {NULL,                           0},
};

const struct NamedCommand sacrifice_unique_desc[] = {
  {"ALL_CREATRS_ANGRY",            UnqF_MkAllAngry},
  {"ALL_CREATRS_HAPPY",            UnqF_MkAllHappy},
  {"COMPLETE_RESEARCH",            UnqF_ComplResrch},
  {"COMPLETE_MANUFACTR",           UnqF_ComplManufc},
  {"KILL_ALL_CHICKENS",            UnqF_KillChickns},
  {"CHEAPER_IMPS",                 UnqF_CheaperImp},
  {"COSTLIER_IMPS",                UnqF_CostlierImp},
  {"ALL_CREATRS_VER_ANGRY",        UnqF_MkAllVerAngry},
  {NULL,                           0},
};

/******************************************************************************/

static void assign_MapCreatureLimit_script(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx)
{
    assign_named_field_value_direct(named_field,value,named_fields_set,idx);
    short count = setup_excess_creatures_to_leave_or_die(game.conf.rules.game.creatures_count);
    if (count > 0)
    {
        SCRPTLOG("Map creature limit reduced, causing %d creatures to leave or die",count);
    }
}

static void assign_AlliesShareVision_script(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx)
{
    assign_named_field_value_direct(named_field,value,named_fields_set,idx);
    panel_map_update(0, 0, gameadd.map_subtiles_x + 1, gameadd.map_subtiles_y + 1);
}

static int64_t value_x10(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx)
{
    
    if (parameter_is_number(value_text))
    {
        return 10 * atoll(value_text);
    }
    else
    {
        CONFWRNLOG("Expected number for field '%s', got '%s'",named_field->name,value_text);
    }
    return 0;
}

/**
 * Returns the first unused sacrifice slot, or invalid slot if no empty one.
 */
struct SacrificeRecipe *get_unused_sacrifice_recipe_slot(void)
{
    for (long i = 1; i < MAX_SACRIFICE_RECIPES; i++)
    {
        struct SacrificeRecipe* sac = &game.conf.rules.sacrifices.sacrifice_recipes[i];
        if (sac->action == SacA_None)
            return sac;
  }
  return &game.conf.rules.sacrifices.sacrifice_recipes[0];
}

/**
 * Clears all sacrifice slots.
 */
void clear_sacrifice_recipes(void)
{
    for (long i = 0; i < MAX_SACRIFICE_RECIPES; i++)
    {
        struct SacrificeRecipe* sac = &game.conf.rules.sacrifices.sacrifice_recipes[i];
        memset(sac, '\0', sizeof(struct SacrificeRecipe));
        sac->action = SacA_None;
  }
}

static int long_compare_fn(const void *ptr_a, const void *ptr_b)
{
    long *a = (long*)ptr_a;
    long *b = (long*)ptr_b;
    return *a < *b;
}

static void set_defaults()
{
    for (size_t i = 0; i < sizeof(ruleblocks)/sizeof(ruleblocks[0]); i++) {
      const struct NamedField* field = ruleblocks[i];
      while (field->name != NULL) {
        assign_named_field_value_direct(field, field->default_value, &rules_named_fields_set, 0);
        field++;
      }
    }
}

TbBool add_sacrifice_victim(struct SacrificeRecipe *sac, long crtr_idx)
{
    // If all slots are taken, then just drop it.
    if (sac->victims[MAX_SACRIFICE_VICTIMS - 1] != 0)
        return false;
    // Otherwise, find place for our item (array is sorted).
    for (long i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
        if (sac->victims[i] == 0)
        {
            sac->victims[i] = crtr_idx;
            qsort(sac->victims, MAX_SACRIFICE_VICTIMS, sizeof(sac->victims[0]), &long_compare_fn);
            return true;
        }
  }
  return false;
}

long get_research_id(long item_type, const char *trg_name, const char *func_name)
{
  long item_id;
  switch (item_type)
  {
  case 1:
       item_id = get_id(power_desc, trg_name);
      if (item_id == -1)
      {
        ERRORMSG("%s(line %lu): " "Unknown magic, '%s'", func_name, text_line_number, trg_name);
        return -1;
      }
      break;
  case 2:
      item_id = get_id(room_desc, trg_name);
      if (item_id == -1)
      {
        ERRORMSG("%s(line %lu): " "Unknown room, '%s'", func_name, text_line_number, trg_name);
        return -1;
      }
      break;
  case 3:
      item_id = get_id(creature_desc, trg_name);
      if (item_id == -1)
      {
        ERRORMSG("%s(line %lu): " "Unknown creature, '%s'", func_name, text_line_number, trg_name);
        return -1;
      }
      break;
  case -1:
  default:
      ERRORMSG("%s(line %lu): " "Unhandled research type, %ld", func_name, text_line_number, item_type);
      return -1;
  }
  return item_id;
}

/**
 * Returns Code Name (name to use in script file) of given player.
 */
const char *player_code_name(PlayerNumber plyr_idx)
{
    const char* name = get_conf_parameter_text(player_desc, plyr_idx);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

TbBool parse_rules_research_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  int i;
  // Block name and parameter word store variables.
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block.
  sprintf(block_buf,"research");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
  // Clear research list if there's new one in this file.
  clear_research_for_all_players();
  // Now we can start with analysis of commands.
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_research_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line.
      int cmd_num = recognize_conf_command(buf, &pos, len, rules_research_commands);
      // Now store the config item in correct place.
      if (cmd_num == ccr_endOfBlock) break; // If next block starts.
      int n = 0;
      int l = 0;
      switch (cmd_num)
      {
      case 1: // RESEARCH
          i = 0;
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
              {
                  i = get_id(research_desc, word_buf);
                  if (i >= 0)
                      n++;
              }
              if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
              {
                  l = get_research_id(i, word_buf, __func__);
                  if (l >= 0)
                      n++;
              }
              if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
              {
                  k = atoi(word_buf);
                  n++;
              }
              if (n < 3)
              {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num), block_buf, config_textname);
                  break;
              }
              add_research_to_all_players(i, l, k);
              break;
      case ccr_comment:
          break;
      case ccr_endOfFile:
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
              cmd_num,block_buf,config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
#undef COMMAND_TEXT
  return true;
}

/**
 * Searches the list of sacrifices for one which is supposed to make special diggers cheaper.
 */
static void mark_cheaper_diggers_sacrifice(void)
{
    game.conf.rules.sacrifices.cheaper_diggers_sacrifice_model = 0;
    for (int i = 1; i < MAX_SACRIFICE_RECIPES; i++)
    {
        struct SacrificeRecipe* sac = &game.conf.rules.sacrifices.sacrifice_recipes[i];
        if (sac->action == SacA_None)
            continue;
        if (((sac->action == SacA_PosUniqFunc) && (sac->param == UnqF_CheaperImp)) 
            || ((sac->action == SacA_NegUniqFunc) && (sac->param == UnqF_CostlierImp)))
        {
            if ((sac->victims[1] == 0) && (game.conf.rules.sacrifices.cheaper_diggers_sacrifice_model == 0)) {
                game.conf.rules.sacrifices.cheaper_diggers_sacrifice_model = sac->victims[0];
            } else {
                WARNLOG("Found unsupported %s sacrifice; either there's more than one, or has one than more victim.",
                    get_conf_parameter_text(sacrifice_unique_desc,UnqF_CheaperImp));
            }
        }
    }
    SYNCDBG(4,"Marked sacrifice of %s",thing_class_and_model_name(TCls_Creature, game.conf.rules.sacrifices.cheaper_diggers_sacrifice_model));
}

TbBool parse_rules_sacrifices_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    int i;
    // Block name and parameter word store variables.
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Find the block.
    sprintf(block_buf,"sacrifices");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
    // If the block exists, clear previous data.
    clear_sacrifice_recipes();
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_sacrifices_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line.
        int cmd_num = recognize_conf_command(buf, &pos, len, rules_sacrifices_commands);
        // Now store the config item in correct place.
        if (cmd_num == ccr_endOfBlock) break; // If next block starts.
        int n = 0;
        struct SacrificeRecipe* sac;
        switch (cmd_num)
        {
        case 1: // MKCREATURE
        case 2: // MKGOODHERO
            i = 0;
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              i = get_id(creature_desc, word_buf);
            }
            if (i <= 0)
            {
              CONFWRNLOG("Incorrect creature \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
            }
            sac = get_unused_sacrifice_recipe_slot();
            if (sac <= &game.conf.rules.sacrifices.sacrifice_recipes[0])
            {
              CONFWRNLOG("No free slot to store \"%s\" from [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
            }
            sac->action = cmd_num;
            sac->param = i;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              i = get_id(creature_desc, word_buf);
              if (i > 0)
              {
                if (add_sacrifice_victim(sac,i))
                {
                  n++;
                } else
                {
                  CONFWRNLOG("Too many victims in \"%s\" from [%s] block of %s file.",
                    word_buf,block_buf,config_textname);
                  break;
                }
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("No victims in \"%s\" from [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
            }
            n++; // Delayed increase for first argument.
            break;
        case 3: // NEGSPELLALL
        case 4: // POSSPELLALL
            i = 0;
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              i = get_id(spell_desc, word_buf);
            }
            if (i <= 0)
            {
              CONFWRNLOG("Incorrect creature spell \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
            }
            sac = get_unused_sacrifice_recipe_slot();
            if (sac <= &game.conf.rules.sacrifices.sacrifice_recipes[0])
            {
              CONFWRNLOG("No free slot to store \"%s\" from [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
            }
            sac->action = cmd_num;
            sac->param = i;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              i = get_id(creature_desc, word_buf);
              if (i > 0)
              {
                if (add_sacrifice_victim(sac,i))
                {
                  n++;
                } else
                {
                  CONFWRNLOG("Too many victims in \"%s\" from [%s] block of %s file.",
                    word_buf,block_buf,config_textname);
                  break;
                }
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("No victims in \"%s\" from [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
            }
            n++; // Delayed increase for first argument.
            break;
        case 5: // NEGUNIQFUNC
        case 6: // POSUNIQFUNC
            i = 0;
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              i = get_id(sacrifice_unique_desc, word_buf);
            }
            if (i <= 0)
            {
              CONFWRNLOG("Incorrect unique function \"%s\" in [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
            }
            sac = get_unused_sacrifice_recipe_slot();
            if (sac <= &game.conf.rules.sacrifices.sacrifice_recipes[0])
            {
              CONFWRNLOG("No free slot to store \"%s\" from [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
            }
            sac->action = cmd_num;
            sac->param = i;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              i = get_id(creature_desc, word_buf);
              if (i > 0)
              {
                if (add_sacrifice_victim(sac,i))
                {
                  n++;
                } else
                {
                  CONFWRNLOG("Too many victims in \"%s\" from [%s] block of %s file.",
                    word_buf,block_buf,config_textname);
                  break;
                }
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("No victims in \"%s\" from [%s] block of %s file.",
                  word_buf,block_buf,config_textname);
              break;
            }
            n++; // Delayed increase for first argument.
            break;
        case ccr_comment:
            break;
        case ccr_endOfFile:
            break;
        default:
            CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
                cmd_num,block_buf,config_textname);
            break;
        }
        skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
    mark_cheaper_diggers_sacrifice();
    return true;
}

TbBool load_rules_config_file(const char *textname, const char *fname, unsigned short flags)
{
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
        return false;
    // Loading file data.
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file.

    parse_named_field_block(buf, len, textname, flags,"game",     rules_game_named_fields,      &rules_named_fields_set, 0);
    parse_named_field_block(buf, len, textname, flags,"creatures",rules_creatures_named_fields, &rules_named_fields_set, 0);
    parse_named_field_block(buf, len, textname, flags,"rooms",    rules_rooms_named_fields,     &rules_named_fields_set, 0);
    parse_named_field_block(buf, len, textname, flags,"magic",    rules_magic_named_fields,     &rules_named_fields_set, 0);
    parse_named_field_block(buf, len, textname, flags,"computer", rules_computer_named_fields,  &rules_named_fields_set, 0);
    parse_named_field_block(buf, len, textname, flags,"workers",  rules_workers_named_fields,   &rules_named_fields_set, 0);
    parse_named_field_block(buf, len, textname, flags,"health",   rules_health_named_fields,    &rules_named_fields_set, 0);

    if (result)
    {
        result = parse_rules_research_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" research blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_rules_sacrifices_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" sacrifices blocks failed.",textname,fname);
    }
    //Freeing and exiting.
    free(buf);
    return result;
}

TbBool load_rules_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global rules config";
    static const char config_campgn_textname[] = "campaign rules config";
    static const char config_level_textname[] = "level rules config";

    set_defaults();

    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_rules_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_rules_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    fname = prepare_file_fmtpath(FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), conf_fname);
    if (strlen(fname) > 0)
    {
        load_rules_config_file(config_level_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting.
    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
