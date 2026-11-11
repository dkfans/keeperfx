# Where KeeperFX keeps your files

KeeperFX keeps two kinds of files apart: the **game's files** (the install: configs, maps, graphics, sounds, and
the files copied from the original Dungeon Keeper) and **your files** (saves, settings, logs, screenshots,
replays, and maps, campaigns and mods you add).

An install you unpacked yourself, in a folder you can write to, is a **portable install**: your files stay in the
game folder, next to `keeperfx.exe`, exactly as KeeperFX has always done. Nothing changes for it, and the
launchers keep working. An install you can't write to (a Linux package, Flatpak or AppImage, `Program Files`
without administrator rights, a macOS app bundle) keeps your files in your own per-user folders instead.

The game writes which folders it uses at the start of its log:

```
User files are kept in the per-user folders, because the game folder is read only for this user
  game   folder: /usr/share/keeperfx (KEEPERFX_GAME_DIR)
  data   folder: /home/me/.local/share/keeperfx ($HOME/.local/share)
  config folder: /home/me/.config/keeperfx ($HOME/.config)
  state  folder: /home/me/.local/state/keeperfx ($HOME/.local/state)
  cache  folder: /home/me/.cache/keeperfx ($HOME/.cache)
```

## The folders

| Folder | What is in it |
|---|---|
| game   | The install. Read only, except in a portable install. |
| data   | `save/` (saved games, campaign progress, high scores, the continue file), `replays/`, `scrshots/` (screenshots and recorded movies), `native_types.lua`, the files map scripts write, and anything you add: `campgns/`, `levels/`, `multiplayer/`, `mods/`, `data/`, `sound/`, `music/` and so on. |
| config | `keeperfx.cfg` with the options you change, `settings.toml` (the options menu and key bindings), `renderer_prefs.ini`. |
| state  | `keeperfx.log`, crash dumps (`keeperfx_crash_*.dmp`, Windows), debug packet dumps (`keeperd*.pck`). |
| cache  | Files the game makes when they are missing: colour tables (`data/tables.dat`, `alpha.col`, `colours.col`, and `redpal.col`, `whitepal.col`, `mapfadeg.dat` when the original files lack them) and speech extracted from map zips. Safe to delete. |

### Per operating system

| | Portable install | Linux, BSD (XDG) | Windows | macOS |
|---|---|---|---|---|
| data   | game folder | `$XDG_DATA_HOME/keeperfx`, default `~/.local/share/keeperfx` | `Saved Games\KeeperFX` | `~/Library/Application Support/KeeperFX` |
| config | game folder | `$XDG_CONFIG_HOME/keeperfx`, default `~/.config/keeperfx` | `AppData\Roaming\KeeperFX` | `~/Library/Application Support/KeeperFX` |
| state  | game folder | `$XDG_STATE_HOME/keeperfx`, default `~/.local/state/keeperfx` | `AppData\Local\KeeperFX\Logs` | `~/Library/Logs/KeeperFX` |
| cache  | `cache` in the game folder | `$XDG_CACHE_HOME/keeperfx`, default `~/.cache/keeperfx` | `AppData\Local\KeeperFX\Cache` | `~/Library/Caches/KeeperFX` |

- **Linux and the BSDs** follow the
  [XDG Base Directory specification](https://specifications.freedesktop.org/basedir-spec/latest/): a variable that
  is unset or empty gets its default, and one holding a relative path is ignored (with a warning in the log), as
  the specification says. Logs are state. When `HOME` is not set the home folder comes from the password database.
  The folders the game creates are private (mode 0700).
- **Windows** uses the Known Folders. Saves go to *Saved Games*, which Windows made for saved games: Explorer shows
  it, backup tools include it, and players find their saves there, unlike the hidden AppData. It is not part of a
  roaming profile, so replays and added maps don't slow down logging on. Settings are small and go to the roaming
  AppData, so they follow the player; logs and caches belong to this machine and go to the local AppData. When
  Windows has no Saved Games folder, saves go to the local AppData. The game's file functions take paths in the
  ANSI code page; a folder whose name has characters that code page lacks (a user name in another script) is used
  by its short 8.3 name, so it still works.
- **macOS** uses `~/Library`. Data and settings share `Application Support`, as Apple recommends for files that
  aren't preference lists. (There is no macOS build yet; the rules are in place and tested.)

SDL3's `SDL_GetPrefPath()` gives one folder per application: it can't keep settings, logs and caches apart on
Linux, and it creates the folder whenever it is asked. KeeperFX therefore works the folders out itself; earlier
versions kept `renderer_prefs.ini` in `SDL_GetPrefPath("keeperfx", "keeperfx")`, and the game copies that file over
once (see below).

When the data and config folders are the same folder (portable installs, `-userdir`, macOS), `settings.toml` is
`save/settings.toml`, as in every portable install; otherwise it is `settings.toml` in the config folder.

## Portable or not

The game decides at every start, in this order:

1. `-userdir <folder>` or `KEEPERFX_USER_DIR`: all your files in that one folder, laid out like a portable
   install (the cache in its `cache` sub folder). You can point it at an old portable install to use its saves.
2. `-portable` or `KEEPERFX_PORTABLE=1`: your files in the game folder. The log says so when that folder is read
   only, since nothing could be saved.
3. `-noportable` or `KEEPERFX_PORTABLE=0`: the per-user folders, even when the game folder is writable.
4. Otherwise, **portable when you can write in the game folder**; the per-user folders when not. The game finds
   out by creating (and deleting) a file there.

Why this rule: an install the player can write to is one KeeperFX has always kept the player's files in, so every
such install stays portable and nothing about it changes: no moved saves, no new places to look, and the
launchers, which read and write `keeperfx.cfg`, `save/`, `replays/` and `mods/` next to the executable, keep
working. An install that the player can't write to could never have worked as a portable one, so switching it to
the per-user folders breaks nothing. Who owns the folder doesn't count: on FAT, exFAT and NTFS mounts (a Windows
partition shared with Linux, an SD card), network shares and container volumes the owner the system reports is
often root or another user, and such an install would otherwise move the player's saves away from where Windows
finds them. A marker file was considered and rejected: the release archives would have to ship it, and an install
updated without it (copying only the executable, or an old patch) would silently lose its saves.

