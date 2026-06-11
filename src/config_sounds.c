/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_sounds.c
 *     Sound name configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for sound name→ID mappings.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     14 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_sounds.h"
#include "config.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "sound_manager.h"
#include "gui_soundmsgs.h"
#include "globals.h"
#include "game_legacy.h"
#include <ctype.h>

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define MIN_CONFIG_FILE_SIZE 4

static TbBool load_sounds_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_sounds_file_data = {
    .filename = "sounds.cfg",
    .load_func = load_sounds_config_file,
    .pre_load_func = NULL,
    .post_load_func = cache_common_sound_ids,
};

/******************************************************************************/
// Cached common sound IDs
/******************************************************************************/

int            snd_gold_pickup_count = 3;
int            snd_door_place_count = 3;
int            snd_tunnel_dig_count = 3;
int            snd_dig_spell_count  = 6;
int            snd_dig_impact_count = 3;
int            snd_foot_spur_count = 4;
int            snd_foot_wet_count  = 4;
int            snd_foot_snow_count = 3;
int            snd_chicken_cluck_count = 3;
int            snd_strike_wall_count = 3;
int            snd_reinforce_hit_count = 7;

// Verified
SoundSmplTblID snd_foot_spur       = 5;    // spur1.wav
SoundSmplTblID snd_foot_wet        = 21;   // footwet1.wav
SoundSmplTblID snd_insect_fly      = 25;   // creature_insect/fly.wav

SoundSmplTblID snd_gold_pickup     = 32;
SoundSmplTblID snd_salary_tiny     = 32;
SoundSmplTblID snd_salary_partial  = 33;
SoundSmplTblID snd_salary_full     = 34;
SoundSmplTblID snd_splash          = 36;  
SoundSmplTblID snd_spell_wall      = 41;


SoundSmplTblID snd_spell_armageddon = 180;

SoundSmplTblID snd_explode         = 47;
SoundSmplTblID snd_spell_frozen    = 50;
SoundSmplTblID snd_tab_click       = 60;
SoundSmplTblID snd_button_click    = 61; 
SoundSmplTblID snd_dig_spell       = 63;
SoundSmplTblID snd_tunnel_dig      = 69;   // dig6.wav
SoundSmplTblID snd_door_place      = 72;   // rocks1.wav, rocks2.wav, rocks3.wav
SoundSmplTblID snd_dig_impact      = 72;   // rocks1.wav, rocks2.wav, rocks3.wav
SoundSmplTblID snd_dig_dirt        = 73;  
SoundSmplTblID snd_spell_stars     = 76;
SoundSmplTblID snd_tile_place     = 77;
// strikes/swonarm3.wav, swonarm4.wav, swonarm5.wav

SoundSmplTblID snd_coin_drop       = 88;
SoundSmplTblID snd_buzzer          = 89;
SoundSmplTblID snd_alarm           = 90;
SoundSmplTblID snd_door_open       = 91; 
SoundSmplTblID snd_door_close      = 92;
SoundSmplTblID snd_heart_engine    = 93;

SoundSmplTblID snd_chicken_cluck   = 112;  // chick4a.wav
SoundSmplTblID snd_tile_sell       = 115;  
SoundSmplTblID snd_room_claim      = 116;
SoundSmplTblID snd_trap_place      = 117;  // PlaceSound default for all traps and doors in trapdoor.cfg
SoundSmplTblID snd_tile_dig        = 118;
SoundSmplTblID snd_refusal         = 119;
SoundSmplTblID snd_strike_wall     = 128;  // strikes/swonarm3.wav, swonarm4.wav, swonarm5.wav

SoundSmplTblID snd_heart_beat_down = 150;
SoundSmplTblID snd_heart_beat_up   = 151;
SoundSmplTblID snd_scavenge        = 156;
SoundSmplTblID snd_cheat_activated = 159;
SoundSmplTblID snd_tab_hit         = 175;  // tabhit1.wav, tabhit2.wav, tabhit3.wav
SoundSmplTblID snd_foot_snow       = 182;  // footsteps/snowft1.wav, snowft2.wav, snowft3.wav

SoundSmplTblID snd_larg_tile_up = 856;
SoundSmplTblID snd_larg_tile_down = 959;

SoundSmplTblID snd_tab_fall        = 947;
SoundSmplTblID snd_reinforce_hit   = 1005;

// Trap trigger sounds
SoundSmplTblID snd_trap_trigger    = 176;  // TriggerSound for BOULDER, ALARM, POISON_GAS, LIGHTNING, WORD_OF_POWER, LAVA
SoundSmplTblID snd_trap_trigger_tnt = 141; // TriggerSound for TNT trap

// Object ambient / effect sounds
SoundSmplTblID snd_torch_ambience       = 78;   // AmbienceSound for TORCH, TEMPLE_STATUE, TORCHUN, CANDLESTCK
SoundSmplTblID snd_dungeon_heart_beam   = 157;  // EffectSound for SOUL_CONTAINER destruction beam
SoundSmplTblID snd_hero_gate_ambience   = 973;  // AmbienceSound for HERO_GATE

