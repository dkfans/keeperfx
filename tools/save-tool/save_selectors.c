// The union selectors the host tool uses. The tables name them; the game registers its own.
#include "save_selectors.h"

#include "pre_inc.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "post_inc.h"
#include "kfx/save/SaveUnionArms.h"

void save_tool_register_selectors(void)
{
    save_register_standalone_selectors();
}
