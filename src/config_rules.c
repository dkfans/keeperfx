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

static int64_t value_x10(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);

static void assign_MapCreatureLimit_script(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);
static void assign_AlliesShareVision_script(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);

/******************************************************************************/
static TbBool load_rules_config_file(const char *fname, unsigned short flags);
static void set_rules_defaults();

const struct ConfigFileData keeper_rules_file_data = {
    .filename = "rules.cfg",
    .load_func = load_rules_config_file,
    .pre_load_func = set_rules_defaults,
    .post_load_func = NULL,
};


const struct NamedCommand rules_game_classicbugs_commands[] = {
  {"RESURRECT_FOREVER",             ClscBug_ResurrectForever      },
  {"OVERFLOW_8BIT",                 ClscBug_Overflow8bitVal       },
  {"CLAIM_ROOM_ALL_THINGS",         ClscBug_ClaimRoomAllThings    },
  {"RESURRECT_REMOVED",             ClscBug_ResurrectRemoved      },
  {"NO_HAND_PURGE_ON_DEFEAT",       ClscBug_NoHandPurgeOnDefeat   },
  {"MUST_OBEY_KEEPS_NOT_DO_JOBS",   ClscBug_MustObeyKeepsNotDoJobs},
  {"BREAK_NEUTRAL_WALLS",           ClscBug_BreakNeutralWalls     },
  {"ALWAYS_TUNNEL_TO_RED",          ClscBug_AlwaysTunnelToRed     },
  {"FULLY_HAPPY_WITH_GOLD",         ClscBug_FullyHappyWithGold    },
  {"FAINTED_IMMUNE_TO_BOULDER",     ClscBug_FaintedImmuneToBoulder},
  {"REBIRTH_KEEPS_SPELLS",          ClscBug_RebirthKeepsSpells    },
  {"STUN_FRIENDLY_UNITS",           ClscBug_FriendlyFaint         },
  {"PASSIVE_NEUTRALS",              ClscBug_PassiveNeutrals       },
  {"NEUTRAL_TORTURE_CONVERTS",      ClscBug_NeutralTortureConverts},
  {NULL,                             0},
};

