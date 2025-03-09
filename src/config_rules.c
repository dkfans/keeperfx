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
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
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

const struct NamedField rules_game_named_fields[] = {
    //name                        //field                                           //field type                                               //min               //max
  {"GOLDPERGOLDBLOCK",           &game.conf.rules.game.gold_per_gold_block,        var_type(game.conf.rules.game.gold_per_gold_block       ), LONG_MIN,           LONG_MAX},
  {"POTOFGOLDHOLDS",             &game.conf.rules.game.pot_of_gold_holds,          var_type(game.conf.rules.game.pot_of_gold_holds         ), LONG_MIN,           LONG_MAX},
  {"CHESTGOLDHOLD",              &game.conf.rules.game.chest_gold_hold,            var_type(game.conf.rules.game.chest_gold_hold           ), LONG_MIN,           LONG_MAX},
  {"GOLDPILEVALUE",              &game.conf.rules.game.gold_pile_value,            var_type(game.conf.rules.game.gold_pile_value           ), LONG_MIN,           LONG_MAX},
  {"GOLDPILEMAXIMUM",            &game.conf.rules.game.gold_pile_maximum,          var_type(game.conf.rules.game.gold_pile_maximum         ), LONG_MIN,           LONG_MAX},
  {"GOLDPERHOARD",               &game.conf.rules.game.gold_per_hoard,             var_type(game.conf.rules.game.gold_per_hoard            ), LONG_MIN,           LONG_MAX},
  {"FOODLIFEOUTOFHATCHERY",      &game.conf.rules.game.food_life_out_of_hatchery,  var_type(game.conf.rules.game.food_life_out_of_hatchery ),        0,          USHRT_MAX},
  {"BOULDERREDUCEHEALTHSLAP",    &game.conf.rules.game.boulder_reduce_health_wall, var_type(game.conf.rules.game.boulder_reduce_health_wall), LONG_MIN,           LONG_MAX},
  {"BOULDERREDUCEHEALTHWALL",    &game.conf.rules.game.boulder_reduce_health_slap, var_type(game.conf.rules.game.boulder_reduce_health_slap), LONG_MIN,           LONG_MAX},
  {"BOULDERREDUCEHEALTHROOM",    &game.conf.rules.game.boulder_reduce_health_room, var_type(game.conf.rules.game.boulder_reduce_health_room), LONG_MIN,           LONG_MAX},
  {"PAYDAYGAP",                  &game.conf.rules.game.pay_day_gap,                var_type(game.conf.rules.game.pay_day_gap               ), LONG_MIN,           LONG_MAX},
  {"PAYDAYSPEED",                &game.conf.rules.game.pay_day_speed,              var_type(game.conf.rules.game.pay_day_speed             ),        0,          ULONG_MAX},
  {"DUNGEONHEARTHEALTIME",       &game.conf.rules.game.dungeon_heart_heal_time,    var_type(game.conf.rules.game.dungeon_heart_heal_time   ),        0,          ULONG_MAX},
  {"DUNGEONHEARTHEALHEALTH",     &game.conf.rules.game.dungeon_heart_heal_health,  var_type(game.conf.rules.game.dungeon_heart_heal_health ), LONG_MIN,           LONG_MAX},
  {"HERODOORWAITTIME",           &game.conf.rules.game.hero_door_wait_time,        var_type(game.conf.rules.game.hero_door_wait_time       ),        0,          ULONG_MAX},
  {"GEMEFFECTIVENESS",           &game.conf.rules.game.gem_effectiveness,          var_type(game.conf.rules.game.gem_effectiveness         ),        0,          ULONG_MAX},
  {"ROOMSELLGOLDBACKPERCENT",    &game.conf.rules.game.room_sale_percent,          var_type(game.conf.rules.game.room_sale_percent         ),        0,           LONG_MAX},
  {"DOORSELLVALUEPERCENT",       &game.conf.rules.game.door_sale_percent,          var_type(game.conf.rules.game.door_sale_percent         ),        0,           LONG_MAX},
  {"TRAPSELLVALUEPERCENT",       &game.conf.rules.game.trap_sale_percent,          var_type(game.conf.rules.game.trap_sale_percent         ),        0,           LONG_MAX},
  {"BAGGOLDHOLD",                &game.conf.rules.game.bag_gold_hold,              var_type(game.conf.rules.game.bag_gold_hold             ), LONG_MIN,           LONG_MAX},
  {"ALLIESSHAREVISION",          &game.conf.rules.game.allies_share_vision,        var_type(game.conf.rules.game.allies_share_vision       ),        0,                  1},
  {"ALLIESSHAREDROP",            &game.conf.rules.game.allies_share_drop,          var_type(game.conf.rules.game.allies_share_drop         ),        0,                  1},
  {"ALLIESSHARECTA",             &game.conf.rules.game.allies_share_cta,           var_type(game.conf.rules.game.allies_share_cta          ),        0,                  1},
  {"DISPLAYPORTALLIMIT",         &game.conf.rules.game.display_portal_limit,       var_type(game.conf.rules.game.display_portal_limit      ),        0,                  1},
  {"MAXTHINGSINHAND",            &game.conf.rules.game.max_things_in_hand,         var_type(game.conf.rules.game.max_things_in_hand        ),        0, MAX_THINGS_IN_HAND},
  {"TORTUREPAYDAY",              &game.conf.rules.game.torture_payday,             var_type(game.conf.rules.game.torture_payday            ),        0,          USHRT_MAX},
  {"TORTURETRAININGCOST",        &game.conf.rules.game.torture_training_cost,      var_type(game.conf.rules.game.torture_training_cost     ), SHRT_MIN,           SHRT_MAX},
  {"TORTURESCAVENGINGCOST",      &game.conf.rules.game.torture_scavenging_cost,    var_type(game.conf.rules.game.torture_scavenging_cost   ), SHRT_MIN,           SHRT_MAX},
  {"EASTEREGGSPEECHCHANCE",      &game.conf.rules.game.easter_egg_speech_chance,   var_type(game.conf.rules.game.easter_egg_speech_chance  ),        0,           LONG_MAX},
  {"EASTEREGGSPEECHINTERVAL",    &game.conf.rules.game.easter_egg_speech_interval, var_type(game.conf.rules.game.easter_egg_speech_interval),        0,           LONG_MAX},
  {"GLOBALAMBIENTLIGHT",         &game.conf.rules.game.global_ambient_light,       var_type(game.conf.rules.game.global_ambient_light      ), LONG_MIN,           LONG_MAX},
  {"LIGHTENABLED",               &game.conf.rules.game.light_enabled,              var_type(game.conf.rules.game.light_enabled             ),        0,                  1},
  {"MAPCREATURELIMIT",           &game.conf.rules.game.creatures_count,            var_type(game.conf.rules.game.creatures_count           ),        0,  CREATURES_COUNT-2},
  {"PAYDAYADVANCED",             &game.conf.rules.game.max_paydays_advanced,       var_type(game.conf.rules.game.max_paydays_advanced      ),        0,          UCHAR_MAX},
  {"PAYDAYOWED",                 &game.conf.rules.game.max_paydays_owed,           var_type(game.conf.rules.game.max_paydays_owed          ),        0,          UCHAR_MAX},
  {"ACCEPTPARTIALPAYDAY",        &game.conf.rules.game.accept_partial_payday,      var_type(game.conf.rules.game.accept_partial_payday     ),        0,                  1},
  {"POCKETGOLD",                 &game.conf.rules.game.pocket_gold,                var_type(game.conf.rules.game.pocket_gold               ),        0,                  1},
  {"TAKEPAYFROMPOCKET",          &game.conf.rules.game.take_pay_from_pocket,       var_type(game.conf.rules.game.take_pay_from_pocket      ),        0,                  1},
  {NULL,                         NULL,                                                                                                     0,        0,                  0},
};
// Special cases rules_game.
const struct NamedCommand rules_game_commands[] = {
  {"PRESERVECLASSICBUGS",          1},
  {NULL,                           0},
};

