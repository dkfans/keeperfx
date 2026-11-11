/******************************************************************************/
/** @file UserDirs.h
 *     The folders of the game and of the user's files, for the rest of the game.
 * @par Purpose:
 *     Keeps the game's own files apart from the files of the person playing:
 *     saves, settings, logs and so on. A portable install (the usual zip,
 *     unpacked anywhere) keeps them all in its own folder, as KeeperFX always
 *     did; a read only install uses the OS's per-user folders. The rules are in
 *     UserDirsPolicy.h, the player's view in docs/user_files.md.
 */
/******************************************************************************/
#ifndef KFX_USERDIRS_H
#define KFX_USERDIRS_H

#include <stddef.h>

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

enum UserDirKind {
    UsrDir_Game = 0, /**< The game's files. Read only unless the install is portable. */
    UsrDir_Data,     /**< Saves, campaign progress, high scores, replays, screenshots, and maps, campaigns and mods the user adds. */
    UsrDir_Config,   /**< keeperfx.cfg changes, settings.toml, renderer_prefs.ini. */
    UsrDir_State,    /**< Logs, crash dumps, debug dumps. */
    UsrDir_Cache,    /**< Files the game makes again when they are missing. */
    UsrDir_Count
};

/** Chooses the folders from the command line, the environment and the OS, and creates the user's folders.
 *  Call it before anything else: the log goes into one of them, so it logs nothing itself. */
void userdirs_init(int argc, char *argv[]);
/** Writes the folders in use, and what went wrong choosing them, to the log. */
void userdirs_log_summary(void);
/** The first time a portable install runs with per-user folders, copies the user's files from it; and copies
 *  renderer_prefs.ini from where older versions kept it. Call after the log is set up. */
void userdirs_migrate(void);

/** A folder, never NULL. "." for the game folder of an install started from its own folder. */
const char *userdirs_get(enum UserDirKind kind);
/** True when the user's files are kept in the game folder. */
TbBool userdirs_is_portable(void);
/** True when the user's data folder is not the game folder. It is then searched before the game folder for game
 *  files, so players can add maps, campaigns, mods and the original game's files there. */
TbBool userdirs_has_user_layer(void);

/** dst = folder of the kind + "/" + fname. Returns dst, which is empty if it doesn't fit. */
char *userdirs_path(char *dst, size_t dst_size, enum UserDirKind kind, const char *fname);
/** The file of the player's options and keys: settings.toml in the config folder, or save/settings.toml in a
 *  portable install. */
const char *userdirs_settings_file(void);

/** When the user's files of the game folder were just copied into the per-user folders (or left there because they
 *  are another user's), a message for the player saying where their files are now; NULL otherwise. Set by
 *  userdirs_migrate(), so once per install. */
const char *userdirs_migration_notice(void);

/** The number of values a folder option takes (-gamedir DIR: 1, -portable: 0), or -1 for other options.
 *  The option is given without its leading '-'. */
int userdirs_option_value_count(const char *option);

#ifdef __cplusplus
}
#endif

#endif // KFX_USERDIRS_H