static const struct NamedField rules_game_named_fields[] = {
    //name                    //param  //field                                             //default  //min               //max   //namedCommand                    //valueFunc
  {"POTOFGOLDHOLDS",            0, field(game.conf.rules[0].game.pot_of_gold_holds         ),        1000, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"CHESTGOLDHOLD",             0, field(game.conf.rules[0].game.chest_gold_hold           ),        1000, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"GOLDPILEVALUE",             0, field(game.conf.rules[0].game.gold_pile_value           ),         500, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"GOLDPILEMAXIMUM",           0, field(game.conf.rules[0].game.gold_pile_maximum         ),        5000, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"GOLDPERHOARD",              0, field(game.conf.rules[0].game.gold_per_hoard            ),        2000, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"FOODLIFEOUTOFHATCHERY",     0, field(game.conf.rules[0].game.food_life_out_of_hatchery ),         100,        0,          USHRT_MAX,NULL,                           value_default, assign_default},
  {"BOULDERREDUCEHEALTHSLAP",   0, field(game.conf.rules[0].game.boulder_reduce_health_slap),          10, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"BOULDERREDUCEHEALTHWALL",   0, field(game.conf.rules[0].game.boulder_reduce_health_wall),          10, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"BOULDERREDUCEHEALTHROOM",   0, field(game.conf.rules[0].game.boulder_reduce_health_room),          10, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"PAYDAYGAP",                 0, field(game.conf.rules[0].game.pay_day_gap               ),        5000, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"PAYDAYSPEED",               0, field(game.conf.rules[0].game.pay_day_speed             ),         100,        0,          UINT32_MAX,NULL,                           value_default, assign_default},
  {"DUNGEONHEARTHEALTIME",      0, field(game.conf.rules[0].game.dungeon_heart_heal_time   ),          10,        0,          UINT32_MAX,NULL,                           value_default, assign_default},
  {"DUNGEONHEARTHEALHEALTH",    0, field(game.conf.rules[0].game.dungeon_heart_heal_health ),           1, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"HERODOORWAITTIME",          0, field(game.conf.rules[0].game.hero_door_wait_time       ),         100,        0,          UINT32_MAX,NULL,                           value_default, assign_default},
  {"ROOMSELLGOLDBACKPERCENT",   0, field(game.conf.rules[0].game.room_sale_percent         ),          50,        0,           INT32_MAX,NULL,                           value_default, assign_default},
  {"DOORSELLVALUEPERCENT",      0, field(game.conf.rules[0].game.door_sale_percent         ),         100,        0,           INT32_MAX,NULL,                           value_default, assign_default},
  {"TRAPSELLVALUEPERCENT",      0, field(game.conf.rules[0].game.trap_sale_percent         ),         100,        0,           INT32_MAX,NULL,                           value_default, assign_default},
  {"BAGGOLDHOLD",               0, field(game.conf.rules[0].game.bag_gold_hold             ),         200, INT32_MIN,           INT32_MAX,NULL,                           value_default, assign_default},
  {"ALLIESSHAREVISION",         0, field(game.conf.rules[0].game.allies_share_vision       ),           0,        0,                  1,NULL,                           value_default, assign_AlliesShareVision_script},
  {"ALLIESSHAREDROP",           0, field(game.conf.rules[0].game.allies_share_drop         ),           0,        0,                  1,NULL,                           value_default, assign_default},
  {"ALLIESSHARECTA",            0, field(game.conf.rules[0].game.allies_share_cta          ),           0,        0,                  1,NULL,                           value_default, assign_default},
  {"DISPLAYPORTALLIMIT",        0, field(game.conf.rules[0].game.display_portal_limit      ),           0,        0,                  1,NULL,                           value_default, assign_default},
  {"MAXTHINGSINHAND",           0, field(game.conf.rules[0].game.max_things_in_hand        ),           8,        0, MAX_THINGS_IN_HAND,NULL,                           value_default, assign_default},
  {"TORTUREPAYDAY",             0, field(game.conf.rules[0].game.torture_payday            ),          50,        0,          USHRT_MAX,NULL,                           value_default, assign_default},
  {"TORTURETRAININGCOST",       0, field(game.conf.rules[0].game.torture_training_cost     ),         100, SHRT_MIN,           SHRT_MAX,NULL,                           value_default, assign_default},
  {"TORTURESCAVENGINGCOST",     0, field(game.conf.rules[0].game.torture_scavenging_cost   ),         100, SHRT_MIN,           SHRT_MAX,NULL,                           value_default, assign_default},
  {"EASTEREGGSPEECHCHANCE",     0, field(game.conf.rules[0].game.easter_egg_speech_chance  ),        2000,        0,           INT32_MAX,NULL,                           value_default, assign_default},
  {"EASTEREGGSPEECHINTERVAL",   0, field(game.conf.rules[0].game.easter_egg_speech_interval),       20000,        0,           INT32_MAX,NULL,                           value_default, assign_default},
  {"GLOBALAMBIENTLIGHT",        0, field(game.conf.rules[0].game.global_ambient_light      ),          10,        0,           INT32_MAX,NULL,                           value_default, assign_default},
  {"THINGMINIMUMILLUMINATION",  0, field(game.conf.rules[0].game.thing_minimum_illumination),          32,        0,           INT32_MAX,NULL,                           value_default, assign_default},
  {"LIGHTENABLED",              0, field(game.conf.rules[0].game.light_enabled             ),           1,        0,                  1,NULL,                           value_default, assign_default},
  {"MAPCREATURELIMIT",          0, field(game.conf.rules[0].game.creatures_count           ),         255,        0,  CREATURES_COUNT-2,NULL,                           value_default, assign_MapCreatureLimit_script},
  {"PRESERVECLASSICBUGS",      -1, field(game.conf.rules[0].game.classic_bugs_flags        ),ClscBug_None,ClscBug_None, ClscBug_ListEnd,rules_game_classicbugs_commands,value_flagsfield, assign_default},
  {NULL},
};


