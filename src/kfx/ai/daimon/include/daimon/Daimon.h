/** @file Daimon.h
 *  Daimon's C facade: the only Daimon header C code includes (docs/plans/ai-plans/v2/02-library-architecture.md 4.1,
 *  modelled on renderer/RendererManager.h). Everything behind it is C++ (include/daimon/Dai*.h); no exception gets
 *  out -- a failure hands the keeper concerned back and logs it.
 *
 *  Grouped by caller: the level and its saves, the turn, keepers taken and handed back, a level script's commands,
 *  the human's assistant, a seat's lieutenant and its directives, the console; docs/Architecture/architecture.md
 *  section 2.5a describes them. */
#ifndef DAIMON_DAIMON_H
#define DAIMON_DAIMON_H

#include "globals.h"
#include "daimon_base_types.h"
#include "bflib_basics.h"

// The facade's enums are C++ enum classes; C sees them as int64_t, as RendererType.
#ifdef __cplusplus
#  include "daimon/DaiTypes.h"
#else
typedef int64_t DaimonMode;
#  define DAIMON_MODE_NONE       (-1)
#  define DAIMON_MODE_OPPONENT   0
#  define DAIMON_MODE_ASSISTANT  1
#  define DAIMON_MODE_LIEUTENANT 2
typedef int64_t DaimonHandBackTo;
#  define DAIMON_TO_COMPUTER2 0
#  define DAIMON_TO_HUMAN     1
typedef int64_t DaimonTakeReason;
#  define DAIMON_WHY_SKIRMISH      0
#  define DAIMON_WHY_SCRIPT        1
#  define DAIMON_WHY_LEVEL_KEY     2
#  define DAIMON_WHY_DROP_IN       3
#  define DAIMON_WHY_HOST_LOST     4
#  define DAIMON_WHY_SEAT_TAKEOVER 5
#  define DAIMON_WHY_ASSISTANT     6
#  define DAIMON_WHY_LIEUTENANT    7
#  define DAIMON_WHY_CONSOLE       8
#  define DAIMON_WHY_LOAD          9
#  define DAIMON_WHY_LOCAL_SEAT    10
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Once at start-up, after the ports are wired. */
void DaimonInit(void);
/** A level is about to start: keepers wanted at its start (personality, difficulty) are noted, and the keeper is
 *  queued for a packet slot (daimon_pending_slots_add); DaimonLevelStart() builds them once the slots are claimed. */
void DaimonQueueLevelStart(PlayerNumber plyr, const char *personality, const char *difficulty);
/** Forgets every keeper queued for the next level start (a new Play from the Skirmish page queues its own). */
void DaimonQueueClear(void);
/** DAIMON_PLAYER and its Lua twin DaimonPlayer (04 section 10.1a), run on every machine alike. Before the level
 *  start (a script's top level, Lua's start): queued as the Skirmish page's choice is, so the replay's header names the
 *  slot. During play: the keeper's packet slot is chosen from the game's own state (the same on every machine, as an
 *  adoption's) and the keeper is built where Daimon's keepers are authored (not on a network client, nor in a
 *  replay's playback). False when the keeper cannot be Daimon's (the local human's, an External seat's, no keeper). */
TbBool DaimonScriptPlayer(PlayerNumber plyr, const char *personality, const char *difficulty);
/** A runtime COMPUTER_PLAYER (or Lua ComputerPlayer) for a keeper Daimon plays: Daimon lets it go on every machine
 *  alike, before the command makes it the built-in AI's. Nothing for a keeper that is not Daimon's, or before the
 *  level start (there a Skirmish or script choice of Daimon stands over the level's COMPUTER_PLAYER line). */
void DaimonScriptHandBack(PlayerNumber plyr);
/** DAIMON_SET's check (Phase 7, 09 section 4): `key` ("module.key", any case) is a personality key and `value` reads
 *  for it; else false and why. */
TbBool DaimonCheckKey(const char *key, const char *value, char *why, size_t why_len);
/** DAIMON_SET / DaimonSet: one of Daimon's keeper's personality keys, clamped to its range, logged; saved with its
 *  personality. Before the level start it waits for the keeper (a DAIMON_PLAYER line's); on a machine that does not
 *  author Daimon's keepers it is the author's to do. False when the keeper is not Daimon's, or the key or value is
 *  bad. */
