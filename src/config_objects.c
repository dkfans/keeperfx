/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_objects.c
 *     Object things configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for object things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Jun 2012 - 16 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_objects.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_dernc.h"
#include "bflib_sound.h"

#include "config.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "custom_sprites.h"
#include "thing_objects.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_objects_file[]="objects.cfg";

const struct NamedCommand objects_common_commands[] = {
  {"OBJECTSCOUNT",    1},
  {NULL,              0},
  };

const struct NamedCommand objects_object_commands[] = {
  {"NAME",               1},
  {"GENRE",              2},
  {"RELATEDCREATURE",    3},
  {"PROPERTIES",         4},
  {"ANIMATIONID",        5},
  {"ANIMATIONSPEED",     6},
  {"SIZE_XY",            7},
  {"SIZE_YZ",            8},
  {"SIZE_Z",             8},
  {"MAXIMUMSIZE",        9},
  {"DESTROYONLIQUID",   10},
  {"DESTROYONLAVA",     11},
  {"HEALTH",            12},
  {"FALLACCELERATION",  13},
  {"LIGHTUNAFFECTED",   14},
  {"LIGHTINTENSITY",    15},
  {"LIGHTRADIUS",       16},
  {"LIGHTISDYNAMIC",    17},
  {"MAPICON",           18},
  {"AMBIENCESOUND",     19},
  {"UPDATEFUNCTION",    20},
  {"DRAWCLASS",         21},
  {"PERSISTENCE",       22},
  {"Immobile",          23},
  {NULL,                 0},
  };

const struct NamedCommand objects_properties_commands[] = {
  {"EXISTS_ONLY_IN_ROOM",     1},
  {"DESTROYED_ON_ROOM_CLAIM", 2},
  {"CHOWNED_ON_ROOM_CLAIM",   3},
  {"DESTROYED_ON_ROOM_PLACE", 4},
  {"BUOYANT",                 5},
  {"BEATING",                 6},
  {"HEART",                   7},
  {NULL,                      0},
  };

const struct NamedCommand objects_genres_desc[] = {
  {"NONE",            OCtg_Unknown},
  {"DECORATION",      OCtg_Decoration},
  {"FURNITURE",       OCtg_Furniture},
  {"VALUABLE",        OCtg_Valuable},
  {"SPELLBOOK",       OCtg_Spellbook},
  {"SPECIALBOX",      OCtg_SpecialBox},
  {"WORKSHOPBOX",     OCtg_WrkshpBox},
  {"TREASURE_HOARD",  OCtg_GoldHoard},
  {"FOOD",            OCtg_Food},
  {"POWER",           OCtg_Power},
  {"LAIR_TOTEM",      OCtg_LairTotem},
  {"EFFECT",          OCtg_Effect},
  {NULL,              0},
  };

/******************************************************************************/
struct NamedCommand object_desc[OBJECT_TYPES_MAX];
/******************************************************************************/
struct ObjectConfigStats *get_object_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= gameadd.object_conf.object_types_count)
        return &gameadd.object_conf.object_cfgstats[0];
    return &gameadd.object_conf.object_cfgstats[tngmodel];
}

ThingClass crate_to_workshop_item_class(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= gameadd.object_conf.object_types_count))
        return gameadd.object_conf.workshop_object_class[0];
    return gameadd.object_conf.workshop_object_class[tngmodel];
}

ThingModel crate_to_workshop_item_model(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= gameadd.object_conf.object_types_count))
        return gameadd.object_conf.object_to_door_or_trap[0];
    return gameadd.object_conf.object_to_door_or_trap[tngmodel];
}

ThingClass crate_thing_to_workshop_item_class(const struct Thing *thing)
{
    if (!thing_is_workshop_crate(thing))
        return thing->class_id;
    ThingModel tngmodel = thing->model;
    if ((tngmodel <= 0) || (tngmodel >= gameadd.object_conf.object_types_count))
        return gameadd.object_conf.workshop_object_class[0];
    return gameadd.object_conf.workshop_object_class[tngmodel];
}

