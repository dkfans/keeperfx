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
#include "config_rules.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_rules_file[]="rules.cfg";

const struct NamedCommand rules_game_commands[] = {
  {"GOLDPERGOLDBLOCK",            1},
  {"POTOFGOLDHOLDS",              2},
  {"CHESTGOLDHOLD",               3},
  {"GOLDPILEVALUE",               4},
  {"GOLDPILEMAXIMUM",             5},
  {"GOLDPERHOARD",                6},
  {"FOODLIFEOUTOFHATCHERY",       7},
  {"DOUBLECLICKSPEED",            8},
  {"DOOROPENFOR",                 9},
  {"BOULDERREDUCEHEALTHSLAP",    10},
  {"BOULDERREDUCEHEALTHWALL",    11},
  {"BOULDERREDUCEHEALTHROOM",    12},
  {"TILESTRENGTH",               13},
  {"GOLDTILESTRENGTH",           14},
  {"MINIMUMGOLD",                15},
  {"MAXGOLDLOOKUP",              16},
  {"MINGOLDTORECORD",            17},
  {"PAYDAYGAP",                  18},
  {"PAYDAYSPEED",                19},
  {"SLABCOLLAPSETIME",           20},
  {"DUNGEONHEARTHEALTH",         21},
  {"DUNGEONHEARTHEALTIME",       22},
  {"DUNGEONHEARTHEALHEALTH",     23},
  {"HERODOORWAITTIME",           24},
  {"PRESERVECLASSICBUGS",        25},
  {"DEATHMATCHSTATUEREAPPERTIME",26},
  {"DEATHMATCHOBJECTREAPPERTIME",27},
  {"GEMEFFECTIVENESS",           28},
  {"ROOMSELLGOLDBACKPERCENT",    29},
  {"DOORSELLVALUEPERCENT",       30},
  {"TRAPSELLVALUEPERCENT",       31},
  {"PLACETRAPSONSUBTILES",       32},
  {"BAGGOLDHOLD",                33},
  {NULL,                          0},
  };

const struct NamedCommand rules_game_classicbugs_commands[] = {
  {"RESURRECT_FOREVER",           1},
  {"OVERFLOW_8BIT",               2},
  {"CLAIM_ROOM_ALL_THINGS",       3},
  {"RESURRECT_REMOVED",           4},
  {"NO_HAND_PURGE_ON_DEFEAT",     5},
  {"MUST_OBEY_KEEPS_NOT_DO_JOBS", 6},
  {"BREAK_NEUTRAL_WALLS",         7},
  {"ALWAYS_TUNNEL_TO_RED",        8},
  {"FULLY_HAPPY_WITH_GOLD",       9},
  {"FAINTED_IMMUNE_TO_BOULDER",  10},
  {"REBIRTH_KEEPS_SPELLS",       11},
  {"STUN_FRIENDLY_UNITS",        12},
  {NULL,                          0},
  };

const struct NamedCommand rules_computer_commands[] = {
  {"AUTODIGLIMIT",               1},
  {"WAITFORROOMTIME",            2},
  {"CHECKEXPANDTIME",            3},
  {"MAXDISTANCETODIG",           4},
  {"WAITAFTERROOMAREA",          5},
  {"DISEASEHPTEMPLEPERCENTAGE",  6},
  {NULL,                         0},
  };

const struct NamedCommand rules_creatures_commands[] = {
  {"RECOVERYFREQUENCY",             1},
  {"FIGHTMAXHATE",                  2},
  {"FIGHTBORDERLINE",               3},
  {"FIGHTMAXLOVE",                  4},
  {"BODYREMAINSFOR",                5},
  {"FIGHTHATEKILLVALUE",            6},
  {"FLEEZONERADIUS",                7},
  {"GAMETURNSINFLEE",               8},
  {"GAMETURNSUNCONSCIOUS",          9},
  {"CRITICALHEALTHPERCENTAGE",     10},
  {"STUNEVILENEMYCHANCE",          11},
  {"STUNGOODENEMYCHANCE",          12},
  {NULL,                            0},
  };

