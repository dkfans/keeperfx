/******************************************************************************/
/** @file UserDirsPolicy.h
 *     Which folders the game uses for its own files and for the user's files.
 * @par Purpose:
 *     The rules only: which option, environment variable or OS convention gives
 *     each folder. Nothing here touches the disk or the OS, the caller hands in
 *     what it found (UserDirs.cpp in the game, the tests in tests/userdirs).
 *     docs/user_files.md describes the rules for players.
 */
/******************************************************************************/
#ifndef KFX_USERDIRSPOLICY_H
#define KFX_USERDIRSPOLICY_H

#include <functional>
#include <string>
#include <vector>

namespace userdirs {

/** The folders. Game is the install, the others belong to the user. */
enum Kind {
    KindGame = 0, /**< The game's own files: read only unless the install is portable. */
    KindData,     /**< Saves, campaign progress, high scores, replays, screenshots, maps, campaigns and mods the user adds. */
    KindConfig,   /**< keeperfx.cfg changes, settings.toml (options and keys), renderer_prefs.ini. */
    KindState,    /**< Logs, crash dumps and debug dumps. */
    KindCache,    /**< Files the game makes again when they are missing. */
    KindCount
};

/** The OS convention for per-user folders. */
enum class Convention {
    Xdg,     /**< Linux and the BSDs: XDG Base Directory specification. */
    MacOS,   /**< ~/Library/Application Support, Logs and Caches. */
    Windows, /**< Known Folders: Saved Games, Roaming and Local AppData. */
};

/** How the user's folders were chosen. */
enum class Mode {
    Portable, /**< Everything in the game folder, as KeeperFX has always done. */
    UserDir,  /**< Everything in one folder given with -userdir or KEEPERFX_USER_DIR, laid out like a portable install. */
    PerUser,  /**< The OS convention for per-user folders. */
};

/** What the game found about its surroundings. */
struct Request {
    Convention convention = Convention::Xdg;
    /** Values of -gamedir, -userdatadir, -configdir, -statedir and -cachedir, indexed by Kind; empty when not given. */
    std::string option_dir[KindCount];
    /** Value of -userdir; empty when not given. */
    std::string option_userdir;
    /** 1 with -portable, 0 with -noportable, -1 without either. */
    int option_portable = -1;
    /** Reads an environment variable; returns an empty string when it isn't set. */
    std::function<std::string(const char *name)> env;
    /** The user's home folder, empty when there is none (Xdg and MacOS). */
    std::string home;
    /** Known Folders, empty when the OS has none (Windows). */
    std::string win_saved_games;
    std::string win_roaming_app_data;
    std::string win_local_app_data;
    /** The game folder, already chosen with ResolveGameDir(). */
    std::string game_dir;
    /** True when the current user can write in the game folder. Only called when the rules need it. */
    std::function<bool()> game_dir_is_writable;
};

/** The folders and how they were found. */
struct Resolution {
    Mode mode = Mode::Portable;
    std::string dir[KindCount];
    /** Where each folder came from, for the log: an option, a variable, or the convention. */
    std::string origin[KindCount];
    /** Why the mode was chosen, for the log. */
    std::string reason;
    /** Things the user should know, e.g. an ignored relative XDG variable. */
    std::vector<std::string> warnings;
    /** Set when no per-user folder could be found; the folders then fall back to the game folder. */
    std::string error;
};

/** Chooses every folder but the game folder, which the request already holds. */
Resolution Resolve(const Request &request);

/** What the game found about the places its files may be in. */
struct GameDirRequest {
    std::string option;            /**< -gamedir */
    std::string env;               /**< KEEPERFX_GAME_DIR */
    bool cwd_has_game = false;     /**< The working folder holds keeperfx.cfg. */
    std::string exe_dir;           /**< The folder of the executable (the Resources folder of a macOS bundle). */
    bool exe_dir_has_game = false; /**< exe_dir holds keeperfx.cfg. */
};

struct GameDirResolution {
    std::string dir;
    std::string origin;
};

/** Chooses the game folder: the option, the variable, the working folder or the executable's folder. */
GameDirResolution ResolveGameDir(const GameDirRequest &request);

/** Names of the command line options and environment variables, indexed by Kind. */
extern const char *const option_names[KindCount];
extern const char *const env_names[KindCount];
/** "userdir", "portable" and "noportable" are the other options. */
extern const char *const option_userdir_name;
extern const char *const env_userdir_name;
extern const char *const env_portable_name;

/** Name of the folder for the game in per-user locations: "keeperfx" for XDG, "KeeperFX" elsewhere. */
const char *AppFolderName(Convention convention);

/** True for "/x", "C:\x", "C:/x" and "\\server\x" (the latter forms on Windows only). */
bool IsAbsolute(const std::string &path, Convention convention);

/** a + separator + b, without doubling a separator that a already ends with. */
std::string Join(const std::string &a, const std::string &b);

/** True when the settings file is kept as save/settings.toml, as in portable installs: when the data and
 *  config folders are the same folder. Otherwise it is settings.toml in the config folder. */
bool SettingsInSaveFolder(const Resolution &resolution);

/** "game", "data", "config", "state" or "cache". */
const char *KindName(int kind);

/** Parses a yes/no value such as KEEPERFX_PORTABLE's. Returns 1, 0, or -1 when the value isn't one. */
int ParseBool(const std::string &value);

} // namespace userdirs

#endif // KFX_USERDIRSPOLICY_H
