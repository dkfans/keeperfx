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
#include "creature_jobs.h"
#include "engine_arrays.h"
#include "game_legacy.h"

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
  {NULL,                     0},
  };

const struct NamedCommand creaturetype_experience_commands[] = {
  {"PAYINCREASEONEXP",      1},
  {"SPELLDAMAGEINCREASEONEXP",2},
  {"RANGEINCREASEONEXP",    3},
  {"JOBVALUEINCREASEONEXP", 4},
  {"HEALTHINCREASEONEXP",   5},
  {"STRENGTHINCREASEONEXP", 6},
  {"DEXTERITYINCREASEONEXP",7},
  {"DEFENSEINCREASEONEXP",  8},
  {"LOYALTYINCREASEONEXP",  9},
  {"ARMOURINCREASEONEXP",  10},
  {NULL,                    0},
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
  {"GRAPHICS",        9},
  {"FUNCTION",       10},
  {"PROPERTIES",     11},
  {NULL,              0},
  };

const struct NamedCommand creaturetype_instance_properties[] = {
  {"REPEAT_TRIGGER",       InstPF_RepeatTrigger},
  {NULL,                     0},
  };

const struct NamedCommand creaturetype_job_commands[] = {
  {"NAME",              1},
  {"RELATEDROOM",       2},
  {"RELATEDEVENT",      3},
  {"ASSIGN",            4},
  {"INITIALSTATE",      5},
  {"PLAYERFUNCTIONS",   6},
  {"COORDSFUNCTIONS",   7},
  {"PROPERTIES",        8},
  {NULL,                0},
  };

const struct NamedCommand creaturetype_job_assign[] = {
  {"HUMAN_DROP_IN_ROOM",     JoKF_AssignHumanDropInRoom},
  {"COMPUTER_DROP_IN_ROOM",  JoKF_AssignComputerDropInRoom},
  {"INIT_IN_ROOM",           JoKF_AssignInitInRoom},
  {"BORDER_ONLY",            JoKF_AssignDropOnRoomBorder},
  {"CENTER_ONLY",            JoKF_AssignDropOnRoomCenter},
  {"WHOLE_ROOM",             JoKF_AssignDropOnRoomBorder|JoKF_AssignDropOnRoomCenter},
  {"OWNED_CREATURES",        JoKF_OwnedCreatures},
  {"ENEMY_CREATURES",        JoKF_EnemyCreatures},
  {"OWNED_DIGGERS",          JoKF_OwnedDiggers},
  {"ENEMY_DIGGERS",          JoKF_EnemyDiggers},
  {"ONE_TIME",               JoKF_AssignOneTime},
  {"NEEDS_HAVE_JOB",         JoKF_NeedsHaveJob},
  {NULL,                     0},
  };

const struct NamedCommand creaturetype_job_properties[] = {
  {"WORK_BORDER_ONLY",       JoKF_WorkOnRoomBorder},
  {"WORK_CENTER_ONLY",       JoKF_WorkOnRoomCenter},
  {"WORK_WHOLE_ROOM",        JoKF_WorkOnRoomBorder|JoKF_WorkOnRoomCenter},
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
// These below seems to be not from CREATURE.JTY
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
struct CreatureConfig crtr_conf;
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
  if ((thing->model < 1) || (thing->model >= crtr_conf.model_count))
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
    struct CreatureStats *crstat;
    crstat = creature_stats_get(crstat_idx);
    // Update old stats by copying part of the new stats memory
    // Note that CreatureStats may only change at end for this hack to work!
    memcpy(&game.creature_stats_OLD[crstat_idx],crstat,sizeof(struct CreatureStatsOLD));
}

