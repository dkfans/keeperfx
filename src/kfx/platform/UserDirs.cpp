/******************************************************************************/
/** @file UserDirs.cpp
 *     The folders of the game and of the user's files, for the rest of the game.
 * @par Comment:
 *     Gathers what the rules in UserDirsPolicy.cpp need from the command line,
 *     the environment and the OS, and keeps the result.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/platform/UserDirs.h"
#include "kfx/platform/UserDirsFs.h"
#include "kfx/platform/UserDirsMigrate.h"
#include "kfx/platform/UserDirsPolicy.h"
#include "kfx/platform/UserDirsScript.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

#include "bflib_fileio.h"
#include "config.h"
#include "globals.h"
#include "post_inc.h"

using namespace userdirs;

namespace {

bool g_ready = false;
bool g_user_layer = false;
Resolution g_res;
std::string g_game_origin;
std::string g_settings_file;
std::string g_migration_notice;
/** Problems found before the log existed; written by userdirs_log_summary(). */
std::vector<std::string> g_errors;

const char *const flag_options[] = {"portable", "noportable"};

Convention CurrentConvention()
{
#if defined(_WIN32)
    return Convention::Windows;
#elif defined(__APPLE__)
    return Convention::MacOS;
#else
    return Convention::Xdg;
#endif
}

std::string StripSeparator(std::string dir)
{
    while ((dir.size() > 1) && ((dir.back() == '/') || (dir.back() == '\\')) && (dir[dir.size() - 2] != ':'))
        dir.pop_back();
    return dir;
}

#ifdef _WIN32

std::wstring Utf8ToWide(const std::string &utf8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
    if (len <= 0)
        return std::wstring();
    std::wstring wide((size_t)len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wide[0], len);
    return wide;
}

/** A folder the user named, in the ANSI code page the file functions take. Unless it is the game folder it is
 *  created first, so that a name the code page lacks has a short name to use. Empty when there is no such form. */
std::string UserFolderArg(const std::wstring &wide, bool create)
{
    if (wide.empty())
        return std::string();
    if (create) {
        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::path(wide), ec);
    }
    return NarrowPath(wide);
}

/** The folder variables are read as wide strings: getenv() would replace the characters the code page lacks. */
std::string GetEnvString(const char *name)
{
    const std::wstring wname = Utf8ToWide(name);
    const wchar_t *value = _wgetenv(wname.c_str());
    if (value == NULL)
        return std::string();
    const std::wstring wvalue(value);
    const bool is_folder = (strstr(name, "_DIR") != NULL);
    if (!is_folder)
        return NarrowPath(wvalue);
    std::string narrow = UserFolderArg(wvalue, strcmp(name, env_names[KindGame]) != 0);
    if (narrow.empty())
        g_errors.push_back(std::string(name) + " names a folder the game can't open (characters the ANSI code page lacks), ignored");
    return narrow;
}

#else

std::string GetEnvString(const char *name)
{
    const char *value = getenv(name);
    return value ? std::string(value) : std::string();
}

#endif

#ifdef _WIN32

std::string KnownFolder(REFKNOWNFOLDERID id, const char *name, std::vector<std::string> &warnings)
{
    PWSTR wide = NULL;
    HRESULT hr = SHGetKnownFolderPath(id, KF_FLAG_CREATE, NULL, &wide);
    if (FAILED(hr) || (wide == NULL)) {
        if (wide != NULL)
            CoTaskMemFree(wide);
        return std::string();
    }
    std::wstring path(wide);
    CoTaskMemFree(wide);
    std::string narrow = NarrowPath(path);
    if (narrow.empty())
        warnings.push_back(std::string("the ") + name + " folder has characters that the ANSI code page lacks, and no short name");
    return narrow;
}

std::string ExeDir()
{
    const char *base = SDL_GetBasePath(); // UTF-8
    if (base == NULL)
        return std::string();
    int len = MultiByteToWideChar(CP_UTF8, 0, base, -1, NULL, 0);
    if (len <= 1)
        return std::string();
    std::wstring wide((size_t)len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, base, -1, &wide[0], len);
    wide.resize((size_t)len - 1);
    return StripSeparator(NarrowPath(wide));
}

#else

