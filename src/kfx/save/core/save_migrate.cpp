/******************************************************************************/
/** @file save_migrate.cpp
 *     Migrations for changes of meaning that name matching can't express.
 */
/******************************************************************************/
#include "save_migrate.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <vector>

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static const struct SaveMigration save_migrations[] = {
    { nullptr, 0, nullptr, nullptr }
};

uint32_t save_migration_count(void)
{
    return static_cast<uint32_t>(std::size(save_migrations)) - 1;   // the last entry only keeps the table from being empty
}

const struct SaveMigration *save_migration_get(uint32_t index)
{
    // cppcheck-suppress unsignedLessThanZero
    return (index < save_migration_count()) ? &save_migrations[index] : nullptr;
}

enum SaveResult save_migrations_run_table(struct SaveLoadCtx *context, const char *struct_name,
    const struct SaveMigration *table, uint32_t count, struct SaveError *err)
{
    // The migrations of this struct that the file's minor still needs, oldest first.
    std::vector<const SaveMigration *> todo;
    for (uint32_t index = 0; index < count; index++)
    {
        if ((strcmp(table[index].struct_name, struct_name) == 0) && (table[index].from_minor >= context->file_minor))
            todo.push_back(&table[index]);
    }
    std::stable_sort(todo.begin(), todo.end(),
        [](const SaveMigration *left, const SaveMigration *right) { return left->from_minor < right->from_minor; });
    const auto failed = std::find_if(todo.begin(), todo.end(), [context](const SaveMigration *migration) {
        return migration->migrate(context) != 0;
    });
    if (failed != todo.end())
        return save_fail(err, SVR_Unsupported, "migration %s failed", (*failed)->name);
    return SVR_Ok;
}

enum SaveResult save_migrations_run(struct SaveLoadCtx *context, const char *struct_name, struct SaveError *err)
{
    return save_migrations_run_table(context, struct_name, save_migrations, save_migration_count(), err);
}