const struct NamedCommand rules_magic_commands[] = {
  {"HOLDAUDIENCETIME",              1},
  {"ARMAGEDONTELEPORTYOURTIMEGAP",  2},
  {"ARMAGEDONTELEPORTENEMYTIMEGAP", 3},
  {"ARMEGEDDONTELEPORTNEUTRALS",    4},
  {"ARMEGEDDONCOUNTDOWN",           5},
  {"ARMEGEDDONDURATION",            6},
  {"DISEASETRANSFERPERCENTAGE",     7},
  {"DISEASELOSEPERCENTAGEHEALTH",   8},
  {"DISEASELOSEHEALTHTIME",         9},
  {"MINDISTANCEFORTELEPORT",       10},
  {"COLLAPSEDUNGEONDAMAGE",        11},
  {"TURNSPERCOLLAPSEDUNGEONDAMAGE",12},
  {"DEFAULTSACRIFICESCOREFORHORNY",13},
  {"POWERHANDGOLDGRABAMOUNT",      14},
  {"FRIENDLYFIGHTAREARANGEPERCENT",15},
  {"FRIENDLYFIGHTAREADAMAGEPERCENT",16},
  {NULL,                            0},
  };

const struct NamedCommand rules_rooms_commands[] = {
  {"SCAVENGECOSTFREQUENCY",                1},
  {"TEMPLESCAVENGEPROTECTIONTIME",         2},
  {"TRAINCOSTFREQUENCY",                   3},
  {"TORTURECONVERTCHANCE",                 4},
  {"TIMESPENTINPRISONWITHOUTBREAK",        5},
  {"GHOSTCONVERTCHANCE",                   6},
  {"ARMORYTIME",                           7},
  {"WORKSHOPTIME",                         8},
  {"OBSERVATORYTIME",                      9},
  {"OBSERVATORYGENERATE",                 10},
  {"DEFAULTGENERATESPEED",                11},
  {"DEFAULTMAXCREATURESGENERATEENTRANCE", 12},
  {"DEFAULTNEUTRALENTRANCELEVEL",         13},
  {"BARRACKTIME",                         14},
  {"FOODGENERATIONSPEED",                 15},
  {"PRISONSKELETONCHANCE",                16},
  {"BODIESFORVAMPIRE",                    17},
  {"GRAVEYARDCONVERTTIME",                18},
  {"SCAVENGEGOODALLOWED",                 19},
  {"SCAVENGENEUTRALALLOWED",              20},
  {"TIMEBETWEENPRISONBREAK",              21},
  {"PRISONBREAKCHANCE",                   22},
  {"TORTUREDEATHCHANCE",                  23},
  {NULL,                                   0},
  };

const struct NamedCommand rules_workers_commands[] = {
  {"HITSPERSLAB",                1},
  {"IMPJOBNOTSOCLOSE",           2},
  {"IMPJOBFARAWAY",              3},
  {"IMPGENERATETIME",            4},
  {"IMPROVEAREA",                5},
  {"DEFAULTIMPDIGDAMAGE",        6},
  {"DEFAULTIMPDIGOWNDAMAGE",     7},
  {"PERIMPGOLDDIGSIZE",          8},
  {"IMPWORKEXPERIENCE",          9},
  {NULL,                         0},
  };

const struct NamedCommand rules_health_commands[] = {
  {"HUNGERHEALTHLOSS",              1},
  {"GAMETURNSPERHUNGERHEALTHLOSS",  2},
  {"FOODHEALTHGAIN",                3},
  {"PRISONHEALTHGAIN",              4},
  {"GAMETURNSPERPRISONHEALTHGAIN",  5},
  {"TORTUREHEALTHLOSS",             6},
  {"GAMETURNSPERTORTUREHEALTHLOSS", 7},
  {NULL,                            0},
  };

const struct NamedCommand rules_research_commands[] = {
  {"RESEARCH",            1},
  {NULL,                  0},
  };

const struct NamedCommand research_desc[] = {
  {"MAGIC",               1},
  {"ROOM",                2},
  {"CREATURE",            3},
  {NULL,                  0},
  };

const struct NamedCommand rules_sacrifices_commands[] = {
  {"MKCREATURE",          SacA_MkCreature},
  {"MKGOODHERO",          SacA_MkGoodHero},
  {"NEGSPELLALL",         SacA_NegSpellAll},
  {"POSSPELLALL",         SacA_PosSpellAll},
  {"NEGUNIQFUNC",         SacA_NegUniqFunc},
  {"POSUNIQFUNC",         SacA_PosUniqFunc},
  {"CUSTOMREWARD",        SacA_CustomReward},
  {"CUSTOMPUNISH",        SacA_CustomPunish},
  {NULL,                  0},
  };