static const struct NamedField rules_computer_named_fields[] = {
    //name                    //param  //field                                           //default    //min     //max
  {"DISEASEHPTEMPLEPERCENTAGE",  0, field(game.conf.rules[0].computer.disease_to_temple_pct),500,        0, USHRT_MAX,NULL,value_default,assign_default},
  {NULL},
};

static const struct NamedField rules_creatures_named_fields[] = {
    //name                    //param  //field                                           //default   //min     //max  //namedCommand //valueFunc
  {"RECOVERYFREQUENCY",          0, field(game.conf.rules[0].creature.recovery_frequency    )    ,  10,        0, UCHAR_MAX,NULL,value_default, assign_default},
  {"BODYREMAINSFOR",             0, field(game.conf.rules[0].creature.body_remains_for      )    ,1000,        0, USHRT_MAX,NULL,value_default, assign_default},
  {"FLEEZONERADIUS",             0, field(game.conf.rules[0].creature.flee_zone_radius      )    ,2048,        0, UINT32_MAX,NULL,value_default, assign_default},
  {"GAMETURNSINFLEE",            0, field(game.conf.rules[0].creature.game_turns_in_flee    )    , 200,        0,  INT32_MAX,NULL,value_default, assign_default},
  {"GAMETURNSUNCONSCIOUS",       0, field(game.conf.rules[0].creature.game_turns_unconscious)    ,2000,        0, USHRT_MAX,NULL,value_default, assign_default},
  {"CRITICALHEALTHPERCENTAGE",   0, field(game.conf.rules[0].creature.critical_health_permil)    , 125,        0,       100,NULL,value_x10    , assign_default},
  {"STUNEVILENEMYCHANCE",        0, field(game.conf.rules[0].creature.stun_enemy_chance_evil)    , 100,        0,       100,NULL,value_default, assign_default},
  {"STUNGOODENEMYCHANCE",        0, field(game.conf.rules[0].creature.stun_enemy_chance_good)    , 100,        0,       100,NULL,value_default, assign_default},
  {"STUNWITHOUTPRISONCHANCE",    0, field(game.conf.rules[0].creature.stun_without_prison_chance),   0,        0,       100,NULL,value_default, assign_default},
  {NULL},
};

static const struct NamedField rules_magic_named_fields[] = {
    //name                        //param  //field                                                     //default //min  //max //namedCommand //valueFunc
  {"HOLDAUDIENCETIME",               0, field(game.conf.rules[0].magic.hold_audience_time                ), 500,        0, INT32_MAX,NULL,value_default, assign_default},
  {"ARMAGEDDONTELEPORTYOURTIMEGAP",  0, field(game.conf.rules[0].magic.armageddon_teleport_your_time_gap ),  10, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"ARMAGEDDONTELEPORTENEMYTIMEGAP", 0, field(game.conf.rules[0].magic.armageddon_teleport_enemy_time_gap),  10,        0, INT32_MAX,NULL,value_default, assign_default},
  {"ARMAGEDDONTELEPORTNEUTRALS",     0, field(game.conf.rules[0].magic.armageddon_teleport_neutrals      ),   0,        0,        1,NULL,value_default, assign_default},
  {"ARMAGEDDONCOUNTDOWN",            0, field(game.conf.rules[0].magic.armageddon_count_down             ), 500, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"ARMAGEDDONDURATION",             0, field(game.conf.rules[0].magic.armageddon_duration               ),4000, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"DISEASETRANSFERPERCENTAGE",      0, field(game.conf.rules[0].magic.disease_transfer_percentage       ),  15,        0, CHAR_MAX,NULL,value_default, assign_default},
  {"DISEASELOSEPERCENTAGEHEALTH",    0, field(game.conf.rules[0].magic.disease_lose_percentage_health    ),   8, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"DISEASELOSEHEALTHTIME",          0, field(game.conf.rules[0].magic.disease_lose_health_time          ), 200, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"MINDISTANCEFORTELEPORT",         0, field(game.conf.rules[0].magic.min_distance_for_teleport         ),  20, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"COLLAPSEDUNGEONDAMAGE",          0, field(game.conf.rules[0].magic.collapse_dungeon_damage           ),  10, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"TURNSPERCOLLAPSEDUNGEONDAMAGE",  0, field(game.conf.rules[0].magic.turns_per_collapse_dngn_dmg       ),   4, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"FRIENDLYFIGHTAREARANGEPERCENT",  0, field(game.conf.rules[0].magic.friendly_fight_area_range_percent ),   0, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"FRIENDLYFIGHTAREADAMAGEPERCENT", 0, field(game.conf.rules[0].magic.friendly_fight_area_damage_percent),   0, INT32_MIN, INT32_MAX,NULL,value_default, assign_default},
  {"WEIGHTCALCULATEPUSH",            0, field(game.conf.rules[0].magic.weight_calculate_push             ),   0,        0, SHRT_MAX,NULL,value_default, assign_default},
  {"ALLOWINSTANTCHARGEUP",           0, field(game.conf.rules[0].magic.allow_instant_charge_up           ),   0,        0,        1,NULL,value_default, assign_default},
  {NULL},
};

