/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_creature.c
 *     Creature names, appearance and parameters configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for creatures list.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 03 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_creature.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "globals.h"
#include "config.h"
#include "config_terrain.h"
#include "config_strings.h"
#include "config_crtrstates.h"
#include "thing_doors.h"
#include "thing_creature.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "creature_jobs.h"
#include "engine_arrays.h"
#include "game_legacy.h"
#include "custom_sprites.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_creaturetp_file[]="creature.cfg";

const struct NamedCommand creaturetype_common_commands[] = {
  {"CREATURES",              1},
  {"INSTANCESCOUNT",         2},
  {"JOBSCOUNT",              3},
  {"ANGERJOBSCOUNT",         4},
  {"ATTACKPREFERENCESCOUNT", 5},
  {"SPRITESIZE",             6},
  {NULL,                     0},
  };

const struct NamedCommand creaturetype_experience_commands[] = {
  {"PAYINCREASEONEXP",         1},
  {"SPELLDAMAGEINCREASEONEXP", 2},
  {"RANGEINCREASEONEXP",       3},
  {"JOBVALUEINCREASEONEXP",    4},
  {"HEALTHINCREASEONEXP",      5},
  {"STRENGTHINCREASEONEXP",    6},
  {"DEXTERITYINCREASEONEXP",   7},
  {"DEFENSEINCREASEONEXP",     8},
  {"LOYALTYINCREASEONEXP",     9},
  {"ARMOURINCREASEONEXP",     10},
  {"SIZEINCREASEONEXP",       11},
  {NULL,                       0},
  };

const struct NamedCommand creaturetype_instance_commands[] = {
  {"NAME",            1},
  {"TIME",            2},
  {"ACTIONTIME",      3},
  {"RESETTIME",       4},
  {"FPTIME",          5},
  {"FPACTIONTIME",    6},
  {"FPRESETTIME",     7},
  {"FORCEVISIBILITY", 8},
  {"TOOLTIPTEXTID",   9},
  {"SYMBOLSPRITES",  10},
  {"GRAPHICS",       11},
  {"FUNCTION",       12},
  {"RANGEMIN",       13},
  {"RANGEMAX",       14},
  {"PROPERTIES",     15},
  {"FPINSTANTCAST",  16},
  {NULL,              0},
  };

const struct NamedCommand creaturetype_instance_properties[] = {
  {"REPEAT_TRIGGER",       InstPF_RepeatTrigger},
  {"RANGED_ATTACK",        InstPF_RangedAttack},
  {"MELEE_ATTACK",         InstPF_MeleeAttack},
  {"RANGED_DEBUFF",        InstPF_RangedDebuff},
  {"SELF_BUFF",            InstPF_SelfBuff},
  {"DANGEROUS",            InstPF_Dangerous},
  {"DESTRUCTIVE",          InstPF_Destructive},
  {"QUICK",                InstPF_Quick},
  {NULL,                     0},
  };

const struct NamedCommand creaturetype_job_commands[] = {
  {"NAME",              1},
  {"RELATEDROOMROLE",   2},
  {"RELATEDEVENT",      3},
  {"ASSIGN",            4},
  {"INITIALSTATE",      5},
  {"CONTINUESTATE",     6},
  {"PLAYERFUNCTIONS",   7},
  {"COORDSFUNCTIONS",   8},
  {"PROPERTIES",        9},
  {NULL,                0},
  };

const struct NamedCommand creaturetype_job_assign[] = {
  {"HUMAN_DROP",             JoKF_AssignHumanDrop},
  {"COMPUTER_DROP",          JoKF_AssignComputerDrop},
  {"CREATURE_INIT",          JoKF_AssignCeatureInit},
  {"AREA_WITHIN_ROOM",       JoKF_AssignAreaWithinRoom},
  {"AREA_OUTSIDE_ROOM",      JoKF_AssignAreaOutsideRoom},
  {"BORDER_ONLY",            JoKF_AssignOnAreaBorder},
  {"CENTER_ONLY",            JoKF_AssignOnAreaCenter},
  {"WHOLE_AREA",             JoKF_AssignOnAreaBorder|JoKF_AssignOnAreaCenter},
  {"OWNED_CREATURES",        JoKF_OwnedCreatures},
  {"ENEMY_CREATURES",        JoKF_EnemyCreatures},
  {"OWNED_DIGGERS",          JoKF_OwnedDiggers},
  {"ENEMY_DIGGERS",          JoKF_EnemyDiggers},
  {"ONE_TIME",               JoKF_AssignOneTime},
  {"NEEDS_HAVE_JOB",         JoKF_NeedsHaveJob},
  {NULL,                     0},
  };

const struct NamedCommand creaturetype_job_properties[] = {
  {"WORK_BORDER_ONLY",       JoKF_WorkOnAreaBorder},
  {"WORK_CENTER_ONLY",       JoKF_WorkOnAreaCenter},
  {"WORK_WHOLE_AREA",        JoKF_WorkOnAreaBorder|JoKF_WorkOnAreaCenter},
  {"NEEDS_CAPACITY",         JoKF_NeedsCapacity},
  {"NO_SELF_CONTROL",        JoKF_NoSelfControl},
  {"NO_GROUPS",              JoKF_NoGroups},
  {"ALLOW_CHICKENIZED",      JoKF_AllowChickenized},
  {NULL,                     0},
  };

const struct NamedCommand creaturetype_angerjob_commands[] = {
  {"NAME",            1},
  {NULL,              0},
  };

const struct NamedCommand creaturetype_attackpref_commands[] = {
  {"NAME",            1},
  {NULL,              0},
  };

const struct NamedCommand creature_graphics_desc[] = {
  {"STAND",             1+CGI_Stand},
  {"AMBULATE",          1+CGI_Ambulate},
  {"DRAG",              1+CGI_Drag},
  {"ATTACK",            1+CGI_Attack},
  {"DIG",               1+CGI_Dig},
  {"SMOKE",             1+CGI_Smoke},
  {"RELAX",             1+CGI_Relax},
  {"PRETTYDANCE",       1+CGI_PrettyDance},
  {"GOTHIT",            1+CGI_GotHit},
  {"POWERGRAB",         1+CGI_PowerGrab},
  {"GOTSLAPPED",        1+CGI_GotSlapped},
  {"CELEBRATE",         1+CGI_Celebrate},
  {"SLEEP",             1+CGI_Sleep},
  {"EATCHICKEN",        1+CGI_EatChicken},
  {"TORTURE",           1+CGI_Torture},
  {"SCREAM",            1+CGI_Scream},
  {"DROPDEAD",          1+CGI_DropDead},
  {"DEADSPLAT",         1+CGI_DeadSplat},
// These below seems to be not from JTY file
  {"GFX18",             1+CGI_GFX18},
  {"QUERYSYMBOL",       1+CGI_QuerySymbol},
  {"HANDSYMBOL",        1+CGI_HandSymbol},
  {"GFX21",             1+CGI_GFX21},
  {NULL,                 0},
  };