std::string HomeDir()
{
    const char *home = getenv("HOME");
    if ((home != NULL) && (home[0] == '/'))
        return home;
    // No usable HOME: what the password database says, as the C library and GLib do.
    struct passwd pw;
    struct passwd *found = NULL;
    std::vector<char> buf(16384);
    if ((getpwuid_r(geteuid(), &pw, buf.data(), buf.size(), &found) == 0) && (found != NULL) && (found->pw_dir != NULL)
        && (found->pw_dir[0] == '/'))
        return found->pw_dir;
    return std::string();
}

std::string ExeDir()
{
    // On macOS this is the Resources folder of an app bundle, where its game files belong.
    const char *base = SDL_GetBasePath();
    return (base != NULL) ? StripSeparator(base) : std::string();
}

#endif

bool HasGameConfig(const std::string &dir)
{
    const std::string fname = dir.empty() ? std::string("keeperfx.cfg") : Join(dir, "keeperfx.cfg");
    return LbFileExists(fname.c_str());
}

/** Where versions before the per-user folders kept renderer_prefs.ini: SDL_GetPrefPath("keeperfx", "keeperfx"),
 *  worked out here because calling SDL_GetPrefPath() creates the folder. */
std::string LegacyRendererPrefs()
{
#if defined(_WIN32)
    std::vector<std::string> ignored;
    const std::string roaming = KnownFolder(FOLDERID_RoamingAppData, "Roaming AppData", ignored);
    return roaming.empty() ? std::string() : Join(Join(Join(roaming, "keeperfx"), "keeperfx"), "renderer_prefs.ini");
#elif defined(__APPLE__)
    return std::string(); // there was no macOS build before
#else
    std::string base = GetEnvString("XDG_DATA_HOME");
    if (!IsAbsolute(base, Convention::Xdg)) {
        const std::string home = HomeDir();
        if (home.empty())
            return std::string();
        base = Join(home, ".local/share");
    }
    return Join(base, "keeperfx/keeperfx/renderer_prefs.ini");
#endif
}

const char *ModeText(Mode mode)
{
    switch (mode) {
    case Mode::Portable: return "kept in the game folder (portable)";
    case Mode::UserDir: return "kept in one folder given by the user";
    case Mode::PerUser: return "kept in the per-user folders";
    }
    return "?";
}

} // namespace

extern "C" int userdirs_option_value_count(const char *option)
{
    if (option == NULL)
        return -1;
    if (strcasecmp(option, option_userdir_name) == 0)
        return 1;
    for (int k = 0; k < KindCount; k++) {
        if (strcasecmp(option, option_names[k]) == 0)
            return 1;
    }
    for (const char *flag : flag_options) {
        if (strcasecmp(option, flag) == 0)
            return 0;
    }
    return -1;
}

extern "C" void userdirs_init(int argc, char *argv[])
{
    Request req;
    req.convention = CurrentConvention();
    GameDirRequest game_req;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if ((arg == NULL) || ((arg[0] != '-') && (arg[0] != '/')))
            continue;
        const char *name = arg + 1;
        const int count = userdirs_option_value_count(name);
        if (count < 0)
            continue;
        if (count == 0) {
            req.option_portable = (strcasecmp(name, "portable") == 0) ? 1 : 0;
            continue;
        }
        if ((i + 1 >= argc) || (argv[i + 1] == NULL) || (argv[i + 1][0] == '\0')) {
            g_errors.push_back(std::string("-") + name + " needs a folder, ignored");
            continue;
        }
        std::string value = argv[++i];
#ifdef _WIN32
        // WinMain gives the arguments in UTF-8; the file functions take the ANSI code page.
        value = UserFolderArg(Utf8ToWide(value), strcasecmp(name, option_names[KindGame]) != 0);
        if (value.empty()) {
            g_errors.push_back(std::string("-") + name + " names a folder the game can't open (characters the ANSI code page lacks), ignored");
            continue;
        }
#endif
        if (strcasecmp(name, option_userdir_name) == 0) {
            req.option_userdir = value;
        } else {
            for (int k = 0; k < KindCount; k++) {
                if (strcasecmp(name, option_names[k]) == 0)
                    req.option_dir[k] = value;
            }
        }
    }

    game_req.option = req.option_dir[KindGame];
    game_req.env = GetEnvString(env_names[KindGame]);
    game_req.cwd_has_game = HasGameConfig("");
    if (game_req.option.empty() && game_req.env.empty() && !game_req.cwd_has_game) {
        game_req.exe_dir = ExeDir();
        game_req.exe_dir_has_game = !game_req.exe_dir.empty() && HasGameConfig(game_req.exe_dir);
    }
    const GameDirResolution game = ResolveGameDir(game_req);
    g_game_origin = game.origin;

    req.game_dir = StripSeparator(game.dir);
    req.env = GetEnvString;
    req.game_dir_is_writable = [&req]() { return IsWritableDir(req.game_dir); };
    std::vector<std::string> warnings;