static const struct NamedField rules_rooms_named_fields[] = {
    //name                             //param  //field                                                  //default //min                //max  //namedCommand //valueFunc
  {"SCAVENGECOSTFREQUENCY",               0, field(game.conf.rules[0].rooms.scavenge_cost_frequency         ),  64, INT32_MIN,            INT32_MAX,NULL,value_default, assign_default},
  {"TEMPLESCAVENGEPROTECTIONTIME",        0, field(game.conf.rules[0].rooms.temple_scavenge_protection_turns),1000,        0,           UINT32_MAX,NULL,value_default, assign_default},
  {"TRAINCOSTFREQUENCY",                  0, field(game.conf.rules[0].rooms.train_cost_frequency            ),  64, INT32_MIN,            INT32_MAX,NULL,value_default, assign_default},
  {"TORTURECONVERTCHANCE",                0, field(game.conf.rules[0].rooms.torture_convert_chance          ),  33,        0,                 100,NULL,value_default, assign_default},
  {"TIMESPENTINPRISONWITHOUTBREAK",       0, field(game.conf.rules[0].rooms.time_in_prison_without_break    ),2400,        0,           UINT32_MAX,NULL,value_default, assign_default},
  {"GHOSTCONVERTCHANCE",                  0, field(game.conf.rules[0].rooms.ghost_convert_chance            ),  10,        0,                 100,NULL,value_default, assign_default},
  {"DEFAULTGENERATESPEED",                0, field(game.conf.rules[0].rooms.default_generate_speed          ), 350,        0,           USHRT_MAX,NULL,value_default, assign_default},
  {"DEFAULTMAXCREATURESGENERATEENTRANCE", 0, field(game.conf.rules[0].rooms.default_max_crtrs_gen_entrance  ), 200,        0,           UINT32_MAX,NULL,value_default, assign_default},
  {"FOODGENERATIONSPEED",                 0, field(game.conf.rules[0].rooms.food_generation_speed           ),2000, INT32_MIN,            INT32_MAX,NULL,value_default, assign_default},
  {"PRISONSKELETONCHANCE",                0, field(game.conf.rules[0].rooms.prison_skeleton_chance          ), 100,        0,                 100,NULL,value_default, assign_default},
  {"BODIESFORVAMPIRE",                    0, field(game.conf.rules[0].rooms.bodies_for_vampire              ),   6,        0,           UCHAR_MAX,NULL,value_default, assign_default},
  {"GRAVEYARDCONVERTTIME",                0, field(game.conf.rules[0].rooms.graveyard_convert_time          ), 300,        0,           USHRT_MAX,NULL,value_default, assign_default},
  {"SCAVENGEGOODALLOWED",                 0, field(game.conf.rules[0].rooms.scavenge_good_allowed           ),   1,        0,                   1,NULL,value_default, assign_default},
  {"SCAVENGENEUTRALALLOWED",              0, field(game.conf.rules[0].rooms.scavenge_neutral_allowed        ),   1,        0,                   1,NULL,value_default, assign_default},
  {"TIMEBETWEENPRISONBREAK",              0, field(game.conf.rules[0].rooms.time_between_prison_break       ),  64,        0,           UINT32_MAX,NULL,value_default, assign_default},
  {"PRISONBREAKCHANCE",                   0, field(game.conf.rules[0].rooms.prison_break_chance             ),  50,        0,           UINT32_MAX,NULL,value_default, assign_default},
  {"TORTUREDEATHCHANCE",                  0, field(game.conf.rules[0].rooms.torture_death_chance            ),   0,        0,                 100,NULL,value_default, assign_default},
  {"BARRACKMAXPARTYSIZE",                 0, field(game.conf.rules[0].rooms.barrack_max_party_size          ),  10,        0, GROUP_MEMBERS_COUNT,NULL,value_default, assign_default},
  {"TRAININGROOMMAXLEVEL",                0, field(game.conf.rules[0].rooms.training_room_max_level         ),   0,        0,CREATURE_MAX_LEVEL+1,NULL,value_default, assign_default},
  {"TRAINEFFICIENCY",                     0, field(game.conf.rules[0].rooms.train_efficiency                ), 256,        0,            USHRT_MAX,NULL,value_default, assign_default},
  {"WORKEFFICIENCY",                      0, field(game.conf.rules[0].rooms.work_efficiency                 ), 256,        0,            USHRT_MAX,NULL,value_default, assign_default},
  {"RESEARCHEFFICIENCY",                  0, field(game.conf.rules[0].rooms.research_efficiency             ), 256,        0,            USHRT_MAX,NULL,value_default, assign_default},
  {"SCAVENGEEFFICIENCY",                  0, field(game.conf.rules[0].rooms.scavenge_efficiency             ), 256,        0,            USHRT_MAX,NULL,value_default, assign_default},
  {NULL},
};

