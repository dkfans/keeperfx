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
struct NamedCommand object_desc[OBJECT_TYPES_MAX];
/******************************************************************************/
static TbBool load_objects_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_objects_file_data = {
    .filename = "objects.cfg",
    .load_func = load_objects_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

const struct NamedCommand objects_properties_commands[] = {
  {"EXISTS_ONLY_IN_ROOM",     OMF_ExistsOnlyInRoom    },
  {"DESTROYED_ON_ROOM_CLAIM", OMF_DestroyedOnRoomClaim},
  {"CHOWNED_ON_ROOM_CLAIM",   OMF_ChOwnedOnRoomClaim  },
  {"DESTROYED_ON_ROOM_PLACE", OMF_DestroyedOnRoomPlace},
  {"BUOYANT",                 OMF_Buoyant             },
  {"BEATING",                 OMF_Beating             },
  {"HEART",                   OMF_Heart               },
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
  {"HEROGATE",        OCtg_HeroGate},
  {NULL,              0},
  };

static const struct NamedField objects_named_fields[] = {
    //name                     //pos    //field                                                                 //default //min     //max    //NamedCommand
    {"NAME",                     0, field(game.conf.object_conf.object_cfgstats[0].code_name),                     0, LONG_MIN,ULONG_MAX, object_desc,                 value_name,      assign_null},
    {"GENRE",                    0, field(game.conf.object_conf.object_cfgstats[0].genre),                         0, LONG_MIN,ULONG_MAX, objects_genres_desc,         value_default,   assign_default},
    {"RELATEDCREATURE",          0, field(game.conf.object_conf.object_cfgstats[0].related_creatr_model),          0, LONG_MIN,ULONG_MAX, creature_desc,               value_default,   assign_default},
    {"PROPERTIES",              -1, field(game.conf.object_conf.object_cfgstats[0].model_flags),                   0, LONG_MIN,ULONG_MAX, objects_properties_commands, value_flagsfield,assign_default},
    {"ANIMATIONID",              0, field(game.conf.object_conf.object_cfgstats[0].sprite_anim_idx),               0, LONG_MIN,ULONG_MAX, NULL,                        value_animid,    assign_animid},
    {"ANIMATIONSPEED",           0, field(game.conf.object_conf.object_cfgstats[0].anim_speed),                    0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"SIZE_XY",                  0, field(game.conf.object_conf.object_cfgstats[0].size_xy),                       0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"SIZE_YZ",                  0, field(game.conf.object_conf.object_cfgstats[0].size_z),                        0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"SIZE_Z",                   0, field(game.conf.object_conf.object_cfgstats[0].size_z),                        0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"MAXIMUMSIZE",              0, field(game.conf.object_conf.object_cfgstats[0].sprite_size_max),               0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"DESTROYONLIQUID",          0, field(game.conf.object_conf.object_cfgstats[0].destroy_on_liquid),             0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"DESTROYONLAVA",            0, field(game.conf.object_conf.object_cfgstats[0].destroy_on_lava),               0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"HEALTH",                   0, field(game.conf.object_conf.object_cfgstats[0].health),                        0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"FALLACCELERATION",         0, field(game.conf.object_conf.object_cfgstats[0].fall_acceleration),             0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"LIGHTUNAFFECTED",          0, field(game.conf.object_conf.object_cfgstats[0].light_unaffected),              0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"LIGHTINTENSITY",           0, field(game.conf.object_conf.object_cfgstats[0].ilght.intensity),               0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"LIGHTRADIUS",              0, field(game.conf.object_conf.object_cfgstats[0].ilght.radius),                  0, LONG_MIN,ULONG_MAX, NULL,                        value_stltocoord,assign_default},
    {"LIGHTISDYNAMIC",           0, field(game.conf.object_conf.object_cfgstats[0].ilght.is_dynamic),              0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"MAPICON",                  0, field(game.conf.object_conf.object_cfgstats[0].map_icon),                      0, LONG_MIN,ULONG_MAX, NULL,                        value_icon,      assign_icon},
    {"TOOLTIPTEXTID",            0, field(game.conf.object_conf.object_cfgstats[0].tooltip_stridx),               -1, SHRT_MIN, SHRT_MAX, NULL,                        value_default,   assign_default},
    {"TOOLTIPTEXTID",            1, field(game.conf.object_conf.object_cfgstats[0].tooltip_optional),              0,        0,        1, NULL,                        value_default,   assign_default},
    {"AMBIENCESOUND",            0, field(game.conf.object_conf.object_cfgstats[0].fp_smpl_idx),                   0,        0,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"UPDATEFUNCTION",           0, field(game.conf.object_conf.object_cfgstats[0].updatefn_idx),                  0, LONG_MIN,ULONG_MAX, object_update_functions_desc,value_default,   assign_default},
    {"DRAWCLASS",                0, field(game.conf.object_conf.object_cfgstats[0].draw_class),          ODC_Default, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"PERSISTENCE",              0, field(game.conf.object_conf.object_cfgstats[0].persistence),                   0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"IMMOBILE",                 0, field(game.conf.object_conf.object_cfgstats[0].immobile),                      0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"INITIALSTATE",             0, field(game.conf.object_conf.object_cfgstats[0].initial_state),                 0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"RANDOMSTARTFRAME",         0, field(game.conf.object_conf.object_cfgstats[0].random_start_frame),            0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"TRANSPARENCYFLAGS",        0, field(game.conf.object_conf.object_cfgstats[0].transparency_flags),            0, LONG_MIN,ULONG_MAX, NULL,                        value_transpflg, assign_default},
    {"EFFECTBEAM",               0, field(game.conf.object_conf.object_cfgstats[0].effect.beam),                   0, LONG_MIN,ULONG_MAX, NULL,                        value_effOrEffEl,assign_default},
    {"EFFECTPARTICLE",           0, field(game.conf.object_conf.object_cfgstats[0].effect.particle),               0, LONG_MIN,ULONG_MAX, NULL,                        value_effOrEffEl,assign_default},
    {"EFFECTEXPLOSION1",         0, field(game.conf.object_conf.object_cfgstats[0].effect.explosion1),             0, LONG_MIN,ULONG_MAX, NULL,                        value_effOrEffEl,assign_default},
    {"EFFECTEXPLOSION2",         0, field(game.conf.object_conf.object_cfgstats[0].effect.explosion2),             0, LONG_MIN,ULONG_MAX, NULL,                        value_effOrEffEl,assign_default},
    {"EFFECTSPACING",            0, field(game.conf.object_conf.object_cfgstats[0].effect.spacing),                0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"EFFECTSOUND",              0, field(game.conf.object_conf.object_cfgstats[0].effect.sound_idx),              0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"EFFECTSOUND",              1, field(game.conf.object_conf.object_cfgstats[0].effect.sound_range),            0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"FLAMEANIMATIONID",         0, field(game.conf.object_conf.object_cfgstats[0].flame.animation_id),            0, LONG_MIN,ULONG_MAX, NULL,                        value_animid,    assign_animid},
    {"FLAMEANIMATIONSPEED",      0, field(game.conf.object_conf.object_cfgstats[0].flame.anim_speed),              0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"FLAMEANIMATIONSIZE",       0, field(game.conf.object_conf.object_cfgstats[0].flame.sprite_size),             0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"FLAMEANIMATIONOFFSET",     0, field(game.conf.object_conf.object_cfgstats[0].flame.fp_add_x),                0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"FLAMEANIMATIONOFFSET",     1, field(game.conf.object_conf.object_cfgstats[0].flame.fp_add_y),                0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"FLAMEANIMATIONOFFSET",     2, field(game.conf.object_conf.object_cfgstats[0].flame.td_add_x),                0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"FLAMEANIMATIONOFFSET",     3, field(game.conf.object_conf.object_cfgstats[0].flame.td_add_y),                0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {"FLAMETRANSPARENCYFLAGS",   0, field(game.conf.object_conf.object_cfgstats[0].flame.transparency_flags),      0, LONG_MIN,ULONG_MAX, NULL,                        value_transpflg, assign_default},
    {"LIGHTFLAGS",               0, field(game.conf.object_conf.object_cfgstats[0].ilght.flags),                   0, LONG_MIN,ULONG_MAX, NULL,                        value_default,   assign_default},
    {NULL},
};

const struct NamedFieldSet objects_named_fields_set = {
    &game.conf.object_conf.object_types_count,
    "object",
    objects_named_fields,
    object_desc,
    OBJECT_TYPES_MAX,
    sizeof(game.conf.object_conf.object_cfgstats[0]),
    game.conf.object_conf.object_cfgstats,
};

/******************************************************************************/
struct ObjectConfigStats *get_object_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= game.conf.object_conf.object_types_count)
        return &game.conf.object_conf.object_cfgstats[0];
    return &game.conf.object_conf.object_cfgstats[tngmodel];
}