// Keeper power incantation voices
SoundSmplTblID snd_power_protect         = 825;
SoundSmplTblID snd_power_call_to_arms  = 826;
SoundSmplTblID snd_power_chicken       = 827;
SoundSmplTblID snd_power_sight         = 828;
SoundSmplTblID snd_power_heal          = 829;
SoundSmplTblID snd_power_hold_audience = 830;
SoundSmplTblID snd_power_imp           = 831;
SoundSmplTblID snd_power_conceal       = 832;
SoundSmplTblID snd_power_lightning     = 833;
SoundSmplTblID snd_power_obey          = 834;
SoundSmplTblID snd_power_disease       = 835;
SoundSmplTblID snd_power_possess       = 836;
SoundSmplTblID snd_power_cave_in       = 837;
SoundSmplTblID snd_power_speed         = 838;
SoundSmplTblID snd_power_destroy_walls = 839;
SoundSmplTblID snd_power_armageddon    = 824;

// Shot projectile travel sounds (ShotSound field in magic.cfg)
SoundSmplTblID snd_shot_freeze            = 49;  // travel; ShotSound for SHOT_FREEZE and SHOT_SLOW
SoundSmplTblID snd_shot_homing_missile    = 53;  // travel; ShotSound for SHOT_NAVI_MISSILE and SHOT_MISSILE
SoundSmplTblID snd_shot_bouncing_grenade  = 54;  // travel; ShotSound for SHOT_GRENADE and SHOT_LIZARD

// Shot impact sounds
SoundSmplTblID snd_shot_freeze_impact     = 50;  // impact; creature frozen effect (same ID as snd_spell_frozen)
SoundSmplTblID snd_shot_splat             = 57;  // impact; generic splat on hit

// Power cast sounds (SoundPlayed field in magic.cfg — played when the power takes effect)
SoundSmplTblID snd_cast_heal              = 37;  // cast; SoundPlayed for POWER_HEAL_CREATURE
SoundSmplTblID snd_cast_speed             = 38;  // cast; SoundPlayed for POWER_SPEED
SoundSmplTblID snd_cast_lightning         = 55;  // cast; SoundPlayed for POWER_LIGHTNING
SoundSmplTblID snd_cast_disease           = 59;  // cast; SoundPlayed for POWER_DISEASE (disease spit on target)

// Shot firing sounds (FiringSound field in magic.cfg — played on the creature at the moment of release)
SoundSmplTblID snd_shot_fire              = 46;  // firing; FiringSound for SHOT_FIREBALL, SHOT_FIREBOMB, SHOT_POISON_CLOUD, SHOT_DRAIN, SHOT_GROUP, SHOT_CHICKEN, SHOT_TIME_BOMB, SHOT_HAILSTORM
SoundSmplTblID snd_shot_bow               = 44;  // firing; FiringSound for SHOT_ARROW and SHOT_BALLISTA
SoundSmplTblID snd_shot_wind              = 40;  // firing; FiringSound for SHOT_WIND
SoundSmplTblID snd_shot_breath            = 56;  // firing; FiringSound for SHOT_FLAME_BREATH
SoundSmplTblID snd_shot_freeze_fire       = 48;  // firing; FiringSound for SHOT_FREEZE and SHOT_SLOW


/******************************************************************************/
// Speech overrides
/******************************************************************************/

/** Per-message speech override paths.  Indexed by SMsg_* enum value.
 *  Empty string = use default speech bank sample. */
char g_speech_overrides[SMsg_MAX][512];

