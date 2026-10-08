/******************************************************************************/
/** @file save_resync.h
 *     Game integration for the save codec: encodes/decodes struct Game for
 *     network resync, in SVM_Sync mode. field tables are matched for portability (it makes QQ happy).
 */
/******************************************************************************/
#ifndef KFX_SAVE_RESYNC_H
#define KFX_SAVE_RESYNC_H

#include "save_codec.h"

struct Game;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

enum SaveResult save_resync_encode_game(struct SaveBuffer *out, const struct Game *game_state, struct SaveError *err);
enum SaveResult save_resync_decode_game(const uint8_t *data, uint32_t len, struct Game *game_state, struct SaveError *err);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