static const struct NamedField rules_workers_named_fields[] = {
    //name                    //param  //field                                              //default  //min   //max  //namedCommand //valueFunc
  {"HITSPERSLAB",                0, field(game.conf.rules[0].workers.hits_per_slab              ),  2,       0, UCHAR_MAX,NULL,value_default, assign_default},
  {"DEFAULTIMPDIGDAMAGE",        0, field(game.conf.rules[0].workers.default_imp_dig_damage     ),  1,       0, UINT32_MAX,NULL,value_default, assign_default},
  {"DEFAULTIMPDIGOWNDAMAGE",     0, field(game.conf.rules[0].workers.default_imp_dig_own_damage ),  2,       0, UINT32_MAX,NULL,value_default, assign_default},
  {"IMPWORKEXPERIENCE",          0, field(game.conf.rules[0].workers.digger_work_experience     ),  0,       0,  INT32_MAX,NULL,value_default, assign_default},
  {"DRAGUNCONSCIOUSTOLAIR",      0, field(game.conf.rules[0].workers.drag_to_lair               ),  0,       0,         2,NULL,value_default, assign_default},
  {NULL},
};

static const struct NamedField rules_health_named_fields[] = {
    //name                       //param  //field                                             //default  //min   //max  //namedCommand //valueFunc
  {"HUNGERHEALTHLOSS",              0, field(game.conf.rules[0].health.hunger_health_loss           ),  1, INT32_MIN,  INT32_MAX,NULL,value_default, assign_default},
  {"GAMETURNSPERHUNGERHEALTHLOSS",  0, field(game.conf.rules[0].health.turns_per_hunger_health_loss ),100,        0, USHRT_MAX,NULL,value_default, assign_default},
  {"FOODHEALTHGAIN",                0, field(game.conf.rules[0].health.food_health_gain             ), 10, INT32_MIN,  INT32_MAX,NULL,value_default, assign_default},
  {"TORTUREHEALTHLOSS",             0, field(game.conf.rules[0].health.torture_health_loss          ),  5, INT32_MIN,  INT32_MAX,NULL,value_default, assign_default},
  {"GAMETURNSPERTORTUREHEALTHLOSS", 0, field(game.conf.rules[0].health.turns_per_torture_health_loss),100,        0, USHRT_MAX,NULL,value_default, assign_default},
  {NULL},
};

static const struct NamedField rules_script_only_named_fields[] = {
  //name            //field                   //min //max
{"PayDayProgress",0,field(game.pay_day_progress[0]),0,0,INT32_MAX,NULL,value_default,assign_default},
{NULL},
};