struct CreatureData creature_data[] = {
  {0x00,  0, GUIStr_Empty},
  {0x05, 57, 277},
  {0x01, 58, 275},
  {0x01, 59, 285},
  {0x01, 60, 286},
  {0x01, 61, 279},
  {0x01, 62, 276},
  {0x01, 63, 547},
  {0x01, 64, 546},
  {0x05, 65, 283},
  {0x01, 66, 284},
  {0x01, 67, 258},
  {0x01, 68, 281},
  {0x01, 69, 282},
  {0x01, 70, 267},
  {0x01, 71, 266},
  {0x01, 72, 261},
  {0x15, 73, 268},
  {0x02, 74, 262},
  {0x02, 75, 264},
  {0x02, 76, 272},
  {0x02, 77, 263},
  {0x02, 78, 273},
  {0x02, 79, 259},
  {0x02, 80, 260},
  {0x02, 81, 274},
  {0x02, 82, 265},
  {0x02, 83, 270},
  {0x02, 84, 271},
  {0x02, 85, 269},
  {0x01,126, 278},
  {0x00,  0, GUIStr_Empty},
  };
/******************************************************************************/
struct NamedCommand creature_desc[CREATURE_TYPES_MAX];
struct NamedCommand instance_desc[INSTANCE_TYPES_MAX];
struct NamedCommand creaturejob_desc[INSTANCE_TYPES_MAX];
struct NamedCommand angerjob_desc[INSTANCE_TYPES_MAX];
struct NamedCommand attackpref_desc[INSTANCE_TYPES_MAX];
/******************************************************************************/
extern const struct NamedCommand creature_job_player_assign_func_type[];
extern Creature_Job_Player_Check_Func creature_job_player_check_func_list[];
extern const struct NamedCommand creature_job_player_check_func_type[];
extern Creature_Job_Player_Assign_Func creature_job_player_assign_func_list[];
extern const struct NamedCommand creature_job_coords_check_func_type[];
extern Creature_Job_Coords_Check_Func creature_job_coords_check_func_list[];
extern const struct NamedCommand creature_job_coords_assign_func_type[];
extern Creature_Job_Coords_Assign_Func creature_job_coords_assign_func_list[];

const struct NamedCommand mevents_desc[] = {
    {"MEVENT_NOTHING",         EvKind_Nothing},
    {"MEVENT_HEARTATTACKED",   EvKind_HeartAttacked},
    {"MEVENT_ENEMYFIGHT",      EvKind_EnemyFight},
    {"MEVENT_OBJECTIVE",       EvKind_Objective},
    {"MEVENT_BREACH",          EvKind_Breach},
    {"MEVENT_NEWROOMRESRCH",   EvKind_NewRoomResrch},
    {"MEVENT_NEWCREATURE",     EvKind_NewCreature},
    {"MEVENT_NEWSPELLRESRCH",  EvKind_NewSpellResrch},
    {"MEVENT_NEWTRAP",         EvKind_NewTrap},
    {"MEVENT_NEWDOOR",         EvKind_NewDoor},
    {"MEVENT_CREATRSCAVENGED", EvKind_CreatrScavenged},
    {"MEVENT_TREASUREROOMFULL",EvKind_TreasureRoomFull},
    {"MEVENT_CREATUREPAYDAY",  EvKind_CreaturePayday},
    {"MEVENT_AREADISCOVERED",  EvKind_AreaDiscovered},
    {"MEVENT_SPELLPICKEDUP",   EvKind_SpellPickedUp},
    {"MEVENT_ROOMTAKENOVER",   EvKind_RoomTakenOver},
    {"MEVENT_CREATRISANNOYED", EvKind_CreatrIsAnnoyed},
    {"MEVENT_NOMORELIVINGSET", EvKind_NoMoreLivingSet},
    {"MEVENT_ALARMTRIGGERED",  EvKind_AlarmTriggered},
    {"MEVENT_ROOMUNDERATTACK", EvKind_RoomUnderAttack},
    {"MEVENT_NEEDTREASUREROOM",EvKind_NeedTreasureRoom},
    {"MEVENT_INFORMATION",     EvKind_Information},
    {"MEVENT_ROOMLOST",        EvKind_RoomLost},
    {"MEVENT_CREATRHUNGRY",    EvKind_CreatrHungry},
    {"MEVENT_TRAPCRATEFOUND",  EvKind_TrapCrateFound},
    {"MEVENT_DOORCRATEFOUND",  EvKind_DoorCrateFound},
    {"MEVENT_DNSPECIALFOUND",  EvKind_DnSpecialFound},
    {"MEVENT_QUICKINFORMATION",EvKind_QuickInformation},
    {"MEVENT_FRIENDLYFIGHT",   EvKind_FriendlyFight},
    {"MEVENT_WORKROMUNREACHBL",EvKind_WorkRoomUnreachable},
    {"MEVENT_STRGROMUNREACHBL",EvKind_StorageRoomUnreachable},
    {NULL,                    0},
};

const char *name_starts[] = {
    "B", "C", "D", "F",
    "G", "H", "J", "K",
    "L", "M", "N", "P",
    "R", "S", "T", "V",
    "Y", "Z", "Ch",
    "Sh", "Al", "Th",
};

const char *name_vowels[] = {
    "a",  "e",  "i", "o",
    "u",  "ee", "oo",
    "oa", "ai", "ea",
};

const char *name_consonants[] = {
    "b", "c", "d", "f",
    "g", "h", "j", "k",
    "l", "m", "n", "p",
    "r", "s", "t", "v",
    "y", "z", "ch", "sh"
};

/******************************************************************************/
/**
 * Returns CreatureStats of given creature model.
 */
struct CreatureStats *creature_stats_get(ThingModel crstat_idx)
{
  if ((crstat_idx < 1) || (crstat_idx >= CREATURE_TYPES_MAX))
    return &gameadd.creature_stats[0];
  return &gameadd.creature_stats[crstat_idx];
}

/**
 * Returns CreatureStats assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureStats *creature_stats_get_from_thing(const struct Thing *thing)
{
  if ((thing->model < 1) || (thing->model >= gameadd.crtr_conf.model_count))
    return &gameadd.creature_stats[0];
  return &gameadd.creature_stats[thing->model];
}

/**
 * Returns if given CreatureStats pointer is incorrect.
 */
TbBool creature_stats_invalid(const struct CreatureStats *crstat)
{
  return (crstat <= &gameadd.creature_stats[0]) || (crstat == NULL);
}

/**
 * Informs the module that creature stats for given creature model were changed.
 * To be removed as soon as it's no longer needed!
 */
void creature_stats_updated(ThingModel crstat_idx)
{
    struct CreatureStats* crstat = creature_stats_get(crstat_idx);
    // Update old stats by copying part of the new stats memory
    // Note that CreatureStats may only change at end for this hack to work!
    memcpy(&game.creature_stats_OLD[crstat_idx],crstat,sizeof(struct CreatureStatsOLD));
}