const struct NamedCommand sacrifice_unique_desc[] = {
  {"ALL_CREATRS_ANGRY",   UnqF_MkAllAngry},
  {"COMPLETE_RESEARCH",   UnqF_ComplResrch},
  {"COMPLETE_MANUFACTR",  UnqF_ComplManufc},
  {"KILL_ALL_CHICKENS",   UnqF_KillChickns},
  {"CHEAPER_IMPS",        UnqF_CheaperImp},
  {"COSTLIER_IMPS",       UnqF_CostlierImp},
  {NULL,                  0},
  };
/******************************************************************************/
/**
 * Returns the first unused sacrifice slot, or invalid slot if no empty one.
 */
struct SacrificeRecipe *get_unused_sacrifice_recipe_slot(void)
{
    for (long i = 1; i < MAX_SACRIFICE_RECIPES; i++)
    {
        struct SacrificeRecipe* sac = &gameadd.sacrifice_recipes[i];
        if (sac->action == SacA_None)
            return sac;
  }
  return &gameadd.sacrifice_recipes[0];
}

/**
 * Clears all sacrifice slots.
 */
void clear_sacrifice_recipes(void)
{
    for (long i = 0; i < MAX_SACRIFICE_RECIPES; i++)
    {
        struct SacrificeRecipe* sac = &gameadd.sacrifice_recipes[i];
        LbMemorySet(sac, '\0', sizeof(struct SacrificeRecipe));
        sac->action = SacA_None;
  }
}

static int long_compare_fn(const void *ptr_a, const void *ptr_b)
{
    long *a = (long*)ptr_a;
    long *b = (long*)ptr_b;
    return *a < *b;
}