/** NamedCommand table mapping SMsg_* name strings to enum values. */
static const struct NamedCommand speech_desc[] = {
    {"CreatrAngryAnyReason", SMsg_CreatrAngryAnyReason},
    {"CreatrAngryNoLair",    SMsg_CreatrAngryNoLair},
    {"CreatrAngryNotPaid",   SMsg_CreatrAngryNotPaid},
    {"CreatrAngryNoFood",    SMsg_CreatrAngryNoFood},
    {"CreatrDestroyRooms",   SMsg_CreatrDestroyRooms},
    {"CreatureLeaving",      SMsg_CreatureLeaving},
    {"WallsBreach",          SMsg_WallsBreach},
    {"HeartUnderAttack",     SMsg_HeartUnderAttack},
    {"BattleDefeat",         SMsg_BattleDefeat},
    {"BattleVictory",        SMsg_BattleVictory},
    {"BattleDeath",          SMsg_BattleDeath},
    {"BattleWon",            SMsg_BattleWon},
    {"CreatureDefending",    SMsg_CreatureDefending},
    {"CreatureAttacking",    SMsg_CreatureAttacking},
    {"EnemyDestroyRooms",    SMsg_EnemyDestroyRooms},
    {"EnemyClaimGround",     SMsg_EnemyClaimGround},
    {"EnemyRoomTakeOver",    SMsg_EnemyRoomTakeOver},
    {"NewRoomTakenOver",     SMsg_NewRoomTakenOver},
    {"LordOfLandComming",    SMsg_LordOfLandComming},
    {"FingthingFriends",     SMsg_FingthingFriends},
    {"BattleOver",           SMsg_BattleOver},
    {"GardenTooSmall",       SMsg_GardenTooSmall},
    {"LairTooSmall",         SMsg_LairTooSmall},
    {"TreasuryTooSmall",     SMsg_TreasuryTooSmall},
    {"LibraryTooSmall",      SMsg_LibraryTooSmall},
    {"PrisonTooSmall",       SMsg_PrisonTooSmall},
    {"TortureTooSmall",      SMsg_TortureTooSmall},
    {"TrainingTooSmall",     SMsg_TrainingTooSmall},
    {"WorkshopTooSmall",     SMsg_WorkshopTooSmall},
    {"ScavengeTooSmall",     SMsg_ScavengeTooSmall},
    {"TempleTooSmall",       SMsg_TempleTooSmall},
    {"GraveyardTooSmall",    SMsg_GraveyardTooSmall},
    {"BarracksTooSmall",     SMsg_BarracksTooSmall},
    {"NoRouteToGarden",      SMsg_NoRouteToGarden},
    {"NoRouteToTreasury",    SMsg_NoRouteToTreasury},
    {"NoRouteToLair",        SMsg_NoRouteToLair},
    {"EntranceClaimed",      SMsg_EntranceClaimed},
    {"EntranceLost",         SMsg_EntranceLost},
    {"RoomTreasrNeeded",     SMsg_RoomTreasrNeeded},
    {"RoomLairNeeded",       SMsg_RoomLairNeeded},
    {"RoomGardenNeeded",     SMsg_RoomGardenNeeded},
    {"ResearchedRoom",       SMsg_ResearchedRoom},
    {"ResearchedSpell",      SMsg_ResearchedSpell},
    {"ManufacturedDoor",     SMsg_ManufacturedDoor},
    {"ManufacturedTrap",     SMsg_ManufacturedTrap},
    {"NoMoreReseach",        SMsg_NoMoreReseach},
    {"SpellbookTaken",       SMsg_SpellbookTaken},
    {"TrapTaken",            SMsg_TrapTaken},
    {"DoorTaken",            SMsg_DoorTaken},
    {"SpellbookStolen",      SMsg_SpellbookStolen},
    {"TrapStolen",           SMsg_TrapStolen},
    {"DoorStolen",           SMsg_DoorStolen},
    {"TortureInformation",   SMsg_TortureInformation},
    {"TortureConverted",     SMsg_TortureConverted},
    {"PrisonMadeSkeleton",   SMsg_PrisonMadeSkeleton},
    {"TortureMadeGhost",     SMsg_TortureMadeGhost},
    {"PrisonersEscaping",    SMsg_PrisonersEscaping},
    {"GraveyardMadeVampire", SMsg_GraveyardMadeVampire},
    {"CreatrFreedPrison",    SMsg_CreatrFreedPrison},
    {"PrisonersStarving",    SMsg_PrisonersStarving},
    {"CreatureScanvenged",   SMsg_CreatureScanvenged},
    {"MinionScanvenged",     SMsg_MinionScanvenged},
    {"CreatureJoinedEnemy",  SMsg_CreatureJoinedEnemy},
    {"CreatureRevealInfo",   SMsg_CreatureRevealInfo},
    {"SacrificeGood",        SMsg_SacrificeGood},
    {"SacrificeReward",      SMsg_SacrificeReward},
    {"SacrificeNeutral",     SMsg_SacrificeNeutral},
    {"SacrificeBad",         SMsg_SacrificeBad},
    {"SacrificePunish",      SMsg_SacrificePunish},
    {"SacrificeWishing",     SMsg_SacrificeWishing},
    {"DiscoveredSpecial",    SMsg_DiscoveredSpecial},
    {"DiscoveredSpell",      SMsg_DiscoveredSpell},
    {"DiscoveredDoor",       SMsg_DiscoveredDoor},
    {"DiscoveredTrap",       SMsg_DiscoveredTrap},
    {"CreaturesJoinedYou",   SMsg_CreaturesJoinedYou},
    {"DugIntoNewArea",       SMsg_DugIntoNewArea},
    {"SpecRevealMap",        SMsg_SpecRevealMap},
    {"SpecResurrect",        SMsg_SpecResurrect},
    {"SpecTransfer",         SMsg_SpecTransfer},
    {"CommonAcknowledge",    SMsg_CommonAcknowledge},
    {"SpecHeroStolen",       SMsg_SpecHeroStolen},
    {"SpecCreatrDoubled",    SMsg_SpecCreatrDoubled},
    {"SpecIncLevel",         SMsg_SpecIncLevel},
    {"SpecWallsFortify",     SMsg_SpecWallsFortify},
    {"SpecHiddenWorld",      SMsg_SpecHiddenWorld},
    {"GoldLow",              SMsg_GoldLow},
    {"GoldNotEnough",        SMsg_GoldNotEnough},
    {"NoGoldToScavenge",     SMsg_NoGoldToScavenge},
    {"NoGoldToTrain",        SMsg_NoGoldToTrain},
    {"Payday",               SMsg_Payday},
    {"FullOfPies",           SMsg_FullOfPies},
    {"SurrealHappen",        SMsg_SurrealHappen},
    {"StrangeAccent",        SMsg_StrangeAccent},
    {"PantsTooTight",        SMsg_PantsTooTight},
    {"CraveChocolate",       SMsg_CraveChocolate},
    {"SmellAgain",           SMsg_SmellAgain},
    {"Hello",                SMsg_Hello},
    {"Glaagh",               SMsg_Glaagh},
    {"Achew",                SMsg_Achew},
    {"Chgreche",             SMsg_Chgreche},
    {"WorkerJobsLimit",      SMsg_WorkerJobsLimit},
    {"GameLoaded",           SMsg_GameLoaded},
    {"GameSaved",            SMsg_GameSaved},
    {"DefeatedKeeper",       SMsg_DefeatedKeeper},
    {"LevelFailed",          SMsg_LevelFailed},
    {"LevelWon",             SMsg_LevelWon},
    {"SenceAvatar",          SMsg_SenceAvatar},
    {"AvatarBodyVanish",     SMsg_AvatarBodyVanish},
    {"GameFinalVictory",     SMsg_GameFinalVictory},
    {NULL, 0},
};