#ifdef _WIN32
    req.win_saved_games = KnownFolder(FOLDERID_SavedGames, "Saved Games", warnings);
    req.win_roaming_app_data = KnownFolder(FOLDERID_RoamingAppData, "Roaming AppData", warnings);
    req.win_local_app_data = KnownFolder(FOLDERID_LocalAppData, "Local AppData", warnings);
#else
    req.home = HomeDir();
#endif

    g_res = Resolve(req);
    g_res.warnings.insert(g_res.warnings.begin(), warnings.begin(), warnings.end());
    g_res.origin[KindGame] = g_game_origin;
    for (int k = 0; k < KindCount; k++)
        g_res.dir[k] = StripSeparator(g_res.dir[k]);

    // The user's folders, when they are not the game folder or a folder in it that the game makes when it needs it.
    for (int k = KindData; k < KindCount; k++) {
        if (g_res.origin[k] == "portable")
            continue;
        std::string error;
        if (!MakeDirs(g_res.dir[k], error)) {
            g_errors.push_back(std::string("the ") + KindName(k) + " folder is unusable: " + error);
        } else if (!IsWritableDir(g_res.dir[k])) {
            g_errors.push_back(std::string("can't write in the ") + KindName(k) + " folder \"" + g_res.dir[k] + "\"");
        }
#ifdef _WIN32
        if (g_res.dir[k].size() > 150)
            g_res.warnings.push_back(std::string("the ") + KindName(k) + " folder has a long path; Windows refuses files whose path is longer than 259 characters");
#endif
    }

    // Portable by request in a folder this user can't write: saves and settings would fail without a word.
    if ((g_res.mode == Mode::Portable) && (g_res.origin[KindData] == "portable") && !IsWritableDir(g_res.dir[KindGame]))
        g_errors.push_back("the game folder \"" + g_res.dir[KindGame] + "\" is read only, so saves, settings and logs can't be "
                           "written; leave out -portable / " + env_portable_name + " or give -userdir");
    // Say it on the console too: these problems may leave the game without a log.
    for (const std::string &e : g_errors)
        fprintf(stderr, "KeeperFX user folders: %s\n", e.c_str());

    if (SettingsInSaveFolder(g_res))
        g_settings_file = Join(g_res.dir[KindData], "save/settings.toml");
    else
        g_settings_file = Join(g_res.dir[KindConfig], "settings.toml");

    g_user_layer = !SameDir(g_res.dir[KindData], g_res.dir[KindGame]);
    snprintf(keeper_runtime_directory, sizeof(keeper_runtime_directory), "%s", g_res.dir[KindGame].c_str());
    if (g_res.dir[KindGame].size() >= sizeof(keeper_runtime_directory))
        g_errors.push_back("the game folder path is too long");
    g_ready = true;
}

extern "C" void userdirs_log_summary(void)
{
    if (!g_ready)
        return;
    JUSTMSG("User files are %s, because %s", ModeText(g_res.mode), g_res.reason.c_str());
    for (int k = 0; k < KindCount; k++) {
        JUSTMSG("  %-6s folder: %s (%s)", KindName(k), AbsolutePath(g_res.dir[k]).c_str(), g_res.origin[k].c_str());
    }
    if (!g_res.error.empty())
        ERRORMSG("No per-user folder: %s. Give one with -userdir or %s.", g_res.error.c_str(), env_userdir_name);
    for (const std::string &w : g_res.warnings)
        WARNMSG("User folders: %s", w.c_str());
    for (const std::string &e : g_errors)
        ERRORMSG("User folders: %s", e.c_str());
}

