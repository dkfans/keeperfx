/******************************************************************************/
/** @file UserDirsMigrate.cpp
 *     Copies a player's files from a portable install into the per-user folders.
 */
/******************************************************************************/
#include "kfx/platform/UserDirsMigrate.h"
#include "kfx/platform/UserDirsFs.h"
#include "kfx/platform/UserDirsPolicy.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <system_error>

namespace fs = std::filesystem;

namespace userdirs {

const char *const migration_record_name = "migrated_installs.txt";

/** Folders of the install whose files belong to the player. */
static const char *const migrated_folders[] = {"save", "replays", "scrshots"};

static bool SameContent(const fs::path &a, const fs::path &b)
{
    std::ifstream fa(a, std::ios::binary);
    std::ifstream fb(b, std::ios::binary);
    if (!fa || !fb)
        return false;
    char ba[65536];
    char bb[65536];
    for (;;) {
        fa.read(ba, sizeof(ba));
        fb.read(bb, sizeof(bb));
        const std::streamsize na = fa.gcount();
        const std::streamsize nb = fb.gcount();
        if ((na != nb) || (memcmp(ba, bb, (size_t)na) != 0))
            return false;
        if (na == 0)
            return fa.eof() && fb.eof();
    }
}

CopyOutcome CopyOnce(const std::string &from, const std::string &to, std::string &message)
{
    std::error_code ec;
    const fs::path src = ToPath(from);
    const fs::path dst = ToPath(to);
    if (fs::exists(dst, ec)) {
        message = "kept " + to + (SameContent(src, dst) ? "" : ", which differs from " + from);
        return CopyOutcome::Kept;
    }
    std::string error;
    if (!MakeDirs(FromPath(dst.parent_path()), error)) {
        message = "can't copy " + from + ": " + error;
        return CopyOutcome::Failed;
    }
    fs::path tmp = dst;
    tmp += ".migrating";
    fs::remove(tmp, ec);
    ec.clear();
    if (!fs::copy_file(src, tmp, fs::copy_options::overwrite_existing, ec) || ec) {
        message = "can't copy " + from + " to " + to + ": " + ec.message();
        fs::remove(tmp, ec);
        return CopyOutcome::Failed;
    }
    if (!SameContent(src, tmp)) {
        message = "copy of " + from + " differs from it (disk full?), not used";
        fs::remove(tmp, ec);
        return CopyOutcome::Failed;
    }
    // The original date, so lists sorted by date stay as they were.
    fs::file_time_type when = fs::last_write_time(src, ec);
    if (!ec)
        fs::last_write_time(tmp, when, ec);
    ec.clear();
    if (fs::exists(dst, ec)) {
        // Another instance of the game copied it meanwhile.
        fs::remove(tmp, ec);
        message = "kept " + to;
        return CopyOutcome::Kept;
    }
    fs::rename(tmp, dst, ec);
    if (ec) {
        message = "can't rename " + FromPath(tmp) + " to " + to + ": " + ec.message();
        fs::remove(tmp, ec);
        return CopyOutcome::Failed;
    }
    message = "copied " + from + " to " + to;
    return CopyOutcome::Copied;
}

static std::string RecordLine(const std::string &game_dir)
{
    return AbsolutePath(game_dir);
}

static bool WasMigrated(const fs::path &record, const std::string &line)
{
    std::ifstream in(record);
    std::string l;
    while (std::getline(in, l)) {
        const size_t tab = l.find('\t');
        if (l.substr(0, tab) == line)
            return true;
    }
    return false;
}

static void Count(MigrationResult &result, CopyOutcome outcome, const std::string &message)
{
    switch (outcome) {
    case CopyOutcome::Copied: result.copied++; break;
    case CopyOutcome::Kept: result.kept++; break;
    case CopyOutcome::Failed: result.failed++; result.complete = false; break;
    }
    result.lines.push_back(message);
}

MigrationResult MigrateInstall(const MigrationPaths &paths)
{
    MigrationResult result;
    std::error_code ec;
    const fs::path game = ToPath(paths.game_dir);
    const fs::path record = ToPath(Join(paths.data_dir, migration_record_name));
    const std::string line = RecordLine(paths.game_dir);
    if (WasMigrated(record, line))
        return result;

    for (const char *folder : migrated_folders) {
        const fs::path root = game / folder;
        if (!fs::is_directory(root, ec))
            continue;
        fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
        if (ec) {
            result.ran = true;
            result.complete = false;
            result.failed++;
            result.lines.push_back("can't read " + FromPath(root) + ": " + ec.message());
            continue;
        }
        for (; it != end; it.increment(ec)) {
            if (ec) {
                result.ran = true;
                result.complete = false;
                result.failed++;
                result.lines.push_back("can't read in " + FromPath(root) + ": " + ec.message());
                break;
            }
            if (!it->is_regular_file(ec))
                continue;
            result.ran = true;
            const fs::path rel = it->path().lexically_relative(game);
            const std::string from = FromPath(it->path());
            std::string to;
            if ((rel == fs::path("save") / "settings.toml") && !paths.settings_in_save_folder)
                to = Join(paths.config_dir, "settings.toml");
            else
                to = FromPath(ToPath(paths.data_dir) / rel);
            if (from.empty() || to.empty()) {
                Count(result, CopyOutcome::Failed, "can't name a copy of a file in " + FromPath(root));
                continue;
            }
            const bool users = paths.file_is_users ? paths.file_is_users(from) : FileIsUsers(from);
            if (!users) {
                result.left++;
                result.lines.push_back("left " + from + ": it belongs to another user");
                continue;
            }
            std::string message;
            Count(result, CopyOnce(from, to, message), message);
        }
    }
    if (!result.ran)
        return result;
    if (result.complete) {
        std::string error;
        MakeDirs(paths.data_dir, error);
        std::ofstream out(record, std::ios::app);
        char date[32] = "";
        time_t now = time(NULL);
        struct tm *tm = localtime(&now);
        if (tm)
            strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tm);
        out << line << '\t' << date << '\n';
        if (!out) {
            result.lines.push_back("can't write " + FromPath(record) + ", the next start checks the files again");
        }
    }
    return result;
}

} // namespace userdirs