/**
 * @brief Parse a [speech] block, mapping SMsg_* names to override file paths.
 *
 * Each line has the form:
 *   GraveyardMadeVampire = speech/custom_vampire.ogg
 *
 * Paths are resolved at playback time, searching campaign configs, levels, media (MEDIA_LOCATION),
 * and game root in that order. Language-specific variants are tried first (e.g. "eng/file.wav").
 */
static TbBool parse_speech_section(char* buf, long len, const char* config_textname,
                                   unsigned short flags)
{
    int32_t pos = 0;
    const char* blockname = NULL;
    int blocknamelen = 0;
    TbBool found_section = false;

    while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
    {
        if (blocknamelen > 0 && strncasecmp(blockname, "speech", blocknamelen) == 0)
        {
            found_section = true;
            break;
        }
    }

    if (!found_section)
        return true;

    while (pos < len)
    {
        if (buf[pos] == '[') break;

        if (buf[pos] == '\n' || buf[pos] == '\r') { pos++; continue; }

        if (buf[pos] == ';' || buf[pos] == '#')
        {
            while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
            continue;
        }

        char name_buf[COMMAND_WORD_LEN];
        char path_buf[512];

        if (get_conf_parameter_single(buf, &pos, len, name_buf, sizeof(name_buf)) <= 0)
        {
            while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
            while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r')) pos++;
            continue;
        }

        long smsg_id = get_id(speech_desc, name_buf);
        if (smsg_id <= 0 || smsg_id >= SMsg_MAX)
        {
            if (smsg_id < 0)
                WARNLOG("Unknown speech message name '%s' in %s", name_buf, config_textname);
            while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
            while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r')) pos++;
            continue;
        }

        /* Skip '=' separator (same logic as parse_sound_line) */
        if (get_conf_parameter_single(buf, &pos, len, path_buf, sizeof(path_buf)) <= 0)
        {
            while (pos < len && (buf[pos] == '=' || buf[pos] == ' ' || buf[pos] == '\t')) pos++;
            if (get_conf_parameter_single(buf, &pos, len, path_buf, sizeof(path_buf)) <= 0)
            {
                WARNLOG("Missing path for speech message '%s' in %s", name_buf, config_textname);
                while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
                while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r')) pos++;
                continue;
            }
        }
        if (path_buf[0] == '=')
        {
            if (get_conf_parameter_single(buf, &pos, len, path_buf, sizeof(path_buf)) <= 0)
            {
                WARNLOG("Missing path after '=' for speech message '%s' in %s", name_buf, config_textname);
                while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
                while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r')) pos++;
                continue;
            }
        }

        snprintf(g_speech_overrides[smsg_id], sizeof(g_speech_overrides[smsg_id]), "%s", path_buf);
        SYNCDBG(8, "Speech override: %s (%ld) -> %s", name_buf, smsg_id, path_buf);

        while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
        while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r')) pos++;
    }

    return true;
}

/**
 * @brief Parse a single line in the format: NAME = ID [count]
 * 
 * Examples:
 *   REFUSAL = 119
 *   GOLD_PICKUP = 32 3    ; IDs 32, 33, 34
 *   CUSTOM_SOUND = sounds/explosion.wav
 */