const struct NamedField rules_computer_named_fields[] = {
    //name                        //field                                           //field type                                              //min     //max
  {"DISEASEHPTEMPLEPERCENTAGE",  &game.conf.rules.computer.disease_to_temple_pct,  var_type(game.conf.rules.computer.disease_to_temple_pct),        0, USHRT_MAX},
  {NULL,                         NULL,                                                                                                    0,        0,         0},
};

const struct NamedField rules_creatures_named_fields[] = {
    //name                        //field                                           //field type                                               //min     //max
  {"RECOVERYFREQUENCY",          &game.conf.rules.creature.recovery_frequency,     var_type(game.conf.rules.creature.recovery_frequency    ),        0, UCHAR_MAX},
  {"FIGHTMAXHATE",               &game.conf.rules.creature.fight_max_hate,         var_type(game.conf.rules.creature.fight_max_hate        ), SHRT_MIN,  SHRT_MAX},
  {"FIGHTBORDERLINE",            &game.conf.rules.creature.fight_borderline,       var_type(game.conf.rules.creature.fight_borderline      ), SHRT_MIN,  SHRT_MAX},
  {"FIGHTMAXLOVE",               &game.conf.rules.creature.fight_max_love,         var_type(game.conf.rules.creature.fight_max_love        ), SHRT_MIN,  SHRT_MAX},
  {"BODYREMAINSFOR",             &game.conf.rules.creature.body_remains_for,       var_type(game.conf.rules.creature.body_remains_for      ),        0, USHRT_MAX},
  {"FIGHTHATEKILLVALUE",         &game.conf.rules.creature.fight_hate_kill_value,  var_type(game.conf.rules.creature.fight_hate_kill_value ), SHRT_MIN,  SHRT_MAX},
  {"FLEEZONERADIUS",             &game.conf.rules.creature.flee_zone_radius,       var_type(game.conf.rules.creature.flee_zone_radius      ),        0, ULONG_MAX},
  {"GAMETURNSINFLEE",            &game.conf.rules.creature.game_turns_in_flee,     var_type(game.conf.rules.creature.game_turns_in_flee    ),        0,  LONG_MAX},
  {"GAMETURNSUNCONSCIOUS",       &game.conf.rules.creature.game_turns_unconscious, var_type(game.conf.rules.creature.game_turns_unconscious),        0, USHRT_MAX},
  {"STUNEVILENEMYCHANCE",        &game.conf.rules.creature.stun_enemy_chance_evil, var_type(game.conf.rules.creature.stun_enemy_chance_evil),        0,       100},
  {"STUNGOODENEMYCHANCE",        &game.conf.rules.creature.stun_enemy_chance_good, var_type(game.conf.rules.creature.stun_enemy_chance_good),        0,       100},
  {NULL,                         NULL,                                                                                                     0,        0,         0},
};