ThingModel crate_thing_to_workshop_item_model(const struct Thing *thing)
{
    if (thing_is_invalid(thing) || (thing->class_id != TCls_Object))
        return gameadd.object_conf.object_to_door_or_trap[0];
    ThingModel tngmodel = thing->model;
    if ((tngmodel <= 0) || (tngmodel >= gameadd.object_conf.object_types_count))
        return gameadd.object_conf.object_to_door_or_trap[0];
    return gameadd.object_conf.object_to_door_or_trap[tngmodel];
}

TbBool parse_objects_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        gameadd.object_conf.object_types_count = OBJECT_TYPES_MAX - 1;
    }
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "common");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(objects_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, objects_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        int n = 0;
        switch (cmd_num)
        {
        case 1: // OBJECTSCOUNT
        {
            char word_buf[COMMAND_WORD_LEN];
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= OBJECT_TYPES_MAX))
              {
                  gameadd.object_conf.object_types_count = k;
                  n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        }
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

TbBool parse_objects_object_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct ObjectConfigStats *objst;
    int tmodel;
    // Block name and parameter word store variables
    // Initialize the objects array
    int arr_size;
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(gameadd.object_conf.object_cfgstats)/sizeof(gameadd.object_conf.object_cfgstats[0]);
        for (tmodel=0; tmodel < arr_size; tmodel++)
        {
            objst = &gameadd.object_conf.object_cfgstats[tmodel];
            LbMemorySet(objst->code_name, 0, COMMAND_WORD_LEN);
            objst->name_stridx = 201;
            objst->map_icon = 0;
            objst->genre = 0;
            if (tmodel < gameadd.object_conf.object_types_count)
            {
                object_desc[tmodel].name = objst->code_name;
                object_desc[tmodel].num = tmodel;
            } else
            {
                object_desc[tmodel].name = NULL;
                object_desc[tmodel].num = 0;
            }
        }
    }
    // Load the file
    for (tmodel = 0; tmodel < OBJECT_TYPES_MAX; tmodel++)
    {
        char block_buf[COMMAND_WORD_LEN];
        sprintf(block_buf, "object%d", tmodel);
        long pos = 0;
        int k = find_conf_block(buf, &pos, len, block_buf);
        if (k < 0)
        {
            if ((flags & CnfLd_AcceptPartial) == 0)
            {
                // Just count all found blocks if we didn't that already
                if (gameadd.object_conf.object_types_count == OBJECT_TYPES_MAX - 1)
                {
                    gameadd.object_conf.object_types_count = tmodel;
                    JUSTMSG("Loaded %d object types from %s", gameadd.object_conf.object_types_count, config_textname);
                    break;
                }
                WARNMSG("Block [%s] not found in %s file.", block_buf, config_textname);
                return false;
            }
            else
            {
                if (tmodel > gameadd.object_conf.object_types_count)
                {
                    gameadd.object_conf.object_types_count = tmodel;
                    JUSTMSG("Extended to %d object types from %s", gameadd.object_conf.object_types_count, config_textname);
                    break;
                }
            }
            continue;
        }

        objst = &gameadd.object_conf.object_cfgstats[tmodel];
        struct Objects* objdat = get_objects_data(tmodel);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(objects_object_commands,cmd_num)
        while (pos<len)
        {
            // Finding command number in this line
            int cmd_num = recognize_conf_command(buf, &pos, len, objects_object_commands);
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
                if (get_conf_parameter_single(buf,&pos,len,objst->code_name,COMMAND_WORD_LEN) <= 0)
                {
                    CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                break;
            case 2: // GENRE
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    n = get_id(objects_genres_desc, word_buf);
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                objst->genre = n;
                break;
            case 3: // RELATEDCREATURE
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    n = get_id(creature_desc, word_buf);
                }
                if (n < 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                objdat->related_creatr_model = n;
                break;
            case 4: // PROPERTIES
                while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                  k = get_id(objects_properties_commands, word_buf);
                  switch (k)
                  {
                  case 1: // EXISTS_ONLY_IN_ROOM
                      objst->model_flags |= OMF_ExistsOnlyInRoom;
                      n++;
                      break;
                  case 2: // DESTROYED_ON_ROOM_CLAIM
                      objst->model_flags |= OMF_DestroyedOnRoomClaim;
                      n++;
                      break;
                  case 3: // CHOWNED_ON_ROOM_CLAIM
                      objst->model_flags |= OMF_ChOwnedOnRoomClaim;
                      n++;
                      break;
                  case 4: // DESTROYED_ON_ROOM_PLACE
                      objst->model_flags |= OMF_DestroyedOnRoomPlace;
                      n++;
                      break;
                  case 5: // BOUYANT
                      objst->model_flags |= OMF_Buoyant;
                      n++;
                      break;
                  case 6: // BEATING
                      objst->model_flags |= OMF_Beating;
                      n++;
                      break;
                  case 7: // HEART
                      objst->model_flags |= OMF_Heart;
                      n++;
                      break;
                  default:
                      CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                          COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                      break;
                  }
                }
                break;
            case 5: // ANIMATIONID
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {

                    n = get_anim_id(word_buf, objdat);
                    objdat->sprite_anim_idx = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 6: // ANIMATIONSPEED
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objdat->anim_speed = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 7: // SIZE_XY
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objdat->size_xy = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 8: // SIZE_Z
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objdat->size_z = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 9: // MAXIMUMSIZE
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objdat->sprite_size_max = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 10: // DESTROYONLIQUID
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objdat->destroy_on_liquid = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 11: // DESTROYONLAVA
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objdat->destroy_on_lava = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 12: // HEALTH
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objst->health = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 13: // FALLACCELERATION
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objst->fall_acceleration = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 14: // LIGHTUNAFFECTED
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objst->light_unaffected = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 15: // LIGHTINTENSITY
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objst->ilght.intensity = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 16: // LIGHTRADIUS
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objst->ilght.radius = n << 8; //Mystery bit shift. Remove it to get divide by 0 errors.
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 17: // LIGHTISDYNAMIC
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objst->ilght.is_dynamic = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 18: // MAPICON
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = get_icon_id(word_buf);
                    if (n >= -1)
                    {
                        objst->map_icon = n;
                    }
                }
                if (n < -1)
                {
                    objst->map_icon = bad_icon_id;
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 19: // AMBIENCESOUND
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    if ( (!SoundDisabled) && ( (n < 0) || (n > (samples_in_bank - 1)) ) )
                    {
                        CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                    }
                    else
                    {
                        objdat->fp_smpl_idx = n;
                    }
                }
                break;
            case 20: // UPDATEFUNCTION
                if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
                {
                    n = get_id(object_update_functions_desc, word_buf);
                }
                if (n < 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),block_buf,config_textname);
                    break;
                }
                objdat->updatefn_idx = n;
                break;
            case 21: // DRAWCLASS
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objdat->draw_class = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 22: // PERSISTENCE
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objdat->persistence = n;
                    n++;
                }
                if (n <= 0)
                {
                    CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num), block_buf, config_textname);
                }
                break;
            case 23: // Immobile
                if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
                {
                    n = atoi(word_buf);
                    objst->immobile = n;
                    n++;
                }
                if (n <= 0)
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
                CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.", cmd_num,block_buf,config_textname);
                break;
            }
            skip_conf_to_next_line(buf,&pos,len);
        }