static TbBool parse_sound_line(const char* buf, int32_t* pos, long len, const char* config_textname)
{
    char name_buf[COMMAND_WORD_LEN];
    char value_buf[COMMAND_WORD_LEN];
    char count_buf[COMMAND_WORD_LEN];
    
    // Get the sound name
    if (get_conf_parameter_single(buf, pos, len, name_buf, sizeof(name_buf)) <= 0)
    {
        return false;  // Empty line or comment
    }

    // Normalize to uppercase so config names are case-insensitive
    for (int i = 0; name_buf[i] != '\0'; i++)
        name_buf[i] = (char)toupper((unsigned char)name_buf[i]);
    
    // Skip if it's a section header or comment
    if (name_buf[0] == '[' || name_buf[0] == ';' || name_buf[0] == '#')
    {
        return false;
    }
    
    // Expect '=' separator - check if next non-whitespace is '='
    // The get_conf_parameter_single function should handle this, but let's verify
    // Actually, the format is "NAME = VALUE" so we need to handle the '=' specially
    
    // Get the value (ID or filepath)
    if (get_conf_parameter_single(buf, pos, len, value_buf, sizeof(value_buf)) <= 0)
    {
        // Skip the '=' if present
        while (*pos < len && (buf[*pos] == '=' || buf[*pos] == ' ' || buf[*pos] == '\t'))
        {
            (*pos)++;
        }
        
        if (get_conf_parameter_single(buf, pos, len, value_buf, sizeof(value_buf)) <= 0)
        {
            WARNLOG("Sound '%s' has no value in %s", name_buf, config_textname);
            return false;
        }
    }
    
    // Handle '=' that might be captured as part of value
    if (value_buf[0] == '=')
    {
        // The '=' was captured, get the actual value
        if (get_conf_parameter_single(buf, pos, len, value_buf, sizeof(value_buf)) <= 0)
        {
            WARNLOG("Sound '%s' has no value after '=' in %s", name_buf, config_textname);
            return false;
        }
    }
    
    // Try to get optional count parameter
    int count = 1;
    if (get_conf_parameter_single(buf, pos, len, count_buf, sizeof(count_buf)) > 0)
    {
        int parsed_count = atoi(count_buf);
        if (parsed_count > 0)
        {
            count = parsed_count;
        }
    }
    
    // Determine if value is numeric ID or filepath
    SoundSmplTblID sample_id;
    char* endptr;
    long id_value = strtol(value_buf, &endptr, 10);
    
    if (*endptr == '\0' && id_value > 0)
    {
        // Numeric ID - register directly
        sample_id = (SoundSmplTblID)id_value;
        
        if (!sound_manager_register(name_buf, sample_id, count))
        {
            WARNLOG("Failed to register sound '%s' with ID %d in %s", 
                    name_buf, sample_id, config_textname);
            return false;
        }
        
        SYNCDBG(8, "Registered sound '%s' -> ID %d (count %d)", name_buf, sample_id, count);
    }
    else
    {
        // Filepath - load as custom sound and register the name
        SoundSmplTblID id = sound_manager_load_named_sound(name_buf, value_buf, count);
        if (id <= 0)
        {
            WARNLOG("Failed to load custom sound '%s' from '%s' in %s",
                    name_buf, value_buf, config_textname);
            return false;
        }
        SYNCDBG(8, "Registered custom sound '%s' -> ID %d (file '%s', count %d)",
                name_buf, id, value_buf, count);
    }
    
    return true;
}

/**
 * @brief Parse a [section] block (like [common], [ui], [creatures])
 */
static TbBool parse_sounds_section(char* buf, long len, const char* config_textname, 
                                   unsigned short flags, const char* section_name)
{
    int32_t pos = 0;
    const char* blockname = NULL;
    int blocknamelen = 0;
    TbBool found_section = false;
    
    // Find the requested section
    while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
    {
        if (blocknamelen > 0 && strncasecmp(blockname, section_name, blocknamelen) == 0)
        {
            found_section = true;
            break;
        }
    }
    
    if (!found_section)
    {
        // Section not found is not an error - it's optional
        return true;
    }
    
    // Parse lines until next section
    while (pos < len)
    {
        // Check if we've hit another section
        if (buf[pos] == '[')
        {
            break;
        }
        
        // Skip empty lines and comments
        if (buf[pos] == '\n' || buf[pos] == '\r')
        {
            pos++;
            continue;
        }
        
        if (buf[pos] == ';' || buf[pos] == '#')
        {
            // Skip to end of line
            while (pos < len && buf[pos] != '\n' && buf[pos] != '\r')
            {
                pos++;
            }
            continue;
        }
        
        // Try to parse as sound definition
        parse_sound_line(buf, &pos, len, config_textname);
        
        // Move to next line
        while (pos < len && buf[pos] != '\n' && buf[pos] != '\r')
        {
            pos++;
        }
        while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r'))
        {
            pos++;
        }
    }
    
    return true;
}

/**
 * @brief Main loader for sounds.cfg
 */
static TbBool load_sounds_config_file(const char *fname, unsigned short flags)
{
    SYNCDBG(0, "%s file \"%s\".", ((flags & CnfLd_ListOnly) == 0) ? "Reading" : "Parsing", fname);
    
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("File \"%s\" doesn't exist or is too small.", fname);
        return false;
    }
    
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
    {
        ERRORLOG("Cannot allocate memory for %s", fname);
        return false;
    }
    
    // Load file data
    len = LbFileLoadAt(fname, buf);
    if (len <= 0)
    {
        free(buf);
        ERRORLOG("Cannot load file %s", fname);
        return false;
    }
    
    TbBool result = true;
    
    // Parse known sections
    const char* sections[] = {"common", "ui", "traps", "creatures", "powers", "effects", "doors", "objects", NULL};
    
    for (int i = 0; sections[i] != NULL; i++)
    {
        if (!parse_sounds_section(buf, len, fname, flags, sections[i]))
        {
            result = false;
        }
    }

    parse_speech_section(buf, len, fname, flags);
    
    free(buf);
    return result;
}

