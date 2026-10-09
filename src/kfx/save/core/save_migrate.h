/******************************************************************************/
/** @file save_migrate.h
 *     Migrations for changes of meaning that name matching can't express.
 */
/******************************************************************************/
#ifndef KFX_SAVE_MIGRATE_H
#define KFX_SAVE_MIGRATE_H

#include <stdint.h>

#include "save_schema.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct SaveLoadCtx {
    void *state;                        /* the decoded state the migration edits */
    const struct SaveDecoder *decoder;  /* holds the stash of fields this build dropped */
    uint16_t file_minor;
};

struct SaveMigration {
    const char *struct_name;
    uint16_t from_minor;                /* runs for files whose minor is <= this */
    const char *name;
    int (*migrate)(struct SaveLoadCtx *context); /* returns 0 on success */
};

uint32_t save_migration_count(void);
const struct SaveMigration *save_migration_get(uint32_t index);

/** Runs the migrations of struct_name in the table that apply to the file's minor, ordered by
 *  from_minor then table order. */
enum SaveResult save_migrations_run_table(struct SaveLoadCtx *context, const char *struct_name,
    const struct SaveMigration *table, uint32_t count, struct SaveError *err);
enum SaveResult save_migrations_run(struct SaveLoadCtx *context, const char *struct_name, struct SaveError *err);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