#undef COMMAND_TEXT
    }
    return true;
}

TbBool load_objects_config_file(const char *textname, const char *fname, unsigned short flags)
{
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
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
        result = parse_objects_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_objects_object_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" object blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

void update_all_object_stats()
{
    const struct StructureList* slist = get_list_for_thing_class(TCls_Object);
    struct DungeonAdd* dungeonadd;
    struct Dungeon* dungeon;
    for (int i = slist->index; i > 0;)
    {
        struct Thing* thing = thing_get(i);
        i = thing->next_of_class
            TRACE_THING(thing);
        struct Objects* objdat = get_objects_data_for_thing(thing);
        set_thing_draw(thing, objdat->sprite_anim_idx, objdat->anim_speed, objdat->sprite_size_max, 0, 0, objdat->draw_class);
        // TODO: Should we rotate this on per-object basis?
        thing->flags = 0;
        thing->flags |= objdat->rotation_flag << TAF_ROTATED_SHIFT;

        dungeon = get_dungeon(thing->owner);
        if ((thing_is_dungeon_heart(thing)) && (thing->index != dungeon->dnheart_idx))
        {
            dungeonadd = get_dungeonadd(thing->owner);
            if (dungeonadd->backup_heart_idx == 0)
            {
                dungeonadd->backup_heart_idx = thing->index;
            }
        }

        struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
        if (thing->light_id != 0)
        {
            light_delete_light(thing->light_id);
        }
        if (objst->ilght.radius != 0)
        {
            struct InitLight ilight;
            LbMemorySet(&ilight, 0, sizeof(struct InitLight));
            LbMemoryCopy(&ilight.mappos, &thing->mappos, sizeof(struct Coord3d));
            ilight.radius = objst->ilght.radius;
            ilight.intensity = objst->ilght.intensity;
            ilight.field_3 = objst->ilght.field_3;
            ilight.is_dynamic = objst->ilght.is_dynamic;
            thing->light_id = light_create_light(&ilight);
        }
    }
}

TbBool load_objects_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global objects config";
    static const char config_campgn_textname[] = "campaign objects config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_objects_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_objects_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Returns Code Name (name to use in script file) of given object model.
 */
const char *object_code_name(ThingModel tngmodel)
{
    const char* name = get_conf_parameter_text(object_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the object model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the object model if found, otherwise -1
 */
ThingModel object_model_id(const char * code_name)
{
    for (int i = 0; i < gameadd.object_conf.object_types_count; ++i)
    {
        if (strncasecmp(gameadd.object_conf.object_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Returns required room capacity for given object model storage in room.
 * @param room_role The room role of target room.
 * @param objmodel The object model to be checked. May be 0 for lair or dead creature check, as this requires related model.
 * @param relmodel Related thing model, if object model is not unequivocal.
 * @return
 */
int get_required_room_capacity_for_object(RoomRole room_role, ThingModel objmodel, ThingModel relmodel)
{
    struct CreatureStats *crstat;
    struct ObjectConfigStats *objst;
    switch (room_role)
    {
    case RoRoF_LairStorage:
        crstat = creature_stats_get(relmodel);
        return crstat->lair_size;
    case RoRoF_DeadStorage:
        crstat = creature_stats_get(relmodel);
        if (!creature_stats_invalid(crstat))
            return 1;
        break;
    case RoRoF_KeeperStorage:
        break;
    case RoRoF_GoldStorage:
        objst = get_object_model_stats(objmodel);
        if (objst->genre == OCtg_GoldHoard)
            return get_wealth_size_of_gold_hoard_model(objmodel);
        break;
    case RoRoF_FoodSpawn:
    case RoRoF_FoodStorage:
        objst = get_object_model_stats(objmodel);
        if ((objst->genre == OCtg_Food) || (objst->genre == OCtg_Furniture)) // non-mature chickens are furniture
            return 1;
        break;
    case RoRoF_CratesStorage:
        objst = get_object_model_stats(objmodel);
        if (objst->genre == OCtg_WrkshpBox)
            return 1;
        break;
    case RoRoF_PowersStorage:
        objst = get_object_model_stats(objmodel);
        if ((objst->genre == OCtg_Spellbook) || (objst->genre == OCtg_SpecialBox))
            return 1;
        break;
    default:
        break;
    }
    return 0;
}

void init_objects(void)
{
    gameadd.object_conf.object_cfgstats[ObjMdl_Torch].ilght.field_3 = 5;
    gameadd.object_conf.object_cfgstats[ObjMdl_HeroGate].ilght.field_3 = 5;
    gameadd.object_conf.object_cfgstats[ObjMdl_StatueLit].ilght.field_3 = 5;
    gameadd.object_conf.object_cfgstats[ObjMdl_SoulCountainer].ilght.field_3 = 5;
    gameadd.object_conf.object_cfgstats[ObjMdl_Candlestick].ilght.field_3 = 5;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