TbBool add_sacrifice_victim(struct SacrificeRecipe *sac, long crtr_idx)
{
    // If all slots are taken, then just drop it.
    if (sac->victims[MAX_SACRIFICE_VICTIMS - 1] != 0)
        return false;
    // Otherwise, find place for our item (array is sorted)
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

TbBool parse_rules_game_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        game.gold_per_gold_block = 1000;
        game.pot_of_gold_holds = 1000;
        game.gold_pile_value = 500;
        game.gold_pile_maximum = 5000;
        game.food_life_out_of_hatchery = 100;
        game.boulder_reduce_health_slap = 10;
        game.boulder_reduce_health_wall = 10;
        game.boulder_reduce_health_room = 10;
        game.tile_strength = 50;
        game.gold_tile_strength = 500;
        game.minimum_gold = 100;
        game.max_gold_lookup = 5000;
        game.min_gold_to_record = 10;
        game.pay_day_gap = 5000;
        game.chest_gold_hold = 1000;
        game.dungeon_heart_health = 100;
        gameadd.object_conf.base_config[5].health = 100;
        game.objects_config[5].health = 100;
        game.dungeon_heart_heal_time = 10;
        game.dungeon_heart_heal_health = 1;
        game.hero_door_wait_time = 100;
        gameadd.bag_gold_hold = 200;
        gameadd.classic_bugs_flags = ClscBug_None;
        gameadd.door_sale_percent = 100;
        gameadd.room_sale_percent = 50;
        gameadd.trap_sale_percent = 100;
        gameadd.gem_effectiveness = 17;
        gameadd.pay_day_speed = 100;
        gameadd.place_traps_on_subtiles = false;
        gameadd.gold_per_hoard = 2000;
    }
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "game");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_game_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, rules_game_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case 1: // GOLDPERGOLDBLOCK
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.gold_per_gold_block = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 2: // POTOFGOLDHOLDS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.pot_of_gold_holds = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 3: // CHESTGOLDHOLD
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.chest_gold_hold = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // GOLDPILEVALUE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.gold_pile_value = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // GOLDPILEMAXIMUM
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.gold_pile_maximum = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // GOLDPERHOARD
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              gameadd.gold_per_hoard = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 7: // FOODLIFEOUTOFHATCHERY
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.food_life_out_of_hatchery = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 8: // DOUBLECLICKSPEED
            //Unused
            break;
        case 9: // DOOROPENFOR
            //Unused
            break;
        case 10: // BOULDERREDUCEHEALTHSLAP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.boulder_reduce_health_slap = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 11: // BOULDERREDUCEHEALTHWALL
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.boulder_reduce_health_wall = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 12: // BOULDERREDUCEHEALTHROOM
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.boulder_reduce_health_room = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 13: // TILESTRENGTH
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.tile_strength = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 14: // GOLDTILESTRENGTH
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.gold_tile_strength = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 15: // MINIMUMGOLD
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.minimum_gold = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 16: // MAXGOLDLOOKUP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.max_gold_lookup = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 17: // MINGOLDTORECORD
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.min_gold_to_record = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 18: // PAYDAYGAP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.pay_day_gap = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 19: // PAYDAYSPEED
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.pay_day_speed = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num), block_buf, config_textname);
            }
            break;
        case 20: // SLABCOLLAPSETIME
            //Unused
            break;
        case 21: // DUNGEONHEARTHEALTH
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.dungeon_heart_health = k;
              game.objects_config[5].health = k;
              gameadd.object_conf.base_config[5].health = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 22: // DUNGEONHEARTHEALTIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.dungeon_heart_heal_time = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 23: // DUNGEONHEARTHEALHEALTH
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.dungeon_heart_heal_health = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 24: // HERODOORWAITTIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.hero_door_wait_time = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 25: // PRESERVECLASSICBUGS
            gameadd.classic_bugs_flags = ClscBug_None;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(rules_game_classicbugs_commands, word_buf);
              switch (k)
              {
              case 1: // RESURRECT_FOREVER
                  gameadd.classic_bugs_flags |= ClscBug_ResurrectForever;
                  n++;
                  break;
              case 2: // OVERFLOW_8BIT
                  gameadd.classic_bugs_flags |= ClscBug_Overflow8bitVal;
                  n++;
                  break;
              case 3: // CLAIM_ROOM_ALL_THINGS
                  gameadd.classic_bugs_flags |= ClscBug_ClaimRoomAllThings;
                  n++;
                  break;
              case 4: // RESURRECT_REMOVED
                  gameadd.classic_bugs_flags |= ClscBug_ResurrectRemoved;
                  n++;
                  break;
              case 5: // NO_HAND_PURGE_ON_DEFEAT
                  gameadd.classic_bugs_flags |= ClscBug_NoHandPurgeOnDefeat;
                  n++;
                  break;
              case 6: // MUST_OBEY_KEEPS_NOT_DO_JOBS
                  gameadd.classic_bugs_flags |= ClscBug_MustObeyKeepsNotDoJobs;
                  n++;
                  break;
              case 7: // BREAK_NEUTRAL_WALLS
                  gameadd.classic_bugs_flags |= ClscBug_BreakNeutralWalls;
                  n++;
                  break;
              case 8: // ALWAYS_TUNNEL_TO_RED
                  gameadd.classic_bugs_flags |= ClscBug_AlwaysTunnelToRed;
                  n++;
                  break;
              case 9: // FULLY_HAPPY_WITH_GOLD
                  gameadd.classic_bugs_flags |= ClscBug_FullyHappyWithGold;
                  n++;
                  break;
              case 10: // FAINTED_IMMUNE_TO_BOULDER
                  gameadd.classic_bugs_flags |= ClscBug_FaintedImmuneToBoulder;
                  n++;
                  break;
              case 11: // REBIRTH_KEEPS_SPELLS
                  gameadd.classic_bugs_flags |= ClscBug_RebirthKeepsSpells;
                  n++;
                  break;
              case 12: // STUN_FRIENDLY_UNITS
                  gameadd.classic_bugs_flags |= ClscBug_FriendlyFaint;
                  n++;
                  break;
              default:
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                break;
              }
            }
            break;
        case 26: // DEATHMATCHSTATUEREAPPERTIME
            //Unused
            break;
        case 27: // DEATHMATCHOBJECTREAPPERTIME
            //Unused
            break;
        case 28: // GEMEFFECTIVENESS
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.gem_effectiveness = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num), block_buf, config_textname);
            }
            break;
        case 29: // ROOMSELLGOLDBACKPERCENT
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.room_sale_percent = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num), block_buf, config_textname);
            }
            break;
        case 30: // DOORSELLVALUEPERCENT
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.door_sale_percent = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num), block_buf, config_textname);
            }
            break;
        case 31: // TRAPSELLVALUEPERCENT
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.trap_sale_percent = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num), block_buf, config_textname);
            }
            break;
        case 32: // PLACETRAPSONSUBTILES
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.place_traps_on_subtiles = (TbBool)k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num), block_buf, config_textname);
            }
            break;
        case 33: // BAGGOLDHOLD
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.bag_gold_hold = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num), block_buf, config_textname);
            }
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
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

