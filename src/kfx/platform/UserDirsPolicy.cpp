/******************************************************************************/
/** @file UserDirsPolicy.cpp
 *     Which folders the game uses for its own files and for the user's files.
 */
/******************************************************************************/
#include "kfx/platform/UserDirsPolicy.h"

#include <cctype>

namespace userdirs {

const char *const option_names[KindCount] = {"gamedir", "userdatadir", "configdir", "statedir", "cachedir"};
const char *const env_names[KindCount] = {
    "KEEPERFX_GAME_DIR", "KEEPERFX_USER_DATA_DIR", "KEEPERFX_CONFIG_DIR", "KEEPERFX_STATE_DIR", "KEEPERFX_CACHE_DIR"};
const char *const option_userdir_name = "userdir";
const char *const env_userdir_name = "KEEPERFX_USER_DIR";
const char *const env_portable_name = "KEEPERFX_PORTABLE";

static const char *const kind_names[KindCount] = {"game", "data", "config", "state", "cache"};

const char *AppFolderName(Convention convention)
{
    return (convention == Convention::Xdg) ? "keeperfx" : "KeeperFX";
}

bool IsAbsolute(const std::string &path, Convention convention)
{
    if (path.empty())
        return false;
    if (path[0] == '/')
        return true;
    if (convention != Convention::Windows)
        return false;
    if (path[0] == '\\')
        return true; // \\server\share, or \x on the current drive
    return (path.size() >= 3) && isalpha((unsigned char)path[0]) && (path[1] == ':') && ((path[2] == '\\') || (path[2] == '/'));
}

std::string Join(const std::string &a, const std::string &b)
{
    if (a.empty())
        return b;
    if (b.empty())
        return a;
    const char last = a[a.size() - 1];
    if ((last == '/') || (last == '\\'))
        return a + b;
    // Keep a Windows path in one style, so the log shows a path players can paste into Explorer.
    const char sep = ((a.find('\\') != std::string::npos) && (a.find('/') == std::string::npos)) ? '\\' : '/';
    return a + sep + b;
}

int ParseBool(const std::string &value)
{
    std::string v;
    for (char c : value)
        v += (char)tolower((unsigned char)c);
    if ((v == "1") || (v == "yes") || (v == "true") || (v == "on"))
        return 1;
    if ((v == "0") || (v == "no") || (v == "false") || (v == "off"))
        return 0;
    return -1;
}

bool SettingsInSaveFolder(const Resolution &resolution)
{
    return resolution.dir[KindConfig] == resolution.dir[KindData];
}

static std::string GetEnv(const Request &request, const char *name)
{
    return request.env ? request.env(name) : std::string();
}

/** One XDG base folder: the variable when it holds an absolute path, else home + fallback. The specification says
 *  a relative path in these variables is invalid and must be ignored. */
static bool XdgBase(const Request &request, const char *var, const char *fallback, std::string &out, std::string &origin,
                    Resolution &res)
{
    const std::string value = GetEnv(request, var);
    if (!value.empty()) {
        if (IsAbsolute(value, Convention::Xdg)) {
            out = value;
            origin = var;
            return true;
        }
        res.warnings.push_back(std::string(var) + "=\"" + value + "\" is not an absolute path, ignored as the XDG specification says");
    }
    if (request.home.empty())
        return false;
    out = Join(request.home, fallback);
    origin = std::string("$HOME/") + fallback;
    return true;
}

static bool PerUserDirs(const Request &request, Resolution &res)
{
    const std::string app = AppFolderName(request.convention);
    switch (request.convention) {
    case Convention::Xdg: {
        static const char *const vars[KindCount] = {nullptr, "XDG_DATA_HOME", "XDG_CONFIG_HOME", "XDG_STATE_HOME", "XDG_CACHE_HOME"};
        static const char *const fallbacks[KindCount] = {nullptr, ".local/share", ".config", ".local/state", ".cache"};
        bool ok = true;
        for (int k = KindData; k < KindCount; k++) {
            std::string base;
            if (XdgBase(request, vars[k], fallbacks[k], base, res.origin[k], res)) {
                res.dir[k] = Join(base, app);
            } else {
                ok = false;
            }
        }
        if (!ok)
            res.error = "there is no home folder (HOME is not set and the user has no home in the password database), "
                        "and no XDG_*_HOME variable gives the missing folders";
        return ok;
    }
    case Convention::MacOS:
        if (request.home.empty()) {
            res.error = "there is no home folder (HOME is not set and the user has no home in the password database)";
            return false;
        }
        res.dir[KindData] = Join(Join(request.home, "Library/Application Support"), app);
        res.dir[KindConfig] = res.dir[KindData];
        res.dir[KindState] = Join(Join(request.home, "Library/Logs"), app);
        res.dir[KindCache] = Join(Join(request.home, "Library/Caches"), app);
        res.origin[KindData] = res.origin[KindConfig] = "~/Library/Application Support";
        res.origin[KindState] = "~/Library/Logs";
        res.origin[KindCache] = "~/Library/Caches";
        return true;
    case Convention::Windows: {
        // Saves and the other files players look for go to Saved Games, which Explorer shows and which is made for
        // them. Settings go to the roaming AppData, the logs and caches to the local one, which doesn't roam.
        const std::string &saved = request.win_saved_games;
        const std::string &roaming = request.win_roaming_app_data;
        const std::string &local = request.win_local_app_data;
        if (saved.empty() && roaming.empty() && local.empty()) {
            res.error = "Windows gave none of the Saved Games, Roaming AppData and Local AppData folders";
            return false;
        }
        const std::string &data_base = !saved.empty() ? saved : !local.empty() ? local : roaming;
        res.dir[KindData] = Join(data_base, app);
        res.origin[KindData] = !saved.empty() ? "Saved Games" : !local.empty() ? "Local AppData" : "Roaming AppData";
        if (saved.empty())
            res.warnings.push_back("Windows gave no Saved Games folder, saves go to " + res.dir[KindData]);
        const std::string &config_base = !roaming.empty() ? roaming : !local.empty() ? local : saved;
        res.dir[KindConfig] = Join(config_base, app);
        res.origin[KindConfig] = !roaming.empty() ? "Roaming AppData" : !local.empty() ? "Local AppData" : "Saved Games";
        const std::string local_app = Join(!local.empty() ? local : !roaming.empty() ? roaming : saved, app);
        const char *local_origin = !local.empty() ? "Local AppData" : !roaming.empty() ? "Roaming AppData" : "Saved Games";
        res.dir[KindState] = Join(local_app, "Logs");
        res.dir[KindCache] = Join(local_app, "Cache");
        res.origin[KindState] = res.origin[KindCache] = local_origin;
        return true;
    }
    }
    return false;
}

/** Every user folder in one folder, laid out like a portable install. */
static void SingleFolder(Resolution &res, const std::string &dir, const std::string &origin)
{
    for (int k = KindData; k < KindCount; k++) {
        res.dir[k] = dir;
        res.origin[k] = origin;
    }
    res.dir[KindCache] = Join(dir, "cache");
}

Resolution Resolve(const Request &request)
{
    Resolution res;
    res.dir[KindGame] = request.game_dir;
    res.origin[KindGame] = "game";

    std::string userdir = request.option_userdir;
    std::string userdir_origin = std::string("-") + option_userdir_name;
    if (userdir.empty()) {
        userdir = GetEnv(request, env_userdir_name);
        userdir_origin = env_userdir_name;
    }

    if (!userdir.empty()) {
        res.mode = Mode::UserDir;
        res.reason = userdir_origin + " is set";
        SingleFolder(res, userdir, userdir_origin);
    } else {
        int portable = request.option_portable;
        if (portable >= 0) {
            res.reason = portable ? "-portable is given" : "-noportable is given";
        } else {
            const std::string value = GetEnv(request, env_portable_name);
            if (!value.empty()) {
                portable = ParseBool(value);
                if (portable < 0)
                    res.warnings.push_back(std::string(env_portable_name) + "=\"" + value + "\" is not 1 or 0, ignored");
                else
                    res.reason = std::string(env_portable_name) + "=" + value;
            }
        }
        if (portable < 0) {
            portable = (request.game_dir_is_users && request.game_dir_is_users()) ? 1 : 0;
            res.reason = portable ? "the game folder is writable and belongs to this user"
                                  : "the game folder is read only for this user, or belongs to another user";
        }
        if (portable) {
            res.mode = Mode::Portable;
            SingleFolder(res, request.game_dir, "portable");
        } else {
            res.mode = Mode::PerUser;
            if (!PerUserDirs(request, res)) {
                // Better a game that can't save than no game: keep the user's files in the game folder, the way
                // KeeperFX always did, and say why.
                res.mode = Mode::Portable;
                res.reason += "; no per-user folder: " + res.error;
                SingleFolder(res, request.game_dir, "portable (fallback)");
            }
        }
    }

    for (int k = KindData; k < KindCount; k++) {
        std::string value = request.option_dir[k];
        std::string origin = std::string("-") + option_names[k];
        if (value.empty()) {
            value = GetEnv(request, env_names[k]);
            origin = env_names[k];
        }
        if (!value.empty()) {
            res.dir[k] = value;
            res.origin[k] = origin;
        }
    }
    return res;
}

GameDirResolution ResolveGameDir(const GameDirRequest &request)
{
    if (!request.option.empty())
        return {request.option, std::string("-") + option_names[KindGame]};
    if (!request.env.empty())
        return {request.env, env_names[KindGame]};
    // The working folder first, because the launchers and every existing install start the game from there, and
    // paths in the log and in replays stay as they always were.
    if (request.cwd_has_game)
        return {".", "working folder"};
    if (request.exe_dir_has_game && !request.exe_dir.empty())
        return {request.exe_dir, "folder of the executable"};
    return {".", "working folder (keeperfx.cfg not found there nor next to the executable)"};
}

const char *KindName(int kind)
{
    return ((kind >= 0) && (kind < KindCount)) ? kind_names[kind] : "?";
}

} // namespace userdirs
