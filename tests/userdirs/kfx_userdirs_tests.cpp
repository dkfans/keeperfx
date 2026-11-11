/******************************************************************************/
/** @file kfx_userdirs_tests.cpp
 *     Standalone tests for the user folder rules, the file helpers and the
 *     migration of a portable install. No game code; the migration tests work
 *     in a temporary folder.
 */
/******************************************************************************/
#include "kfx/platform/UserDirsFs.h"
#include "kfx/platform/UserDirsMigrate.h"
#include "kfx/platform/UserDirsPolicy.h"
#include "kfx/platform/UserDirsScript.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;
using namespace userdirs;

static int checks;
static int fails;

static void expect(const char *what, bool cond)
{
    checks++;
    if (!cond) {
        printf("FAIL %s\n", what);
        fails++;
    }
}

static void expect_eq(const char *what, const std::string &got, const std::string &want)
{
    checks++;
    if (got != want) {
        printf("FAIL %s: got \"%s\", want \"%s\"\n", what, got.c_str(), want.c_str());
        fails++;
    }
}

/** A request with a fake environment. */
struct Env {
    std::map<std::string, std::string> vars;
    int probes = 0;
    bool game_is_writable = false;

    Request Make(Convention convention, const std::string &home = "/home/u")
    {
        Request r;
        r.convention = convention;
        r.home = home;
        r.game_dir = "/opt/keeperfx";
        r.env = [this](const char *name) {
            auto it = vars.find(name);
            return (it == vars.end()) ? std::string() : it->second;
        };
        r.game_dir_is_writable = [this]() {
            probes++;
            return game_is_writable;
        };
        return r;
    }
};

static void test_xdg_defaults()
{
    Env e;
    Resolution r = Resolve(e.Make(Convention::Xdg));
    expect("xdg: per user when the game folder is read only", r.mode == Mode::PerUser);
    expect_eq("xdg: game", r.dir[KindGame], "/opt/keeperfx");
    expect_eq("xdg: data", r.dir[KindData], "/home/u/.local/share/keeperfx");
    expect_eq("xdg: config", r.dir[KindConfig], "/home/u/.config/keeperfx");
    expect_eq("xdg: state", r.dir[KindState], "/home/u/.local/state/keeperfx");
    expect_eq("xdg: cache", r.dir[KindCache], "/home/u/.cache/keeperfx");
    expect("xdg: no warnings", r.warnings.empty());
    expect("xdg: no error", r.error.empty());
    expect("xdg: settings.toml in the config folder", !SettingsInSaveFolder(r));
    expect("xdg: probed once", e.probes == 1);
}

static void test_xdg_variables()
{
    Env e;
    e.vars["XDG_DATA_HOME"] = "/data";
    e.vars["XDG_CONFIG_HOME"] = "/cfg/";
    e.vars["XDG_STATE_HOME"] = "relative/state";
    e.vars["XDG_CACHE_HOME"] = ""; // empty is the same as unset
    Resolution r = Resolve(e.Make(Convention::Xdg));
    expect_eq("xdg vars: data", r.dir[KindData], "/data/keeperfx");
    expect_eq("xdg vars: config, trailing slash", r.dir[KindConfig], "/cfg/keeperfx");
    expect_eq("xdg vars: relative state ignored", r.dir[KindState], "/home/u/.local/state/keeperfx");
    expect_eq("xdg vars: empty cache ignored", r.dir[KindCache], "/home/u/.cache/keeperfx");
    expect("xdg vars: one warning, for the relative one", (r.warnings.size() == 1) && (r.warnings[0].find("XDG_STATE_HOME") != std::string::npos));
    expect_eq("xdg vars: origin", r.origin[KindData], "XDG_DATA_HOME");
}

static void test_xdg_no_home()
{
    Env e;
    Resolution r = Resolve(e.Make(Convention::Xdg, ""));
    expect("no home: falls back to the game folder", r.mode == Mode::Portable);
    expect("no home: says why", !r.error.empty());
    expect_eq("no home: data in the game folder", r.dir[KindData], "/opt/keeperfx");
    expect_eq("no home: cache under the game folder", r.dir[KindCache], "/opt/keeperfx/cache");

    e.vars["XDG_DATA_HOME"] = "/d";
    e.vars["XDG_CONFIG_HOME"] = "/c";
    e.vars["XDG_STATE_HOME"] = "/s";
    e.vars["XDG_CACHE_HOME"] = "/k";
    r = Resolve(e.Make(Convention::Xdg, ""));
    expect("no home but every variable: per user", (r.mode == Mode::PerUser) && r.error.empty());
    expect_eq("no home but every variable: state", r.dir[KindState], "/s/keeperfx");

    e.vars.erase("XDG_CACHE_HOME");
    r = Resolve(e.Make(Convention::Xdg, ""));
    expect("no home, one variable missing: game folder", (r.mode == Mode::Portable) && !r.error.empty());
}

