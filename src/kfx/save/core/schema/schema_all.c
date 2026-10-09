/** @file schema_all.c
 *     Root struct tables of everything the game saves.
 */
#include "kfx/save/core/save_tables.h"
#include "kfx/save/core/schema/save_tables_decl.h"

static const struct SaveStructDesc *const save_roots[] = {
    &save_desc_Game,
    &save_desc_CatalogueEntry,
    &save_desc_FileChunkHeader,
    &save_desc_IntralevelData,
    &save_desc_PacketSaveHead,
    &save_desc_Packet,
    &save_desc_HighScore,
    &save_desc_ConfigInfo,
};

const struct SaveStructDesc *const *save_root_structs(uint32_t *count)
{
    *count = (uint32_t)(sizeof(save_roots) / sizeof(save_roots[0]));
    return save_roots;
}