void check_and_auto_fix_stats(void)
{
    SYNCDBG(8,"Starting for %d models",(int)gameadd.crtr_conf.model_count);
    //_DK_check_and_auto_fix_stats();
    for (long model = 0; model < gameadd.crtr_conf.model_count; model++)
    {
        struct CreatureStats* crstat = creature_stats_get(model);
        if ( (crstat->lair_size <= 0) && (crstat->heal_requirement != 0) )
        {
            ERRORLOG("Creature model %d No LairSize But Heal Requirment - Fixing", (int)model);
            crstat->heal_threshold = 0;
            crstat->heal_requirement = 0;
        }
        if (crstat->heal_requirement > crstat->heal_threshold)
        {
            ERRORLOG("Creature model %d Heal Requirment > Heal Threshold - Fixing", (int)model);
            crstat->heal_threshold = crstat->heal_requirement;
        }
        if ( (crstat->hunger_rate != 0) && (crstat->hunger_fill == 0) )
        {
            ERRORLOG("Creature model %d HungerRate > 0 & Hunger Fill = 0 - Fixing", (int)model);
            crstat->hunger_fill = 1;
        }
        if ( (crstat->sleep_exp_slab != 0) && (crstat->sleep_experience == 0) )
        {
            ERRORLOG("Creature model %d SleepSlab set but SleepExperience = 0 - Fixing", (int)model);
            crstat->sleep_exp_slab = 0;
        }
        if (crstat->grow_up >= gameadd.crtr_conf.model_count)
        {
            ERRORLOG("Creature model %d Invalid GrowUp model - Fixing", (int)model);
            crstat->grow_up = 0;
        }
        if (crstat->grow_up > 0)
        {
          if ( (crstat->grow_up_level < 1) || (crstat->grow_up_level > CREATURE_MAX_LEVEL) )
          {
              ERRORLOG("Creature model %d GrowUp & GrowUpLevel invalid - Fixing", (int)model);
              crstat->grow_up_level = 1;
          }
        }
        if (crstat->rebirth > CREATURE_MAX_LEVEL)
        {
            ERRORLOG("Creature model %d Rebirth Invalid - Fixing", (int)model);
            crstat->rebirth = 0;
        }
        for (long i = 0; i < LEARNED_INSTANCES_COUNT; i++)
        {
            long n = crstat->learned_instance_level[i];
            if (crstat->learned_instance_id[i] != CrInst_NULL)
            {
                if ((n < 1) || (n > CREATURE_MAX_LEVEL))
                {
                    ERRORLOG("Creature model %d Learn Level for Instance slot %d Invalid - Fixing", (int)model, (int)(i+1));
                    crstat->learned_instance_level[i] = 1;
                }
            } else
            {
                if (n != 0)
                {
                    ERRORLOG("Creature model %d Learn Level for Empty Instance slot %d - Fixing", (int)model, (int)(i+1));
                    crstat->learned_instance_level[i] = 0;
                }
            }
        }
        creature_stats_updated(model);
    }
    SYNCDBG(9,"Finished");
}

/**
 * Returns CreatureData of given creature model.
 */
struct CreatureData *creature_data_get(ThingModel crstat_idx)
{
  if ((crstat_idx < 1) || (crstat_idx >= CREATURE_TYPES_COUNT))
    return &creature_data[0];
  return &creature_data[crstat_idx];
}

/**
 * Returns CreatureData assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureData *creature_data_get_from_thing(const struct Thing *thing)
{
  if ((thing->model < 1) || (thing->model >= CREATURE_TYPES_COUNT))
    return &creature_data[0];
  return &creature_data[thing->model];
}

TbBool is_creature_model_wildcard(ThingModel crmodel)
{
    if((crmodel == CREATURE_ANY) || (crmodel == CREATURE_NOT_A_DIGGER) || (crmodel == CREATURE_DIGGER))
    {
        return true;
    }
    return false;
}

/**
 * Returns Code Name (name to use in script file) of given creature model.
 */