static void test_portable_detection()
{
    Env e;
    e.game_is_writable = true;
    Resolution r = Resolve(e.Make(Convention::Xdg));
    expect("auto: portable when the game folder is writable", r.mode == Mode::Portable);
    expect_eq("portable: data", r.dir[KindData], "/opt/keeperfx");
    expect_eq("portable: config", r.dir[KindConfig], "/opt/keeperfx");
    expect_eq("portable: state", r.dir[KindState], "/opt/keeperfx");
    expect_eq("portable: cache", r.dir[KindCache], "/opt/keeperfx/cache");
    expect("portable: settings.toml stays in save/", SettingsInSaveFolder(r));

    Request req = e.Make(Convention::Xdg);
    req.option_portable = 0;
    e.probes = 0;
    r = Resolve(req);
    expect("-noportable: per user although writable", r.mode == Mode::PerUser);
    expect("-noportable: no probe", e.probes == 0);

    e.game_is_writable = false;
    req = e.Make(Convention::Xdg);
    req.option_portable = 1;
    r = Resolve(req);
    expect("-portable: portable although read only", r.mode == Mode::Portable);
    expect("-portable: no probe", e.probes == 0);

    e.vars["KEEPERFX_PORTABLE"] = "yes";
    r = Resolve(e.Make(Convention::Xdg));
    expect("KEEPERFX_PORTABLE=yes", r.mode == Mode::Portable);
    e.vars["KEEPERFX_PORTABLE"] = "0";
    e.game_is_writable = true;
    r = Resolve(e.Make(Convention::Xdg));
    expect("KEEPERFX_PORTABLE=0", r.mode == Mode::PerUser);
    req = e.Make(Convention::Xdg);
    req.option_portable = 1;
    r = Resolve(req);
    expect("-portable beats KEEPERFX_PORTABLE=0", r.mode == Mode::Portable);
    e.vars["KEEPERFX_PORTABLE"] = "maybe";
    e.probes = 0;
    r = Resolve(e.Make(Convention::Xdg));
    expect("KEEPERFX_PORTABLE=maybe: ignored, warned, probed", (r.mode == Mode::Portable) && (r.warnings.size() == 1) && (e.probes == 1));
}

static void test_userdir_and_overrides()
{
    Env e;
    Request req = e.Make(Convention::Xdg);
    req.option_userdir = "/tmp/me";
    req.option_portable = 1;
    Resolution r = Resolve(req);
    expect("-userdir: mode", r.mode == Mode::UserDir);
    expect_eq("-userdir: data", r.dir[KindData], "/tmp/me");
    expect_eq("-userdir: config", r.dir[KindConfig], "/tmp/me");
    expect_eq("-userdir: state", r.dir[KindState], "/tmp/me");
    expect_eq("-userdir: cache", r.dir[KindCache], "/tmp/me/cache");
    expect("-userdir: beats -portable, no probe", e.probes == 0);

    e.vars["KEEPERFX_USER_DIR"] = "/env/me";
    r = Resolve(e.Make(Convention::Xdg));
    expect_eq("KEEPERFX_USER_DIR", r.dir[KindData], "/env/me");
    req = e.Make(Convention::Xdg);
    req.option_userdir = "/opt/me";
    r = Resolve(req);
    expect_eq("-userdir beats KEEPERFX_USER_DIR", r.dir[KindData], "/opt/me");
    e.vars.erase("KEEPERFX_USER_DIR");

    e.vars["KEEPERFX_STATE_DIR"] = "/logs";
    e.vars["KEEPERFX_CONFIG_DIR"] = "/envcfg";
    req = e.Make(Convention::Xdg);
    req.option_dir[KindConfig] = "/optcfg";
    req.option_dir[KindCache] = "relative/cache";
    r = Resolve(req);
    expect_eq("override: state from the variable", r.dir[KindState], "/logs");
    expect_eq("override: option beats variable", r.dir[KindConfig], "/optcfg");
    expect_eq("override: relative option kept", r.dir[KindCache], "relative/cache");
    expect_eq("override: data untouched", r.dir[KindData], "/home/u/.local/share/keeperfx");
    expect_eq("override: origin", r.origin[KindConfig], "-configdir");

    e.game_is_writable = true;
    r = Resolve(e.Make(Convention::Xdg));
    expect("override in portable mode", (r.mode == Mode::Portable) && (r.dir[KindState] == "/logs") && (r.dir[KindData] == "/opt/keeperfx"));
}

