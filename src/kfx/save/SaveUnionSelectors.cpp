/******************************************************************************/
/** @file SaveUnionSelectors.cpp
 *     The functions that tell the save tables which member of a union is in use, where that depends on the game's
 *     config and not only on the struct.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "globals.h"
#include "config_objects.h"
#include "post_inc.h"

#include "kfx/save/SaveUnionArms.h"

namespace {

int64_t thing_arm(const void *owner)
{
    const auto *thing = static_cast<const Thing *>(owner);
    int genre = -1;
    if (thing->class_id == TCls_Object)
    {
        const ObjectConfigStats *stats = get_object_model_stats(thing->model);
        if (stats != nullptr)
            genre = stats->genre;
    }
    return save_thing_arm(thing, genre);
}

// Registers the selectors before main() runs.
struct Registration {
    Registration() { save_register_union_selector("thing_arm", thing_arm); }
} registration;

} // namespace