const struct NamedCommand rules_creature_commands[] = {
  {"CRITICALHEALTHPERCENTAGE",     1},
  {NULL,                           0},
};

const struct NamedField rules_magic_named_fields[] = {
    //name                            //field                                                    //field type                                                        //min     //max
  {"HOLDAUDIENCETIME",               &game.conf.rules.magic.hold_audience_time,                 var_type(game.conf.rules.magic.hold_audience_time                ),        0, LONG_MAX},
  {"ARMAGEDDONTELEPORTYOURTIMEGAP",  &game.conf.rules.magic.armageddon_teleport_your_time_gap,  var_type(game.conf.rules.magic.armageddon_teleport_your_time_gap ), LONG_MIN, LONG_MAX},
  {"ARMAGEDDONTELEPORTENEMYTIMEGAP", &game.conf.rules.magic.armageddon_teleport_enemy_time_gap, var_type(game.conf.rules.magic.armageddon_teleport_enemy_time_gap),        0, LONG_MAX},
  {"ARMAGEDDONTELEPORTNEUTRALS",     &game.conf.rules.magic.armageddon_teleport_neutrals,       var_type(game.conf.rules.magic.armageddon_teleport_neutrals      ),        0,        1},
  {"ARMAGEDDONCOUNTDOWN",            &game.armageddon.count_down,                               var_type(game.armageddon.count_down                              ), LONG_MIN, LONG_MAX},
  {"ARMAGEDDONDURATION",             &game.armageddon.duration,                                 var_type(game.armageddon.duration                                ), LONG_MIN, LONG_MAX},
  {"DISEASETRANSFERPERCENTAGE",      &game.conf.rules.magic.disease_transfer_percentage,        var_type(game.conf.rules.magic.disease_transfer_percentage       ),        0, CHAR_MAX},
  {"DISEASELOSEPERCENTAGEHEALTH",    &game.conf.rules.magic.disease_lose_percentage_health,     var_type(game.conf.rules.magic.disease_lose_percentage_health    ), LONG_MIN, LONG_MAX},
  {"DISEASELOSEHEALTHTIME",          &game.conf.rules.magic.disease_lose_health_time,           var_type(game.conf.rules.magic.disease_lose_health_time          ), LONG_MIN, LONG_MAX},
  {"MINDISTANCEFORTELEPORT",         &game.conf.rules.magic.min_distance_for_teleport,          var_type(game.conf.rules.magic.min_distance_for_teleport         ), LONG_MIN, LONG_MAX},
  {"COLLAPSEDUNGEONDAMAGE",          &game.conf.rules.magic.collapse_dungeon_damage,            var_type(game.conf.rules.magic.collapse_dungeon_damage           ), LONG_MIN, LONG_MAX},
  {"TURNSPERCOLLAPSEDUNGEONDAMAGE",  &game.conf.rules.magic.turns_per_collapse_dngn_dmg,        var_type(game.conf.rules.magic.turns_per_collapse_dngn_dmg       ), LONG_MIN, LONG_MAX},
  {"FRIENDLYFIGHTAREARANGEPERCENT",  &game.conf.rules.magic.friendly_fight_area_range_percent,  var_type(game.conf.rules.magic.friendly_fight_area_range_percent ), LONG_MIN, LONG_MAX},
  {"FRIENDLYFIGHTAREADAMAGEPERCENT", &game.conf.rules.magic.friendly_fight_area_damage_percent, var_type(game.conf.rules.magic.friendly_fight_area_damage_percent), LONG_MIN, LONG_MAX},
  {"WEIGHTCALCULATEPUSH",            &game.conf.rules.magic.weight_calculate_push,              var_type(game.conf.rules.magic.weight_calculate_push             ),        0, SHRT_MAX},
  {NULL,                             NULL,                                                                                                                       0,        0,        0},
};