TbBool parse_rules_computer_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    // Default values
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        game.wait_for_room_time = 800;
        game.check_expand_time = 1000;
        game.max_distance_to_dig = 96;
        game.wait_after_room_area = 200;
        gameadd.disease_to_temple_pct = 500;
    }
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "computer");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_computer_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, rules_computer_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case 1: // AUTODIGLIMIT
            //Unused
            break;
        case 2: // WAITFORROOMTIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.wait_for_room_time = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 3: // CHECKEXPANDTIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.check_expand_time = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // MAXDISTANCETODIG
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.max_distance_to_dig = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // WAITAFTERROOMAREA
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              game.wait_after_room_area = k;
              n++;
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // DISEASEHPTEMPLEPERCENTAGE
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.disease_to_temple_pct = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num), block_buf, config_textname);
            }
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
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

TbBool parse_rules_creatures_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  // Block name and parameter word store variables
  // Default values
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      game.recovery_frequency = 10;
      game.fight_max_hate = 200;
      game.fight_borderline = 0;
      game.fight_max_love = -100;
      game.body_remains_for = 1000;
      game.fight_hate_kill_value = -5;
      gameadd.flee_zone_radius = 2048;
      game.game_turns_in_flee = 200;
      gameadd.game_turns_unconscious = 2000;
      gameadd.critical_health_permil = 125;
      gameadd.stun_enemy_chance_good = 100;
      gameadd.stun_enemy_chance_evil = 100;
  }
  // Find the block
  char block_buf[COMMAND_WORD_LEN];
  sprintf(block_buf, "creatures");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_creatures_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, rules_creatures_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // RECOVERYFREQUENCY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.recovery_frequency = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 2: // FIGHTMAXHATE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.fight_max_hate = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // FIGHTBORDERLINE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.fight_borderline = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // FIGHTMAXLOVE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.fight_max_love = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // BODYREMAINSFOR
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.body_remains_for = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 6: // FIGHTHATEKILLVALUE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.fight_hate_kill_value = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // FLEEZONERADIUS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.flee_zone_radius = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // GAMETURNSINFLEE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.game_turns_in_flee = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 9: // GAMETURNSUNCONSCIOUS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.game_turns_unconscious = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 10: // CRITICALHEALTHPERCENTAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.critical_health_permil = k*10;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 11: // STUNEVILENEMYCHANCE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              gameadd.stun_enemy_chance_evil = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 12: // STUNGOODENEMYCHANCE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              gameadd.stun_enemy_chance_good = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
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

TbBool parse_rules_magic_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  // Block name and parameter word store variables
  // Default values
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      game.hold_audience_time = 500;
      game.armagedon_teleport_your_time_gap = 10;
      game.armagedon_teleport_enemy_time_gap = 10;
      game.armageddon.count_down = 500;
      game.armageddon.duration = 4000;
      game.disease_transfer_percentage = 15;
      game.disease_lose_percentage_health = 8;
      game.disease_lose_health_time = 200;
      game.min_distance_for_teleport = 20;
      game.collapse_dungeon_damage = 10;
      game.turns_per_collapse_dngn_dmg = 4;
      game.power_hand_gold_grab_amount = 100;
  }
  // Find the block
  char block_buf[COMMAND_WORD_LEN];
  sprintf(block_buf, "magic");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_magic_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, rules_magic_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // HOLDAUDIENCETIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.hold_audience_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 2: // ARMAGEDONTELEPORTYOURTIMEGAP
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.armagedon_teleport_your_time_gap = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // ARMAGEDONTELEPORTENEMYTIMEGAP
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.armagedon_teleport_enemy_time_gap = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // ARMEGEDDONTELEPORTNEUTRALS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.armegeddon_teleport_neutrals = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // ARMEGEDDONCOUNTDOWN
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.armageddon.count_down = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 6: // ARMEGEDDONDURATION
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.armageddon.duration = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // DISEASETRANSFERPERCENTAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.disease_transfer_percentage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // DISEASELOSEPERCENTAGEHEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.disease_lose_percentage_health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 9: // DISEASELOSEHEALTHTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.disease_lose_health_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 10: // MINDISTANCEFORTELEPORT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.min_distance_for_teleport = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 11: // COLLAPSEDUNGEONDAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.collapse_dungeon_damage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 12: // TURNSPERCOLLAPSEDUNGEONDAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.turns_per_collapse_dngn_dmg = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 13: // DEFAULTSACRIFICESCOREFORHORNY
          //Unused - scores are computed automatically
          break;
      case 14: // POWERHANDGOLDGRABAMOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.power_hand_gold_grab_amount = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 15: // FRIENDLYFIGHTAREARANGEPERCENT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.friendly_fight_area_range_permil = k * 10;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 16: // FRIENDLYFIGHTAREADAMAGEPERCENT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.friendly_fight_area_damage_permil = k * 10;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
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

