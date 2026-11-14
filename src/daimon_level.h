/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_level.h
 *     A level's DAIMON key: the keepers Daimon plays from the level start, in its campaign [mapN] block or .lof:
 *       DAIMON = PLAYER1:warlord:normal PLAYER2:steward:hard
 *       DAIMON = ALL:steward              ; every keeper the level gives the built-in AI
 *       DAIMON = MODELS                   ; each by its COMPUTER_PLAYER model, through daimon.toml's [model_map]
 */
/******************************************************************************/
#ifndef DK_DAIMON_LEVEL_H
#define DK_DAIMON_LEVEL_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LEVEL_DAIMON_LEN 256

/** One entry of a DAIMON key: a player (PLAYER0..), every AI keeper (DAIMON_KEY_ALL) or each by its built-in model's
 *  daimon.toml [model_map] personality (DAIMON_KEY_MODELS); personality and difficulty as written ("" none). */
#define DAIMON_KEY_ALL    (-1)
#define DAIMON_KEY_MODELS (-2)
#define DAIMON_KEY_MAX_ENTRIES 16
struct DaimonLevelKeyEntry {
  int64_t player;
  char personality[32];
  char difficulty[64];
};

/** The entries of a DAIMON key's text (at most max); a malformed entry is warned about and skipped. */
int64_t daimon_level_key_parse(const char *text, struct DaimonLevelKeyEntry *out, int64_t max);
/** At the level start, after its script has run: the keepers the loaded level's DAIMON key names, queued for Daimon. */
void daimon_queue_level_key(void);

#ifdef __cplusplus
}
#endif
#endif
