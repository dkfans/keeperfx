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
#include "bflib_sndlib.h"
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
// Cached common sound IDs — defaults are defined in config/fxdata/sounds.cfg
// These are populated at startup by cache_common_sound_ids() after sounds.cfg loads.
/******************************************************************************/

int            snd_gold_pickup_count = 0;
int            snd_door_place_count = 0;
int            snd_tunnel_dig_count = 0;
int            snd_dig_spell_count  = 0;
int            snd_dig_impact_count = 0;
int            snd_foot_spur_count = 0;
int            snd_foot_wet_count  = 0;
int            snd_foot_snow_count = 0;
int            snd_chicken_cluck_count = 0;
int            snd_strike_wall_count = 0;
int            snd_reinforce_hit_count = 0;

SoundSmplTblID snd_foot_spur       = 0;
SoundSmplTblID snd_foot_wet        = 0;
SoundSmplTblID snd_insect_fly      = 0;

SoundSmplTblID snd_gold_pickup     = 0;
SoundSmplTblID snd_salary_tiny     = 0;
SoundSmplTblID snd_salary_partial  = 0;
SoundSmplTblID snd_salary_full     = 0;
SoundSmplTblID snd_splash          = 0;
SoundSmplTblID snd_spell_wall      = 0;

SoundSmplTblID snd_spell_armageddon = 0;

SoundSmplTblID snd_explode         = 0;
SoundSmplTblID snd_spell_frozen    = 0;
SoundSmplTblID snd_tab_click       = 0;
SoundSmplTblID snd_button_click    = 0;
SoundSmplTblID snd_dig_spell       = 0;
SoundSmplTblID snd_tunnel_dig      = 0;
SoundSmplTblID snd_door_place      = 0;
SoundSmplTblID snd_dig_impact      = 0;
SoundSmplTblID snd_dig_dirt        = 0;
SoundSmplTblID snd_spell_stars     = 0;
SoundSmplTblID snd_tile_place      = 0;

SoundSmplTblID snd_coin_drop       = 0;
SoundSmplTblID snd_buzzer          = 0;
SoundSmplTblID snd_alarm           = 0;
SoundSmplTblID snd_door_open       = 0;
SoundSmplTblID snd_door_close      = 0;
SoundSmplTblID snd_heart_engine    = 0;

SoundSmplTblID snd_chicken_cluck   = 0;
SoundSmplTblID snd_tile_sell       = 0;
SoundSmplTblID snd_room_claim      = 0;
SoundSmplTblID snd_trap_place      = 0;
SoundSmplTblID snd_tile_dig        = 0;
SoundSmplTblID snd_refusal         = 0;
SoundSmplTblID snd_strike_wall     = 0;

SoundSmplTblID snd_heart_beat_down = 0;
SoundSmplTblID snd_heart_beat_up   = 0;
SoundSmplTblID snd_scavenge        = 0;
SoundSmplTblID snd_cheat_activated = 0;
SoundSmplTblID snd_tab_hit         = 0;
SoundSmplTblID snd_foot_snow       = 0;

SoundSmplTblID snd_larg_tile_up    = 0;
SoundSmplTblID snd_larg_tile_down  = 0;

SoundSmplTblID snd_tab_fall        = 0;
SoundSmplTblID snd_reinforce_hit   = 0;

// Trap trigger sounds
SoundSmplTblID snd_trap_trigger     = 0;
SoundSmplTblID snd_trap_trigger_tnt = 0;

// Object ambient / effect sounds
SoundSmplTblID snd_torch_ambience       = 0;
SoundSmplTblID snd_dungeon_heart_beam   = 0;
SoundSmplTblID snd_hero_gate_ambience   = 0;

// Keeper power sounds — voice (SoundSamples) paired with cast effect (SoundPlayed)
SoundSmplTblID snd_power_imp           = 0;

SoundSmplTblID snd_power_possess       = 0;
SoundSmplTblID snd_cast_possess        = 0;

SoundSmplTblID snd_power_sight         = 0;
SoundSmplTblID snd_cast_sight          = 0;

SoundSmplTblID snd_power_obey          = 0;
SoundSmplTblID snd_power_hold_audience = 0;
SoundSmplTblID snd_cast_obey           = 0;