void check_and_auto_fix_stats(void)
{
    struct CreatureStats *crstat;
    long model;
    long i,n;
    SYNCDBG(8,"Starting for %d models",(int)crtr_conf.model_count);
    //_DK_check_and_auto_fix_stats();
    for (model=0; model < crtr_conf.model_count; model++)
    {
        crstat = creature_stats_get(model);
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
            ERRORLOG("Creature model %d SleepSlab & SleepExperience = 0 - Fixing", (int)model);
            crstat->sleep_exp_slab = 0;
        }
        if (crstat->grow_up > 30)
        {
            ERRORLOG("Creature model %d Invalid GrowUp - Fixing", (int)model);
            crstat->grow_up = 0;
        }
        if (crstat->grow_up > 0)
        {
          if ( (crstat->grow_up_level < 1) || (crstat->grow_up_level > 10) )
          {
              ERRORLOG("Creature model %d GrowUp & GrowUpLevel invalid - Fixing", (int)model);
              crstat->grow_up_level = 1;
          }
        }
        if (crstat->rebirth > 10)
        {
            ERRORLOG("Creature model %d Rebirth Invalid - Fixing", (int)model);
            crstat->rebirth = 0;
        }
        for (i=0; i < 10; i++)
        {
            n = crstat->instance_level[i];
            if (n != 0)
            {
                if ( (n < 1) || (n > 10) )
                {
                    ERRORLOG("Creature model %d Instance Level For Slot %d Invalid - Fixing", (int)model, (int)(i+1));
                    crstat->instance_level[i] = 1;
                }
            } else
            {
                if ( (n >= 1) && (n <= 10) )
                {
                    ERRORLOG("Creature model %d Instance Level For Not Used Spell %d - Fixing", (int)model, (int)(i+1));
                    crstat->instance_level[i] = 0;
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

/**
 * Returns Code Name (name to use in script file) of given creature model.
 */
const char *creature_code_name(ThingModel crmodel)
{
    const char *name;
    name = get_conf_parameter_text(creature_desc,crmodel);
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
    int i;

    for (i = 0; i < crtr_conf.model_count; ++i) {
        if (strncmp(name, crtr_conf.model[i].name, COMMAND_WORD_LEN) == 0) {
            return i + 1;
        }
    }

    return -1;
}

TbBool parse_creaturetypes_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        crtr_conf.model_count = 1;
        crtr_conf.instances_count = 1;
        crtr_conf.jobs_count = 1;
        crtr_conf.angerjobs_count = 1;
        crtr_conf.attackpref_count = 1;
        crtr_conf.special_digger_good = 0;
        crtr_conf.special_digger_evil = 0;
        crtr_conf.spectator_breed = 0;
    }
    k = sizeof(crtr_conf.model)/sizeof(crtr_conf.model[0]);
    for (i=0; i < k; i++)
    {
      LbMemorySet(crtr_conf.model[i].name, 0, COMMAND_WORD_LEN);
    }
    LbStringCopy(crtr_conf.model[0].name, "NOCREATURE", COMMAND_WORD_LEN);
    // Find the block
    sprintf(block_buf,"common");
    pos = 0;
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
        cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // CREATURES
            while (get_conf_parameter_single(buf,&pos,len,crtr_conf.model[n+1].name,COMMAND_WORD_LEN) > 0)
            {
              creature_desc[n].name = crtr_conf.model[n+1].name;
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
            crtr_conf.model_count = n+1;
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
                crtr_conf.instances_count = k;
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
                crtr_conf.jobs_count = k;
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
                crtr_conf.angerjobs_count = k;
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
                crtr_conf.attackpref_count = k;
                n++;
              }
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
    if (crtr_conf.model_count < 1)
    {
        WARNLOG("No creature species defined in [%s] block of %s file.",
            block_buf,config_textname);
    }
    return true;
}

TbBool parse_creaturetype_experience_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        crtr_conf.exp.pay_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        crtr_conf.exp.spell_damage_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        crtr_conf.exp.range_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        crtr_conf.exp.job_value_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        crtr_conf.exp.health_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        crtr_conf.exp.strength_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        crtr_conf.exp.dexterity_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        crtr_conf.exp.defense_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        crtr_conf.exp.loyalty_increase_on_exp = CREATURE_PROPERTY_INCREASE_ON_EXP;
        crtr_conf.exp.armour_increase_on_exp = 0;
    }
    // Find the block
    sprintf(block_buf,"experience");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
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
        cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_experience_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // PAYINCREASEONEXP
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                crtr_conf.exp.pay_increase_on_exp = k;
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
                crtr_conf.exp.spell_damage_increase_on_exp = k;
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
                crtr_conf.exp.range_increase_on_exp = k;
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
                crtr_conf.exp.job_value_increase_on_exp = k;
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
                crtr_conf.exp.health_increase_on_exp = k;
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
                crtr_conf.exp.strength_increase_on_exp = k;
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
                crtr_conf.exp.dexterity_increase_on_exp = k;
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
                crtr_conf.exp.defense_increase_on_exp = k;
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
                crtr_conf.exp.loyalty_increase_on_exp = k;
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
                crtr_conf.exp.armour_increase_on_exp = k;
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
    if (crtr_conf.model_count < 1)
    {
        WARNLOG("No creature species defined in [%s] block of %s file.",
            block_buf,config_textname);
    }
    return true;
}

