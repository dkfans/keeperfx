// Host tool for the save codec. No game or SDL code.
// Build with tools/save-tool/build.sh
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include "kfx/save/core/save_codec.h"
#include "kfx/save/core/save_migrate.h"
#include "kfx/save/core/save_schema.h"
#include "kfx/save/core/save_tables.h"
#include "kfx/save/core/schema/save_tables_decl.h"
#include "save_selectors.h"
#include "save_text.h"
#include "save_validate.h"

static int fail(const struct SaveError *err)
{
    fprintf(stderr, "%s\n", err->message);
    return (int)err->result;
}

static int cmd_schema_build(void)
{
    struct SaveError err;
    struct SaveBuffer out = { NULL, 0, 0 };
    uint32_t count;
    const struct SaveStructDesc *const *roots = save_root_structs(&count);
    enum SaveResult r = SVR_Ok;
    for (uint32_t i = 0; (i < count) && (r == SVR_Ok); i++)
        r = save_desc_validate(roots[i], &err);
    if (r == SVR_Ok)
        r = save_schema_text_build(&out, roots, count, SVM_Save, &err);
    if (r != SVR_Ok)
        return fail(&err);
    printf("format %d.%d\n", SAVE_FORMAT_MAJOR, SAVE_FORMAT_MINOR);
    if (out.len > 0)
        fwrite(out.data, 1, out.len, stdout);
    save_buf_free(&out);
    return 0;
}

static int cmd_migrations(void)
{
    for (uint32_t i = 0; i < save_migration_count(); i++)
    {
        const struct SaveMigration *m = save_migration_get(i);
        printf("%s %u %s\n", m->struct_name, (unsigned)m->from_minor, m->name);
    }
    return 0;
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY); /* schema text has to be the same on every platform */
#endif
    save_tool_register_selectors();
    if ((argc == 3) && (strcmp(argv[1], "schema") == 0) && (strcmp(argv[2], "--build") == 0))
        return cmd_schema_build();
    if ((argc == 2) && (strcmp(argv[1], "migrations") == 0))
        return cmd_migrations();
    fprintf(stderr, "usage: kfx-savetool schema --build | migrations\n");
    return 64;
}