SoundSmplTblID snd_power_call_to_arms  = 0;
SoundSmplTblID snd_cast_call_to_arms   = 0;

SoundSmplTblID snd_power_cave_in       = 0;
SoundSmplTblID snd_cast_cave_in        = 0;

SoundSmplTblID snd_power_heal          = 0;
SoundSmplTblID snd_cast_heal           = 0;

SoundSmplTblID snd_power_lightning     = 0;
SoundSmplTblID snd_cast_lightning      = 0;

SoundSmplTblID snd_power_speed         = 0;
SoundSmplTblID snd_cast_speed          = 0;

SoundSmplTblID snd_power_protect       = 0;
SoundSmplTblID snd_cast_armour         = 0;

SoundSmplTblID snd_power_conceal       = 0;
SoundSmplTblID snd_cast_conceal        = 0;

SoundSmplTblID snd_power_disease       = 0;
SoundSmplTblID snd_cast_disease        = 0;

SoundSmplTblID snd_power_chicken       = 0;
SoundSmplTblID snd_cast_pickup         = 0;

SoundSmplTblID snd_power_destroy_walls = 0;
SoundSmplTblID snd_power_armageddon    = 0;

// Powers with no voice — cast effect only
SoundSmplTblID snd_cast_slap           = 0;
SoundSmplTblID snd_cast_rebound        = 0;
SoundSmplTblID snd_cast_flight         = 0;
SoundSmplTblID snd_cast_vision         = 0;

// Shot projectile travel sounds (ShotSound field in magic.cfg)
SoundSmplTblID snd_shot_freeze            = 0;
SoundSmplTblID snd_shot_homing_missile    = 0;
SoundSmplTblID snd_shot_bouncing_grenade  = 0;

// Shot impact sounds
SoundSmplTblID snd_shot_freeze_impact     = 0;
SoundSmplTblID snd_shot_splat             = 0;

// Shot firing sounds (FiringSound field in magic.cfg — played on the creature at the moment of release)
SoundSmplTblID snd_shot_fire              = 0;
SoundSmplTblID snd_shot_bow               = 0;
SoundSmplTblID snd_shot_wind              = 0;
SoundSmplTblID snd_shot_breath            = 0;
SoundSmplTblID snd_shot_freeze_fire       = 0;

// Melee swing / misc shot sounds
int            snd_melee_swing_count      = 0;
SoundSmplTblID snd_melee_swing            = 0;
SoundSmplTblID snd_boulder_roll           = 0;
SoundSmplTblID snd_shot_magic_travel      = 0;
SoundSmplTblID snd_trap_tnt_fire          = 0;
SoundSmplTblID snd_fear_shriek            = 0;
SoundSmplTblID snd_cast_cleanse           = 0;

// Hit impact sounds (HitXxxSound fields in magic.cfg)
SoundSmplTblID snd_hit_creature_sword    = 0;
SoundSmplTblID snd_hit_creature          = 0;
int            snd_hit_wall_count        = 0;
SoundSmplTblID snd_hit_wall              = 0;
int            snd_hit_door_sword_count  = 0;
SoundSmplTblID snd_hit_door_sword        = 0;
int            snd_hit_door_count        = 0;
SoundSmplTblID snd_hit_door              = 0;
SoundSmplTblID snd_hit_heart             = 0;
int            snd_hit_heart_fist_count  = 0;
SoundSmplTblID snd_hit_heart_fist        = 0;
SoundSmplTblID snd_hit_wall_boulder      = 0;


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
 * @brief Parse a single line.
 *
 * Handles three formats:
 *   NAME = id [count]            — register a named alias for a sound.dat ID
 *   NAME = filepath [count]      — load a custom file and register it under NAME
 *   id   = filepath [count] [ALIAS] — redirect a raw sound.dat ID to a custom file;
 *                                     optionally also register ALIAS in the name registry
 */
