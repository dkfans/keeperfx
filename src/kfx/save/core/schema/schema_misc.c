/** @file schema_misc.c
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

SAVE_STRUCT(CatalogueEntry, struct CatalogueEntry,
    SAVE_FIELD(struct CatalogueEntry, flags, SV_U16),
    SAVE_STRING(struct CatalogueEntry, textname),  /* plain-char */
    SAVE_FIELD(struct CatalogueEntry, level_num, SV_I32),
    SAVE_STRING(struct CatalogueEntry, campaign_name),  /* plain-char */
    SAVE_STRING(struct CatalogueEntry, campaign_fname),  /* plain-char */
    SAVE_STRING(struct CatalogueEntry, player_name),  /* plain-char */
    SAVE_FIELD(struct CatalogueEntry, game_ver_major, SV_U16),
    SAVE_FIELD(struct CatalogueEntry, game_ver_minor, SV_U16),
    SAVE_FIELD(struct CatalogueEntry, game_ver_release, SV_U16),
    SAVE_FIELD(struct CatalogueEntry, game_ver_build, SV_U16),
);

SAVE_STRUCT(Column, struct Column,
    SAVE_FIELD(struct Column, use, SV_I16),
    SAVE_FIELD(struct Column, bitfields, SV_U8),
    SAVE_FIELD(struct Column, solidmask, SV_U16),
    SAVE_FIELD(struct Column, floor_texture, SV_U16),
    SAVE_FIELD(struct Column, orient, SV_U8),
    SAVE_ARRAY(struct Column, cubes, SV_U16),
);

SAVE_STRUCT(ConfigInfo, struct ConfigInfo,
    SAVE_STRING(struct ConfigInfo, str_join),  /* plain-char */
    SAVE_STRING(struct ConfigInfo, net_player_name),  /* plain-char */
    SAVE_STRING(struct ConfigInfo, net_lobby_name),  /* plain-char */
    SAVE_FIELD(struct ConfigInfo, max_players, SV_U8),
    SAVE_FIELD(struct ConfigInfo, spectators_enabled, SV_U8),
    SAVE_FIELD(struct ConfigInfo, spectator_chat, SV_U8),
);

SAVE_STRUCT(ContinueData, struct ContinueData,
    SAVE_RUNTIME(struct ContinueData, reserved),
    SAVE_STRING(struct ContinueData, link_fname),  /* plain-char */
);

SAVE_STRUCT(Coord2d, struct Coord2d,
    SAVE_FIELD(struct Coord2d, x.val, SV_I32),  /* union-view */
    SAVE_FIELD(struct Coord2d, y.val, SV_I32),  /* union-view */
);

SAVE_STRUCT(Coord3d, struct Coord3d,
    SAVE_FIELD(struct Coord3d, x.val, SV_I32),  /* union-view */
    SAVE_FIELD(struct Coord3d, y.val, SV_I32),  /* union-view */
    SAVE_FIELD(struct Coord3d, z.val, SV_I32),  /* union-view */
);

SAVE_STRUCT(CoordDelta3d, struct CoordDelta3d,
    SAVE_FIELD(struct CoordDelta3d, x.val, SV_I32),  /* union-view */
    SAVE_FIELD(struct CoordDelta3d, y.val, SV_I32),  /* union-view */
    SAVE_FIELD(struct CoordDelta3d, z.val, SV_I32),  /* union-view */
);

SAVE_STRUCT(CreatureStorage, struct CreatureStorage,
    SAVE_FIELD(struct CreatureStorage, model, SV_I16),
    SAVE_FIELD(struct CreatureStorage, exp_level, SV_U8),
    SAVE_FIELD(struct CreatureStorage, count, SV_U8),
    SAVE_STRING(struct CreatureStorage, creature_name),  /* plain-char */
);

SAVE_STRUCT(FileChunkHeader, struct FileChunkHeader,
    SAVE_FIELD(struct FileChunkHeader, len, SV_U32),
    SAVE_FIELD(struct FileChunkHeader, id, SV_U32),
    SAVE_FIELD(struct FileChunkHeader, ver, SV_U32),
);

SAVE_STRUCT(HighScore, struct HighScore,
    SAVE_FIELD(struct HighScore, score, SV_I32),
    SAVE_STRING(struct HighScore, name),  /* plain-char */
    SAVE_FIELD(struct HighScore, lvnum, SV_I32),
);