const char *creature_code_name(ThingModel crmodel)
{
    const char* name = get_conf_parameter_text(creature_desc, crmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the creature associated with a given model name.
 * Linear lookup time so don't use in tight loop.
 * @param name
 * @return
 */
long creature_model_id(const char * name)
{
    for (int i = 0; i < gameadd.crtr_conf.model_count; ++i)
    {
        if (strncmp(name, gameadd.crtr_conf.model[i].name, COMMAND_WORD_LEN) == 0) {
            return i + 1;
        }
    }

    return -1;
}

TbBool parse_creaturetypes_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        gameadd.crtr_conf.model_count = 1;
        gameadd.crtr_conf.instances_count = 1;
        gameadd.crtr_conf.jobs_count = 1;
        gameadd.crtr_conf.angerjobs_count = 1;
        gameadd.crtr_conf.attacktypes_count = 1;
        gameadd.crtr_conf.special_digger_good = 0;
        gameadd.crtr_conf.special_digger_evil = 0;
        gameadd.crtr_conf.spectator_breed = 0;
        gameadd.crtr_conf.sprite_size = 300;
    }
    int k = sizeof(gameadd.crtr_conf.model) / sizeof(gameadd.crtr_conf.model[0]);
    for (int i = 0; i < k; i++)
    {
      LbMemorySet(gameadd.crtr_conf.model[i].name, 0, COMMAND_WORD_LEN);
    }
    LbStringCopy(gameadd.crtr_conf.model[0].name, "NOCREATURE", COMMAND_WORD_LEN);
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "common");
    long pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creaturetype_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creaturetype_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case 1: // CREATURES
            while (get_conf_parameter_single(buf,&pos,len,gameadd.crtr_conf.model[n+1].name,COMMAND_WORD_LEN) > 0)
            {
              creature_desc[n].name = gameadd.crtr_conf.model[n+1].name;
              creature_desc[n].num = n+1;
              n++;
              if (n+1 >= CREATURE_TYPES_MAX)
              {
                CONFWRNLOG("Too many species defined with \"%s\" in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
                break;
              }
            }
            if (n+1 > CREATURE_TYPES_COUNT)
            {
              CONFWRNLOG("Hard-coded limit exceeded by amount of species defined with \"%s\" in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            gameadd.crtr_conf.model_count = n+1;
            while (n < CREATURE_TYPES_MAX)
            {
              creature_desc[n].name = NULL;
              creature_desc[n].num = 0;
              n++;
            }
            break;
        case 2: // INSTANCESCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
              {
                gameadd.crtr_conf.instances_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 3: // JOBSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
              {
                gameadd.crtr_conf.jobs_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // ANGERJOBSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
              {
                gameadd.crtr_conf.angerjobs_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // ATTACKPREFERENCESCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= INSTANCE_TYPES_MAX))
              {
                gameadd.crtr_conf.attacktypes_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // SPRITESIZE
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if ((k > 0) && (k <= 1024))
                {
                    gameadd.crtr_conf.sprite_size = k;
                    n++;
                }
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
    if (gameadd.crtr_conf.model_count < 1)
    {
        WARNLOG("No creature species defined in [%s] block of %s file.",
            block_buf,config_textname);
    }
    return true;
}

TbBool parse_creaturetype_experience_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        gameadd.crtr_conf.exp.size_increase_on_exp = 0;
        gameadd.crtr_conf.exp.pay_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        gameadd.crtr_conf.exp.spell_damage_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        gameadd.crtr_conf.exp.range_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        gameadd.crtr_conf.exp.job_value_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        gameadd.crtr_conf.exp.health_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        gameadd.crtr_conf.exp.strength_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        gameadd.crtr_conf.exp.dexterity_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        gameadd.crtr_conf.exp.defense_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        gameadd.crtr_conf.exp.loyalty_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        gameadd.crtr_conf.exp.armour_increase_on_exp = 0;
    }
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "experience");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creaturetype_experience_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creaturetype_experience_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case 1: // PAYINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.pay_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 2: // SPELLDAMAGEINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.spell_damage_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 3: // RANGEINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.range_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // JOBVALUEINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.job_value_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // HEALTHINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.health_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // STRENGTHINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.strength_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 7: // DEXTERITYINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.dexterity_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 8: // DEFENSEINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.defense_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 9: // LOYALTYINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.loyalty_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 10: // ARMOURINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.armour_increase_on_exp = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 11: // SIZEINCREASEONEXP
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                gameadd.crtr_conf.exp.size_increase_on_exp = k;
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
    if (gameadd.crtr_conf.model_count < 1)
    {
        WARNLOG("No creature species defined in [%s] block of %s file.",
            block_buf,config_textname);
    }
    return true;
}

TbBool parse_creaturetype_instance_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    int i;
    // Block name and parameter word store variables
    int arr_size;
    // Initialize the array
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(gameadd.crtr_conf.instances)/sizeof(gameadd.crtr_conf.instances[0]);
        for (i=0; i < arr_size; i++)
        {
            LbMemorySet(gameadd.crtr_conf.instances[i].name, 0, COMMAND_WORD_LEN);
            if (i < gameadd.crtr_conf.instances_count)
            {
                instance_desc[i].name = gameadd.crtr_conf.instances[i].name;
                instance_desc[i].num = i;
            } else
            {
                instance_desc[i].name = NULL;
                instance_desc[i].num = 0;
            }
        }
    }
    arr_size = gameadd.crtr_conf.instances_count;
    // Load the file blocks
    for (i=0; i < arr_size; i++)
    {
        char block_buf[COMMAND_WORD_LEN];
        sprintf(block_buf, "instance%d", i);
        long pos = 0;
        int k = find_conf_block(buf, &pos, len, block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0)
            {
                WARNMSG("Block [%s] not found in %s file.", block_buf, config_textname);
                return false;
            }
            continue;
      }
      struct InstanceInfo* inst_inf = creature_instance_info_get(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creaturetype_instance_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, creaturetype_instance_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        if ((flags & CnfLd_ListOnly) != 0) {
            // In "List only" mode, accept only name command
            if (cmd_num > 1) {
                cmd_num = 0;
            }
        }
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case 1: // NAME
            if (get_conf_parameter_single(buf,&pos,len,gameadd.crtr_conf.instances[i].name,COMMAND_WORD_LEN) <= 0)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
                break;
            }
            n++;
            break;
        case 2: // TIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              inst_inf->time = k;
              n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 3: // ACTIONTIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              inst_inf->action_time = k;
              n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // RESETTIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              inst_inf->reset_time = k;
              n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // FPTIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              inst_inf->fp_time = k;
              n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // FPACTIONTIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              inst_inf->fp_action_time = k;
              n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 7: // FPRESETTIME
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              inst_inf->fp_reset_time = k;
              n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 8: // FORCEVISIBILITY
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              inst_inf->force_visibility = k;
              n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 9: // TOOLTIPTEXTID
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
              {
                  //TODO CONFIG Add when InstanceInfo can be changed
                  //inst_inf->tooltip_stridx = k;
                  instance_button_init[i].tooltip_stridx = k;
                  n++;
              }
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 10: // SYMBOLSPRITES
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_icon_id(word_buf);
              if (k >= 0)
              {
                  //TODO CONFIG Add when InstanceInfo can be changed
                  //inst_inf->symbol_spridx = k;
                  instance_button_init[i].symbol_spridx = k;
                  n++;
              }
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 11: // GRAPHICS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = get_id(creature_graphics_desc, word_buf);
                if (k > 0)
                {
                    inst_inf->graphics_idx = k-1;
                    n++;
                }
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 12: // FUNCTION
            k = recognize_conf_parameter(buf,&pos,len,creature_instances_func_type);
            if (k > 0)
            {
                inst_inf->func_cb = creature_instances_func_list[k];
                n++;
                //JUSTLOG("Function = %s %s %d",creature_instances_func_type[k-1].name,spell_code_name(inst_inf->func_params[0]),inst_inf->func_params[1]);
            }
            // Second parameter may be a different thing based on first parameter
            switch (k)
            {
            case 2: // Special code for casting spell instances
                k = recognize_conf_parameter(buf,&pos,len,spell_desc);
                if (k > 0)
                {
                    inst_inf->func_params[0] = k;
                    n++;
                }
                break;
            case 3: // Special code for firing shot instances
                k = recognize_conf_parameter(buf,&pos,len,shot_desc);
                if (k > 0)
                {
                    inst_inf->func_params[0] = k;
                    n++;
                }
                break;
            default:
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = atoi(word_buf);
                    inst_inf->func_params[0] = k;
                    n++;
                }
            }
            // Third parameter is always integer
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                inst_inf->func_params[1] = k;
                n++;
            }
            if (n < 3)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 13: //RANGEMIN
                 if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              int j = 0;
              int l = 0;
              for (j=0; j < 23; j++) // Size of offensive_weapon
              {
                  if (offensive_weapon[j].inst_id == i)
                  {
                      l = 1;
                      break;
                  }
              }
              if (l == 1)
              {
                  k = atoi(word_buf);
                  offensive_weapon[j].range_min = k;
                  n++;
              }
          }
          if (n < 1)
          {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
                   break;
        case 14: //RANGEMAX
                 if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              int j = 0;
              int l = 0;
              for (j=0; j < 23; j++) // Size of offensive_weapon
              {
                  if (offensive_weapon[j].inst_id == i)
                  {
                      l = 1;
                      break;
                  }
              }
              if (l == 1)
              {
                  k = atoi(word_buf);
                  offensive_weapon[j].range_max = k;
                  n++;
              }
          }
          if (n < 1)
          {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
                   break;
        case 15: // PROPERTIES
            inst_inf->flags = 0;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = get_id(creaturetype_instance_properties, word_buf);
                if (k > 0)
                {
                    inst_inf->flags |= k;
                  n++;
                } else {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                    break;
                }
            }
            break;
        case 16: // FPINSTANTCAST
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              inst_inf->instant = (TbBool)k;
              n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
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
    }
    return true;
}