/******************************************************************************/
// Public API
/******************************************************************************/

TbBool load_sounds_config(void)
{
    return load_config(&keeper_sounds_file_data, CnfLd_Standard);
}

TbBool load_campaign_sounds_config(const char* levels_location)
{
    if (levels_location == NULL || levels_location[0] == '\0')
    {
        SYNCDBG(7,"load_campaign_sounds_config: no location provided");
        return false;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/sounds.cfg", levels_location);

    const char* fullpath = prepare_file_path(FGrp_Main, filepath);
    SYNCDBG(7,"load_campaign_sounds_config: dir='%s' -> full='%s'", levels_location, fullpath ? fullpath : "(null)");
    if (fullpath == NULL)
    {
        return false;
    }

    TbBool result = load_sounds_config_file(fullpath, CnfLd_Standard | CnfLd_IgnoreErrors);
    cache_common_sound_ids();
    return result;
}

TbBool load_mod_sounds_config(const char* mod_name)
{
    if (mod_name == NULL || mod_name[0] == '\0')
    {
        return false;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "mods/%s/sounds.cfg", mod_name);

    const char* fullpath = prepare_file_path(FGrp_Main, filepath);
    if (fullpath == NULL)
    {
        return false;
    }

    TbBool result = load_sounds_config_file(fullpath, CnfLd_Standard | CnfLd_IgnoreErrors);
    cache_common_sound_ids();
    return result;
}

TbBool load_level_sounds_config(const char* level_name)
{
    if (level_name == NULL || level_name[0] == '\0')
    {
        return false;
    }
    
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "levels/%s/sounds.cfg", level_name);
    
    const char* fullpath = prepare_file_path(FGrp_Main, filepath);
    if (fullpath == NULL)
    {
        return false;
    }
    
    TbBool result = load_sounds_config_file(fullpath, CnfLd_Standard | CnfLd_IgnoreErrors);
    cache_common_sound_ids();
    return result;
}

SoundSmplTblID get_sound_id(const char* name)
{
    return sound_manager_get_id(name);
}

TbBool is_sound_registered(const char* name)
{
    return sound_manager_is_registered(name);
}