static void test_macos()
{
    Env e;
    Resolution r = Resolve(e.Make(Convention::MacOS, "/Users/u"));
    expect("macos: per user", r.mode == Mode::PerUser);
    expect_eq("macos: data", r.dir[KindData], "/Users/u/Library/Application Support/KeeperFX");
    expect_eq("macos: config", r.dir[KindConfig], "/Users/u/Library/Application Support/KeeperFX");
    expect_eq("macos: state", r.dir[KindState], "/Users/u/Library/Logs/KeeperFX");
    expect_eq("macos: cache", r.dir[KindCache], "/Users/u/Library/Caches/KeeperFX");
    expect("macos: settings.toml in save/, data and config share a folder", SettingsInSaveFolder(r));
    e.vars["XDG_DATA_HOME"] = "/x";
    r = Resolve(e.Make(Convention::MacOS, "/Users/u"));
    expect_eq("macos: XDG variables don't apply", r.dir[KindData], "/Users/u/Library/Application Support/KeeperFX");
    r = Resolve(e.Make(Convention::MacOS, ""));
    expect("macos: no home, game folder", (r.mode == Mode::Portable) && !r.error.empty());
}

static void test_windows()
{
    Env e;
    Request req = e.Make(Convention::Windows, "");
    req.game_dir = "C:\\Program Files\\KeeperFX";
    req.win_saved_games = "C:\\Users\\J\xf6rg\\Saved Games";
    req.win_roaming_app_data = "C:\\Users\\J\xf6rg\\AppData\\Roaming";
    req.win_local_app_data = "C:\\Users\\J\xf6rg\\AppData\\Local";
    Resolution r = Resolve(req);
    expect("windows: per user", r.mode == Mode::PerUser);
    expect_eq("windows: data", r.dir[KindData], "C:\\Users\\J\xf6rg\\Saved Games\\KeeperFX");
    expect_eq("windows: config", r.dir[KindConfig], "C:\\Users\\J\xf6rg\\AppData\\Roaming\\KeeperFX");
    expect_eq("windows: state", r.dir[KindState], "C:\\Users\\J\xf6rg\\AppData\\Local\\KeeperFX\\Logs");
    expect_eq("windows: cache", r.dir[KindCache], "C:\\Users\\J\xf6rg\\AppData\\Local\\KeeperFX\\Cache");
    expect("windows: settings.toml in the config folder", !SettingsInSaveFolder(r));

    req.win_saved_games.clear();
    r = Resolve(req);
    expect_eq("windows: no Saved Games, data in Local AppData", r.dir[KindData], "C:\\Users\\J\xf6rg\\AppData\\Local\\KeeperFX");
    expect("windows: no Saved Games, warned", r.warnings.size() == 1);

    req.win_roaming_app_data.clear();
    req.win_local_app_data.clear();
    r = Resolve(req);
    expect("windows: no known folder, game folder", (r.mode == Mode::Portable) && !r.error.empty());
    expect_eq("windows: no known folder, cache", r.dir[KindCache], "C:\\Program Files\\KeeperFX\\cache");

    expect("absolute: C:\\x", IsAbsolute("C:\\x", Convention::Windows));
    expect("absolute: C:/x", IsAbsolute("C:/x", Convention::Windows));
    expect("absolute: \\\\server\\x", IsAbsolute("\\\\server\\x", Convention::Windows));
    expect("not absolute: C:x", !IsAbsolute("C:x", Convention::Windows));
    expect("not absolute on Unix: C:\\x", !IsAbsolute("C:\\x", Convention::Xdg));
    expect("not absolute: x/y", !IsAbsolute("x/y", Convention::Xdg));
}

static void test_game_dir()
{
    GameDirRequest g;
    g.exe_dir = "/usr/lib/keeperfx";
    g.exe_dir_has_game = true;
    expect_eq("game dir: next to the executable", ResolveGameDir(g).dir, "/usr/lib/keeperfx");
    g.cwd_has_game = true;
    expect_eq("game dir: working folder first", ResolveGameDir(g).dir, ".");
    g.env = "/env/game";
    expect_eq("game dir: variable", ResolveGameDir(g).dir, "/env/game");
    g.option = "/opt/game";
    expect_eq("game dir: option", ResolveGameDir(g).dir, "/opt/game");
    GameDirRequest none;
    none.exe_dir = "/somewhere";
    expect_eq("game dir: nothing found, working folder", ResolveGameDir(none).dir, ".");
}