TbBool parse_creaturetype_job_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct CreatureJobConfig *jobcfg;
    int i;
    // Block name and parameter word store variables
    int arr_size;
    // Initialize the array
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(gameadd.crtr_conf.jobs)/sizeof(gameadd.crtr_conf.jobs[0]);
        for (i=0; i < arr_size; i++)
        {
            jobcfg = &gameadd.crtr_conf.jobs[i];
            LbMemorySet(jobcfg->name, 0, COMMAND_WORD_LEN);
            jobcfg->room_role = RoRoF_None;
            jobcfg->initial_crstate = CrSt_Unused;
            jobcfg->continue_crstate = CrSt_Unused;
            jobcfg->job_flags = 0;
            jobcfg->func_plyr_check = NULL;
            jobcfg->func_plyr_assign = NULL;
            jobcfg->func_cord_check = NULL;
            jobcfg->func_cord_assign = NULL;
            if (i < gameadd.crtr_conf.jobs_count)
            {
                creaturejob_desc[i].name = gameadd.crtr_conf.jobs[i].name;
                if (i > 0)
                    creaturejob_desc[i].num = (1 << (i-1));
                else
                    creaturejob_desc[i].num = 0;
            } else
            {
                creaturejob_desc[i].name = NULL;
                creaturejob_desc[i].num = 0;
            }
        }
    }
    arr_size = gameadd.crtr_conf.jobs_count;
    // Load the file blocks
    for (i=0; i < arr_size; i++)
    {
        char block_buf[COMMAND_WORD_LEN];
        sprintf(block_buf, "job%d", i);
        long pos = 0;
        int k = find_conf_block(buf, &pos, len, block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0) {
                WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
                return false;
            }
            continue;
        }
        jobcfg = &gameadd.crtr_conf.jobs[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creaturetype_job_commands,cmd_num)
        while (pos<len)
        {
            // Finding command number in this line
            int cmd_num = recognize_conf_command(buf, &pos, len, creaturetype_job_commands);
            // Now store the config item in correct place
            if (cmd_num == -3) break; // if next block starts
            if ((flags & CnfLd_ListOnly) != 0) {
                // In "List only" mode, accept only name command
                if (cmd_num > 1) {
                    cmd_num = 0;
                }
            }
            int n = 0;
            char word_buf[COMMAND_WORD_LEN];
            switch (cmd_num)
            {
            case 1: // NAME
                if (get_conf_parameter_single(buf,&pos,len,gameadd.crtr_conf.jobs[i].name,COMMAND_WORD_LEN) <= 0)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                n++;
                break;
            case 2: // RELATEDROOMROLE
                jobcfg->room_role = RoRoF_None;
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = get_id(room_roles_desc, word_buf);
                    if (k >= 0)
                    {
                        jobcfg->room_role = k;
                        n++;
                    } else
                    {
                        if (strcasecmp(word_buf,"NULL") == 0)
                            n++;
                    }
                }
                if (n < 1)
                {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),block_buf,config_textname);
                }
                break;
            case 3: // RELATEDEVENT
                jobcfg->event_kind = 0;
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = get_id(mevents_desc, word_buf);
                    if (k >= 0)
                    {
                        jobcfg->event_kind = k;
                        n++;
                    }
                }
                if (n < 1)
                {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),block_buf,config_textname);
                }
                break;
            case 4: // ASSIGN
                jobcfg->job_flags &= ~(JoKF_AssignHumanDrop|JoKF_AssignComputerDrop|JoKF_AssignCeatureInit|
                    JoKF_AssignAreaWithinRoom|JoKF_AssignAreaOutsideRoom|JoKF_AssignOnAreaBorder|JoKF_AssignOnAreaCenter|
                    JoKF_OwnedCreatures|JoKF_EnemyCreatures|JoKF_OwnedDiggers|JoKF_EnemyDiggers|
                    JoKF_AssignOneTime|JoKF_NeedsHaveJob);
                while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = get_id(creaturetype_job_assign, word_buf);
                    if (k > 0)
                    {
                        jobcfg->job_flags |= k;
                      n++;
                    } else {
                        CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                            COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                        break;
                    }
                }
                break;
            case 5: // INITIALSTATE
                jobcfg->initial_crstate = CrSt_Unused;
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = get_id(creatrstate_desc, word_buf);
                    if (k >= 0)
                    {
                        jobcfg->initial_crstate = k;
                        n++;
                    } else
                    {
                        if (strcasecmp(word_buf,"NONE") == 0)
                            n++;
                    }
                }
                if (n < 1)
                {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),block_buf,config_textname);
                }
                break;
            case 6: // CONTINUESTATE
                jobcfg->continue_crstate = CrSt_Unused;
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = get_id(creatrstate_desc, word_buf);
                    if (k >= 0)
                    {
                        jobcfg->continue_crstate = k;
                        n++;
                    } else
                    {
                        if (strcasecmp(word_buf,"NONE") == 0)
                            n++;
                    }
                }
                if (n < 1)
                {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),block_buf,config_textname);
                }
                break;
            case 7: // PLAYERFUNCTIONS
                jobcfg->func_plyr_check = NULL;
                jobcfg->func_plyr_assign = NULL;
                k = recognize_conf_parameter(buf,&pos,len,creature_job_player_check_func_type);
                if (k > 0)
                {
                    jobcfg->func_plyr_check = creature_job_player_check_func_list[k];
                    n++;
                }
                k = recognize_conf_parameter(buf,&pos,len,creature_job_player_assign_func_type);
                if (k > 0)
                {
                    jobcfg->func_plyr_assign = creature_job_player_assign_func_list[k];
                    n++;
                }
                if (n < 2)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                }
                break;
            case 8: // COORDSFUNCTIONS
                jobcfg->func_cord_check = NULL;
                jobcfg->func_cord_assign = NULL;
                k = recognize_conf_parameter(buf,&pos,len,creature_job_coords_check_func_type);
                if (k > 0)
                {
                    jobcfg->func_cord_check = creature_job_coords_check_func_list[k];
                    n++;
                }
                k = recognize_conf_parameter(buf,&pos,len,creature_job_coords_assign_func_type);
                if (k > 0)
                {
                    jobcfg->func_cord_assign = creature_job_coords_assign_func_list[k];
                    n++;
                }
                if (n < 2)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                }
                break;
            case 9: // PROPERTIES
                jobcfg->job_flags &= ~(JoKF_WorkOnAreaBorder|JoKF_WorkOnAreaCenter|JoKF_NeedsCapacity|JoKF_NoSelfControl|JoKF_NoGroups|JoKF_AllowChickenized);
                while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = get_id(creaturetype_job_properties, word_buf);
                    if (k > 0)
                    {
                        jobcfg->job_flags |= k;
                      n++;
                    } else {
                        CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                            COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                        break;
                    }
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
        if (((jobcfg->job_flags & JoKF_NeedsHaveJob) != 0) && ((jobcfg->job_flags & JoKF_AssignOneTime) != 0))
        {
            WARNLOG("Job configured to need to have worker primary or secondary job set, but is one time job which cannot; in [%s] block of %s file.",block_buf,config_textname);
        }
#undef COMMAND_TEXT
    }
    return true;
}

TbBool parse_creaturetype_angerjob_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct CreatureAngerJobConfig *agjobcfg;
    int i;
    // Block name and parameter word store variables
    int arr_size;
    // Initialize the array
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(gameadd.crtr_conf.angerjobs)/sizeof(gameadd.crtr_conf.angerjobs[0]);
        for (i=0; i < arr_size; i++)
        {
            agjobcfg = &gameadd.crtr_conf.angerjobs[i];
            LbMemorySet(agjobcfg->name, 0, COMMAND_WORD_LEN);
            if (i < gameadd.crtr_conf.angerjobs_count)
            {
                angerjob_desc[i].name = gameadd.crtr_conf.angerjobs[i].name;
                if (i > 0)
                    angerjob_desc[i].num = (1 << (i-1));
                else
                    angerjob_desc[i].num = 0;
            } else
            {
                angerjob_desc[i].name = NULL;
                angerjob_desc[i].num = 0;
            }
        }
    }
    arr_size = gameadd.crtr_conf.angerjobs_count;
    // Load the file blocks
    for (i=0; i < arr_size; i++)
    {
        char block_buf[COMMAND_WORD_LEN];
        sprintf(block_buf, "angerjob%d", i);
        long pos = 0;
        int k = find_conf_block(buf, &pos, len, block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0) {
                WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
                return false;
            }
            continue;
        }
        agjobcfg = &gameadd.crtr_conf.angerjobs[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creaturetype_angerjob_commands,cmd_num)
        while (pos<len)
        {
            // Finding command number in this line
            int cmd_num = recognize_conf_command(buf, &pos, len, creaturetype_angerjob_commands);
            // Now store the config item in correct place
            if (cmd_num == -3) break; // if next block starts
            if ((flags & CnfLd_ListOnly) != 0) {
                // In "List only" mode, accept only name command
                if (cmd_num > 1) {
                    cmd_num = 0;
                }
            }
            int n = 0;
            switch (cmd_num)
            {
            case 1: // NAME
                if (get_conf_parameter_single(buf,&pos,len,gameadd.crtr_conf.angerjobs[i].name,COMMAND_WORD_LEN) <= 0)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                n++;
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
    }
    return true;
}