An install that several users can write to (`/opt/keeperfx` writable by a group) is therefore portable for all of
them, and they share its saves and settings, as before. To give each user their own, start the game with
`-noportable` or set `KEEPERFX_PORTABLE=0` for them (in a wrapper script, or system wide), or make the folder read
only. Running the game as root in a system folder makes it portable; don't do that.

### Command line options and environment variables

| Option | Variable | Sets |
|---|---|---|
| `-gamedir <folder>` | `KEEPERFX_GAME_DIR` | The game folder. |
| `-userdir <folder>` | `KEEPERFX_USER_DIR` | All your folders, in one. |
| `-userdatadir <folder>` | `KEEPERFX_USER_DATA_DIR` | The data folder. |
| `-configdir <folder>` | `KEEPERFX_CONFIG_DIR` | The config folder. |
| `-statedir <folder>` | `KEEPERFX_STATE_DIR` | The state folder (the log). |
| `-cachedir <folder>` | `KEEPERFX_CACHE_DIR` | The cache folder. |
| `-portable`, `-noportable` | `KEEPERFX_PORTABLE=1` or `0` | Portable or not, see above. |

An option beats its variable. The four folder options apply on top of whatever the rules above chose, also in a
portable install. Relative paths are relative to the working folder. On Windows the folders may have any name,
also in a script the ANSI code page lacks (they are then used by their short 8.3 name). `-log <file>` writes the log to that file,
in the state folder unless it is an absolute path.

Without `-gamedir`, the game folder is the working folder when it holds `keeperfx.cfg` (how the launchers and
every install start the game), else the folder of the executable when that holds `keeperfx.cfg` (the `Resources`
folder in a macOS app bundle), else the working folder. A package that puts the executable elsewhere than the game
data starts it with `-gamedir` or `KEEPERFX_GAME_DIR`, for instance from a wrapper script.

## Your folder layered over the game folder

When your data folder is not the game folder, the game looks for every game file there first, then in the game
folder, the same way a mod's files are found before the game's. Lists of files, such as the campaigns, map packs,
`.lif` and `.lof` level lists, sprite zips and music, hold the files of both folders, yours winning when a name is
in both. So you can drop a campaign, a map pack or a mod (and `mods/load_order.cfg`) into your data folder, and a
read only install can take the original Dungeon Keeper files in your data folder's `data/` and `sound/`.

The files the game writes (saves, screenshots, replays) are in your data folder only.

`keeperfx.cfg` is read from the game folder, then the one in your config folder, which only needs the options you
change. `-config <file>` reads that file instead of both.

Multiplayer compares the game files that players loaded by their names inside the game or data folder, so a file
you added counts as the game's file it replaces: everyone must have the same.

### Map scripts

Lua scripts of maps open files by relative paths. A file a script writes goes to your data folder (making the sub
folders it names); a file it reads is your copy if there is one, else the game folder's; `os.remove` only removes
files in your data folder. Absolute paths are unchanged. In a portable install all of these are the game folder,
as before.

## Moving from a portable install

When an install that was portable starts keeping your files elsewhere (it was made read only, or you asked with
`-noportable` or `-userdir`), the game copies your files over at that start:

- `save/` (with `settings.toml` going to the config folder), `replays/` and `scrshots/` of the game folder, but
  only the files that are yours: those you own, or, on a file system that keeps no owners (FAT, exFAT, NTFS and
  network shares on Linux, FAT and exFAT on Windows), those you can read. The files of whoever else played in the
  install (root, an administrator, another user) stay theirs, settings included;
- `renderer_prefs.ini` from where earlier versions kept it (`SDL_GetPrefPath`), in every mode, when the config
  folder has none.