extern "C" void userdirs_migrate(void)
{
    if (!g_ready)
        return;
    const std::string prefs = Join(g_res.dir[KindConfig], "renderer_prefs.ini");
    const std::string legacy_prefs = LegacyRendererPrefs();
    if (!legacy_prefs.empty() && LbFileExists(legacy_prefs.c_str()) && !LbFileExists(prefs.c_str())) {
        std::string message;
        CopyOnce(legacy_prefs, prefs, message);
        JUSTMSG("Renderer settings: %s", message.c_str());
    }

    if ((g_res.mode == Mode::Portable) || SameDir(g_res.dir[KindGame], g_res.dir[KindData]))
        return;
    MigrationPaths paths;
    paths.game_dir = g_res.dir[KindGame];
    paths.data_dir = g_res.dir[KindData];
    paths.config_dir = g_res.dir[KindConfig];
    paths.settings_in_save_folder = SettingsInSaveFolder(g_res);
    MigrationResult result = MigrateInstall(paths);
    if (!result.ran)
        return;
    JUSTMSG("The game folder %s is not used for your files (%s). Copying your saves, settings, replays and "
            "screenshots from it into the user folders, once:", AbsolutePath(g_res.dir[KindGame]).c_str(), g_res.reason.c_str());
    for (const std::string &line : result.lines)
        JUSTMSG("  %s", line.c_str());
    if (result.complete) {
        JUSTMSG("Copied %d files, kept %d that were already there, left %d of other users. The game folder was left as it was.",
                result.copied, result.kept, result.left);
    } else {
        ERRORMSG("Copied %d files, kept %d, left %d of other users, %d could not be copied. Nothing in the game folder "
                 "was changed or deleted; the next start tries again.", result.copied, result.kept, result.left, result.failed);
    }
    JUSTMSG("Your files are now in %s (settings in %s).", AbsolutePath(g_res.dir[KindData]).c_str(),
            AbsolutePath(g_res.dir[KindConfig]).c_str());

    std::string notice = "Your saves and settings are now kept in\n" + AbsolutePath(g_res.dir[KindData]) + "\n";
    if (!SettingsInSaveFolder(g_res))
        notice += "and " + AbsolutePath(g_res.dir[KindConfig]) + "\n";
    notice += "because " + g_res.reason + ".\n";
    if (result.copied + result.kept > 0)
        notice += "The " + std::to_string(result.copied + result.kept) + " files of the game folder were copied there; the game folder was not changed.\n";
    if (result.left > 0)
        notice += std::to_string(result.left) + " files of other users were left in the game folder.\n";
    if (!result.complete)
        notice += std::to_string(result.failed) + " files could not be copied; the game tries again at the next start.\n";
    notice += "The log, " + std::string(log_file_name) + ", lists them.";
    g_migration_notice = notice;
}

extern "C" const char *userdirs_migration_notice(void)
{
    return g_migration_notice.empty() ? NULL : g_migration_notice.c_str();
}

namespace {

bool CaseInsensitivePaths()
{
#if defined(_WIN32) || defined(__APPLE__)
    return true;
#else
    return false;
#endif
}

/** The absolute folders a script's files may be in: the data folder, the game folder, then extra_root. */
std::vector<std::string> ScriptRoots(const char *extra_root)
{
    std::vector<std::string> roots;
    roots.push_back(AbsolutePath(g_res.dir[KindData]));
    roots.push_back(AbsolutePath(g_res.dir[KindGame]));
    if ((extra_root != NULL) && (extra_root[0] != '\0'))
        roots.push_back(AbsolutePath(extra_root));
    return roots;
}

bool CopyOut(const std::string &value, char *dst, size_t dst_size)
{
    if (value.size() >= dst_size) {
        if (dst_size > 0)
            dst[0] = '\0';
        return false;
    }
    memcpy(dst, value.c_str(), value.size() + 1);
    return true;
}

} // namespace

