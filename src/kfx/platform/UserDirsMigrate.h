/******************************************************************************/
/** @file UserDirsMigrate.h
 *     Copies a player's files from a portable install into the per-user folders.
 * @par Purpose:
 *     When an install that kept saves in its own folder starts using per-user
 *     folders (it became read only, or the player asked for it), the saves,
 *     settings, replays and screenshots there are copied over once.
 * @par Comment:
 *     It only ever copies. A file that is already in the per-user folders is
 *     kept, whatever its date, nothing in the install is changed or deleted,
 *     and a copy is checked against the original before it gets its final name.
 *     When something fails, the next start tries again. Only the files of the
 *     user who plays are copied: in an install another user (or root, or an
 *     administrator) played in, their saves and settings stay theirs.
 */
/******************************************************************************/
#ifndef KFX_USERDIRSMIGRATE_H
#define KFX_USERDIRSMIGRATE_H

#include <functional>
#include <string>
#include <vector>

namespace userdirs {

struct MigrationPaths {
    std::string game_dir;   /**< The install the files come from. */
    std::string data_dir;   /**< Where save/, replays/ and scrshots/ go. */
    std::string config_dir; /**< Where settings.toml goes, unless it stays in save/. */
    bool settings_in_save_folder = false;
    /** True when a file of the install is the current user's to copy; FileIsUsers() when not set. */
    std::function<bool(const std::string &path)> file_is_users;
};

struct MigrationResult {
    /** False when there was nothing to do: no such files, or this install was migrated before. */
    bool ran = false;
    /** True when every file was copied or already there; the install is then recorded as migrated. */
    bool complete = true;
    int copied = 0;
    int kept = 0;
    int failed = 0;
    /** Files left in the install because they belong to another user. */
    int left = 0;
    /** One line per thing done, for the log. */
    std::vector<std::string> lines;
};

/** Name of the file in the data folder that lists the installs already migrated. */
extern const char *const migration_record_name;

/** Copies the user's files of a portable install into the per-user folders, once per install. */
MigrationResult MigrateInstall(const MigrationPaths &paths);

enum class CopyOutcome { Copied, Kept, Failed };

/** Copies one file unless the target exists. The copy is written beside the target, compared with the source,
 *  given the source's time, and then renamed to the target. message says what happened. */
CopyOutcome CopyOnce(const std::string &from, const std::string &to, std::string &message);

} // namespace userdirs

#endif // KFX_USERDIRSMIGRATE_H