TbBool DaimonScriptSet(PlayerNumber plyr, const char *key, const char *value);
/** DAIMON_BUILD (Phase 7, 09 section 13.2a): a goal for a room of the game's kind `room` on the slab rectangle
 *  (x, y, w x h), outranking the director's; dug to and built there, rebuilt while the goal stands. */
TbBool DaimonScriptBuild(PlayerNumber plyr, RoomKind room, DaiSlabCoord x, DaiSlabCoord y, int64_t w, int64_t h);
/** DAIMON_ATTACK: an attack on `rival` (when ready, as any of Daimon's attacks), its first target `objective`
 *  ("HEART", "PORTAL", "ROOMS"; "" or "ANY": the personality's); rival < 0 takes the script's attacks back. */
TbBool DaimonScriptAttack(PlayerNumber plyr, PlayerNumber rival, const char *objective);
/** DAIMON_DIG: a route dug and claimed to the slab (x, y). */
TbBool DaimonScriptDig(PlayerNumber plyr, DaiSlabCoord x, DaiSlabCoord y);
/** The human's touch log (09 section 5.3, Daimon P7.11): whether Daimon wants the keeper's touches (it runs the keeper's
 *  assistant), and the touches of one packet (keeper_touch_log.h), turned into leases. */
struct KeeperTouch;
TbBool DaimonWantsTouches(PlayerNumber plyr);
void DaimonNoteTouches(PlayerNumber plyr, const struct KeeperTouch *touches, int64_t n);
/** Whether the action in the user's packet this turn is Daimon's own -- its assistant's, written into the human's
 *  packet (P7.12), or a seat's lieutenant's, from its lane (P8.3): not the human's or the agent's touch. */
TbBool DaimonAssistantWroteAction(int64_t user);
/** The human's assistant (09 section 5, P7.12): on the local human's own keeper (single player, the built-in assistant
 *  off), a preset ("builder", "steward", "defender", "raider") and its personality, or the domains given (a
 *  DaiAssistDomain mask; 0: the preset's), at a difficulty ("" normal). It writes self-contained gestures into the
 *  human's packet on turns they leave free; the keeper stays the human's. */
TbBool DaimonAssistStart(PlayerNumber plyr, const char *preset, uint32_t domains, const char *difficulty);
void DaimonAssistStop(PlayerNumber plyr);
/** An External seat's lieutenant (docs/plans/ai-plans/v2/10, P8.2): on with the domains delegated to it (a
 *  DaiAssistDomain mask), its personality ("" or NULL: steward) and difficulty (NULL: unchanged, else "" normal); a
 *  running one changes its delegation in place, a new personality or difficulty is a new keeper; off stops it, the
 *  agent's verbs left alone. NULL on success, else an error code (NOT_A_VALID_SEAT, TAKEN, UNKNOWN_PERSONALITY,
 *  UNKNOWN_DIFFICULTY, FAILED). */
const char *DaimonLieutenantSet(PlayerNumber plyr, TbBool enabled, const char *personality, const char *difficulty,
                                uint32_t delegated, uint32_t advised);
TbBool DaimonLieutenantOn(PlayerNumber plyr);
/** An agent's directive, as set_directives gives it (10 section 4, P8.4). kind: build (room_kind, count slabs),
 *  expand_to (x, y), keep (imps, paydays), defend (the keeper's home), attack (rival, objective heart/portal/rooms/any),
 *  forbid (action "dismiss"), perimeter (action "open" / "close"). priority 1-4 weighs its goal (0: 2); expires_turn 0:
 *  never. budget, bias, custody, staff and ally are not built yet. */