TbBool cache_common_sound_ids(void)
{
    SoundSmplTblID id;
    int count;

    #define CACHE_SND(var, name)           id = sound_manager_get_id(name); if (id > 0) { SYNCDBG(7,"cache_common_sound_ids: %s -> %d (was %d)", name, id, var); var = id; }
    #define CACHE_SND_COUNT(var, cvar, nm) id = sound_manager_get_id(nm);   if (id > 0) { var = id; count = sound_manager_get_count(nm); if (count > 0) cvar = count; }

    CACHE_SND(snd_refusal,         "REFUSAL")
    CACHE_SND(snd_tab_click,       "TAB_CLICK")
    CACHE_SND(snd_room_claim,      "ROOM_CLAIM")
    CACHE_SND_COUNT(snd_gold_pickup, snd_gold_pickup_count, "GOLD_PICKUP")
    CACHE_SND(snd_salary_full,     "SALARY_FULL")
    CACHE_SND(snd_salary_partial,  "SALARY_PARTIAL")
    CACHE_SND(snd_salary_tiny,     "SALARY_NONE")
    CACHE_SND(snd_heart_beat_down, "HEART_BEAT_DOWN")
    CACHE_SND(snd_heart_beat_up,   "HEART_BEAT_UP")
    CACHE_SND(snd_door_open,       "DOOR_OPEN")
    CACHE_SND(snd_door_close,      "DOOR_CLOSE")
    CACHE_SND_COUNT(snd_door_place,  snd_door_place_count,  "DOOR_PLACE")
    CACHE_SND_COUNT(snd_dig_impact,  snd_dig_impact_count,  "DIG_IMPACT")
    CACHE_SND(snd_dig_dirt,        "DIG_DIRT")

    CACHE_SND_COUNT(snd_foot_spur,   snd_foot_spur_count,   "FOOT_SPUR")
    CACHE_SND_COUNT(snd_foot_wet,    snd_foot_wet_count,    "FOOT_WET")
    CACHE_SND_COUNT(snd_foot_snow,   snd_foot_snow_count,   "FOOT_SNOW")
    CACHE_SND(snd_insect_fly,      "INSECT_FLY")
    CACHE_SND_COUNT(snd_chicken_cluck, snd_chicken_cluck_count, "CHICKEN_CLUCK")
    CACHE_SND(snd_splash,          "SPLASH")
    CACHE_SND(snd_explode,         "EXPLODE")
    CACHE_SND_COUNT(snd_strike_wall, snd_strike_wall_count, "STRIKE_WALL")
    CACHE_SND_COUNT(snd_reinforce_hit, snd_reinforce_hit_count, "REINFORCE_HIT")
    CACHE_SND(snd_spell_wall,      "SPELL_WALL")
    CACHE_SND(snd_spell_frozen,    "SPELL_FROZEN")
    CACHE_SND(snd_spell_stars,     "SPELL_STARS")
    CACHE_SND(snd_spell_armageddon, "SPELL_ARMAGEDDON")
    CACHE_SND_COUNT(snd_dig_spell,   snd_dig_spell_count,   "DIG_SPELL")
    CACHE_SND_COUNT(snd_tunnel_dig,  snd_tunnel_dig_count,  "TUNNEL_DIG")
    CACHE_SND(snd_button_click,   "BUTTON_CLICK2")
    CACHE_SND(snd_buzzer,          "BUZZER")
    CACHE_SND(snd_tab_fall,        "TAB_FALL")
    CACHE_SND(snd_heart_engine,    "HEART_ENGINE")
    CACHE_SND(snd_scavenge,        "SCAVENGE")
    CACHE_SND(snd_tile_place,      "ROOM_BUILD")
    CACHE_SND(snd_coin_drop,       "COIN_DROP")
    CACHE_SND(snd_alarm,           "TRAP_TRIGGER_ALARM")
    CACHE_SND(snd_trap_place,      "TRAP_PLACE")
    CACHE_SND(snd_trap_trigger,    "TRAP_TRIGGER")
    CACHE_SND(snd_trap_trigger_tnt, "TRAP_TRIGGER_TNT")
    CACHE_SND(snd_torch_ambience,       "TORCH_AMBIENCE")
    CACHE_SND(snd_dungeon_heart_beam,   "DUNGEON_HEART_BEAM")
    CACHE_SND(snd_hero_gate_ambience,   "HERO_GATE_AMBIENCE")
    CACHE_SND(snd_tile_sell,       "TILE_SELL")
    CACHE_SND(snd_tile_dig,        "TILE_DIG")
    CACHE_SND(snd_cheat_activated, "CHEAT_ACTIVATED")
    CACHE_SND(snd_tab_hit,         "TAB_HIT")
    CACHE_SND(snd_larg_tile_up,    "LARG_TILE_UP")
    CACHE_SND(snd_larg_tile_down,  "LARG_TILE_DOWN") 
    CACHE_SND(snd_power_protect,         "POWER_PROTECT")
    CACHE_SND(snd_power_call_to_arms,  "POWER_CALL_TO_ARMS")
    CACHE_SND(snd_power_chicken,       "POWER_CHICKEN")
    CACHE_SND(snd_power_sight,         "POWER_SIGHT")
    CACHE_SND(snd_power_heal,          "POWER_HEAL")
    CACHE_SND(snd_power_hold_audience, "POWER_HOLD_AUDIENCE")
    CACHE_SND(snd_power_imp,           "POWER_IMP")
    CACHE_SND(snd_power_conceal,       "POWER_CONCEAL")
    CACHE_SND(snd_power_lightning,     "POWER_LIGHTNING")
    CACHE_SND(snd_power_obey,          "POWER_OBEY")
    CACHE_SND(snd_power_disease,       "POWER_DISEASE")
    CACHE_SND(snd_power_possess,       "POWER_POSSESS")
    CACHE_SND(snd_power_cave_in,       "POWER_CAVE_IN")
    CACHE_SND(snd_power_speed,         "POWER_SPEED")
    CACHE_SND(snd_power_destroy_walls, "POWER_DESTROY_WALLS")
    CACHE_SND(snd_power_armageddon,    "POWER_ARMAGEDDON")
    CACHE_SND(snd_shot_freeze,           "SHOT_FREEZE")
    CACHE_SND(snd_shot_homing_missile,   "SHOT_HOMING_MISSILE")
    CACHE_SND(snd_shot_bouncing_grenade, "SHOT_BOUNCING_GRENADE")
    CACHE_SND(snd_shot_freeze_impact,    "SHOT_FREEZE_IMPACT")
    CACHE_SND(snd_shot_splat,            "SHOT_SPLAT")
    CACHE_SND(snd_cast_heal,             "CAST_HEAL")
    CACHE_SND(snd_cast_speed,            "CAST_SPEED")
    CACHE_SND(snd_cast_lightning,        "CAST_LIGHTNING")
    CACHE_SND(snd_cast_disease,          "CAST_DISEASE")
    CACHE_SND(snd_shot_fire,             "SHOT_FIRE")
    CACHE_SND(snd_shot_bow,              "SHOT_BOW")
    CACHE_SND(snd_shot_wind,             "SHOT_WIND")
    CACHE_SND(snd_shot_breath,           "SHOT_BREATH")
    CACHE_SND(snd_shot_freeze_fire,      "SHOT_FREEZE_FIRE")

    #undef CACHE_SND
    #undef CACHE_SND_COUNT

    SYNCDBG(8, "Common sound IDs cached");
    return true;
}

/******************************************************************************/

static TbBool value_is_sound_filepath(const char* text)
{
    const char* dot = strrchr(text, '.');
    if (dot == NULL) return false;
    char ext[8]; int i;
    for (i = 0; i < 7 && dot[i]; i++)
        ext[i] = (char)tolower((unsigned char)dot[i]);
    ext[i] = '\0';
    return strcmp(ext, ".wav") == 0 || strcmp(ext, ".mp3") == 0;
}

