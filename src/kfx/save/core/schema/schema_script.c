/** @file schema_script.c
 *     Field tables for saved structs.
 */
#include "pre_inc.h"
#include "kfx/save/SaveTypes.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "packets.h"
#include "config_campaigns.h"
#include "front_network.h"
#include "post_inc.h"
#include "kfx/save/core/save_schema.h"
#include "kfx/save/core/schema/save_tables_decl.h"

SAVE_STRUCT(Condition, struct Condition,
    SAVE_FIELD(struct Condition, condit_idx, SV_I16),
    SAVE_FIELD(struct Condition, status, SV_U8),
    SAVE_FIELD(struct Condition, plyr_range, SV_U8),
    SAVE_FIELD(struct Condition, variabl_type, SV_U8),
    SAVE_FIELD(struct Condition, variabl_idx, SV_U16),
    SAVE_FIELD(struct Condition, operation, SV_U8),
    SAVE_FIELD(struct Condition, rvalue, SV_U32),
    SAVE_FIELD(struct Condition, plyr_range_right, SV_U8),
    SAVE_FIELD(struct Condition, variabl_type_right, SV_U8),
    SAVE_FIELD(struct Condition, variabl_idx_right, SV_U16),
    SAVE_FIELD(struct Condition, use_second_variable, SV_BOOL8),
);

SAVE_STRUCT(LevelScript, struct LevelScript,
    SAVE_SUB_ARRAY(struct LevelScript, tunneller_triggers, TunnellerTrigger),
    SAVE_FIELD(struct LevelScript, tunneller_triggers_num, SV_U32),
    SAVE_SUB_ARRAY(struct LevelScript, party_triggers, PartyTrigger),
    SAVE_FIELD(struct LevelScript, party_triggers_num, SV_U32),
    SAVE_SUB_ARRAY(struct LevelScript, values, ScriptValue),
    SAVE_FIELD(struct LevelScript, values_num, SV_U32),
    SAVE_SUB_ARRAY(struct LevelScript, conditions, Condition),
    SAVE_FIELD(struct LevelScript, conditions_num, SV_U32),
    SAVE_SUB_ARRAY(struct LevelScript, creature_partys, Party),
    SAVE_FIELD(struct LevelScript, creature_partys_num, SV_U32),
    SAVE_ARRAY(struct LevelScript, win_conditions, SV_U16),
    SAVE_FIELD(struct LevelScript, win_conditions_num, SV_U32),
    SAVE_ARRAY(struct LevelScript, lose_conditions, SV_U16),
    SAVE_FIELD(struct LevelScript, lose_conditions_num, SV_U32),
    SAVE_STRING(struct LevelScript, strings),  /* plain-char */
    SAVE_FIELD(struct LevelScript, next_string_offset, SV_I32),
);

SAVE_STRUCT(Party, struct Party,
    SAVE_STRING(struct Party, prtname),  /* plain-char */
    SAVE_SUB_ARRAY(struct Party, members, PartyMember),
    SAVE_FIELD(struct Party, members_num, SV_U32),
);

SAVE_STRUCT(PartyMember, struct PartyMember,
    SAVE_FIELD(struct PartyMember, flags, SV_U8),
    SAVE_FIELD(struct PartyMember, crtr_kind, SV_I16),
    SAVE_FIELD(struct PartyMember, objectv, SV_U8),
    SAVE_FIELD(struct PartyMember, countdown, SV_I32),
    SAVE_FIELD(struct PartyMember, exp_level, SV_U8),
    SAVE_FIELD(struct PartyMember, carried_gold, SV_U16),
    SAVE_FIELD(struct PartyMember, is_active, SV_U16),
    SAVE_FIELD(struct PartyMember, target, SV_I8),
);

SAVE_STRUCT(PartyTrigger, struct PartyTrigger,
    SAVE_FIELD(struct PartyTrigger, flags, SV_U8),
    SAVE_FIELD(struct PartyTrigger, condit_idx, SV_U16),
    SAVE_FIELD(struct PartyTrigger, creatr_id, SV_I8),
    SAVE_FIELD(struct PartyTrigger, plyr_idx, SV_U8),  /* union-view */
    SAVE_FIELD(struct PartyTrigger, location, SV_U32),  /* union-view */
    SAVE_FIELD(struct PartyTrigger, spawn_type, SV_I8),
    SAVE_FIELD(struct PartyTrigger, exp_level, SV_U8),
    SAVE_FIELD(struct PartyTrigger, carried_gold, SV_U16),
    SAVE_FIELD(struct PartyTrigger, ncopies, SV_U16),  /* union-view */
);

SAVE_STRUCT(ScriptValue, struct ScriptValue,
    SAVE_FIELD(struct ScriptValue, flags, SV_U8),
    SAVE_FIELD(struct ScriptValue, condit_idx, SV_U16),
    SAVE_FIELD(struct ScriptValue, valtype, SV_U8),
    SAVE_FIELD(struct ScriptValue, plyr_range, SV_U8),
    SAVE_OPAQUE(struct ScriptValue, sac, "union:sac", (uint32_t)(sizeof(struct ScriptValue) - offsetof(struct ScriptValue, sac))),  /* union */
);

SAVE_STRUCT(ScriptVariable, struct ScriptVariable,
    SAVE_FIELD(struct ScriptVariable, value_type, SV_U8),
    SAVE_FIELD(struct ScriptVariable, value_id, SV_U8),
    SAVE_FIELD(struct ScriptVariable, variable_player, SV_I8),
    SAVE_FIELD(struct ScriptVariable, variable_target, SV_I32),
    SAVE_FIELD(struct ScriptVariable, variable_target_type, SV_U8),
    SAVE_FIELD(struct ScriptVariable, include_icon, SV_BOOL8),
    SAVE_FIELD(struct ScriptVariable, icon_idx, SV_I16),
    SAVE_FIELD(struct ScriptVariable, is_active, SV_BOOL8),
);

SAVE_STRUCT(TunnellerTrigger, struct TunnellerTrigger,
    SAVE_FIELD(struct TunnellerTrigger, flags, SV_U8),
    SAVE_FIELD(struct TunnellerTrigger, condit_idx, SV_U16),
    SAVE_FIELD(struct TunnellerTrigger, plyr_idx, SV_U8),
    SAVE_FIELD(struct TunnellerTrigger, location, SV_U32),
    SAVE_FIELD(struct TunnellerTrigger, heading, SV_U32),
    SAVE_FIELD(struct TunnellerTrigger, carried_gold, SV_I32),
    SAVE_FIELD(struct TunnellerTrigger, exp_level, SV_U8),
    SAVE_FIELD(struct TunnellerTrigger, party_id, SV_I8),
);