/** The most directives a lieutenant keeps, and the most proposals the API lists (14 S5). */
#define DAIMON_MAX_DIRECTIVES 32
#define DAIMON_MAX_PROPOSALS 32
struct DaimonDirectiveSpec {
    char id[32];
    char kind[16];
    int64_t room_kind, count, x, y, rival, imps, paydays, priority, expires_turn;
    char objective[16];
    char action[16];
};
struct DaimonDirectiveStatus {
    char id[32];
    char kind[16];
    char status[16];    /**< active, done, blocked, expired */
    char why[32];
    int64_t since;      /**< the turn it arrived */
};
/** The agent's directives to its lieutenant: added or updated by id (replace: the whole set swapped), each checked first
 *  -- nothing is set unless all are valid; dry_run checks only. NULL on success, else an error code and *bad the
 *  directive's index (NOT_A_LIEUTENANT, MISSING_ID, UNKNOWN_KIND, NOT_BUILT_YET, BAD_ROOM, BAD_POSITION, BAD_RIVAL,
 *  BAD_OBJECTIVE, BAD_ACTION, BAD_COUNT, TOO_MANY). Each arrival is logged. */
const char *DaimonSetDirectives(PlayerNumber plyr, const struct DaimonDirectiveSpec *d, int64_t n, TbBool replace, TbBool dry_run,
                                int64_t *bad);
/** The directives and their status, in their order; how many were written. */
int64_t DaimonDirectiveStatuses(PlayerNumber plyr, struct DaimonDirectiveStatus *out, int64_t max);
/** Its domains: delegated (Daimon acts) and advised (Daimon proposes, P8.5); either may be NULL. */
TbBool DaimonLieutenantDelegated(PlayerNumber plyr, uint32_t *delegated, uint32_t *advised);
/** A proposal of an advised domain (P8.5): a job the lieutenant planned and holds for the agent's accept. */
struct DaimonProposal {
    int64_t id;         /**< the job's id: what accept names */
    char domain[16];
    char what[32];      /**< the job's kind */
    int64_t cost;       /**< the gold it needs */
    char why[32];
    int64_t since;      /**< the turn it was planned */
};
int64_t DaimonProposals(PlayerNumber plyr, struct DaimonProposal *out, int64_t max);
/** What a seat's agent sees of its lieutenant (own.ai, 10 sections 6.1 and 15, P8.6). Fixed arrays, counts beside them. */
struct DaimonLieutenantInfo {
    char personality[32], difficulty[32], posture[16];
    uint32_t delegated, advised;
    struct { int64_t running, ready, blocked; int64_t top_n; struct { int64_t id; char kind[24]; char why[32]; } top[5]; } jobs[16];
    int64_t domains;            /**< entries of jobs[] used, in DaiDomain order (with their names in domain_names) */
    char domain_names[16][16];
    struct { int64_t id; char kind[16]; char state[16]; int64_t own_force, enemy_force; } objectives[16];
    int64_t objectives_n;
    int64_t reserved_gold;
    struct { int64_t side_gold, horizon_turns, urgency_pct, runway_paydays; TbBool short_next; } outlook;
    struct { TbBool open; int64_t gaps_n; struct { int64_t x, y, slabs; TbBool closed; } gaps[8]; } perimeter;
    struct { int64_t kind; char cls[8]; char name[32]; char role[16]; int64_t worth; TbBool overridden; } content[32];
    int64_t content_n;
};
TbBool DaimonLieutenantStatus(PlayerNumber plyr, struct DaimonLieutenantInfo *out);
/** Takeover (10 section 8, P8.8): `daimon` (true, the default) -- a lost agent's seat becomes an ordinary Daimon keeper,
 *  every domain its, the agent's directives kept as goals, and goes back to a lieutenant seat when the agent claims
 *  it again; `computer` -- the built-in AI takes it, as a seat without a lieutenant. */
TbBool DaimonLieutenantSetTakeover(PlayerNumber plyr, TbBool daimon);
/** kfx_net's takeover for a lieutenant seat (the daimon_port's on_seat_takeover): true if Daimon took the keeper. */
TbBool DaimonSeatTakeover(PlayerNumber plyr);
/** Around claim_seat for a keeper Daimon took over from its seat: before, the keeper given back to the built-in AI
 *  (so the claim finds the computer keeper it expects); after a claim that worked, the lieutenant on again as it was. */