SAVE_STRUCT(IntralevelData, struct IntralevelData,
    SAVE_ARRAY(struct IntralevelData, bonuses_found, SV_U8),
    SAVE_SUB_ARRAY2(struct IntralevelData, transferred_creatures, CreatureStorage),
    SAVE_ARRAY2(struct IntralevelData, campaign_flags, SV_I32),
    SAVE_FIELD(struct IntralevelData, next_level, SV_I32),
    SAVE_SUB_ARRAY(struct IntralevelData, ensign_overrides, LevelEnsignOverride),
    SAVE_FIELD(struct IntralevelData, continue_level, SV_U32),
    SAVE_ARRAY(struct IntralevelData, levels_completed, SV_U32),
);

SAVE_STRUCT(LevelEnsignOverride, struct LevelEnsignOverride,
    SAVE_FIELD(struct LevelEnsignOverride, lvnum, SV_I32),
    SAVE_FIELD(struct LevelEnsignOverride, active, SV_BOOL8),
    SAVE_FIELD(struct LevelEnsignOverride, ensign_type, SV_U16),
);

SAVE_STRUCT(MapTask, struct MapTask,
    SAVE_FIELD(struct MapTask, kind, SV_U8),
    SAVE_FIELD(struct MapTask, coords, SV_I32),
);

SAVE_STRUCT(Packet, struct Packet,
    SAVE_FIELD(struct Packet, turn, SV_U32),
    SAVE_FIELD(struct Packet, checksum, SV_U32),
    SAVE_FIELD(struct Packet, input_lag_turns, SV_I8),
    SAVE_FIELD(struct Packet, action, SV_U8),
    SAVE_FIELD(struct Packet, actn_par1, SV_I32),
    SAVE_FIELD(struct Packet, actn_par2, SV_I32),
    SAVE_FIELD(struct Packet, pos_x, SV_I32),
    SAVE_FIELD(struct Packet, pos_y, SV_I32),
    SAVE_FIELD(struct Packet, control_flags, SV_U32),
    SAVE_FIELD(struct Packet, additional_packet_values, SV_U8),
    SAVE_FIELD_EX(struct Packet, cam_x, SV_U16, .aliases = (const char *const[]){ "actn_par3", NULL }),  /* union-view */
    SAVE_FIELD_EX(struct Packet, cam_y, SV_U16, .aliases = (const char *const[]){ "actn_par4", NULL }),  /* union-view */
);

SAVE_STRUCT(PacketSaveHead, struct PacketSaveHead,
    SAVE_FIELD(struct PacketSaveHead, game_ver_major, SV_U16),
    SAVE_FIELD(struct PacketSaveHead, game_ver_minor, SV_U16),
    SAVE_FIELD(struct PacketSaveHead, game_ver_release, SV_U16),
    SAVE_FIELD(struct PacketSaveHead, game_ver_build, SV_U16),
    SAVE_FIELD(struct PacketSaveHead, level_num, SV_U32),
    SAVE_FIELD(struct PacketSaveHead, players_exist, SV_U16),
    SAVE_FIELD(struct PacketSaveHead, players_comp, SV_U16),
    SAVE_FIELD(struct PacketSaveHead, flags, SV_U8),  /* PacketSaveHeadFlags bitmask */
    SAVE_FIELD(struct PacketSaveHead, action_seed, SV_U32),
    SAVE_FIELD(struct PacketSaveHead, timestamp, SV_I64),
    SAVE_FIELD(struct PacketSaveHead, timestamp_tz, SV_I32),
    SAVE_FIELD(struct PacketSaveHead, timestamp_turn, SV_U32),
    SAVE_ARRAY(struct PacketSaveHead, map_checksums, SV_U32),
    SAVE_ARRAY2(struct PacketSaveHead, user_prefs, SV_U32),
    SAVE_ARRAY(struct PacketSaveHead, user_players, SV_I8),
    SAVE_FIELD(struct PacketSaveHead, recording_user, SV_I8),
    SAVE_FIELD(struct PacketSaveHead, frontend_alliances, SV_I8),
    SAVE_ARRAY2(struct PacketSaveHead, user_names, SV_STR),  /* plain-char */
);