static TbBool parse_sound_line(const char* buf, int32_t* pos, long len, const char* config_textname)
{
    char name_buf[COMMAND_WORD_LEN];
    char value_buf[COMMAND_WORD_LEN];
    char count_buf[COMMAND_WORD_LEN];

    // Get the key (either a NAME or a raw numeric ID)
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

    // --- Detect whether this is a numeric-key (raw ID redirect) line ---
    char* key_endptr;
    long raw_id = strtol(name_buf, &key_endptr, 10);
    TbBool is_raw_id = (*key_endptr == '\0' && raw_id > 0);

    // Get the value (ID or filepath), skipping the '=' separator
    if (get_conf_parameter_single(buf, pos, len, value_buf, sizeof(value_buf)) <= 0)
    {
        while (*pos < len && (buf[*pos] == '=' || buf[*pos] == ' ' || buf[*pos] == '\t'))
            (*pos)++;
        if (get_conf_parameter_single(buf, pos, len, value_buf, sizeof(value_buf)) <= 0)
        {
            WARNLOG("Sound '%s' has no value in %s", name_buf, config_textname);
            return false;
        }
    }
    if (value_buf[0] == '=')
    {
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
            count = parsed_count;
    }

    // -----------------------------------------------------------------------
    // Numeric-key path: redirect a raw sound.dat ID to a custom file
    // -----------------------------------------------------------------------
    if (is_raw_id)
    {
        // Only filepaths make sense as the target of a raw-ID redirect.
        char* val_endptr;
        long val_num = strtol(value_buf, &val_endptr, 10);
        (void)val_num;
        if (*val_endptr == '\0')
        {
            WARNLOG("Raw-ID redirect '%s' has a numeric value in %s; use a filepath instead",
                    name_buf, config_textname);
            return false;
        }

        // Optional alias token — must start with a letter or underscore
        char alias_buf[COMMAND_WORD_LEN] = {0};
        char next_buf[COMMAND_WORD_LEN];
        if (get_conf_parameter_single(buf, pos, len, next_buf, sizeof(next_buf)) > 0)
        {
            if (isalpha((unsigned char)next_buf[0]) || next_buf[0] == '_')
            {
                // Normalize alias to uppercase
                for (int i = 0; next_buf[i] != '\0'; i++)
                    next_buf[i] = (char)toupper((unsigned char)next_buf[i]);
                snprintf(alias_buf, sizeof(alias_buf), "%s", next_buf);
            }
            // If it wasn't an alpha token it was likely a stray comment — ignore it
        }

        // Build the internal name: use alias if given, else synthesise __RAW_<id>
        char internal_name[COMMAND_WORD_LEN];
        if (alias_buf[0] != '\0')
            snprintf(internal_name, sizeof(internal_name), "%s", alias_buf);
        else
            snprintf(internal_name, sizeof(internal_name), "__RAW_%ld", raw_id);

        // Load the custom file(s) and register under internal_name
        SoundSmplTblID first_custom_id = sound_manager_load_named_sound(internal_name, value_buf, count);
        if (first_custom_id <= 0)
        {
            WARNLOG("Raw-ID redirect %ld: failed to load '%s' in %s", raw_id, value_buf, config_textname);
            return false;
        }

        // Register per-ID redirects: raw_id+i → first_custom_id+i
        for (int i = 0; i < count; i++)
            sound_register_id_redirect((SoundSmplTblID)(raw_id + i), (SoundSmplTblID)(first_custom_id + i));

        if (alias_buf[0] != '\0') {
            SYNCDBG(5, "Raw-ID redirect: %ld..%ld -> custom IDs %d..%d (alias '%s')",
                    raw_id, raw_id + count - 1, first_custom_id, first_custom_id + count - 1, alias_buf);
        } else {
            SYNCDBG(5, "Raw-ID redirect: %ld..%ld -> custom IDs %d..%d",
                    raw_id, raw_id + count - 1, first_custom_id, first_custom_id + count - 1);
        }

        return true;
    }

    // -----------------------------------------------------------------------
    // Named-key path: NAME = id [count]  OR  NAME = filepath [count]
    // -----------------------------------------------------------------------
    char* endptr;
    long id_value = strtol(value_buf, &endptr, 10);

    if (*endptr == '\0' && id_value > 0)
    {
        // Numeric ID — register name → ID mapping
        SoundSmplTblID sample_id = (SoundSmplTblID)id_value;
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
        // Filepath — load custom file and register under NAME
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
 * @brief Parse a [system] block with engine-level audio settings.
 *
* Supported keys:
*   speech_queue_limit = <positive integer>
*/
static TbBool parse_system_section(char* buf, long len, const char* config_textname,
                                       unsigned short flags)
{
    (void)flags;
    int32_t pos = 0;
    const char* blockname = NULL;
    int blocknamelen = 0;
    TbBool found_section = false;

    while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
    {
        if (blocknamelen > 0 && strncasecmp(blockname, "system", blocknamelen) == 0)
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
        char value_buf[COMMAND_WORD_LEN];

        if (get_conf_parameter_single(buf, &pos, len, name_buf, sizeof(name_buf)) <= 0)
        {
            while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
            while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r')) pos++;
            continue;
        }

        if (get_conf_parameter_single(buf, &pos, len, value_buf, sizeof(value_buf)) <= 0)
        {
            while (pos < len && (buf[pos] == '=' || buf[pos] == ' ' || buf[pos] == '\t')) pos++;
            if (get_conf_parameter_single(buf, &pos, len, value_buf, sizeof(value_buf)) <= 0)
            {
                WARNLOG("Missing value for system setting '%s' in %s", name_buf, config_textname);
                while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
                while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r')) pos++;
                continue;
            }
        }
        if (value_buf[0] == '=')
        {
            if (get_conf_parameter_single(buf, &pos, len, value_buf, sizeof(value_buf)) <= 0)
            {
                WARNLOG("Missing value after '=' for system setting '%s' in %s", name_buf, config_textname);
                while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
                while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r')) pos++;
                continue;
            }
        }

        if (strcasecmp(name_buf, "speech_queue_limit") == 0)
        {
            long limit = strtol(value_buf, NULL, 10);
            if (limit <= 0)
            {
                WARNLOG("Invalid speech queue limit '%s' in %s; expected a positive integer", value_buf, config_textname);
            }
            else
            {
                g_speech_queue_limit = (int)limit;
                SYNCDBG(8, "Speech queue limit set to %d", g_speech_queue_limit);
            }
        } else {
            WARNLOG("Unknown system setting '%s' in %s", name_buf, config_textname);
        }

        while (pos < len && buf[pos] != '\n' && buf[pos] != '\r') pos++;
        while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r')) pos++;
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

    parse_system_section(buf, len, fname, flags);
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

TbBool load_level_sounds_config(short fgroup, LevelNumber lvnum)
{
    char* fullpath = prepare_file_fmtpath(fgroup, "map%05lu.sounds.cfg", (unsigned long)lvnum);
    if (fullpath == NULL || !LbFileExists(fullpath))
    {
        return false;
    }

    TbBool result = load_sounds_config_file(fullpath, CnfLd_Standard | CnfLd_IgnoreErrors);
    cache_common_sound_ids();
    return result;
}

void sound_save_campaign_snapshot(void)
{
    sound_manager_save_snapshot();
    sound_save_id_redirect_snapshot();
    SYNCDBG(7, "sound_save_campaign_snapshot: snapshot saved");
}

void sound_restore_to_campaign_snapshot(void)
{
    sound_manager_restore_snapshot();
    sound_restore_id_redirect_snapshot();
    cache_common_sound_ids();
    SYNCDBG(7, "sound_restore_to_campaign_snapshot: snapshot restored");
}

void sound_reset_to_fxdata_baseline(void)
{
    sound_manager_clear_custom_sounds();
    sound_manager_clear_registry();
    g_speech_queue_limit = 4;
    load_sounds_config();
    SYNCDBG(7, "sound_reset_to_fxdata_baseline: reset to fxdata defaults");
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
    CACHE_SND(snd_power_imp,             "POWER_IMP")
    CACHE_SND(snd_power_possess,         "POWER_POSSESS")
    CACHE_SND(snd_cast_possess,          "CAST_POSSESS")
    CACHE_SND(snd_power_sight,           "POWER_SIGHT")
    CACHE_SND(snd_cast_sight,            "CAST_SIGHT")
    CACHE_SND(snd_power_obey,            "POWER_OBEY")
    CACHE_SND(snd_power_hold_audience,   "POWER_HOLD_AUDIENCE")
    CACHE_SND(snd_cast_obey,             "CAST_OBEY")
    CACHE_SND(snd_power_call_to_arms,    "POWER_CALL_TO_ARMS")
    CACHE_SND(snd_cast_call_to_arms,     "CAST_CALL_TO_ARMS")
    CACHE_SND(snd_power_cave_in,         "POWER_CAVE_IN")
    CACHE_SND(snd_cast_cave_in,          "CAST_CAVE_IN")
    CACHE_SND(snd_power_heal,            "POWER_HEAL")
    CACHE_SND(snd_cast_heal,             "CAST_HEAL")
    CACHE_SND(snd_power_lightning,       "POWER_LIGHTNING")
    CACHE_SND(snd_cast_lightning,        "CAST_LIGHTNING")
    CACHE_SND(snd_power_speed,           "POWER_SPEED")
    CACHE_SND(snd_cast_speed,            "CAST_SPEED")
    CACHE_SND(snd_power_protect,         "POWER_PROTECT")
    CACHE_SND(snd_cast_armour,           "CAST_ARMOUR")
    CACHE_SND(snd_power_conceal,         "POWER_CONCEAL")
    CACHE_SND(snd_cast_conceal,          "CAST_CONCEAL")
    CACHE_SND(snd_power_disease,         "POWER_DISEASE")
    CACHE_SND(snd_cast_disease,          "CAST_DISEASE")
    CACHE_SND(snd_power_chicken,         "POWER_CHICKEN")
    CACHE_SND(snd_cast_pickup,           "CAST_PICKUP")
    CACHE_SND(snd_power_destroy_walls,   "POWER_DESTROY_WALLS")
    CACHE_SND(snd_power_armageddon,      "POWER_ARMAGEDDON")
    CACHE_SND(snd_cast_slap,             "CAST_SLAP")
    CACHE_SND(snd_cast_rebound,          "CAST_REBOUND")
    CACHE_SND(snd_cast_flight,           "CAST_FLIGHT")
    CACHE_SND(snd_cast_vision,           "CAST_VISION")
    CACHE_SND(snd_shot_freeze,           "SHOT_FREEZE")
    CACHE_SND(snd_shot_homing_missile,   "SHOT_HOMING_MISSILE")
    CACHE_SND(snd_shot_bouncing_grenade, "SHOT_BOUNCING_GRENADE")
    CACHE_SND(snd_shot_freeze_impact,    "SHOT_FREEZE_IMPACT")
    CACHE_SND(snd_shot_splat,            "SHOT_SPLAT")
    CACHE_SND(snd_shot_fire,             "SHOT_FIRE")
    CACHE_SND(snd_shot_bow,              "SHOT_BOW")
    CACHE_SND(snd_shot_wind,             "SHOT_WIND")
    CACHE_SND(snd_shot_breath,           "SHOT_BREATH")
    CACHE_SND(snd_shot_freeze_fire,      "SHOT_FREEZE_FIRE")
    CACHE_SND_COUNT(snd_melee_swing,     snd_melee_swing_count, "MELEE_SWING")
    CACHE_SND(snd_boulder_roll,          "BOULDER_ROLL")
    CACHE_SND(snd_shot_magic_travel,     "SHOT_MAGIC_TRAVEL")
    CACHE_SND(snd_trap_tnt_fire,         "TRAP_TNT_FIRE")
    CACHE_SND(snd_fear_shriek,           "FEAR_SHRIEK")
    CACHE_SND(snd_cast_cleanse,          "CAST_CLEANSE")
    CACHE_SND(snd_hit_creature_sword,    "HIT_CREATURE_SWORD")
    CACHE_SND(snd_hit_creature,          "HIT_CREATURE")
    CACHE_SND_COUNT(snd_hit_wall,          snd_hit_wall_count,       "HIT_WALL")
    CACHE_SND_COUNT(snd_hit_door_sword,    snd_hit_door_sword_count, "HIT_DOOR_SWORD")
    CACHE_SND_COUNT(snd_hit_door,          snd_hit_door_count,       "HIT_DOOR")
    CACHE_SND(snd_hit_heart,             "HIT_HEART")
    CACHE_SND_COUNT(snd_hit_heart_fist,    snd_hit_heart_fist_count, "HIT_HEART_FIST")
    CACHE_SND(snd_hit_wall_boulder,      "HIT_WALL_BOULDER")

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