TbBool parse_creaturetype_instance_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct InstanceInfo *inst_inf;
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    int arr_size;
    // Initialize the array
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(crtr_conf.instances)/sizeof(crtr_conf.instances[0]);
        for (i=0; i < arr_size; i++)
        {
            LbMemorySet(crtr_conf.instances[i].name, 0, COMMAND_WORD_LEN);
            if (i < crtr_conf.instances_count)
            {
                instance_desc[i].name = crtr_conf.instances[i].name;
                instance_desc[i].num = i;
            } else
            {
                instance_desc[i].name = NULL;
                instance_desc[i].num = 0;
            }
        }
    }
    arr_size = crtr_conf.instances_count;
    // Load the file blocks
    for (i=0; i < arr_size; i++)
    {
      sprintf(block_buf,"instance%d",i);
      pos = 0;
      k = find_conf_block(buf,&pos,len,block_buf);
      if (k < 0)
      {
          if ((flags & CnfLd_AcceptPartial) == 0) {
              WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
              return false;
          }
          continue;
      }
      inst_inf = creature_instance_info_get(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creaturetype_instance_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_instance_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        if ((flags & CnfLd_ListOnly) != 0) {
            // In "List only" mode, accept only name command
            if (cmd_num > 1) {
                cmd_num = 0;
            }
        }
        n = 0;
        switch (cmd_num)
        {
        case 1: // NAME
            if (get_conf_parameter_single(buf,&pos,len,crtr_conf.instances[i].name,COMMAND_WORD_LEN) <= 0)
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
        case 9: // GRAPHICS
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
        case 10: // FUNCTION
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
        case 11: // PROPERTIES
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
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    int arr_size;
    // Initialize the array
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(crtr_conf.jobs)/sizeof(crtr_conf.jobs[0]);
        for (i=0; i < arr_size; i++)
        {
            jobcfg = &crtr_conf.jobs[i];
            LbMemorySet(jobcfg->name, 0, COMMAND_WORD_LEN);
            jobcfg->room_kind = RoK_NONE;
            jobcfg->initial_crstate = CrSt_Unused;
            jobcfg->job_flags = 0;
            jobcfg->func_plyr_check = NULL;
            jobcfg->func_plyr_assign = NULL;
            jobcfg->func_cord_check = NULL;
            jobcfg->func_cord_assign = NULL;
            if (i < crtr_conf.jobs_count)
            {
                creaturejob_desc[i].name = crtr_conf.jobs[i].name;
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
    arr_size = crtr_conf.jobs_count;
    // Load the file blocks
    for (i=0; i < arr_size; i++)
    {
        sprintf(block_buf,"job%d",i);
        pos = 0;
        k = find_conf_block(buf,&pos,len,block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0) {
                WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
                return false;
            }
            continue;
        }
        jobcfg = &crtr_conf.jobs[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creaturetype_job_commands,cmd_num)
        while (pos<len)
        {
            // Finding command number in this line
            cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_job_commands);
            // Now store the config item in correct place
            if (cmd_num == -3) break; // if next block starts
            if ((flags & CnfLd_ListOnly) != 0) {
                // In "List only" mode, accept only name command
                if (cmd_num > 1) {
                    cmd_num = 0;
                }
            }
            n = 0;
            switch (cmd_num)
            {
            case 1: // NAME
                if (get_conf_parameter_single(buf,&pos,len,crtr_conf.jobs[i].name,COMMAND_WORD_LEN) <= 0)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                n++;
                break;
            case 2: // RELATEDROOM
                jobcfg->room_kind = RoK_NONE;
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    k = get_id(room_desc, word_buf);
                    if (k >= 0)
                    {
                        jobcfg->room_kind = k;
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
                jobcfg->job_flags &= ~(JoKF_AssignHumanDropInRoom|JoKF_AssignComputerDropInRoom|JoKF_AssignInitInRoom|
                    JoKF_AssignDropOnRoomBorder|JoKF_AssignDropOnRoomCenter|JoKF_OwnedCreatures|JoKF_EnemyCreatures|
                    JoKF_OwnedDiggers|JoKF_EnemyDiggers|JoKF_AssignOneTime|JoKF_NeedsHaveJob);
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
            case 6: // PLAYERFUNCTIONS
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
            case 7: // COORDSFUNCTIONS
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
            case 8: // PROPERTIES
                jobcfg->job_flags &= ~(JoKF_WorkOnRoomBorder|JoKF_WorkOnRoomCenter|JoKF_NeedsCapacity|JoKF_NoSelfControl|JoKF_NoGroups|JoKF_AllowChickenized);
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
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    int arr_size;
    // Initialize the array
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(crtr_conf.angerjobs)/sizeof(crtr_conf.angerjobs[0]);
        for (i=0; i < arr_size; i++)
        {
            agjobcfg = &crtr_conf.angerjobs[i];
            LbMemorySet(agjobcfg->name, 0, COMMAND_WORD_LEN);
            if (i < crtr_conf.angerjobs_count)
            {
                angerjob_desc[i].name = crtr_conf.angerjobs[i].name;
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
    arr_size = crtr_conf.angerjobs_count;
    // Load the file blocks
    for (i=0; i < arr_size; i++)
    {
        sprintf(block_buf,"angerjob%d",i);
        pos = 0;
        k = find_conf_block(buf,&pos,len,block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0) {
                WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
                return false;
            }
            continue;
        }
        agjobcfg = &crtr_conf.angerjobs[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(creaturetype_angerjob_commands,cmd_num)
        while (pos<len)
        {
            // Finding command number in this line
            cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_angerjob_commands);
            // Now store the config item in correct place
            if (cmd_num == -3) break; // if next block starts
            if ((flags & CnfLd_ListOnly) != 0) {
                // In "List only" mode, accept only name command
                if (cmd_num > 1) {
                    cmd_num = 0;
                }
            }
            n = 0;
            switch (cmd_num)
            {
            case 1: // NAME
                if (get_conf_parameter_single(buf,&pos,len,crtr_conf.angerjobs[i].name,COMMAND_WORD_LEN) <= 0)
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
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    int arr_size;
    // Initialize the array
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(crtr_conf.attackpref_names)/sizeof(crtr_conf.attackpref_names[0]);
        for (i=0; i < arr_size; i++)
        {
            LbMemorySet(crtr_conf.attackpref_names[i].text, 0, COMMAND_WORD_LEN);
            if (i < crtr_conf.attackpref_count)
            {
                attackpref_desc[i].name = crtr_conf.attackpref_names[i].text;
                attackpref_desc[i].num = i;
            } else
            {
                attackpref_desc[i].name = NULL;
                attackpref_desc[i].num = 0;
            }
        }
    }
    arr_size = crtr_conf.attackpref_count;
    // Load the file blocks
    for (i=0; i < arr_size; i++)
    {
        sprintf(block_buf,"attackpref%d",i);
        pos = 0;
        k = find_conf_block(buf,&pos,len,block_buf);
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
            cmd_num = recognize_conf_command(buf,&pos,len,creaturetype_attackpref_commands);
            // Now store the config item in correct place
            if (cmd_num == -3) break; // if next block starts
            if ((flags & CnfLd_ListOnly) != 0) {
                // In "List only" mode, accept only name command
                if (cmd_num > 1) {
                    cmd_num = 0;
                }
            }
            n = 0;
            switch (cmd_num)
            {
            case 1: // NAME
                if (get_conf_parameter_single(buf,&pos,len,crtr_conf.attackpref_names[i].text,COMMAND_WORD_LEN) <= 0)
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
    char *buf;
    long len;
    TbBool result;
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    len = LbFileLengthRnc(fname);
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
    buf = (char *)LbMemoryAlloc(len+256);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    result = (len > 0);
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
    char *fname;
    TbBool result;
    fname = prepare_file_path(FGrp_FxData,conf_fname);
    result = load_creaturetypes_config_file(config_global_textname,fname,flags);
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
    if ((thing->model < 1) || (thing->model >= crtr_conf.model_count))
      return 0;
  return crtr_conf.model[thing->model].model_flags;
}

ThingModel get_creature_model_with_model_flags(unsigned short needflags)
{
    ThingModel crmodel;
    for (crmodel=0; crmodel < crtr_conf.model_count; crmodel++)
    {
        if ((crtr_conf.model[crmodel].model_flags & needflags) == needflags) {
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
    struct Dungeon *dungeon;
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Can't set trap availability; player %d has no dungeon.",(int)plyr_idx);
        return false;
    }
    if ((crtr_model < 1) || (crtr_model >= CREATURE_TYPES_COUNT))
        return false;
    if (force_avail < 0)
        force_avail = 0;
    if (force_avail >= CREATURES_COUNT)
        force_avail = CREATURES_COUNT-1;
    dungeon->creature_allowed[crtr_model] = can_be_avail;
    dungeon->creature_force_enabled[crtr_model] = force_avail;
    return true;
}

ThingModel get_players_special_digger_model(PlayerNumber plyr_idx)
{
    ThingModel crmodel;
    if (plyr_idx == hero_player_number)
    {
        crmodel = crtr_conf.special_digger_good;
        if (crmodel == 0)
        {
            WARNLOG("Heroes (player %d) have no digger breed!",(int)plyr_idx);
            crmodel = crtr_conf.special_digger_evil;
        }
    } else
    {
        crmodel = crtr_conf.special_digger_evil;
        if (crmodel == 0)
        {
            WARNLOG("Keepers have no digger breed!");
            crmodel = crtr_conf.special_digger_good;
        }
    }
    return crmodel;
}

ThingModel get_players_spectator_model(PlayerNumber plyr_idx)
{
    ThingModel breed;
    breed = crtr_conf.spectator_breed;
    if (breed == 0)
    {
        WARNLOG("There is no spectator breed for player %d!",(int)plyr_idx);
        breed = crtr_conf.special_digger_good;
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
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    char *text;
    if ((get_creature_model_flags(creatng) & CMF_OneOfKind) != 0) {
        struct CreatureData *crdata;
        crdata = creature_data_get_from_thing(creatng);
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
        unsigned long seed;
        //TODO CREATURE store creature name seed somewhere in CreatureControl instead making it from other parameters
        seed = creatng->creation_turn + creatng->index + (cctrl->blood_type << 8);
        // Get amount of nucleus
        int name_len;
        int n;
        n = LB_RANDOM(65536, &seed);
        name_len = ((n & 7) + ((n>>8) & 7)) >> 1;
        if (name_len < 2) {
            name_len = 2;
        } else
        if (name_len > 8) {
            name_len = 8;
        }
        // Get starting part of a name
        {
            const char *part;
            int n;
            n = LB_RANDOM(starts_len, &seed);
            part = starts[n];
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
    if ((inst_id < 0) || (inst_id >= crtr_conf.instances_count)) {
        return &crtr_conf.instances[0];
    }
    return &crtr_conf.instances[inst_id];
}

/**
 * Returns Code Name (name to use in script file) of given creature instance.
 */
const char *creature_instance_code_name(CrInstance inst_id)
{
    struct CreatureInstanceConfig *crinstcfg;
    crinstcfg = get_config_for_instance(inst_id);
    const char *name;
    name = crinstcfg->name;
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

struct CreatureJobConfig *get_config_for_job(CreatureJob job_flags)
{
    long i;
    unsigned long k;
    i = 0;
    k = job_flags;
    while (k)
    {
        k >>= 1;
        i++;
    }
    if (i >= crtr_conf.jobs_count) {
        return &crtr_conf.jobs[0];
    }
    return &crtr_conf.jobs[i];
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
    struct Room *room;
    room = get_room_thing_is_on(creatng);
    if (!room_is_invalid(room))
    {
        unsigned long required_kind_flags;
        // Detect the job which we will do in the room
        required_kind_flags = drop_kind_flags;
        if (creatng->owner == room->owner)
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
        if (slab_is_area_inner_fill(subtile_slab(stl_x), subtile_slab(stl_y))) {
            required_kind_flags |= JoKF_AssignDropOnRoomCenter;
        } else {
            required_kind_flags |= JoKF_AssignDropOnRoomBorder;
        }
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(creatng);
        CreatureJob jobpref;
        jobpref = get_job_for_room(room->kind, required_kind_flags, crstat->job_primary|crstat->job_secondary);
        return jobpref;
    }
    //TODO CREATURE_JOBS make support for jobs outside of rooms
    return Job_NULL;
}

/**
 * Returns a job creature can do in a room.
 * @param rkind Room kind for which job is to be returned.
 * @param required_kind_flags Only jobs which have all of the flags set can be returned.
 *     For example, to only include jobs which can be assigned by dropping creatures by computer player,
 *     use JoKF_AssignComputerDropInRoom flag.
 * @param has_jobs Primary and secondary jobs of a creature to be assigned; if only jobs which have
 *     no need to be in primary/secondary list should be qualified, this can be Job_NULL.
 * @return A single job flag.
 */
CreatureJob get_job_for_room(RoomKind rkind, unsigned long required_kind_flags, CreatureJob has_jobs)
{
    long i;
    if (rkind == RoK_NONE) {
        return Job_NULL;
    }
    for (i=0; i < crtr_conf.jobs_count; i++)
    {
        struct CreatureJobConfig *jobcfg;
        jobcfg = &crtr_conf.jobs[i];
        if ((jobcfg->job_flags & required_kind_flags) == required_kind_flags)
        {
            CreatureJob new_job;
            new_job = 1<<(i-1);
            if (((jobcfg->job_flags & JoKF_NeedsHaveJob) == 0) || ((has_jobs & new_job) != 0))
            {
                if (jobcfg->room_kind == rkind) {
                    return new_job;
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
    long i;
    if (rkind == RoK_NONE) {
        return Job_NULL;
    }
    for (i=0; i < crtr_conf.jobs_count; i++)
    {
        struct CreatureJobConfig *jobcfg;
        jobcfg = &crtr_conf.jobs[i];
        if ((jobcfg->job_flags & qualify_flags) != 0)
        {
            if ((jobcfg->job_flags & prevent_flags) == 0)
            {
                if (jobcfg->room_kind == rkind) {
                    return 1<<(i-1);
                }
            }
        }
    }
    return Job_NULL;
}

RoomKind get_room_for_job(CreatureJob job_flags)
{
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(job_flags);
    return jobcfg->room_kind;
}

EventKind get_event_for_job(CreatureJob job_flags)
{
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(job_flags);
    return jobcfg->event_kind;
}

CrtrStateId get_initial_state_for_job(CreatureJob jobpref)
{
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(jobpref);
    return jobcfg->initial_crstate;
}

unsigned long get_flags_for_job(CreatureJob jobpref)
{
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(jobpref);
    return jobcfg->job_flags;
}

CrtrStateId get_arrive_at_state_for_job(CreatureJob jobpref)
{
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(jobpref);
    return jobcfg->initial_crstate;
}

/**
 * Returns state for creature.
 * @param rkind
 * @return
 * @deprecated Room kind isn't pointing a specific job.
 */
CrtrStateId get_arrive_at_state_for_room(RoomKind rkind)
{
    CreatureJob jobpref;
    jobpref = get_job_for_room(rkind, JoKF_None, Job_NULL);
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(jobpref);
    return jobcfg->initial_crstate;
}

/**
 * Returns Code Name (name to use in script file) of given creature model.
 */
const char *creature_job_code_name(CreatureJob job_flag)
{
    struct CreatureJobConfig *jobcfg;
    jobcfg = get_config_for_job(job_flag);
    const char *name;
    name = jobcfg->name;
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
    CreatureJob qualified_job;
    // Allowing one-time jobs to be stressful would make this job selection ambiguous
    // TODO CREATURE_JOBS it would be better to get stressful job based on creature state, not on room
    qualified_job = get_job_which_qualify_for_room(rkind, JoKF_OwnedCreatures|JoKF_OwnedDiggers, JoKF_AssignOneTime);
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
    CreatureJob qualified_job;
    qualified_job = get_job_which_qualify_for_room(rkind, JoKF_OwnedCreatures|JoKF_OwnedDiggers, JoKF_EnemyCreatures|JoKF_EnemyDiggers|JoKF_AssignOneTime|JoKF_NoSelfControl);
    return (job_flags & qualified_job);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