const struct NamedField rules_rooms_named_fields[] = {
    //name                                 //field                                                  //field type                                                      //min                //max
  {"SCAVENGECOSTFREQUENCY",               &game.conf.rules.rooms.scavenge_cost_frequency,          var_type(game.conf.rules.rooms.scavenge_cost_frequency         ), LONG_MIN,            LONG_MAX},
  {"TEMPLESCAVENGEPROTECTIONTIME",        &game.conf.rules.rooms.temple_scavenge_protection_turns, var_type(game.conf.rules.rooms.temple_scavenge_protection_turns),        0,           ULONG_MAX},
  {"TRAINCOSTFREQUENCY",                  &game.conf.rules.rooms.train_cost_frequency,             var_type(game.conf.rules.rooms.train_cost_frequency            ), LONG_MIN,            LONG_MAX},
  {"TORTURECONVERTCHANCE",                &game.conf.rules.rooms.torture_convert_chance,           var_type(game.conf.rules.rooms.torture_convert_chance          ),        0,                 100},
  {"TIMESPENTINPRISONWITHOUTBREAK",       &game.conf.rules.rooms.time_in_prison_without_break,     var_type(game.conf.rules.rooms.time_in_prison_without_break    ),        0,           ULONG_MAX},
  {"GHOSTCONVERTCHANCE",                  &game.conf.rules.rooms.ghost_convert_chance,             var_type(game.conf.rules.rooms.ghost_convert_chance            ),        0,                 100},
  {"DEFAULTGENERATESPEED",                &game.conf.rules.rooms.default_generate_speed,           var_type(game.conf.rules.rooms.default_generate_speed          ),        0,           USHRT_MAX},
  {"DEFAULTMAXCREATURESGENERATEENTRANCE", &game.conf.rules.rooms.default_max_crtrs_gen_entrance,   var_type(game.conf.rules.rooms.default_max_crtrs_gen_entrance  ),        0,           ULONG_MAX},
  {"FOODGENERATIONSPEED",                 &game.conf.rules.rooms.food_generation_speed,            var_type(game.conf.rules.rooms.food_generation_speed           ), LONG_MIN,            LONG_MAX},
  {"PRISONSKELETONCHANCE",                &game.conf.rules.rooms.prison_skeleton_chance,           var_type(game.conf.rules.rooms.prison_skeleton_chance          ),        0,                 100},
  {"BODIESFORVAMPIRE",                    &game.conf.rules.rooms.bodies_for_vampire,               var_type(game.conf.rules.rooms.bodies_for_vampire              ),        0,           UCHAR_MAX},
  {"GRAVEYARDCONVERTTIME",                &game.conf.rules.rooms.graveyard_convert_time,           var_type(game.conf.rules.rooms.graveyard_convert_time          ),        0,           USHRT_MAX},
  {"SCAVENGEGOODALLOWED",                 &game.conf.rules.rooms.scavenge_good_allowed,            var_type(game.conf.rules.rooms.scavenge_good_allowed           ),        0,                   1},
  {"SCAVENGENEUTRALALLOWED",              &game.conf.rules.rooms.scavenge_neutral_allowed,         var_type(game.conf.rules.rooms.scavenge_neutral_allowed        ),        0,                   1},
  {"TIMEBETWEENPRISONBREAK",              &game.conf.rules.rooms.time_between_prison_break,        var_type(game.conf.rules.rooms.time_between_prison_break       ),        0,           ULONG_MAX},
  {"PRISONBREAKCHANCE",                   &game.conf.rules.rooms.prison_break_chance,              var_type(game.conf.rules.rooms.prison_break_chance             ),        0,           ULONG_MAX},
  {"TORTUREDEATHCHANCE",                  &game.conf.rules.rooms.torture_death_chance,             var_type(game.conf.rules.rooms.torture_death_chance            ),        0,                 100},
  {"BARRACKMAXPARTYSIZE",                 &game.conf.rules.rooms.barrack_max_party_size,           var_type(game.conf.rules.rooms.barrack_max_party_size          ),        0, GROUP_MEMBERS_COUNT},
  {"TRAININGROOMMAXLEVEL",                &game.conf.rules.rooms.training_room_max_level,          var_type(game.conf.rules.rooms.training_room_max_level         ),        0,CREATURE_MAX_LEVEL+1},
  {NULL,                                  NULL,                                                                                                                   0,        0,                   0},
};

