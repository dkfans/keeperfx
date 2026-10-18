// Schema as text, for the CI schema diff (kfx-savetool schema --build). Not part of the game.
#ifndef KFX_SAVE_TEXT_H
#define KFX_SAVE_TEXT_H

#include "kfx/save/core/save_schema.h"

/** Stable sorted text, one line per field. */
enum SaveResult save_schema_text_build(struct SaveBuffer *out, const struct SaveStructDesc *const *roots,
    uint32_t root_count, enum SaveMode mode, struct SaveError *err);

#endif
