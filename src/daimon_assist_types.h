/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_assist_types.h
 *     The human's Daimon assistant (docs/plans/ai-plans/v2/09 section 5): its domains, a bit each, and its presets --
 *     one table for Daimon's assistant mode (DaiModeAssistant), the profile (config_daimon_assist.h), the Options
 *     screen and PckA_AssistDomains, which carries a preset as its index here. Data only.
 */
/******************************************************************************/
#ifndef DK_DAIMON_ASSIST_TYPES_H
#define DK_DAIMON_ASSIST_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** The assistant's domains, chosen by the human (a bit each). */
enum DaiAssistDomain {
    DAI_ASSIST_ECONOMY = 1u << 0, DAI_ASSIST_EXCAVATION = 1u << 1, DAI_ASSIST_CONSTRUCTION = 1u << 2,
    DAI_ASSIST_WORKFORCE = 1u << 3, DAI_ASSIST_CREATURES = 1u << 4, DAI_ASSIST_CUSTODY = 1u << 5,
    DAI_ASSIST_DEFENCE = 1u << 6, DAI_ASSIST_OFFENCE = 1u << 7,
    DAI_ASSIST_ALL = (1u << 8) - 1
};
#define DAI_ASSIST_DOMAIN_COUNT 8

struct DaiAssistDomainDef {
    const char *name;       /**< as daimon_assistant.toml writes it */
    const char *label;      /**< the Options screen's */
    const char *help;
};
/** The domains in bit order. */
static const struct DaiAssistDomainDef dai_assist_domains[DAI_ASSIST_DOMAIN_COUNT] = {
    { "economy", "Economy", "Gold: the treasury, mining, selling what is not needed." },
    { "excavation", "Excavation", "Digging: routes, veins, new ground." },
    { "construction", "Construction", "Rooms: where they go, building and growing them." },
    { "workforce", "Workforce", "Imps: making more, and dropping them where the work is." },
    { "creatures", "Creatures", "Your creatures' training, pay and rest." },
    { "custody", "Prisoners", "Prisoners and torture." },
    { "defence", "Defence", "Traps, doors and fights at home; it then runs your combat assistant's rules." },
    { "offence", "Offence", "Attacks on your rivals." },
};

struct DaiAssistPresetDef {
    const char *name;           /**< as daimon_assistant.toml and DaimonAssistStart name it */
    const char *label;
    uint32_t domains;
    const char *personality;    /**< daimon.toml's [personality.<id>] it plays as */
    const char *help;
};
#define DAI_ASSIST_PRESET_COUNT 4
static const struct DaiAssistPresetDef dai_assist_presets[DAI_ASSIST_PRESET_COUNT] = {
    { "builder", "Builder", DAI_ASSIST_ECONOMY | DAI_ASSIST_EXCAVATION | DAI_ASSIST_CONSTRUCTION | DAI_ASSIST_WORKFORCE,
      "builder", "Digs, builds and keeps your imps busy; leaves your creatures and fights to you." },
    { "steward", "Steward", DAI_ASSIST_ECONOMY | DAI_ASSIST_EXCAVATION | DAI_ASSIST_CONSTRUCTION | DAI_ASSIST_WORKFORCE
      | DAI_ASSIST_CREATURES | DAI_ASSIST_CUSTODY, "steward", "Runs the dungeon -- building, imps, creatures, prisoners -- and leaves the fighting to you." },
    { "defender", "Defender", DAI_ASSIST_CONSTRUCTION | DAI_ASSIST_WORKFORCE | DAI_ASSIST_DEFENCE,
      "turtle", "Fortifies and defends your dungeon: doors, traps and the fights at home." },
    { "raider", "Raider", DAI_ASSIST_ALL, "warlord", "Plays the whole keeper alongside you, attacks included." },
};

#ifdef __cplusplus
}
#endif
#endif