const struct NamedField rules_workers_named_fields[] = {
    //name                        //field                                              //field type                                                   //min     //max
  {"HITSPERSLAB",                &game.conf.rules.workers.hits_per_slab,              var_type(game.conf.rules.workers.hits_per_slab              ),        0, UCHAR_MAX},
  {"DEFAULTIMPDIGDAMAGE",        &game.conf.rules.workers.default_imp_dig_damage,     var_type(game.conf.rules.workers.default_imp_dig_damage     ),        0, ULONG_MAX},
  {"DEFAULTIMPDIGOWNDAMAGE",     &game.conf.rules.workers.default_imp_dig_own_damage, var_type(game.conf.rules.workers.default_imp_dig_own_damage ),        0, ULONG_MAX},
  {"IMPWORKEXPERIENCE",          &game.conf.rules.workers.digger_work_experience,     var_type(game.conf.rules.workers.digger_work_experience     ),        0,  LONG_MAX},
  {"DRAGUNCONSCIOUSTOLAIR",      &game.conf.rules.workers.drag_to_lair,               var_type(game.conf.rules.workers.drag_to_lair               ),        0,         2},
  {NULL,                         NULL,                                                                                                            0,        0,         0},
};

const struct NamedField rules_health_named_fields[] = {
    //name                           //field                                                //field type                                                    //min      //max
  {"HUNGERHEALTHLOSS",              &game.conf.rules.health.hunger_health_loss,            var_type(game.conf.rules.health.hunger_health_loss           ), LONG_MIN,  LONG_MAX},
  {"GAMETURNSPERHUNGERHEALTHLOSS",  &game.conf.rules.health.turns_per_hunger_health_loss,  var_type(game.conf.rules.health.turns_per_hunger_health_loss ),        0, USHRT_MAX},
  {"FOODHEALTHGAIN",                &game.conf.rules.health.food_health_gain,              var_type(game.conf.rules.health.food_health_gain             ), LONG_MIN,  LONG_MAX},
  {"TORTUREHEALTHLOSS",             &game.conf.rules.health.torture_health_loss,           var_type(game.conf.rules.health.torture_health_loss          ), LONG_MIN,  LONG_MAX},
  {"GAMETURNSPERTORTUREHEALTHLOSS", &game.conf.rules.health.turns_per_torture_health_loss, var_type(game.conf.rules.health.turns_per_torture_health_loss),        0, USHRT_MAX},
  {NULL,                            NULL,                                                                                                               0,        0,         0},
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
    // Game block.
    game.conf.rules.game.gold_per_gold_block = 1000;
    game.conf.rules.game.pot_of_gold_holds = 1000;
    game.conf.rules.game.gold_pile_value = 500;
    game.conf.rules.game.gold_pile_maximum = 5000;
    game.conf.rules.game.food_life_out_of_hatchery = 100;
    game.conf.rules.game.boulder_reduce_health_slap = 10;
    game.conf.rules.game.boulder_reduce_health_wall = 10;
    game.conf.rules.game.boulder_reduce_health_room = 10;
    game.conf.rules.game.pay_day_gap = 5000;
    game.conf.rules.game.chest_gold_hold = 1000;
    game.conf.rules.game.dungeon_heart_heal_time = 10;
    game.conf.rules.game.dungeon_heart_heal_health = 1;
    game.conf.rules.game.hero_door_wait_time = 100;
    game.conf.rules.game.bag_gold_hold = 200;
    game.conf.rules.game.classic_bugs_flags = ClscBug_None;
    game.conf.rules.game.door_sale_percent = 100;
    game.conf.rules.game.room_sale_percent = 50;
    game.conf.rules.game.trap_sale_percent = 100;
    game.conf.rules.game.gem_effectiveness = 17;
    game.conf.rules.game.pay_day_speed = 100;
    game.conf.rules.game.gold_per_hoard = 2000;
    game.conf.rules.game.torture_payday = 50;
    game.conf.rules.game.torture_training_cost = 100;
    game.conf.rules.game.torture_scavenging_cost = 100;
    game.conf.rules.game.creatures_count = 255;
    game.conf.rules.game.max_paydays_advanced = 5;
    game.conf.rules.game.max_paydays_owed = 5;
    game.conf.rules.game.accept_partial_payday = true;
    game.conf.rules.game.pocket_gold = false;
    game.conf.rules.game.take_pay_from_pocket = false;
    // Creature block.
    game.conf.rules.creature.recovery_frequency = 10;
    game.conf.rules.creature.fight_max_hate = 200;
    game.conf.rules.creature.fight_borderline = 0;
    game.conf.rules.creature.fight_max_love = -100;
    game.conf.rules.creature.body_remains_for = 1000;
    game.conf.rules.creature.fight_hate_kill_value = -5;
    game.conf.rules.creature.flee_zone_radius = 2048;
    game.conf.rules.creature.game_turns_in_flee = 200;
    game.conf.rules.creature.game_turns_unconscious = 2000;
    game.conf.rules.creature.critical_health_permil = 125;
    game.conf.rules.creature.stun_enemy_chance_good = 100;
    game.conf.rules.creature.stun_enemy_chance_evil = 100;
    // Magic block.
    game.conf.rules.magic.hold_audience_time = 500;
    game.conf.rules.magic.armageddon_teleport_your_time_gap = 10;
    game.conf.rules.magic.armageddon_teleport_enemy_time_gap = 10;
    game.armageddon.count_down = 500;
    game.armageddon.duration = 4000;
    game.conf.rules.magic.disease_transfer_percentage = 15;
    game.conf.rules.magic.disease_lose_percentage_health = 8;
    game.conf.rules.magic.disease_lose_health_time = 200;
    game.conf.rules.magic.min_distance_for_teleport = 20;
    game.conf.rules.magic.collapse_dungeon_damage = 10;
    game.conf.rules.magic.turns_per_collapse_dngn_dmg = 4;
    game.conf.rules.magic.weight_calculate_push = 0;
    // Health block.
    game.conf.rules.health.hunger_health_loss = 1;
    game.conf.rules.health.turns_per_hunger_health_loss = 100;
    game.conf.rules.health.food_health_gain = 10;
    game.conf.rules.health.torture_health_loss = 5;
    game.conf.rules.health.turns_per_torture_health_loss = 100;
    // Rooms block.
    game.conf.rules.rooms.scavenge_cost_frequency = 64;
    game.conf.rules.rooms.temple_scavenge_protection_turns = 1000;
    game.conf.rules.rooms.train_cost_frequency = 64;
    game.conf.rules.rooms.ghost_convert_chance = 10;
    game.conf.rules.rooms.default_generate_speed = 350;
    game.conf.rules.rooms.default_max_crtrs_gen_entrance = 200;
    game.conf.rules.rooms.food_generation_speed = 2000;
    game.conf.rules.rooms.prison_skeleton_chance = 100;
    game.conf.rules.rooms.bodies_for_vampire = 6;
    game.conf.rules.rooms.graveyard_convert_time = 300;
    game.conf.rules.rooms.barrack_max_party_size = 10;
    game.conf.rules.rooms.training_room_max_level = 0;
    game.conf.rules.rooms.scavenge_good_allowed = 1;
    game.conf.rules.rooms.scavenge_neutral_allowed = 1;
    game.conf.rules.rooms.time_between_prison_break = 64;
    // Computer block - maybe it should be moved to computer config on a later PR?
    game.conf.rules.computer.disease_to_temple_pct = 500;
    // Workers block.
    game.conf.rules.workers.hits_per_slab = 2;
    game.conf.rules.workers.default_imp_dig_damage = 1;
    game.conf.rules.workers.default_imp_dig_own_damage = 2;
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

static void game_block_special_cases(int cmd_num,const char *buf,long *pos,long len)
{
    #define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_game_commands,cmd_num)
    char word_buf[COMMAND_WORD_LEN];
    switch (cmd_num)
    {
        case 1: // PRESERVECLASSICBUGS
            game.conf.rules.game.classic_bugs_flags = ClscBug_None;
            while (get_conf_parameter_single(buf,pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                int k = get_id(rules_game_classicbugs_commands, word_buf);
                switch (k)
                {
                case 1: // RESURRECT_FOREVER
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_ResurrectForever;
                    break;
                case 2: // OVERFLOW_8BIT
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_Overflow8bitVal;
                    break;
                case 3: // CLAIM_ROOM_ALL_THINGS
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_ClaimRoomAllThings;
                    break;
                case 4: // RESURRECT_REMOVED
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_ResurrectRemoved;
                    break;
                case 5: // NO_HAND_PURGE_ON_DEFEAT
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_NoHandPurgeOnDefeat;
                    break;
                case 6: // MUST_OBEY_KEEPS_NOT_DO_JOBS
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_MustObeyKeepsNotDoJobs;
                    break;
                case 7: // BREAK_NEUTRAL_WALLS
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_BreakNeutralWalls;
                    break;
                case 8: // ALWAYS_TUNNEL_TO_RED
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_AlwaysTunnelToRed;
                    break;
                case 9: // FULLY_HAPPY_WITH_GOLD
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_FullyHappyWithGold;
                    break;
                case 10: // FAINTED_IMMUNE_TO_BOULDER
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_FaintedImmuneToBoulder;
                    break;
                case 11: // REBIRTH_KEEPS_SPELLS
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_RebirthKeepsSpells;
                    break;
                case 12: // STUN_FRIENDLY_UNITS
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_FriendlyFaint;
                    break;
                case 13: // PASSIVE_NEUTRALS
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_PassiveNeutrals;
                    break;
                case 14: // NEUTRAL_TORTURE_CONVERTS
                    game.conf.rules.game.classic_bugs_flags |= ClscBug_NeutralTortureConverts;
                    break;
                default:

                    break;
                }
            }
            break;
    }
#undef COMMAND_TEXT
}

static void creatures_block_special_cases(int cmd_num,const char *buf,long *pos,long len)
{
    char word_buf[COMMAND_WORD_LEN];
    switch (cmd_num)
    {
       case 1: // CRITICALHEALTHPERCENTAGE
          if (get_conf_parameter_single(buf,pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            int k = atoi(word_buf);
            game.conf.rules.creature.critical_health_permil = k*10;
          }
          break;
    }
}

TbBool parse_rules_block(const char *buf, long len, const char *config_textname, unsigned short flags,const char* blockname,
                         const struct NamedField named_field[],const struct NamedCommand *named_command,void (*specialCases)(int cmd_num,const char *bf,long *ps,long ln))
{
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, blockname);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",blockname,config_textname);
        return false;
    }

    while (pos<len)
    {
        // Finding command number in this line.
        int assignresult = assign_conf_command_field(buf, &pos, len, named_field);
        if( assignresult == ccr_ok || assignresult == ccr_comment )
        {
            skip_conf_to_next_line(buf,&pos,len);
            continue;
        }
        else if( assignresult == ccr_unrecognised)
        {
            // Finding command number in this line.
            if (named_command != NULL)
            {
                int cmd_num = recognize_conf_command(buf, &pos, len, named_command);
                specialCases(cmd_num,buf,&pos,len);
            }
            skip_conf_to_next_line(buf,&pos,len);
            continue;
        }
        else if( assignresult == ccr_endOfBlock || assignresult == ccr_error || assignresult == ccr_endOfFile)
        {
            break;
        }
    }
    return true;
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

    parse_rules_block(buf, len, textname, flags,"game",     rules_game_named_fields,     rules_game_commands,    &game_block_special_cases);
    parse_rules_block(buf, len, textname, flags,"creatures",rules_creatures_named_fields,rules_creature_commands,&creatures_block_special_cases);
    parse_rules_block(buf, len, textname, flags,"rooms",    rules_rooms_named_fields,    NULL,                   NULL);
    parse_rules_block(buf, len, textname, flags,"magic",    rules_magic_named_fields,    NULL,                   NULL);
    parse_rules_block(buf, len, textname, flags,"computer", rules_computer_named_fields, NULL,                   NULL);
    parse_rules_block(buf, len, textname, flags,"workers",  rules_workers_named_fields,  NULL,                   NULL);
    parse_rules_block(buf, len, textname, flags,"health",   rules_health_named_fields,   NULL,                   NULL);

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