TbBool parse_rules_rooms_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  // Block name and parameter word store variables
  // Default values
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      game.scavenge_cost_frequency = 64;
      game.temple_scavenge_protection_turns = 1000;
      game.train_cost_frequency = 64;
      game.ghost_convert_chance = 10;
      game.default_generate_speed = 500;
      game.default_max_crtrs_gen_entrance = 200;
      game.food_generation_speed = 2000;
      game.prison_skeleton_chance = 100;
      game.bodies_for_vampire = 6;
      game.graveyard_convert_time = 300;
      gameadd.scavenge_good_allowed = 1;
      gameadd.scavenge_neutral_allowed = 1;
      gameadd.time_between_prison_break = 64;
  }
  // Find the block
  char block_buf[COMMAND_WORD_LEN];
  sprintf(block_buf, "rooms");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_rooms_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, rules_rooms_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // SCAVENGECOSTFREQUENCY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.scavenge_cost_frequency = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 2: // TEMPLESCAVENGEPROTECTIONTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.temple_scavenge_protection_turns = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // TRAINCOSTFREQUENCY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.train_cost_frequency = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // TORTURECONVERTCHANCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.torture_convert_chance = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 5: // TIMESPENTINPRISONWITHOUTBREAK
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.time_in_prison_without_break = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 6: // GHOSTCONVERTCHANCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.ghost_convert_chance = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // ARMORYTIME
          //Unused
          break;
      case 8: // WORKSHOPTIME
          //Unused
          break;
      case 9: // OBSERVATORYTIME
          //Unused
          break;
      case 10: // OBSERVATORYGENERATE
          //Unused
          break;
      case 11: // DEFAULTGENERATESPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.default_generate_speed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 12: // DEFAULTMAXCREATURESGENERATEENTRANCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.default_max_crtrs_gen_entrance = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 13: // DEFAULTNEUTRALENTRANCELEVEL
          //Unused
          break;
      case 14: // BARRACKTIME
          //Unused
          break;
      case 15: // FOODGENERATIONSPEED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.food_generation_speed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 16: // PRISONSKELETONCHANCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.prison_skeleton_chance = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 17: // BODIESFORVAMPIRE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.bodies_for_vampire = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 18: // GRAVEYARDCONVERTTIME
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.graveyard_convert_time = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 19: // SCAVENGEGOODALLOWED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.scavenge_good_allowed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 20: // SCAVENGENEUTRALALLOWED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.scavenge_neutral_allowed = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 21: // TIMEBETWEENPRISONBREAK
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.time_between_prison_break = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 22: // PRISONBREAKCHANCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            gameadd.prison_break_chance = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 23: // TORTUREDEATHCHANCE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              gameadd.torture_death_chance = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
              cmd_num,block_buf,config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
#undef COMMAND_TEXT
  //SYNCMSG("Prison skeleton chance = %d",game.prison_skeleton_chance);
  return true;
}

TbBool parse_rules_workers_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  // Block name and parameter word store variables
  // Default values
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      game.hits_per_slab = 2;
      game.default_imp_dig_damage = 1;
      game.default_imp_dig_own_damage = 2;
      game.per_imp_gold_dig_size = 30;
  }
  // Find the block
  char block_buf[COMMAND_WORD_LEN];
  sprintf(block_buf, "workers");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_workers_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, rules_workers_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // HITSPERSLAB
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.hits_per_slab = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 2: // IMPJOBNOTSOCLOSE
          //Unused
          break;
      case 3: // IMPJOBFARAWAY
          //Unused
          break;
      case 4: // IMPGENERATETIME
          //Unused
          break;
      case 5: // IMPROVEAREA
          //Unused
          break;
      case 6: // DEFAULTIMPDIGDAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.default_imp_dig_damage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // DEFAULTIMPDIGOWNDAMAGE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.default_imp_dig_own_damage = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 8: // PERIMPGOLDDIGSIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.per_imp_gold_dig_size = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 9: // IMPWORKEXPERIENCE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              gameadd.digger_work_experience = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num), block_buf, config_textname);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
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