TbBool parse_creaturetype_attackpref_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    int i;
    // Block name and parameter word store variables
    int arr_size;
    // Initialize the array
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(gameadd.crtr_conf.attacktypes)/sizeof(gameadd.crtr_conf.attacktypes[0]);
        for (i=0; i < arr_size; i++)
        {
            LbMemorySet(gameadd.crtr_conf.attacktypes[i].text, 0, COMMAND_WORD_LEN);
            if (i < gameadd.crtr_conf.attacktypes_count)
            {
                attackpref_desc[i].name = gameadd.crtr_conf.attacktypes[i].text;
                attackpref_desc[i].num = i;
            } else
            {
                attackpref_desc[i].name = NULL;
                attackpref_desc[i].num = 0;
            }
        }
    }
    arr_size = gameadd.crtr_conf.attacktypes_count;
    // Load the file blocks
    for (i=0; i < arr_size; i++)
    {
        char block_buf[COMMAND_WORD_LEN];
        sprintf(block_buf, "attackpref%d", i);
        long pos = 0;
        int k = find_conf_block(buf, &pos, len, block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0) {
                WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
                return false;
            }
            continue;
        }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creaturetype_attackpref_commands,cmd_num)
        while (pos<len)
        {
            // Finding command number in this line
            int cmd_num = recognize_conf_command(buf, &pos, len, creaturetype_attackpref_commands);
            // Now store the config item in correct place
            if (cmd_num == -3) break; // if next block starts
            if ((flags & CnfLd_ListOnly) != 0) {
                // In "List only" mode, accept only name command
                if (cmd_num > 1) {
                    cmd_num = 0;
                }
            }
            int n = 0;
            switch (cmd_num)
            {
            case 1: // NAME
                if (get_conf_parameter_single(buf,&pos,len,gameadd.crtr_conf.attacktypes[i].text,COMMAND_WORD_LEN) <= 0)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                n++;
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
    }
    return true;
}