void DaimonBeforeSeatClaim(PlayerNumber plyr);
void DaimonAfterSeatClaim(PlayerNumber plyr, TbBool claimed);
/** The agent accepts a proposal: its job runs. NULL, or NOT_A_LIEUTENANT / NO_SUCH_PROPOSAL. */
const char *DaimonAcceptProposal(PlayerNumber plyr, int64_t proposal_id);
/** The keeper has Daimon's assistant on. */
TbBool DaimonAssistOn(PlayerNumber plyr);
/** The Options screen's settings, as PckA_AssistDomains carries them (P7.13): off (or no domain) stops the assistant;
 *  on starts it, or changes a running one's domains in place -- a new personality is a new keeper, the human's
 *  standing decisions carried over. `personality` "" or NULL: the preset's. */
TbBool DaimonAssistApply(PlayerNumber plyr, TbBool on, const char *preset, uint32_t domains, const char *personality);
/** The running assistant's domains and personality id (false when it is off). */
TbBool DaimonAssistSettings(PlayerNumber plyr, uint32_t *domains, char *personality, size_t len);
/** A short line on what the assistant is doing (the Options screen's), false when it is off. */
TbBool DaimonAssistStatus(PlayerNumber plyr, char *buf, size_t len);
/** After the level's keepers were handed their slots (startup_network_game_tail): one Daimon keeper per PlaF_Daimon
 *  player. Nothing in a replay's playback (the recorded packets act). */
void DaimonLevelStart(void);
/** Forgets every keeper (a new level, the menu). The players' flags are the simulation's; they are not touched. */
void DaimonLevelEnd(void);
/** The major version of daimon.bin this build writes and reads (the save's manifest entry carries it). */
#define DAIMON_SAVE_MAJOR 2
/** Daimon's state for a saved game (daimon.bin): a malloc'd blob in *out and its size, or 0 (nothing written) when
 *  Daimon plays no keeper. */
size_t DaimonSaveBlob(char **out);
/** A load found daimon.bin (manifest `version`): kept until DaimonAfterLoad. A version this build does not read is
 *  noted, not kept. Not called when the save has none. */
void DaimonStageLoad(const char *data, size_t len, int64_t version);
/** After a load (the players' slots restored): Daimon's keepers come back from the staged daimon.bin, exactly as they
 *  were saved. A save without one gives every PlaF_Daimon player a new keeper (default personality); a damaged one, or
 *  one of another major, hands them all to the built-in AI with a warning -- the save itself still loads. */
void DaimonAfterLoad(void);
/** After a network game's resync set the game up again: the state is the host's own, the game Daimon was playing. On
 *  the host each keeper goes on where its seat still is -- a Daimon keeper on its slot, an assistant on its keeper's
 *  co-seat -- only its gesture in progress dropped; one whose seat is gone stops. A client plays none (as at a level
 *  start). (A resync used to run DaimonAfterLoad: every keeper was forgotten, the assistants stayed off with their
 *  co-seats on, and the Daimon keepers started again without their memory or their level-start personality.) */
void DaimonAfterResync(void);
/** Hands a computer keeper to Daimon now (or, in a local game, the local human's own seat: daimon_assign_slot), in
 *  opponent mode -- the assistant and the lieutenant start through their own entries (DaimonAssistStart,
 *  DaimonLieutenantSet) -- local games only. A take during play is not in the replay's header: use it from the
 *  console and tests, not for anything a replay must reproduce. */
TbBool DaimonTake(PlayerNumber plyr, const char *personality, const char *difficulty, DaimonMode mode, DaimonTakeReason why);
/** Gives the keeper back: DAIMON_TO_COMPUTER2 (its model before the take) for a keeper Daimon took from the built-in
 *  AI, DAIMON_TO_HUMAN for a human's own seat (DaimonSeatHandBack); false for the other way round. */
TbBool DaimonHandBack(PlayerNumber plyr, DaimonHandBackTo to);
/** "Let Daimon play this seat" (PckA_ToggleSpectate with actn_par1 1, docs/plans/ai-plans/v2/09 section 12), on every
 *  machine alike: the local human's own keeper becomes Daimon's (PlaF_Daimon | PlaF_DaimonSeat) on a slot chosen from the
 *  game's own state, and the keeper is built (`personality`, `difficulty`; NULL for the defaults) where Daimon's keepers
 *  are authored (not in a replay's playback). Local games only. False when the seat cannot be Daimon's. */