TbBool parse_rules_health_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  // Block name and parameter word store variables
  // Default values
  if ((flags & CnfLd_AcceptPartial) == 0)
  {
      game.hunger_health_loss = 1;
      game.turns_per_hunger_health_loss = 100;
      game.food_health_gain = 10;
      game.torture_health_loss = 5;
      game.turns_per_torture_health_loss = 100;
  }
  // Find the block
  char block_buf[COMMAND_WORD_LEN];
  sprintf(block_buf, "health");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_health_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, rules_health_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // HUNGERHEALTHLOSS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.hunger_health_loss = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 2: // GAMETURNSPERHUNGERHEALTHLOSS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.turns_per_hunger_health_loss = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // FOODHEALTHGAIN
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.food_health_gain = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 4: // PRISONHEALTHGAIN
          //Unused
          break;
      case 5: // GAMETURNSPERPRISONHEALTHGAIN
          //Unused
          break;
      case 6: // TORTUREHEALTHLOSS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.torture_health_loss = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 7: // GAMETURNSPERTORTUREHEALTHLOSS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.turns_per_torture_health_loss = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
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
      ERRORMSG("%s(line %lu): " "Unhandled research type, %d", func_name, text_line_number, item_type);
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
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Find the block
  sprintf(block_buf,"research");
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
      if ((flags & CnfLd_AcceptPartial) == 0)
          WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      return false;
  }
  // Clear research list if there's new one in this file
  clear_research_for_all_players();
  // Now we can start with analysis of commands
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_research_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, rules_research_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
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
      case 0: // comment
          break;
      case -1: // end of buffer
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
    gameadd.cheaper_diggers_sacrifice_model = 0;
    for (int i = 1; i < MAX_SACRIFICE_RECIPES; i++)
    {
        struct SacrificeRecipe* sac = &gameadd.sacrifice_recipes[i];
        if (sac->action == SacA_None)
            continue;
        if (((sac->action == SacA_PosUniqFunc) && (sac->param == UnqF_CheaperImp)) 
            || ((sac->action == SacA_NegUniqFunc) && (sac->param == UnqF_CostlierImp)))
        {
            if ((sac->victims[1] == 0) && (gameadd.cheaper_diggers_sacrifice_model == 0)) {
                gameadd.cheaper_diggers_sacrifice_model = sac->victims[0];
            } else {
                WARNLOG("Found unsupported %s sacrifice; either there's more than one, or has one than more victim.",
                    get_conf_parameter_text(sacrifice_unique_desc,UnqF_CheaperImp));
            }
        }
    }
    SYNCDBG(4,"Marked sacrifice of %s",thing_class_and_model_name(TCls_Creature, gameadd.cheaper_diggers_sacrifice_model));
}

TbBool parse_rules_sacrifices_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    int i;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Find the block
    sprintf(block_buf,"sacrifices");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
    // If the block exists, clear previous data
    clear_sacrifice_recipes();
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(rules_sacrifices_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, rules_sacrifices_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
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
            if (sac <= &gameadd.sacrifice_recipes[0])
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
            n++; // delayed increase for first argument
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
            if (sac <= &gameadd.sacrifice_recipes[0])
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
            n++; // delayed increase for first argument
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
            if (sac <= &gameadd.sacrifice_recipes[0])
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
            n++; // delayed increase for first argument
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
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
    if (len > MAX_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" is too large.",textname,fname);
        return false;
    }
    char* buf = (char*)LbMemoryAlloc(len + 256);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file
    if (result)
    {
        result = parse_rules_game_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" game blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_rules_computer_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" computer blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_rules_creatures_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" creatures blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_rules_magic_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" magic blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_rules_rooms_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" rooms blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_rules_workers_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" workers blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_rules_health_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" health blocks failed.",textname,fname);
    }
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
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_rules_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global rules config";
    static const char config_campgn_textname[] = "campaign rules config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_rules_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_rules_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