TbBool load_creaturetypes_config_file(const char *textname, const char *fname, unsigned short flags)
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
        result = parse_creaturetypes_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
          WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if ((result) && ((flags & CnfLd_ListOnly) == 0)) // This block doesn't have anything we'd like to parse in list mode
    {
        result = parse_creaturetype_experience_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
          WARNMSG("Parsing %s file \"%s\" experience block failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturetype_instance_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
          WARNMSG("Parsing %s file \"%s\" instance blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturetype_job_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
          WARNMSG("Parsing %s file \"%s\" job blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturetype_angerjob_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
          WARNMSG("Parsing %s file \"%s\" angerjob blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_creaturetype_attackpref_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
          WARNMSG("Parsing %s file \"%s\" attackpref blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_creaturetypes_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global creature types config";
    static const char config_campgn_textname[] = "campaign creature types config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_creaturetypes_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_creaturetypes_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

unsigned short get_creature_model_flags(const struct Thing *thing)
{
    if ((thing->model < 1) || (thing->model >= gameadd.crtr_conf.model_count))
      return 0;
  return gameadd.crtr_conf.model[thing->model].model_flags;
}

ThingModel get_creature_model_with_model_flags(unsigned short needflags)
{
    for (ThingModel crmodel = 0; crmodel < gameadd.crtr_conf.model_count; crmodel++)
    {
        if ((gameadd.crtr_conf.model[crmodel].model_flags & needflags) == needflags) {
            return crmodel;
        }
    }
    return 0;
}

/**
 * Sets creature availability state.
 */
TbBool set_creature_available(PlayerNumber plyr_idx, ThingModel crtr_model, long can_be_avail, long force_avail)
{
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Cannot set %s availability; player %d has no dungeon.",thing_class_and_model_name(TCls_Creature, crtr_model),(int)plyr_idx);
        return false;
    }
    if ((crtr_model < 1) || (crtr_model >= CREATURE_TYPES_COUNT)) {
        ERRORDBG(4,"Cannot set creature availability; invalid model %d.",(int)plyr_idx,(int)crtr_model);
        return false;
    }
    if (force_avail < 0)
        force_avail = 0;
    if (force_avail >= CREATURES_COUNT)
        force_avail = CREATURES_COUNT-1;
    SYNCDBG(7,"Setting %s availability for player %d to allowed=%d, forced=%d.",thing_class_and_model_name(TCls_Creature, crtr_model),(int)plyr_idx,(int)can_be_avail,(int)force_avail);
    dungeon->creature_allowed[crtr_model] = can_be_avail;
    dungeon->creature_force_enabled[crtr_model] = force_avail;
    return true;
}

ThingModel get_players_special_digger_model(PlayerNumber plyr_idx)
{
    ThingModel crmodel;
    if (plyr_idx == hero_player_number)
    {
        crmodel = gameadd.crtr_conf.special_digger_good;
        if (crmodel == 0)
        {
            WARNLOG("Heroes (player %d) have no digger breed!",(int)plyr_idx);
            crmodel = gameadd.crtr_conf.special_digger_evil;
        }
    } else
    {
        crmodel = gameadd.crtr_conf.special_digger_evil;
        if (crmodel == 0)
        {
            WARNLOG("Keepers have no digger breed!");
            crmodel = gameadd.crtr_conf.special_digger_good;
        }
    }
    return crmodel;
}

ThingModel get_players_spectator_model(PlayerNumber plyr_idx)
{
    ThingModel breed = gameadd.crtr_conf.spectator_breed;
    if (breed == 0)
    {
        WARNLOG("There is no spectator breed for player %d!",(int)plyr_idx);
        breed = gameadd.crtr_conf.special_digger_good;
    }
    return breed;
}

/**
 * Returns personal name of a creature.
 *
 * @param creatng The input creature.
 * @return Pointer to the buffer containing name.
 */
const char *creature_own_name(const struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    char *text;
    if ((get_creature_model_flags(creatng) & CMF_OneOfKind) != 0) {
        struct CreatureData* crdata = creature_data_get_from_thing(creatng);
        text = buf_sprintf("%s",get_string(crdata->namestr_idx));
        return text;
    }
    const char ** starts;
    long starts_len;
    const char ** vowels;
    long vowels_len;
    const char ** consonants;
    long consonants_len;
    const char ** end_vowels;
    long end_vowels_len;
    const char ** end_consonants;
    long end_consonants_len;
    {
        starts = name_starts;
        starts_len = sizeof(name_starts)/sizeof(name_starts[0]);
        vowels = name_vowels;
        vowels_len = sizeof(name_vowels)/sizeof(name_vowels[0]);
        consonants = name_consonants;
        consonants_len = sizeof(name_consonants)/sizeof(name_consonants[0]);
        end_vowels = name_vowels;
        end_vowels_len = sizeof(name_vowels)/sizeof(name_vowels[0]);
        end_consonants = name_consonants;
        end_consonants_len = sizeof(name_consonants)/sizeof(name_consonants[0]);
    }
    {
        //TODO CREATURE store creature name seed somewhere in CreatureControl instead making it from other parameters
        unsigned long seed = creatng->creation_turn + creatng->index + (cctrl->blood_type << 8);
        // Get amount of nucleus
        int name_len;
        {
        int n = LB_RANDOM(65536, &seed);
        name_len = ((n & 7) + ((n>>8) & 7)) >> 1;
        if (name_len < 2)
            name_len = 2;
        else
        if (name_len > 8)
            name_len = 8;
        }
        // Get starting part of a name
        {
            int n = LB_RANDOM(starts_len, &seed);
            const char* part = starts[n];
            text = buf_sprintf("%s", part);
        }
        // Append nucleus items to the name
        int i;
        for (i=0; i < name_len-1; i++)
        {
            const char *part;
            int n;
            if (i & 1) {
                n = LB_RANDOM(consonants_len, &seed);
                part = consonants[n];
            } else {
                n = LB_RANDOM(vowels_len, &seed);
                part = vowels[n];
            }
            strcat(text,part);
        }
        {
            const char *part;
            int n;
            if (i & 1) {
                n = LB_RANDOM(end_consonants_len, &seed);
                part = end_consonants[n];
            } else {
                n = LB_RANDOM(end_vowels_len, &seed);
                part = end_vowels[n];
            }
            strcat(text,part);
        }
    }
    return text;
}

struct CreatureInstanceConfig *get_config_for_instance(CrInstance inst_id)
{
    if ((inst_id < 0) || (inst_id >= gameadd.crtr_conf.instances_count)) {
        return &gameadd.crtr_conf.instances[0];
    }
    return &gameadd.crtr_conf.instances[inst_id];
}

/**
 * Returns Code Name (name to use in script file) of given creature instance.
 */
const char *creature_instance_code_name(CrInstance inst_id)
{
    struct CreatureInstanceConfig* crinstcfg = get_config_for_instance(inst_id);
    const char* name = crinstcfg->name;
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

struct CreatureJobConfig *get_config_for_job(CreatureJob job_flags)
{
    long i = 0;
    unsigned long k = job_flags;
    while (k)
    {
        k >>= 1;
        i++;
    }
    if (i >= gameadd.crtr_conf.jobs_count) {
        return &gameadd.crtr_conf.jobs[0];
    }
    return &gameadd.crtr_conf.jobs[i];
}

/**
 * Returns a job which creature could be doing on specific subtile.
 * @param creatng
 * @param stl_x
 * @param stl_y
 * @return
 */
CreatureJob get_job_for_subtile(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long drop_kind_flags)
{
    // Detect the job which we will do in the area
    unsigned long required_kind_flags = drop_kind_flags;
    if (slab_is_area_inner_fill(subtile_slab(stl_x), subtile_slab(stl_y))) {
        required_kind_flags |= JoKF_AssignOnAreaCenter;
    } else {
        required_kind_flags |= JoKF_AssignOnAreaBorder;
    }
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (creatng->owner == slabmap_owner(slb))
    {
        if (creatng->model == get_players_special_digger_model(creatng->owner)) {
            required_kind_flags |= JoKF_OwnedDiggers;
        } else {
            required_kind_flags |= JoKF_OwnedCreatures;
        }
    } else
    {
        if (creatng->model == get_players_special_digger_model(creatng->owner)) {
            required_kind_flags |= JoKF_EnemyDiggers;
        } else {
            required_kind_flags |= JoKF_EnemyCreatures;
        }
    }
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    RoomKind rkind;
    struct Room* room = get_room_thing_is_on(creatng);
    if (!room_is_invalid(room)) {
        required_kind_flags |= JoKF_AssignAreaWithinRoom;
        rkind = room->kind;
    } else {
        required_kind_flags |= JoKF_AssignAreaOutsideRoom;
        rkind = RoK_NONE;
    }
    CreatureJob jobpref = get_job_for_room(rkind, required_kind_flags, crstat->job_primary | crstat->job_secondary);
    return jobpref;
}

/**
 * Returns a job creature can do in a room of given role, or anywhere else.
 * @param rrole Room roles for which at least one needs to match the job to be returned.
 * @param required_kind_flags Only jobs which have all of the flags set can be returned.
 *     For example, to only include jobs which can be assigned by dropping creatures by computer player,
 *     use JoKF_AssignComputerDrop flag.
 * @param has_jobs Primary and secondary jobs of a creature to be assigned; if only jobs which have
 *     no need to be in primary/secondary list should be qualified, this can be Job_NULL.
 * @return A single job flag.
 */
CreatureJob get_job_for_room_role(RoomRole rrole, unsigned long required_kind_flags, CreatureJob has_jobs)
{
    for (long i = 0; i < gameadd.crtr_conf.jobs_count; i++)
    {
        struct CreatureJobConfig* jobcfg = &gameadd.crtr_conf.jobs[i];
        if ((jobcfg->job_flags & required_kind_flags) == required_kind_flags)
        {
            CreatureJob new_job = 1 << (i - 1);
            if (((jobcfg->job_flags & JoKF_NeedsHaveJob) == 0) || ((has_jobs & new_job) != 0))
            {
                if (((jobcfg->room_role & rrole) != 0) || ((jobcfg->job_flags & JoKF_AssignAreaOutsideRoom) != 0)) {
                    return new_job;
                }
            }
        }
    }
    return Job_NULL;
}

/**
 * Returns a job creature can do in a room, or anywhere else.
 * @param rkind Room kind for which job is to be returned.
 * @param required_kind_flags Only jobs which have all of the flags set can be returned.
 *     For example, to only include jobs which can be assigned by dropping creatures by computer player,
 *     use JoKF_AssignComputerDrop flag.
 * @param has_jobs Primary and secondary jobs of a creature to be assigned; if only jobs which have
 *     no need to be in primary/secondary list should be qualified, this can be Job_NULL.
 * @return A single job flag.
 */
CreatureJob get_job_for_room(RoomKind rkind, unsigned long required_kind_flags, CreatureJob has_jobs)
{
    return get_job_for_room_role(get_room_roles(rkind), required_kind_flags, has_jobs);
}

/**
 * Returns a job creature can do in a room with given role.
 * @param rrole Room roles for which at least one needs to match the job to be returned.
 * @param qualify_flags Only jobs which have at least one of the flags set can be returned.
 * @param prevent_flags Only jobs which have none of the flags set can be returned.
 * @return A single job flag.
 */
CreatureJob get_job_which_qualify_for_room_role(RoomRole rrole, unsigned long qualify_flags, unsigned long prevent_flags)
{
    if (rrole == RoRoF_None) {
        return Job_NULL;
    }
    for (long i = 0; i < gameadd.crtr_conf.jobs_count; i++)
    {
        struct CreatureJobConfig* jobcfg = &gameadd.crtr_conf.jobs[i];
        if ((jobcfg->job_flags & qualify_flags) != 0)
        {
            if ((jobcfg->job_flags & prevent_flags) == 0)
            {
                if ((jobcfg->room_role & rrole) != 0) {
                    return 1<<(i-1);
                }
            }
        }
    }
    return Job_NULL;
}

/**
 * Returns a job creature can do in a room.
 * @param rkind Room kind for which job is to be returned.
 * @param qualify_flags Only jobs which have at least one of the flags set can be returned.
 * @param prevent_flags Only jobs which have none of the flags set can be returned.
 * @return A single job flag.
 */
CreatureJob get_job_which_qualify_for_room(RoomKind rkind, unsigned long qualify_flags, unsigned long prevent_flags)
{
    return get_job_which_qualify_for_room_role(get_room_roles(rkind), qualify_flags, prevent_flags);
}

/**
 * Returns jobs which creatures owned by enemy players may be assigned to do work in rooms of specific role.
 * @param rrole Room roles for which at least one needs to match the job to be returned.
 * @return Job flags matching.
 */
CreatureJob get_jobs_enemies_may_do_in_room_role(RoomRole rrole)
{
    CreatureJob jobpref = Job_NULL;
    for (long i = 0; i < gameadd.crtr_conf.jobs_count; i++)
    {
        struct CreatureJobConfig* jobcfg = &gameadd.crtr_conf.jobs[i];
        // Accept only jobs in given room
        if ((jobcfg->room_role & rrole) != 0)
        {
            // Check whether enemies can do this job
            if ((jobcfg->job_flags & (JoKF_EnemyCreatures|JoKF_EnemyDiggers)) != 0)
            {
                jobpref |= 1<<(i-1);
            }
        }
    }
    return jobpref;
}

/**
 * Returns jobs which creatures owned by enemy players may be assigned to do work in specific room.
 * @param rkind Room kind to be checked.
 * @return Job flags matching.
 */
CreatureJob get_jobs_enemies_may_do_in_room(RoomKind rkind)
{
    return get_jobs_enemies_may_do_in_room_role(get_room_roles(rkind));
}

/**
 * Returns first room kind which matches role from given job.
 * Note that more than one room kind may have given role, so use
 * of this function should be limited.
 * @param job_flags
 * @return
 */
RoomKind get_room_for_job(CreatureJob job_flags)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(job_flags);
    for (RoomKind rkind = 0; rkind < slab_conf.room_types_count; rkind++)
    {
        if (room_role_matches(rkind, jobcfg->room_role))
            return rkind;
    }
    return RoK_NONE;
}

/**
 * Returns room role from given job.
 * @param job_flags
 * @return
 */
RoomRole get_room_role_for_job(CreatureJob job_flags)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(job_flags);
    return jobcfg->room_role;
}

EventKind get_event_for_job(CreatureJob job_flags)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(job_flags);
    return jobcfg->event_kind;
}

CrtrStateId get_initial_state_for_job(CreatureJob jobpref)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(jobpref);
    return jobcfg->initial_crstate;
}

unsigned long get_flags_for_job(CreatureJob jobpref)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(jobpref);
    return jobcfg->job_flags;
}

int get_required_room_capacity_for_job(CreatureJob jobpref, ThingModel crmodel)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(jobpref);
    switch (jobcfg->room_role)
    {
    case RoRoF_None:
        WARNLOG("Job needs capacity but has no related room role.");
        return 0;
    case RoRoF_LairStorage:
    case RoRoF_CrHealSleep:
    {
        struct CreatureStats* crstat = creature_stats_get(crmodel);
        return crstat->lair_size;
    }
    default:
        break;
    }
    if ((jobcfg->job_flags & JoKF_NeedsCapacity) == 0)
    {
        return 0;
    }
    return 1;
}