int sound_id_from_text(const char* text)
{
    int id = sound_manager_get_id(text);
    if (id > 0)
        return id;
    if (value_is_sound_filepath(text))
    {
        SoundSmplTblID loaded_id = sound_manager_load_named_sound(text, text, 1);
        if (loaded_id > 0)
            return (int)loaded_id;
    }
    return 0;
}

int64_t value_sound_id(const struct NamedField* named_field, const char* value_text,
                       const struct NamedFieldSet* named_fields_set, int idx,
                       const char* src_str, unsigned char flags)
{
    if (parameter_is_number(value_text))
    {
        return value_default(named_field, value_text, named_fields_set, idx, src_str, flags);
    }
    int id = sound_id_from_text(value_text);
    if (id > 0)
    {
        return (int64_t)id;
    }
    if (!value_is_sound_filepath(value_text))
    {
        NAMFIELDWRNLOG("Unrecognized sound name for field '%s', got '%s'", named_field->name, value_text);
    }
    return 0;
}

/******************************************************************************/
// SpeechRef helpers
/******************************************************************************/

/** Temporary path set by value_speech_ref() when a file-path value is detected. */
static char s_speech_ref_pending_path[512];

/** Sentinel returned by value_speech_ref() to signal a pending file path. */
#define SPEECH_REF_PATH_SENTINEL INT64_MIN

/**
 * Parses a config text value into a SpeechRef.
 * Accepts: numeric SMsg_* ID, named speech message (e.g. "LevelWon"), or file path (.wav/.mp3).
 */
void speech_ref_parse(SpeechRef* ref, const char* text)
{
    ref->id = 0;
    ref->path[0] = '\0';
    if (text == NULL || text[0] == '\0')
        return;
    char* endptr;
    long id = strtol(text, &endptr, 10);
    if (*endptr == '\0') {
        if (id >= 0 && id < SMsg_MAX)
            ref->id = (int32_t)id;
        else
            WARNLOG("Speech ID %ld out of range [0,%d], ignoring", id, SMsg_MAX - 1);
        return;
    }
    int smsg = get_id(speech_desc, text);
    if (smsg > 0) {
        ref->id = (int32_t)smsg;
        return;
    }
    if (!value_is_sound_filepath(text)) {
        WARNLOG("Unrecognized speech value '%s': not a number, known name, or audio file path", text);
        return;
    }
    snprintf(ref->path, sizeof(ref->path), "%s", text);
}

/**
 * NamedField parse function for SpeechRef fields.
 * Accepts numeric IDs, named speech messages, or file paths.
 * Sets s_speech_ref_pending_path and returns SPEECH_REF_PATH_SENTINEL for file paths.
 */
int64_t value_speech_ref(const struct NamedField* named_field, const char* value_text,
                         const struct NamedFieldSet* named_fields_set, int idx,
                         const char* src_str, unsigned char flags)
{
    s_speech_ref_pending_path[0] = '\0';
    char* endptr;
    long id = strtol(value_text, &endptr, 10);
    if (*endptr == '\0') {
        if (id < 0 || id >= SMsg_MAX) {
            NAMFIELDWRNLOG("Speech ID %ld out of range [0,%d] for field '%s'", id, SMsg_MAX - 1, named_field->name);
            return 0;
        }
        return (int64_t)id;
    }
    int smsg = get_id(speech_desc, value_text);
    if (smsg > 0)
        return (int64_t)smsg;
    if (!value_is_sound_filepath(value_text)) {
        NAMFIELDWRNLOG("Unrecognized speech value '%s' for field '%s': not a number, known name, or audio file path",
                       value_text, named_field->name);
        return 0;
    }
    snprintf(s_speech_ref_pending_path, sizeof(s_speech_ref_pending_path), "%s", value_text);
    return SPEECH_REF_PATH_SENTINEL;
}

/**
 * NamedField assign function for SpeechRef fields.
 * Expects the field pointer to point to the start of a SpeechRef struct.
 */
void assign_speech_ref(const struct NamedField* named_field, int64_t value,
                       const struct NamedFieldSet* named_fields_set, int idx,
                       const char* src_str, unsigned char flags)
{
    char* field_ptr = (char*)named_field->field + named_fields_set->struct_size * idx;
    char* base = (char*)named_fields_set->struct_base;
    if (named_fields_set->struct_base == NULL || idx < 0 || idx >= named_fields_set->max_count ||
        field_ptr < base || field_ptr >= base + named_fields_set->struct_size * named_fields_set->max_count)
    {
        NAMFIELDERRLOG("Field '%s' index %d out of bounds", named_field->name, idx);
        return;
    }
    SpeechRef* ref = (SpeechRef*)field_ptr;
    if (value == SPEECH_REF_PATH_SENTINEL && s_speech_ref_pending_path[0] != '\0') {
        ref->id = 0;
        snprintf(ref->path, sizeof(ref->path), "%s", s_speech_ref_pending_path);
    } else {
        ref->id = (int32_t)value;
        ref->path[0] = '\0';
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