const struct NamedField* ruleblocks[] = {rules_game_named_fields,rules_rooms_named_fields,rules_magic_named_fields,
rules_creatures_named_fields,rules_computer_named_fields,rules_workers_named_fields,rules_health_named_fields,rules_script_only_named_fields};

//rules don't need all the fields as it's always only 1 entry in the cfg
const struct NamedFieldSet rules_named_fields_set = {
  NULL,
  "",
  NULL,
  NULL,
  PLAYERS_COUNT,
  sizeof(game.conf.rules[0]),
  &game.conf.rules,
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

static void assign_MapCreatureLimit_script(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    assign_default(named_field,value,named_fields_set,idx,src_str,flags);
    if (flag_is_set(flags,ccf_DuringLevel))
    {
        short count = setup_excess_creatures_to_leave_or_die(game.conf.rules[idx].game.creatures_count);
        if (count > 0)
        {
            SCRPTLOG("Map creature limit reduced, causing %d creatures to leave or die",count);
        }
    }
}

static void assign_AlliesShareVision_script(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    assign_default(named_field,value,named_fields_set,idx,src_str,flags);
    if (flag_is_set(flags,ccf_DuringLevel))
    {
      panel_map_update(0, 0, game.map_subtiles_x + 1, game.map_subtiles_y + 1);
    }
}

static int64_t value_x10(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
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
        struct SacrificeRecipe* sac = &game.conf.rules[0].sacrifices.sacrifice_recipes[i];
        if (sac->action == SacA_None)
            return sac;
  }
  return &game.conf.rules[0].sacrifices.sacrifice_recipes[0];
}

/**
 * Clears all sacrifice slots.
 */
void clear_sacrifice_recipes(void)
{
    for (long i = 0; i < MAX_SACRIFICE_RECIPES; i++)
    {
        struct SacrificeRecipe* sac = &game.conf.rules[0].sacrifices.sacrifice_recipes[i];
        memset(sac, '\0', sizeof(struct SacrificeRecipe));
        sac->action = SacA_None;
  }
}

int sac_compare_fn(const void* ptr_a, const void* ptr_b)
{
    ThingModel a = *(const ThingModel*)ptr_a;
    ThingModel b = *(const ThingModel*)ptr_b;
    return a < b;
}

static void set_rules_defaults()
{
    for (size_t i = 0; i < sizeof(ruleblocks) / sizeof(ruleblocks[0]); i++) {
        const struct NamedField* field = ruleblocks[i];
        while (field->name != NULL) {
            for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
            {
                assign_default(field, field->default_value, &rules_named_fields_set, plyr_idx, "rules", ccf_SplitExecution | ccf_DuringLevel);
            }
            field++;
        }
    }
}

TbBool add_sacrifice_victim(struct SacrificeRecipe *sac, ThingModel crtr_idx)
{
    // If all slots are taken, then just drop it.
    if (sac->victims[MAX_SACRIFICE_VICTIMS - 1] != 0)
        return false;
    // Otherwise, find place for our item (array is sorted).
    for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
        if (sac->victims[i] == 0)
        {
            sac->victims[i] = crtr_idx;
            qsort(sac->victims, MAX_SACRIFICE_VICTIMS, sizeof(sac->victims[0]), &sac_compare_fn);
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
  const char * block_name = "research";
  char word_buf[COMMAND_WORD_LEN];
  // Find the block.
  int32_t pos = 0;
  int k = find_conf_block(buf, &pos, len, block_name);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.", block_name, config_textname);
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
                      COMMAND_TEXT(cmd_num), block_name, config_textname);
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
              cmd_num, block_name, config_textname);
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
    game.conf.rules[0].sacrifices.cheaper_diggers_sacrifice_model = 0;
    for (int i = 1; i < MAX_SACRIFICE_RECIPES; i++)
    {
        struct SacrificeRecipe* sac = &game.conf.rules[0].sacrifices.sacrifice_recipes[i];
        if (sac->action == SacA_None)
            continue;
        if (((sac->action == SacA_PosUniqFunc) && (sac->param == UnqF_CheaperImp))
            || ((sac->action == SacA_NegUniqFunc) && (sac->param == UnqF_CostlierImp)))
        {
            if ((sac->victims[1] == 0) && (game.conf.rules[0].sacrifices.cheaper_diggers_sacrifice_model == 0)) {
                game.conf.rules[0].sacrifices.cheaper_diggers_sacrifice_model = sac->victims[0];
            } else {
                WARNLOG("Found unsupported %s sacrifice; either there's more than one, or has one than more victim.",
                    get_conf_parameter_text(sacrifice_unique_desc,UnqF_CheaperImp));
            }
        }
    }
    SYNCDBG(4,"Marked sacrifice of %s",thing_class_and_model_name(TCls_Creature, game.conf.rules[0].sacrifices.cheaper_diggers_sacrifice_model));
}