It only copies: nothing in the game folder is changed or deleted, a file already in your folders is kept (the log
says when it differs from the game folder's), and every copy is compared with its original before it gets its
name. When a copy fails (a full disk, say) the log says so and the next start tries again; once all went well the
install is recorded in `migrated_installs.txt` in the data folder and isn't copied again. Saves made later in the
game folder by an older version are not copied after that.

The log lists every file copied, kept or left, and says where your files are now. The player is told too, once: a
message in the main menu says where the saves and settings are kept, why, and how many files were copied or left.

`keeperfx.cfg` is not copied: the game still reads the game folder's, so edits made there still count. Maps,
campaigns and mods you put in the game folder stay there and are still found.

## Launchers, packages, the Steam Deck

- The launchers work with portable installs, which is what they install and update. They start the game in the
  game folder and edit `keeperfx.cfg`, `mods/load_order.cfg`, `save/` and `replays/` there; none of that changes.
- The Windows and Linux release archives and the Steam Deck bundle are portable installs; they need nothing new.
- A Linux package installs the game data read only (for instance in `/usr/share/keeperfx`) and starts the game
  with `-gamedir` or `KEEPERFX_GAME_DIR`. The player puts the original Dungeon Keeper files into
  `~/.local/share/keeperfx/data` and `sound`. In a Flatpak the XDG variables point into `~/.var/app/<id>`.

## What the game reads and writes

Every place the game creates or changes a file, and every file of the player it reads:

| File | Kind | Code | Where |
|---|---|---|---|
| `save/fx1g*.sav` saved games | data | `SaveManager.cpp`, `SaveCatalogue.cpp` (`FGrp_Save`) | data |
| `save/fx1contn.sav`, `fx1lastf.sav`, campaign progress `*.sav` | data | `SaveContinue.cpp` (`FGrp_Save`) | data |
| `save/scr_*.dat` high scores | data | `highscores.c` (`FGrp_Save`) | data |
| `replays/<kind>/*.fxpkt` automatic replays | data | `replay.c` (`FGrp_Replays`) | data |
| `-packetsave`/`-packetload` file | data | `replay.c` | as given, relative to the working folder |
| `scrshots/scr*.png|bmp` screenshots | data | `scrcapt.c` (`FGrp_SShots`) | data |
| `scrshots/game*.flc` recorded movies | data | `bflib_fmvids.cpp` (`FGrp_SShots`) | data |
| `native_types.lua` (`!luatypedump`) | data | `lua_base.c` | data |
| files of map scripts (`io.open`, `os.remove`) | data | `lua_base.c` | data for writing, data then game for reading |
| maps, campaigns, map packs, mods, original game files the player adds | user content | `config.c` file groups | data, then game |
| `keeperfx.cfg` | config | `config_keeperfx.c` | game, then config |
| `mods/load_order.cfg`, `campgn_order.txt`, `mappck_order.txt` | config | `config_mods.c`, `game_loop.c` | data, then game |
| `save/settings.toml` options and keys | config | `config_settings.c` | config (written beside and renamed over) |
| `renderer_prefs.ini` | config | `RendererSettings.c` | config |
| `keeperfx.log` (or `-log`) | state | `logging.c`, `main.cpp` | state |
| `keeperfx_crash_*.dmp` (Windows) | state | `bflib_crash.c` | state |
| `keeperd*.pck`, `keeperd*.spck` debug dumps | state | `packets_misc.c` | state |
| `data/tables.dat`, `alpha.col`, `colours.col`, `redpal.col`, `whitepal.col`, `mapfadeg.dat` | cache | `vidmode.c`, `engine_redraw.c` | game copy, else cache |
| speech extracted from `mapNNNNN.zip` | cache | `gui_soundmsgs.cpp` (was the shared temporary folder) | cache |
| everything else (`data`, `fxdata`, `ldata`, `sound`, `music`, `levels`, `campgns`, `creatrs`, `lang`, `keeperfx.map`) | game | file groups, `bflib_filelst.c` | data, then game (read only) |

Nothing in the simulation depends on these folders.

## Tests

`tests/userdirs` holds the unit tests of the rules and of the migration: every XDG case (set, unset, empty,
relative), no home, portable detection and its overrides, `-userdir`, the per-folder overrides, macOS and Windows
paths and their fallbacks, the game folder choice, and the migration in a temporary folder (copy, keep, once only,
failure and retry, nothing changed in the install). On Windows they also check a folder named in a script the ANSI
code page lacks. Run them with:

```bash
bash tests/userdirs/build.sh /tmp/kfx_userdirs_tests && /tmp/kfx_userdirs_tests
# or: cmake -S . -B out -DKFX_USERDIRS_TESTS=ON && cmake --build out --target kfx_userdirs_tests && ctest --test-dir out
```

The workflow `userdirs-tests.yml` runs them on Linux and, under Wine, as 32 and 64 bit Windows programs.
