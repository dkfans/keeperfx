// Checks that a field table is consistent with itself: sizes, offsets, sub-structs, unions. The
// tables are constants, so this runs in the tests and in kfx-savetool, not in the game.
#ifndef KFX_SAVE_VALIDATE_H
#define KFX_SAVE_VALIDATE_H

#include "kfx/save/core/save_schema.h"

enum SaveResult save_desc_validate(const struct SaveStructDesc *d, struct SaveError *err);

#endif