static void test_join()
{
    expect_eq("join", Join("/a", "b"), "/a/b");
    expect_eq("join: trailing slash", Join("/a/", "b"), "/a/b");
    expect_eq("join: windows style", Join("C:\\a", "b"), "C:\\a\\b");
    expect_eq("join: mixed uses /", Join("C:\\a/x", "b"), "C:\\a/x/b");
    expect_eq("join: empty", Join("", "b"), "b");
    expect("parse bool", (ParseBool("ON") == 1) && (ParseBool("false") == 0) && (ParseBool("2") == -1));
}

/* File system tests, in a temporary folder. */

static std::string tmp_root;

static std::string T(const std::string &rel)
{
    return Join(tmp_root, rel);
}

static void write_file(const std::string &path, const std::string &content)
{
    std::string error;
    MakeDirs(FromPath(ToPath(path).parent_path()), error);
    std::ofstream out(ToPath(path), std::ios::binary | std::ios::trunc);
    out << content;
}

static std::string read_file(const std::string &path)
{
    std::ifstream in(ToPath(path), std::ios::binary);
    if (!in)
        return "<missing>";
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

static void test_make_dirs()
{
    std::string error;
    expect("make dirs: nested", MakeDirs(T("a/b/c"), error) && fs::is_directory(ToPath(T("a/b/c"))));
    expect("make dirs: existing", MakeDirs(T("a/b"), error));
    write_file(T("file"), "x");
    expect("make dirs: a file in the way", !MakeDirs(T("file/sub"), error) && !error.empty());
#ifndef _WIN32
    struct stat st;
    expect("make dirs: created folders are private", (stat(T("a/b").c_str(), &st) == 0) && ((st.st_mode & 0777) == 0700));
#endif
    expect("writable: temp folder", IsWritableDir(T("a")));
    expect("writable: missing folder", !IsWritableDir(T("nope")));
    expect("writable: no probe file left", !fs::exists(ToPath(T("a/b/.keeperfx-write-test"))) && (std::distance(fs::directory_iterator(ToPath(T("a"))), fs::directory_iterator()) == 1));
#ifndef _WIN32
    if (geteuid() != 0) {
        chmod(T("a/b/c").c_str(), 0500);
        expect("writable: read only folder", !IsWritableDir(T("a/b/c")));
        chmod(T("a/b/c").c_str(), 0700);
    }
#endif
    expect("same dir: itself through ..", SameDir(T("a/b"), T("a/b/c/..")));
    expect("same dir: different", !SameDir(T("a/b"), T("a")));
}

static void make_install(const std::string &game)
{
    write_file(Join(game, "keeperfx.cfg"), "INSTALL_PATH=./\n");
    write_file(Join(game, "save/fx1g0000.sav"), "slot zero");
    write_file(Join(game, "save/fx1g0003.sav"), "slot three");
    write_file(Join(game, "save/settings.toml"), "[video]\n");
    write_file(Join(game, "save/backup/old.zip"), "backup");
    write_file(Join(game, "replays/campaign/20260101T000000_c1.fxpkt"), "replay");
    write_file(Join(game, "scrshots/scr00000.png"), "png");
    write_file(Join(game, "scrshots/\xc3\xa9t\xc3\xa9.png"), "unicode name");
    write_file(Join(game, "data/main.pal"), "game data, not the player's");
}

static void test_migration()
{
    const std::string game = T("install");
    make_install(game);
    MigrationPaths p;
    p.game_dir = game;
    p.data_dir = T("user/data");
    p.config_dir = T("user/config");
    p.settings_in_save_folder = false;

    // Keep one file the player already has in the per-user folder.
    write_file(T("user/data/save/fx1g0003.sav"), "newer slot three");
    fs::file_time_type src_time = fs::last_write_time(ToPath(Join(game, "save/fx1g0000.sav")));

    MigrationResult r = MigrateInstall(p);
    expect("migrate: ran", r.ran);
    expect("migrate: complete", r.complete);
    expect("migrate: counts", (r.copied == 6) && (r.kept == 1) && (r.failed == 0));
    expect_eq("migrate: save copied", read_file(T("user/data/save/fx1g0000.sav")), "slot zero");
    expect_eq("migrate: existing save kept", read_file(T("user/data/save/fx1g0003.sav")), "newer slot three");
    expect_eq("migrate: settings to the config folder", read_file(T("user/config/settings.toml")), "[video]\n");
    expect("migrate: no settings left in save/", !fs::exists(ToPath(T("user/data/save/settings.toml"))));
    expect_eq("migrate: sub folder", read_file(T("user/data/save/backup/old.zip")), "backup");
    expect_eq("migrate: replay", read_file(T("user/data/replays/campaign/20260101T000000_c1.fxpkt")), "replay");
    expect_eq("migrate: unicode name", read_file(T("user/data/scrshots/\xc3\xa9t\xc3\xa9.png")), "unicode name");
    expect("migrate: game data not copied", !fs::exists(ToPath(T("user/data/data/main.pal"))));
    expect("migrate: date kept", fs::last_write_time(ToPath(T("user/data/save/fx1g0000.sav"))) == src_time);
    expect("migrate: no temporary file left", !fs::exists(ToPath(T("user/data/save/fx1g0000.sav.migrating"))));
    expect_eq("migrate: source untouched", read_file(Join(game, "save/fx1g0003.sav")), "slot three");
    expect_eq("migrate: source still there", read_file(Join(game, "save/settings.toml")), "[video]\n");
    expect("migrate: install recorded", read_file(T("user/data/migrated_installs.txt")).find(AbsolutePath(game)) == 0);
    bool said_differs = false;
    for (const std::string &l : r.lines)
        said_differs |= (l.find("differs") != std::string::npos);
    expect("migrate: the log says a kept file differs", said_differs);

    // Once only.
    write_file(Join(game, "save/fx1g0004.sav"), "made later");
    r = MigrateInstall(p);
    expect("migrate: once per install", !r.ran);
    expect("migrate: later file not copied", !fs::exists(ToPath(T("user/data/save/fx1g0004.sav"))));

    // Nothing to migrate: nothing recorded.
    const std::string fresh = T("fresh");
    write_file(Join(fresh, "keeperfx.cfg"), "");
    MigrationPaths pf = p;
    pf.game_dir = fresh;
    pf.data_dir = T("fresh_user");
    r = MigrateInstall(pf);
    expect("migrate: nothing to do", !r.ran && !fs::exists(ToPath(T("fresh_user/migrated_installs.txt"))));

    // One shared folder (-userdir, macOS): settings.toml stays in save/.
    MigrationPaths ps = p;
    ps.data_dir = ps.config_dir = T("shared");
    ps.settings_in_save_folder = true;
    r = MigrateInstall(ps);
    expect_eq("migrate shared: settings in save/", read_file(T("shared/save/settings.toml")), "[video]\n");
}

static void test_migration_failure()
{
#ifndef _WIN32
    if (geteuid() == 0)
        return;
    const std::string game = T("install2");
    make_install(game);
    MigrationPaths p;
    p.game_dir = game;
    p.data_dir = T("user2/data");
    p.config_dir = T("user2/config");
    std::string error;
    MakeDirs(T("user2/data/save"), error);
    chmod(T("user2/data/save").c_str(), 0500); // can't write saves
    MigrationResult r = MigrateInstall(p);
    expect("migrate failure: incomplete", r.ran && !r.complete && (r.failed > 0));
    expect("migrate failure: other files still copied", read_file(T("user2/data/replays/campaign/20260101T000000_c1.fxpkt")) == "replay");
    expect("migrate failure: not recorded", !fs::exists(ToPath(T("user2/data/migrated_installs.txt"))));
    expect_eq("migrate failure: source untouched", read_file(Join(game, "save/fx1g0000.sav")), "slot zero");
    chmod(T("user2/data/save").c_str(), 0700);
    r = MigrateInstall(p);
    expect("migrate failure: next start finishes", r.ran && r.complete && (r.failed == 0) && (r.copied > 0));
    expect_eq("migrate failure: save copied now", read_file(T("user2/data/save/fx1g0000.sav")), "slot zero");
    r = MigrateInstall(p);
    expect("migrate failure: then done", !r.ran);
#endif
}

static void test_narrow_paths()
{
#ifdef _WIN32
    // A user name in a script the ANSI code page lacks: the path must still reach the narrow file functions.
    const std::wstring base = WidePath(tmp_root);
    const std::wstring wide = base + L"\\\u0418\u0432\u0430\u043d \u4e2d"; // Cyrillic and CJK
    expect("narrow: made the folder", CreateDirectoryW(wide.c_str(), NULL) != 0);
    const std::string narrow = NarrowPath(wide);
    expect("narrow: has a narrow form", !narrow.empty());
    write_file(Join(narrow, "fx1g0000.sav"), "saved");
    const std::wstring wide_file = wide + L"\\fx1g0000.sav";
    expect("narrow: a file written through it is in the folder", GetFileAttributesW(wide_file.c_str()) != INVALID_FILE_ATTRIBUTES);
    expect_eq("narrow: and reads back", read_file(Join(narrow, "fx1g0000.sav")), "saved");
    std::string error;
    expect("narrow: make dirs below it", MakeDirs(Join(narrow, "save"), error));
    expect_eq("narrow: ASCII stays as it is", NarrowPath(L"C:\\Games\\KeeperFX"), "C:\\Games\\KeeperFX");
    expect("narrow: a missing folder with such a name has none", NarrowPath(base + L"\\\u0416\u0416").empty());
#endif
}


static ScriptPath SP(const std::string &path, bool case_insensitive = false)
{
    static const std::vector<std::string> roots = {"/home/u/.local/share/keeperfx", "/opt/keeperfx", "C:\\Games\\KeeperFX"};
    return ScriptLogicalPath(path, roots, case_insensitive);
}

static void expect_script_ok(const char *what, const std::string &path, const std::string &logical, bool ci = false)
{
    ScriptPath r = SP(path, ci);
    checks++;
    if (!r.ok || (r.logical != logical)) {
        printf("FAIL %s: \"%s\" gave %s \"%s\", want \"%s\"\n", what, path.c_str(), r.ok ? "ok" : "refused",
               r.ok ? r.logical.c_str() : r.error.c_str(), logical.c_str());
        fails++;
    }
}

static void expect_script_refused(const char *what, const std::string &path, bool ci = false)
{
    ScriptPath r = SP(path, ci);
    checks++;
    if (r.ok || r.error.empty()) {
        printf("FAIL %s: \"%s\" was allowed as \"%s\"\n", what, path.c_str(), r.logical.c_str());
        fails++;
    }
}

static void test_script_paths()
{
    // What the Workshop maps do.
    expect_script_ok("script: Dungeon Architect's seed file", "levels/dungeon_architect/dungeon_architect_seed.txt",
                     "levels/dungeon_architect/dungeon_architect_seed.txt");
    expect_script_ok("script: a file beside the script, by its chunk name", "./campgns/heroes_journey/tempsavefile.txt",
                     "campgns/heroes_journey/tempsavefile.txt");
    expect_script_ok("script: absolute, in the data folder", "/home/u/.local/share/keeperfx/campgns/hj/tempsavefile.txt",
                     "campgns/hj/tempsavefile.txt");
    expect_script_ok("script: absolute, in the game folder", "/opt/keeperfx/levels/x/seed.txt", "levels/x/seed.txt");
    expect_script_ok("script: Windows absolute, backslashes", "C:\\Games\\KeeperFX\\levels\\x\\seed.txt", "levels/x/seed.txt");
    expect_script_ok("script: Windows absolute, slashes, other case", "c:/games/keeperfx/Levels/x.txt", "Levels/x.txt", true);
    expect_script_refused("script: other case is another folder where case counts", "/OPT/keeperfx/levels/x.txt");
    expect_script_ok("script: backslashes", "levels\\x\\seed.txt", "levels/x/seed.txt");
    expect_script_ok("script: .. that stays inside", "levels/x/../y/seed.txt", "levels/y/seed.txt");
    expect_script_ok("script: absolute with .. back inside", "/opt/keeperfx/levels/../campgns/a.txt", "campgns/a.txt");
    expect_script_ok("script: doubled separators", "levels//x///a.txt", "levels/x/a.txt");
    expect_script_ok("script: a dot inside a name", "levels/x/v1.2.txt", "levels/x/v1.2.txt");

    // Out of the folders.
    expect_script_refused("script: .. out of the folders", "../keeperfx.cfg");
    expect_script_refused("script: .. out after a folder", "levels/../../x.txt");
    expect_script_refused("script: .. out, backslashes", "levels\\..\\..\\x.txt");
    expect_script_refused("script: absolute elsewhere", "/etc/passwd");
    expect_script_refused("script: absolute .. out of the game folder", "/opt/keeperfx/../keeperfx2/x.txt");
    expect_script_refused("script: a folder whose name starts like the game's", "/opt/keeperfx2/x.txt");
    expect_script_refused("script: home", "/home/u/.bashrc");
    expect_script_refused("script: another drive", "D:\\Games\\KeeperFX\\levels\\x.txt");
    expect_script_refused("script: Windows absolute elsewhere", "C:\\Windows\\win.ini");
    expect_script_refused("script: drive relative", "C:levels\\x.txt");
    expect_script_refused("script: UNC", "\\\\server\\share\\x.txt");
    expect_script_refused("script: UNC with slashes", "//server/share/x.txt");
    expect_script_refused("script: device path", "\\\\.\\PhysicalDrive0");
    expect_script_refused("script: long path prefix", "\\\\?\\C:\\Games\\KeeperFX\\levels\\x.txt");
    expect_script_refused("script: the folder itself", "/opt/keeperfx");
    expect_script_refused("script: empty", "");
    expect_script_refused("script: only dots", "./.");

    // Names Windows reads as something else.
    expect_script_refused("script: alternate data stream", "levels/x/seed.txt:hidden");
    expect_script_refused("script: stream of a folder", "levels/x:$DATA");
    expect_script_refused("script: device CON", "levels/CON");
    expect_script_refused("script: device nul with an extension", "levels/nul.txt");
    expect_script_refused("script: device in lower case, as a folder", "levels/aux/x.txt");
    expect_script_refused("script: COM1", "levels/com1.log");
    expect_script_refused("script: LPT9", "LPT9");
    expect_script_refused("script: COM superscript one", "levels/COM\xc2\xb9.txt");
    expect_script_refused("script: CONOUT$", "levels/CONOUT$");
    expect_script_refused("script: device with a trailing space", "levels/CON .txt");
    expect_script_ok("script: a name that only starts like a device", "levels/console.txt", "levels/console.txt");
    expect_script_ok("script: COM10 is no device", "levels/COM10.txt", "levels/COM10.txt");
    expect_script_refused("script: trailing dot", "levels/x/seed.txt.");
    expect_script_refused("script: trailing space", "levels/x/seed.txt ");
    expect_script_refused("script: wildcard", "levels/*.txt");
    expect_script_refused("script: angle bracket", "levels/<x>.txt");
    expect_script_refused("script: pipe", "levels/a|b");
    expect_script_refused("script: control character", "levels/a\tb");

    // Where scripts may write.
    expect("write: levels", ScriptMayWrite("levels/x/seed.txt"));
    expect("write: campgns", ScriptMayWrite("campgns/hj/tempsavefile.txt"));
    expect("write: multiplayer", ScriptMayWrite("multiplayer/classic/x.txt"));
    expect("write: mods", ScriptMayWrite("mods/m/x.txt"));
    expect("write: any case", ScriptMayWrite("Levels/x.txt"));
    expect("write: not the game's files", !ScriptMayWrite("keeperfx.cfg"));
    expect("write: not the data folder", !ScriptMayWrite("data/main.pal"));
    expect("write: not saves", !ScriptMayWrite("save/fx1g0000.sav"));
    expect("write: not the settings", !ScriptMayWrite("save/settings.toml"));
    expect("write: not fxdata", !ScriptMayWrite("fxdata/lua/init.lua"));
    expect("write: not a folder only", !ScriptMayWrite("levels/"));
    expect("write: not a name that starts like one", !ScriptMayWrite("levelsx/a.txt"));

    // Names kept in saves and sent to other players.
    const std::vector<std::string> roots = {"/opt/keeperfx", "/opt/keeperfx/users/u", "C:\\Games\\KeeperFX"};
    expect_eq("relative: game", RelativeToRoots("/opt/keeperfx/campgns/a/media/s.ogg", roots, false), "campgns/a/media/s.ogg");
    expect_eq("relative: the deepest folder", RelativeToRoots("/opt/keeperfx/users/u/music/s.ogg", roots, false), "music/s.ogg");
    expect_eq("relative: Windows", RelativeToRoots("c:\\games\\keeperfx\\music\\s.ogg", roots, true), "music/s.ogg");
    expect_eq("relative: elsewhere", RelativeToRoots("/home/u/music/s.ogg", roots, false), "");
    expect_eq("relative: relative paths are not compared", RelativeToRoots("music/s.ogg", roots, false), "");
}

static void test_within_dir()
{
    std::string error;
    MakeDirs(T("within/data/levels"), error);
    MakeDirs(T("outside"), error);
    write_file(T("outside/secret.txt"), "secret");
    expect("within: a file", IsWithinDir(T("within/data"), T("within/data/levels/x.txt")));
    expect("within: a missing folder", IsWithinDir(T("within/data"), T("within/data/new/x.txt")));
    expect("within: not a sibling", !IsWithinDir(T("within/data"), T("within/data2/x.txt")));
    expect("within: not through ..", !IsWithinDir(T("within/data"), T("within/data/../../outside/secret.txt")));
#ifdef _WIN32
    std::string upper = T("within/data");
    for (char &c : upper)
        c = (char)toupper((unsigned char)c);
    expect("within: Windows ignores case", IsWithinDir(upper, T("within/data/levels/x.txt")));
    expect("within: Windows, backslashes", IsWithinDir(T("within\\data"), T("within/data/levels\\x.txt")));
#endif
    std::error_code ec;
    fs::create_directory_symlink(ToPath(T("outside")), ToPath(T("within/data/levels/link")), ec);
    if (ec) {
        printf("skipped the link tests: %s\n", ec.message().c_str());
    } else {
        expect("within: a link out", !IsWithinDir(T("within/data"), T("within/data/levels/link/secret.txt")));
        expect("within: a link out, file to make", !IsWithinDir(T("within/data"), T("within/data/levels/link/new.txt")));
    }
    // The data folder itself reached through a link is fine.
    fs::create_directory_symlink(ToPath(T("within/data")), ToPath(T("datalink")), ec);
    if (!ec)
        expect("within: the folder through a link", IsWithinDir(T("datalink"), T("within/data/levels/x.txt")));
}

static void test_portable_rule_on_disk()
{
    // A writable folder counts, whoever owns it: the system's temporary folder belongs to root and everyone may
    // write in it, like an NTFS or FAT mount with uid=0, a shared install or a container volume.
    std::error_code ec;
    const fs::path sys_tmp = fs::temp_directory_path(ec);
#ifndef _WIN32
    struct stat st;
    if (!ec && (stat(sys_tmp.c_str(), &st) == 0) && (st.st_uid != geteuid()))
        expect("portable: a writable folder of another owner", IsWritableDir(FromPath(sys_tmp)));
    // Files: the user's own, or one of root's on a file system with owners.
    write_file(T("mine.txt"), "x");
    expect("file is users: own file", FileIsUsers(T("mine.txt")));
    if ((geteuid() != 0) && (stat("/etc/passwd", &st) == 0) && (st.st_uid != geteuid()))
        expect("file is users: root's file", !FileIsUsers("/etc/passwd"));
    expect("file is users: missing file", !FileIsUsers(T("nope.txt")));
#else
    write_file(T("mine.txt"), "x");
    expect("file is users: own file", FileIsUsers(T("mine.txt")));
#endif
}

static void test_migration_other_users()
{
    const std::string game = T("install3");
    make_install(game);
    MigrationPaths p;
    p.game_dir = game;
    p.data_dir = T("user3/data");
    p.config_dir = T("user3/config");
    // Only the replays are this user's: the saves and settings are those of whoever played in the install before.
    p.file_is_users = [](const std::string &path) { return path.find("replays") != std::string::npos; };
    MigrationResult r = MigrateInstall(p);
    expect("migrate others: complete", r.ran && r.complete);
    expect("migrate others: counts", (r.copied == 1) && (r.left == 6) && (r.failed == 0));
    expect_eq("migrate others: own replay copied", read_file(T("user3/data/replays/campaign/20260101T000000_c1.fxpkt")), "replay");
    expect("migrate others: saves left", !fs::exists(ToPath(T("user3/data/save/fx1g0000.sav"))));
    expect("migrate others: settings left", !fs::exists(ToPath(T("user3/config/settings.toml"))));
    bool said = false;
    for (const std::string &l : r.lines)
        said |= (l.find("belongs to another user") != std::string::npos);
    expect("migrate others: the log says why", said);
    r = MigrateInstall(p);
    expect("migrate others: once", !r.ran);

    // The default test is the file's owner: the test's own files are copied.
    const std::string game4 = T("install4");
    make_install(game4);
    MigrationPaths p4;
    p4.game_dir = game4;
    p4.data_dir = T("user4/data");
    p4.config_dir = T("user4/config");
    r = MigrateInstall(p4);
    expect("migrate own: all copied", r.ran && r.complete && (r.copied == 7) && (r.left == 0));
}

int main()
{
    test_xdg_defaults();
    test_xdg_variables();
    test_xdg_no_home();
    test_portable_detection();
    test_userdir_and_overrides();
    test_macos();
    test_windows();
    test_game_dir();
    test_join();
    test_script_paths();

    std::error_code ec;
    fs::path base = fs::temp_directory_path(ec);
    if (ec) {
        printf("FAIL no temporary folder\n");
        return 1;
    }
    char name[64];
    snprintf(name, sizeof(name), "kfx_userdirs_tests_%u", (unsigned)rand() ^ (unsigned)time(NULL));
    tmp_root = FromPath(base / name);
    std::string error;
    if (!MakeDirs(tmp_root, error)) {
        printf("FAIL %s\n", error.c_str());
        return 1;
    }
    test_make_dirs();
    test_migration();
    test_migration_failure();
    test_narrow_paths();
    test_within_dir();
    test_portable_rule_on_disk();
    test_migration_other_users();
    fs::remove_all(ToPath(tmp_root), ec);

    printf("%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
