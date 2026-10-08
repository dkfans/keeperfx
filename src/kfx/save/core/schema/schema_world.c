/** @file schema_world.c
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

SAVE_STRUCT(ActionPoint, struct ActionPoint,
    SAVE_FIELD(struct ActionPoint, exists, SV_BOOL8),
    SAVE_SUB(struct ActionPoint, mappos, Coord2d),
    SAVE_FIELD(struct ActionPoint, range, SV_U16),
    SAVE_FIELD(struct ActionPoint, num, SV_U16),
    SAVE_FIELD(struct ActionPoint, activated, SV_U16),
);

SAVE_STRUCT(Event, struct Event,
    SAVE_FIELD(struct Event, flags, SV_U8),
    SAVE_FIELD(struct Event, index, SV_U8),
    SAVE_FIELD(struct Event, mappos_x, SV_I32),
    SAVE_FIELD(struct Event, mappos_y, SV_I32),
    SAVE_FIELD(struct Event, owner, SV_U8),
    SAVE_FIELD(struct Event, kind, SV_U8),
    SAVE_FIELD(struct Event, target, SV_I32),
    SAVE_FIELD(struct Event, lifespan_turns, SV_U32),
    SAVE_FIELD(struct Event, icon_idx, SV_I16),
);

SAVE_STRUCT(GoldLookup, struct GoldLookup,
    SAVE_FIELD(struct GoldLookup, flags, SV_U8),
    SAVE_ARRAY(struct GoldLookup, player_interested, SV_U8),
    SAVE_FIELD(struct GoldLookup, stl_x, SV_I32),
    SAVE_FIELD(struct GoldLookup, stl_y, SV_I32),
    SAVE_FIELD(struct GoldLookup, num_gold_slabs, SV_U16),
    SAVE_FIELD(struct GoldLookup, num_gem_slabs, SV_U32),
);

SAVE_STRUCT(Light, struct Light,
    SAVE_FIELD(struct Light, flags, SV_U8),
    SAVE_FIELD(struct Light, flags2, SV_U8),
    SAVE_FIELD(struct Light, intensity, SV_U8),
    SAVE_FIELD(struct Light, intensity_toggling_field, SV_U8),
    SAVE_FIELD(struct Light, intensity_delta, SV_U8),
    SAVE_FIELD(struct Light, range, SV_U8),
    SAVE_FIELD(struct Light, radius_oscillation_direction, SV_U8),
    SAVE_FIELD(struct Light, max_intensity, SV_U8),
    SAVE_FIELD(struct Light, min_radius, SV_U8),
    SAVE_FIELD(struct Light, index, SV_U16),
    SAVE_FIELD(struct Light, shadow_index, SV_U16),
    SAVE_FIELD(struct Light, attached_slb, SV_U32),
    SAVE_FIELD(struct Light, radius, SV_U16),
    SAVE_FIELD(struct Light, force_render_update, SV_U16),
    SAVE_FIELD(struct Light, radius_delta, SV_U16),
    SAVE_FIELD(struct Light, max_radius, SV_U16),
    SAVE_FIELD(struct Light, min_radius2, SV_U16),
    SAVE_FIELD(struct Light, min_intensity, SV_U16),
    SAVE_FIELD(struct Light, next_in_list, SV_U16),
    SAVE_SUB(struct Light, mappos, Coord3d),
    SAVE_SUB(struct Light, previous_mappos, Coord3d),
    SAVE_FIELD(struct Light, intensity_random, SV_U16),
    SAVE_FIELD(struct Light, previous_intensity_random, SV_U16),
    SAVE_FIELD(struct Light, last_turn_moved, SV_U32),
    SAVE_FIELD(struct Light, last_turn_randomized, SV_U32),
    SAVE_FIELD(struct Light, reset_interpolation, SV_BOOL8),
);

SAVE_STRUCT(LightSystemState, struct LightSystemState,
    SAVE_ARRAY(struct LightSystemState, bitmask, SV_I32),
    SAVE_FIELD(struct LightSystemState, static_light_needs_updating, SV_I32),
    SAVE_FIELD(struct LightSystemState, total_dynamic_lights, SV_I32),
    SAVE_FIELD(struct LightSystemState, total_stat_lights, SV_I32),
    SAVE_FIELD(struct LightSystemState, rendered_dynamic_lights, SV_I32),
    SAVE_FIELD(struct LightSystemState, rendered_optimised_dynamic_lights, SV_I32),
    SAVE_FIELD(struct LightSystemState, updated_stat_lights, SV_I32),
    SAVE_FIELD(struct LightSystemState, out_of_date_stat_lights, SV_I32),
);

SAVE_STRUCT(LightingTable, struct LightingTable,
    SAVE_FIELD(struct LightingTable, is_populated, SV_BOOL8),
    SAVE_FIELD(struct LightingTable, distance, SV_U8),
    SAVE_FIELD(struct LightingTable, delta_x, SV_I8),
    SAVE_FIELD(struct LightingTable, delta_y, SV_I8),
    SAVE_FIELD(struct LightingTable, diagonal_length, SV_U32),
);

SAVE_STRUCT(LightsShadows, struct LightsShadows,
    SAVE_DERIVED(struct LightsShadows, lighting_tables, "light_initialise_lighting_tables"),
    SAVE_ARRAY(struct LightsShadows, shadow_limits, SV_U8),
    SAVE_SUB_ARRAY(struct LightsShadows, lights, Light),
    SAVE_DERIVED(struct LightsShadows, shadow_cache, "light rendering each frame"),
    SAVE_DERIVED(struct LightsShadows, stat_light_map, "clear_stat_light_map + light rendering"),
    SAVE_FIELD(struct LightsShadows, global_ambient_light, SV_I32),
    SAVE_FIELD(struct LightsShadows, light_enabled, SV_BOOL8),
    SAVE_FIELD(struct LightsShadows, light_auto_sync, SV_BOOL8),
    SAVE_DERIVED(struct LightsShadows, lighting_tables_initialised, "light_initialise_lighting_tables"),
    SAVE_DERIVED(struct LightsShadows, lighting_tables_count, "light_initialise_lighting_tables"),
    SAVE_DERIVED(struct LightsShadows, subtile_lightness, "clear_subtiles_lightness + light rendering"),
);

SAVE_STRUCT(Map, struct Map,
    SAVE_FIELD(struct Map, flags, SV_U8),
    SAVE_FIELD(struct Map, filled_subtiles, SV_U8),
    SAVE_FIELD(struct Map, wibble_value, SV_U8),
    SAVE_FIELD(struct Map, col_idx, SV_I16),
    SAVE_FIELD(struct Map, mapwho, SV_U16),
    SAVE_FIELD(struct Map, revealed, SV_U16),
);

SAVE_STRUCT(Room, struct Room,
    SAVE_FIELD(struct Room, alloc_flags, SV_U8),
    SAVE_FIELD(struct Room, index, SV_U16),
    SAVE_FIELD(struct Room, owner, SV_I8),
    SAVE_FIELD(struct Room, prev_of_owner, SV_U16),
    SAVE_FIELD(struct Room, next_of_owner, SV_U16),
    SAVE_FIELD(struct Room, central_stl_x, SV_I32),
    SAVE_FIELD(struct Room, central_stl_y, SV_I32),
    SAVE_FIELD_EX(struct Room, kind, SV_U8, .flags = 0, .name_domain = SND_RoomKind),
    SAVE_FIELD(struct Room, health, SV_I32),
    SAVE_FIELD(struct Room, total_capacity, SV_U16),
    SAVE_FIELD(struct Room, used_capacity, SV_U16),
    SAVE_ARRAY(struct Room, player_interested, SV_U8),
    SAVE_FIELD(struct Room, capacity_used_for_storage, SV_U32),
    SAVE_DERIVED(struct Room, cached_nearby_creature_index, "self-invalidating per-turn cache, thing_objects.c"),
    SAVE_FIELD(struct Room, prev_of_kind, SV_U16),
    SAVE_FIELD(struct Room, next_of_kind, SV_U16),
    SAVE_ARRAY(struct Room, content_per_model, SV_U8),
    SAVE_FIELD(struct Room, hatch_gameturn, SV_U32),
    SAVE_FIELD(struct Room, slabs_list, SV_U32),
    SAVE_FIELD(struct Room, slabs_list_tail, SV_U32),
    SAVE_FIELD(struct Room, slabs_count, SV_U16),
    SAVE_FIELD(struct Room, creatures_list, SV_U16),
    SAVE_FIELD(struct Room, efficiency, SV_U16),
    SAVE_FIELD(struct Room, flame_slb, SV_U32),
    SAVE_FIELD(struct Room, flames_around_idx, SV_U8),
    SAVE_FIELD(struct Room, flame_stl, SV_U8),
    SAVE_FIELD(struct Room, creation_turn, SV_U32),
);

SAVE_STRUCT(ScriptFxLine, struct ScriptFxLine,
    SAVE_FIELD(struct ScriptFxLine, used, SV_I32),
    SAVE_SUB(struct ScriptFxLine, from, Coord3d),
    SAVE_SUB(struct ScriptFxLine, here, Coord3d),
    SAVE_SUB(struct ScriptFxLine, to, Coord3d),
    SAVE_FIELD(struct ScriptFxLine, cx, SV_I32),
    SAVE_FIELD(struct ScriptFxLine, cy, SV_I32),
    SAVE_FIELD(struct ScriptFxLine, curvature, SV_I32),
    SAVE_FIELD(struct ScriptFxLine, spatial_step, SV_I32),
    SAVE_FIELD(struct ScriptFxLine, steps_per_turn, SV_I32),
    SAVE_FIELD(struct ScriptFxLine, partial_steps, SV_I32),
    SAVE_FIELD(struct ScriptFxLine, effect, SV_I32),
    SAVE_FIELD(struct ScriptFxLine, total_steps, SV_I32),
    SAVE_FIELD(struct ScriptFxLine, step, SV_I32),
);

SAVE_STRUCT(ShadowCache, struct ShadowCache,
    SAVE_FIELD(struct ShadowCache, flags, SV_U8),
    SAVE_ARRAY(struct ShadowCache, lighting_bitmask, SV_U32),
);

SAVE_STRUCT(SlabMap, struct SlabMap,
    SAVE_FIELD(struct SlabMap, next_in_room, SV_U32),
    SAVE_FIELD(struct SlabMap, health, SV_I32),
    SAVE_FIELD(struct SlabMap, kind, SV_U8),
    SAVE_FIELD(struct SlabMap, room_index, SV_U16),
    SAVE_FIELD(struct SlabMap, wlb_type, SV_U8),
    SAVE_FIELD(struct SlabMap, owner, SV_I8),
);

SAVE_STRUCT(SlabObj, struct SlabObj,
    SAVE_FIELD(struct SlabObj, isLight, SV_BOOL8),
    SAVE_FIELD(struct SlabObj, slabset_id, SV_I16),
    SAVE_FIELD(struct SlabObj, stl_id, SV_U8),
    SAVE_FIELD(struct SlabObj, offset_x, SV_I16),
    SAVE_FIELD(struct SlabObj, offset_y, SV_I16),
    SAVE_FIELD(struct SlabObj, offset_z, SV_I16),
    SAVE_FIELD(struct SlabObj, class_id, SV_U8),
    SAVE_FIELD(struct SlabObj, model, SV_I16),
    SAVE_FIELD(struct SlabObj, range, SV_U8),
);

SAVE_STRUCT(SlabSet, struct SlabSet,
    SAVE_ARRAY(struct SlabSet, col_idx, SV_I16),
);
