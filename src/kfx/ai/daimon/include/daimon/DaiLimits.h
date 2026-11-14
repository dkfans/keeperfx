/** @file DaiLimits.h
 *  Every pool size and default budget of a Daimon keeper: compile-time, not configuration (02 section 12 C22), so the
 *  arena and the save layout are fixed. */
#ifndef DAIMON_DAILIMITS_H
#define DAIMON_DAILIMITS_H

#include <cstdint>

constexpr int64_t DAI_MAX_JOBS = 128;
constexpr int64_t DAI_MAX_CLAIMS = 256;
constexpr int64_t DAI_MAX_INBOX = 64;
constexpr int64_t DAI_MAX_GOALS = 32;
constexpr int64_t DAI_MAX_REQUESTS = 32;
/** Objectives the keeper's fights serve (06 section 5.2): Defence's standing ones and its intercepts, later Offence's. */
constexpr int64_t DAI_MAX_OBJECTIVES = 32;
/** Dig marks a keeper may have (the game's MAPTASKS_COUNT, checked in Daimon.cpp). */
constexpr int64_t DAI_MAX_MARKS = 300;
/** Dig searches a keeper may hold at once ([budgets] max_searches runs at most this many). */
constexpr int64_t DAI_MAX_SEARCHES = 4;
/** The stall window of the digging kinds (route, mining, bridge, area, treasury; 03 section 7.3): digging is slow, and
 *  a claim wait can be long. Other jobs take [jobs] stall_window. */
constexpr int64_t DAI_DIG_STALL_WINDOW = 1500;
/** Slabs a keeper's jobs may have re-read each turn, all together (IDaiJobKind::Watch). */
constexpr int64_t DAI_MAX_WATCH = 128;
/** Own imps the knowledge lists each turn (more are counted, not listed). */
constexpr int64_t DAI_MAX_OWN_IMPS = 64;
/** The keeper's own creatures, imps apart, the knowledge reads each turn (more are not seen by its managers). */
constexpr int64_t DAI_MAX_OWN_CREATURES = 255;
/** Phase 4's fight reads, each update: battles the keeper's creatures are in, its traps and doors, enemy creatures it
 *  sees (all of them, map-wide). */
constexpr int64_t DAI_MAX_BATTLES = 64;
constexpr int64_t DAI_MAX_OWN_TRAPS = 256;
constexpr int64_t DAI_MAX_OWN_DOORS = 128;
constexpr int64_t DAI_MAX_ENEMIES = 256;
/** Hero gates the knowledge keeps, and how often it reads them (they are few and rarely change). */
constexpr int64_t DAI_MAX_HERO_GATES = 32;
constexpr uint64_t DAI_HERO_GATE_READ = 64;
/** Phase 5 (07 section 3.1): other keepers' traps and doors and rooms the knowledge remembers, and how often each is
 *  read (doors and traps change in fights; rooms slowly). */
constexpr int64_t DAI_MAX_ENEMY_DEFENCES = 256;
constexpr int64_t DAI_MAX_RIVAL_ROOMS = 256;
constexpr uint64_t DAI_ENEMY_DEFENCE_READ = 25;
constexpr uint64_t DAI_RIVAL_ROOM_READ = 100;
/** Creature kinds the knowledge keeps (the game's CREATURE_TYPES_MAX). */
constexpr int64_t DAI_MAX_KINDS = 128;
/** Temple recipes read (the rules table holds at most 100). */
constexpr int64_t DAI_MAX_RECIPES = 100;
/** Daimon's own room identities (04 section 2.3): standing rooms and lost ones a rebuild may still want. */
constexpr int64_t DAI_MAX_ROOM_RECORDS = 96;
/** Mature chickens the knowledge lists (the hand feeds one at a time). */
constexpr int64_t DAI_MAX_OWN_FOOD = 64;
/** Library slabs holding a spellbook the knowledge lists (a keeper has a few dozen spells at most). */
constexpr int64_t DAI_MAX_OWN_BOOK_SLABS = 128;
/** Players the game has (PLAYERS_COUNT, checked in Daimon.cpp): Daimon's own name for it, so the decision and knowledge
 *  classes need no simulation header. */
constexpr int64_t DAI_MAX_PLAYERS = 9;
/** A fighter counts as healthy -- free for a fight, an attack's force -- at this health % or more (Tactics, Rivals). */
constexpr int64_t DAI_HEALTHY_PCT = 60;
/** Longest personality inheritance chain (default -> ... -> the one named). */
constexpr int64_t DAI_MAX_INHERIT_DEPTH = 8;

#endif