CrtrStateId get_arrive_at_state_for_job(CreatureJob jobpref)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(jobpref);
    return jobcfg->initial_crstate;
}

CrtrStateId get_continue_state_for_job(CreatureJob jobpref)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(jobpref);
    return jobcfg->continue_crstate;
}

CreatureJob get_job_for_creature_state(CrtrStateId crstat_id)
{
    if (crstat_id == CrSt_Unused) {
        return Job_NULL;
    }
    for (long i = 0; i < gameadd.crtr_conf.jobs_count; i++)
    {
        struct CreatureJobConfig* jobcfg = &gameadd.crtr_conf.jobs[i];
        //TODO CREATURE_JOBS Add other job-related states here
        if ((jobcfg->initial_crstate == crstat_id)
         || (jobcfg->continue_crstate == crstat_id)) {
            return 1<<(i-1);
        }
    }
    // Some additional hacks
    switch (crstat_id)
    {
    case CrSt_CreatureEat:
    case CrSt_CreatureEatingAtGarden:
    case CrSt_CreatureToGarden:
    case CrSt_CreatureArrivedAtGarden:
        return Job_TAKE_FEED;
    case CrSt_CreatureWantsSalary:
    case CrSt_CreatureTakeSalary:
        return Job_TAKE_SALARY;
    case CrSt_CreatureSleep:
    case CrSt_CreatureGoingHomeToSleep:
    case CrSt_AtLairToSleep:
    case CrSt_CreatureChooseRoomForLairSite:
    case CrSt_CreatureAtNewLair:
    case CrSt_CreatureWantsAHome:
    case CrSt_CreatureChangeLair:
    case CrSt_CreatureAtChangedLair:
        return Job_TAKE_SLEEP;
    default:
        break;
    }
    return Job_NULL;
}

/**
 * Returns Code Name (name to use in script file) of given creature model.
 */
const char *creature_job_code_name(CreatureJob job_flag)
{
    struct CreatureJobConfig* jobcfg = get_config_for_job(job_flag);
    const char* name = jobcfg->name;
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Gives the job which can cause creature stress in specific room.
 *
 * @param job_flags Primary job flags of a creature kind to be checked.
 * @param rkind Room kind to be checked.
 * @return Returns a single job flag, or Job_NULL.
 */
CreatureJob get_creature_job_causing_stress(CreatureJob job_flags, RoomKind rkind)
{
    // Allowing one-time jobs to be stressful would make this job selection ambiguous
    // TODO CREATURE_JOBS it would be better to get stressful job based on creature state, not on room
    CreatureJob qualified_job = get_job_which_qualify_for_room(rkind, JoKF_OwnedCreatures | JoKF_OwnedDiggers, JoKF_AssignOneTime);
    return (job_flags & qualified_job);
}

/**
 * Gives the job which can cause creature going postal in specific room.
 *
 * @param job_flags Primary job flags of a creature kind to be checked.
 * @param rkind Room kind to be checked.
 * @return Returns a single job flag, or Job_NULL.
 */
CreatureJob get_creature_job_causing_going_postal(CreatureJob job_flags, RoomKind rkind)
{
    CreatureJob qualified_job = get_job_which_qualify_for_room(rkind, JoKF_OwnedCreatures | JoKF_OwnedDiggers, JoKF_EnemyCreatures | JoKF_EnemyDiggers | JoKF_AssignOneTime | JoKF_NoSelfControl);
    return (job_flags & qualified_job);
}

const char *attack_type_job_code_name(CrAttackType attack_type)
{
    const struct CommandWord * attack_type_info;
    if (attack_type < gameadd.crtr_conf.attacktypes_count) {
        attack_type_info = &gameadd.crtr_conf.attacktypes[attack_type];
    } else {
        attack_type_info = &gameadd.crtr_conf.attacktypes[0];
    }
    const char* name = attack_type_info->text;
    if (name[0] != '\0')
        return name;
    return "INVALID";
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