TbBool parse_rules_sacrifices_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    int i;
    const char * block_name = "sacrifices";
    char word_buf[COMMAND_WORD_LEN];
    // Find the block.
    int32_t pos = 0;
    int k = find_conf_block(buf, &pos, len, block_name);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.", block_name, config_textname);
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
                  word_buf, block_name, config_textname);
              break;
            }
            sac = get_unused_sacrifice_recipe_slot();
            if (sac <= &game.conf.rules[0].sacrifices.sacrifice_recipes[0])
            {
              CONFWRNLOG("No free slot to store \"%s\" from [%s] block of %s file.",
                  word_buf, block_name, config_textname);
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
                    word_buf, block_name, config_textname);
                  break;
                }
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("No victims in \"%s\" from [%s] block of %s file.",
                  word_buf, block_name, config_textname);
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
                  word_buf, block_name, config_textname);
              break;
            }
            sac = get_unused_sacrifice_recipe_slot();
            if (sac <= &game.conf.rules[0].sacrifices.sacrifice_recipes[0])
            {
              CONFWRNLOG("No free slot to store \"%s\" from [%s] block of %s file.",
                  word_buf, block_name, config_textname);
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
                    word_buf, block_name, config_textname);
                  break;
                }
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("No victims in \"%s\" from [%s] block of %s file.",
                  word_buf, block_name, config_textname);
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
                  word_buf, block_name, config_textname);
              break;
            }
            sac = get_unused_sacrifice_recipe_slot();
            if (sac <= &game.conf.rules[0].sacrifices.sacrifice_recipes[0])
            {
              CONFWRNLOG("No free slot to store \"%s\" from [%s] block of %s file.",
                  word_buf, block_name, config_textname);
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
                    word_buf, block_name, config_textname);
                  break;
                }
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("No victims in \"%s\" from [%s] block of %s file.",
                  word_buf, block_name, config_textname);
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
                cmd_num, block_name, config_textname);
            break;
        }
        skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
    mark_cheaper_diggers_sacrifice();
    return true;
}

static TbBool load_rules_config_file(const char *fname, unsigned short flags)
{
    SYNCDBG(0,"%s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("file \"%s\" doesn't exist or is too small.",fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
        return false;
    // Loading file data.
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file.

    if (result)
    {
        for (int i = 0; i < PLAYERS_COUNT; i++)
        {
          parse_named_field_block(buf, len, fname, flags,"game",     rules_game_named_fields,      &rules_named_fields_set, i);
          parse_named_field_block(buf, len, fname, flags,"creatures",rules_creatures_named_fields, &rules_named_fields_set, i);
          parse_named_field_block(buf, len, fname, flags,"rooms",    rules_rooms_named_fields,     &rules_named_fields_set, i);
          parse_named_field_block(buf, len, fname, flags,"magic",    rules_magic_named_fields,     &rules_named_fields_set, i);
          parse_named_field_block(buf, len, fname, flags,"computer", rules_computer_named_fields,  &rules_named_fields_set, i);
          parse_named_field_block(buf, len, fname, flags,"workers",  rules_workers_named_fields,   &rules_named_fields_set, i);
          parse_named_field_block(buf, len, fname, flags,"health",   rules_health_named_fields,    &rules_named_fields_set, i);
        }


        parse_rules_research_blocks(buf, len, fname, flags);
        parse_rules_sacrifices_blocks(buf, len, fname, flags);
    }
    //Freeing and exiting.
    free(buf);
    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