ThingClass crate_to_workshop_item_class(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= game.conf.object_conf.object_types_count))
        return game.conf.object_conf.workshop_object_class[0];
    return game.conf.object_conf.workshop_object_class[tngmodel];
}

ThingModel crate_to_workshop_item_model(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= game.conf.object_conf.object_types_count))
        return game.conf.object_conf.object_to_door_or_trap[0];
    return game.conf.object_conf.object_to_door_or_trap[tngmodel];
}

ThingClass crate_thing_to_workshop_item_class(const struct Thing *thing)
{
    if (!thing_is_workshop_crate(thing))
        return thing->class_id;
    ThingModel tngmodel = thing->model;
    if ((tngmodel <= 0) || (tngmodel >= game.conf.object_conf.object_types_count))
        return game.conf.object_conf.workshop_object_class[0];
    return game.conf.object_conf.workshop_object_class[tngmodel];
}

ThingModel crate_thing_to_workshop_item_model(const struct Thing *thing)
{
    if (thing_is_invalid(thing) || (thing->class_id != TCls_Object))
        return game.conf.object_conf.object_to_door_or_trap[0];
    ThingModel tngmodel = thing->model;
    if ((tngmodel <= 0) || (tngmodel >= game.conf.object_conf.object_types_count))
        return game.conf.object_conf.object_to_door_or_trap[0];
    return game.conf.object_conf.object_to_door_or_trap[tngmodel];
}