TbBool DaimonTakeSeat(PlayerNumber plyr, const char *personality, const char *difficulty);
/** The way back from DaimonTakeSeat (a toggle, the console, the level start's take): Daimon lets the seat go and it is
 *  the human's again, on every machine alike. False when Daimon does not play a human's seat there. */
TbBool DaimonSeatHandBack(PlayerNumber plyr);
TbBool DaimonControls(PlayerNumber plyr);
/** How the end screen names a Daimon keeper: "Steward (random), hard (cheats: sees all)" -- its personality's display
 *  name, revealed even when "random" chose it, and its difficulty. False when Daimon plays no keeper there. */
TbBool DaimonDescribe(PlayerNumber plyr, char *out, size_t len);
DaimonMode DaimonModeOf(PlayerNumber plyr);
/** A keeper was defeated (set_player_as_lost_level, through daimon_port): Daimon stops playing it; its flag and
 *  slot stay, as the replay recorded them. */
void DaimonOnKeeperDefeated(PlayerNumber plyr);
/** PckA_AdoptKeeper was applied (game_commands.c): the keeper of a human who left is Daimon's now; the
 *  network host plays it, as the `steady` personality (and sends the clients a copy of Daimon's state at once).
 *  Nothing on a client. */
void DaimonOnDropIn(PlayerNumber plyr);
/** PckA_RejoinSeat (game_commands.c), on every machine before the reseat: a drop-in keeper Daimon plays is let go
 *  (the host stops playing it) and its slot goes back to network user `user` (dai_release_slot_to_user). True when
 *  the keeper is no longer Daimon's afterwards. */
TbBool DaimonRejoinRelease(PlayerNumber plyr, int64_t user);
/** A client lost the host and goes on as a local game (daimon_port's on_host_lost): its Daimon keepers come back from
 *  the host's last copy of Daimon's state; one without a usable copy goes to the built-in AI. */
void DaimonOnHostLost(void);
/** Every keeper this machine authors, once per turn: its turn (02 section 5), then its controller writes the running
 *  gesture's step into its slot's packet. The game loop calls it after input() and before the packets are exchanged
 *  (game_session_loop.cpp), not in a replay's playback or while paused; in a network game on the host only (its
 *  keepers' steps go out with its packet, dai_net_host_author_slots). A second call for the same turn does nothing. */
void DaimonTickAuthors(DaiTurn now);
/** The keeper's state as lines (`what`: "status", "know", "goals", "requests", "jobs", "claims", "inbox", or "dump" for
 *  all of them); "no Daimon keeper" for any other player. */
void DaimonDump(PlayerNumber plyr, const char *what, void (*line)(const char *));

#ifdef __cplusplus
}
class DaiKeeper;
class DaiWorldReaderSim;
/** The turn driver's counters since the level started (tests, the log). */
struct DaimonDriverStats {
    int64_t ticks = 0;          /**< turns DaimonTickAuthors() played */
    int64_t repeats = 0;        /**< calls for a turn already played (did nothing) */
    int64_t keeper_turns = 0;   /**< keeper turns, each with its one packet write */
    DaiTurn last = 0;
};
const DaimonDriverStats &DaimonDriver();
struct DaiRequest;
/** C++ only (tests, kfx_daimon): the keeper of a player, or nullptr. */
const DaiKeeper *DaimonKeeper(PlayerNumber plyr);
/** Posts a request into the keeper's requests as manager `from` would (ftests and the console); its id, 0 if none. */
uint32_t DaimonPostRequest(PlayerNumber plyr, DaiDomain from, const DaiRequest &r);
struct DaiGoal;
/** Sets a goal on the keeper's table (ftests: an Attack goal as a directive, source Directive, which the director leaves
 *  alone, 07 P5.8); its id, 0 if none. */
uint32_t DaimonSetGoal(PlayerNumber plyr, const DaiGoal &g);
/** C++ only (tests, ftests): the game reader every keeper uses (its fog audit). */
DaiWorldReaderSim &DaimonWorldReader();
class DaiActorKeeperCtl;
/** C++ only (ftests): the actor every keeper writes through (its refusing world's question count). */
const DaiActorKeeperCtl &DaimonActor();
#endif

#endif
