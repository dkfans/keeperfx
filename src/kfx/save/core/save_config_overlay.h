/******************************************************************************/
/** @file save_config_overlay.h
 *     CONF chunk: only the config fields a level changed since its baseline.
 *     Diffed and applied by field path, so a config struct layout change never
 *     breaks a save - a field that's gone just isn't in the overlay, or is
 *     skipped as an unknown path on apply.
 */
/******************************************************************************/
#ifndef KFX_SAVE_CONFIG_OVERLAY_H
#define KFX_SAVE_CONFIG_OVERLAY_H

#include <stdint.h>

#include "kfx/save/core/save_codec.h"

struct Configs;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** Writes the CONF chunk body (not yet wrapped in a SaveChunk): every field where cur differs
 *  from baseline, by field path. */
enum SaveResult save_config_overlay_write(const struct Configs *current, const struct Configs *baseline,
    struct SaveBuffer *out, struct SaveError *err);

/** Applies a CONF chunk body onto cur (normally freshly loaded from config files). A path
 *  this build has no matching field for is skipped, not refused. */
enum SaveResult save_config_overlay_apply(const uint8_t *conf_data, uint32_t conf_len,
    struct Configs *current, struct SaveError *err);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
