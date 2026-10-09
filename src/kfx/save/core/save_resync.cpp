/******************************************************************************/
/** @file save_resync.cpp
 *     Game integration for the save codec: encodes/decodes struct Game for
 *     network resync.
 */
/******************************************************************************/
#include "kfx/save/core/save_resync.h"
#include "kfx/save/core/save_schema.h"
#include "kfx/save/core/schema/save_tables_decl.h"
/******************************************************************************/
enum SaveResult save_resync_encode_game(struct SaveBuffer *out, const struct Game *game_state, struct SaveError *err)
{
    return save_encode_record(out, &save_desc_Game, game_state, SVM_Sync, err);
}

enum SaveResult save_resync_decode_game(const uint8_t *data, uint32_t len, struct Game *game_state, struct SaveError *err)
{
    SaveBufferOwner schema_buffer;
    const SaveStructDesc *roots[1] = { &save_desc_Game };
    enum SaveResult result = save_schema_encode(&schema_buffer, roots, 1, SVM_Sync, err);
    if (result != SVR_Ok)
        return result;
    SaveFileSchemaOwner schema;
    result = save_schema_decode(schema_buffer.data, schema_buffer.len, &schema, err);
    if (result != SVR_Ok)
        return result;
    SaveDecoderOwner decoder;
    save_decoder_init(&decoder, &schema, SVM_Sync);
    uint32_t used;
    return save_decode_record(&decoder, &save_desc_Game, data, len, &used, game_state, err);
}