extern "C" TbBool userdirs_script_file(const char *path, enum ScriptFileAccess access, const char *extra_root, char *dst,
                                       size_t dst_size, char *error, size_t error_size)
{
    std::string why;
    std::string result;
    if (!g_ready || (path == NULL)) {
        why = "the folders are not set up";
    } else {
        const std::vector<std::string> roots = ScriptRoots(extra_root);
        const ScriptPath sp = ScriptLogicalPath(path, roots, CaseInsensitivePaths());
        if (!sp.ok) {
            why = sp.error;
        } else if (access == ScrFile_Read) {
            // The user's copy, else the game's, else the INSTALL_PATH folder's; the game's when there is none, so
            // that opening it fails as it should.
            const std::string &data_dir = g_res.dir[KindData];
            std::vector<std::string> bases;
            if (g_user_layer)
                bases.push_back(data_dir);
            bases.push_back(g_res.dir[KindGame]);
            if ((extra_root != NULL) && (extra_root[0] != '\0'))
                bases.push_back(extra_root);
            std::string base = g_res.dir[KindGame];
            for (const std::string &b : bases) {
                if (LbFileExists(Join(b, sp.logical).c_str())) {
                    base = b;
                    break;
                }
            }
            result = Join(base, sp.logical);
            if (!IsWithinDir(base, result))
                why = "a link leads out of the game and data folders";
        } else if (!ScriptMayWrite(sp.logical)) {
            why = std::string("scripts may only write and remove files in ") + script_write_folders_text;
        } else {
            const std::string &data_dir = g_res.dir[KindData];
            result = Join(data_dir, sp.logical);
            if (!IsWithinDir(data_dir, result)) {
                why = "a link leads out of the data folder";
            } else if (access != ScrFile_Remove) {
                std::string mkerr;
                const std::string parent = FromPath(ToPath(result).parent_path());
                if (!MakeDirs(parent, mkerr)) {
                    why = mkerr;
                } else if (!IsWithinDir(data_dir, result)) {
                    why = "a link leads out of the data folder";
                } else if ((access == ScrFile_Update) && g_user_layer && !LbFileExists(result.c_str())) {
                    // Change the game's copy as a portable install would: copy it first.
                    const std::string game_copy = Join(g_res.dir[KindGame], sp.logical);
                    if (LbFileExists(game_copy.c_str()) && IsWithinDir(g_res.dir[KindGame], game_copy)) {
                        std::string message;
                        if (CopyOnce(game_copy, result, message) == CopyOutcome::Failed)
                            why = message;
                    }
                }
            }
        }
    }
    if (why.empty() && !CopyOut(result, dst, dst_size))
        why = "the path is too long";
    if (!why.empty()) {
        if (dst_size > 0)
            dst[0] = '\0';
        snprintf(error, error_size, "%s: %s", (path != NULL) ? path : "(null)", why.c_str());
        return false;
    }
    if (error_size > 0)
        error[0] = '\0';
    return true;
}

extern "C" TbBool userdirs_relative_name(const char *path, const char *extra_root, char *dst, size_t dst_size)
{
    if (!g_ready || (path == NULL) || (path[0] == '\0'))
        return false;
    const std::string rel = RelativeToRoots(AbsolutePath(path), ScriptRoots(extra_root), CaseInsensitivePaths());
    if (rel.empty())
        return false;
    return CopyOut(rel, dst, dst_size);
}

extern "C" const char *userdirs_get(enum UserDirKind kind)
{
    if (!g_ready || (kind < 0) || (kind >= UsrDir_Count))
        return ".";
    return g_res.dir[kind].c_str();
}

extern "C" TbBool userdirs_is_portable(void)
{
    return !g_ready || (g_res.mode == Mode::Portable);
}

extern "C" TbBool userdirs_has_user_layer(void)
{
    return g_ready && g_user_layer;
}

extern "C" char *userdirs_path(char *dst, size_t dst_size, enum UserDirKind kind, const char *fname)
{
    const std::string path = Join(userdirs_get(kind), (fname != NULL) ? fname : "");
    if (path.size() >= dst_size) {
        ERRORMSG("Path too long: %s", path.c_str());
        if (dst_size > 0)
            dst[0] = '\0';
        return dst;
    }
    memcpy(dst, path.c_str(), path.size() + 1);
    return dst;
}

extern "C" const char *userdirs_settings_file(void)
{
    if (!g_ready)
        return "save/settings.toml";
    return g_settings_file.c_str();
}