static TbBool load_objects_config_file(const char *fname, unsigned short flags)
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
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    
    parse_named_field_blocks(buf, len, fname, flags, &objects_named_fields_set);
    //Freeing and exiting
    free(buf);
    return true;
}

void update_all_objects_of_model(ThingModel model)
{
    const struct StructureList* slist = get_list_for_thing_class(TCls_Object);
    struct ObjectConfigStats* objst = get_object_model_stats(model);
    struct Dungeon* dungeon;
    for (int i = slist->index; i > 0;)
    {
        struct Thing* thing = thing_get(i);
        i = thing->next_of_class;
        if (thing->model != model)
        {
            continue;
        }
        TRACE_THING(thing);
        int start_frame = 0;
        if(objst->random_start_frame)
        {
            start_frame = -1;
        }
        set_thing_draw(thing, objst->sprite_anim_idx, objst->anim_speed, objst->sprite_size_max, 0, start_frame, objst->draw_class);
        // TODO: Should we rotate this on per-object basis?
        thing->flags = 0;
        thing->flags |= objst->rotation_flag << TAF_ROTATED_SHIFT;

        if (thing->owner != game.neutral_player_num)
        {
            dungeon = get_dungeon(thing->owner);
            if ((thing_is_dungeon_heart(thing)) && (thing->index != dungeon->dnheart_idx))
            {
                if (dungeon->backup_heart_idx == 0)
                {
                    dungeon->backup_heart_idx = thing->index;
                }
            }
        }

        
        if (thing->light_id != 0)
        {
            light_delete_light(thing->light_id);
        }
        if (objst->ilght.radius != 0)
        {
            struct InitLight ilight;
            memset(&ilight, 0, sizeof(struct InitLight));
            memcpy(&ilight.mappos, &thing->mappos, sizeof(struct Coord3d));
            ilight.radius = objst->ilght.radius;
            ilight.intensity = objst->ilght.intensity;
            ilight.flags = objst->ilght.flags;
            ilight.is_dynamic = objst->ilght.is_dynamic;
            thing->light_id = light_create_light(&ilight);
        }
    }
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
    for (int i = 0; i < game.conf.object_conf.object_types_count; ++i)
    {
        if (strncasecmp(game.conf.object_conf.object_cfgstats[i].code_name, code_name,
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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
