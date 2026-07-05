# KeeperFX

![KeeperFX Logo](/docs/assets/readme-banner.png)

![PRs welcome](https://img.shields.io/badge/PRs-welcome-brightgreen?style=flat-square)
![Release](https://img.shields.io/github/v/release/dkfans/keeperfx?style=flat-square)
![Downloads](https://img.shields.io/github/downloads/dkfans/keeperfx/total?style=flat-square)
[![Discord](https://img.shields.io/discord/480505152806191114?style=flat-square)](https://discord.gg/hE4p7vy2Hb)

[Visit our website](https://keeperfx.net) | [Join our Discord (Keeper Klan)](https://discord.gg/hE4p7vy2Hb)


## Intro
KeeperFX (Dungeon Keeper Fan eXpansion) is an open-source project that aims to fix up, enhance and modernize 
the classic dungeon management game, [Dungeon Keeper](https://en.wikipedia.org/wiki/Dungeon_Keeper).
This project is dedicated to providing an improved and customizable gaming experience while staying true to the spirit of the original game.

KeeperFX is a standalone game but requires a copy of the original game files as proof of ownership.
These files can be automatically copied from your old CDs, or from a digital edition like the ones from EA or GOG.

Originally, KeeperFX started out as a decompilation project, where we took the original game executables and reversed them back into usable code. 
Currently the whole codebase of Dungeon Keeper is remade and all code has been rewritten.


## Features
- Windows 7/10/11 support
- Native Linux and macOS (Apple Silicon / arm64) builds
- Higher screen resolutions
- Increased FPS, decoupled gfx and game logic
- Improved and modernized controls
- Many bugfixes
- Map, campaign and modding customizability
- Improved AI
- Modern multiplayer protocol
- Additional campaigns, maps, creatures and other content
- ...


## How to play

Installation instructions and a FAQ can be found on the [Github Wiki](https://github.com/dkfans/keeperfx/wiki).

You will need the original Dungeon Keeper files, either from an old CD or from the digital edition available on
[EA](https://www.ea.com/games/dungeon-keeper/dungeon-keeper),
[GOG](https://www.gog.com/game/dungeon_keeper)
or [Steam](https://store.steampowered.com/app/1996630/Dungeon_Keeper_Gold/).


## macOS (Apple Silicon)

This fork adds a **native Apple Silicon (arm64) build** — a real Mach-O binary,
no Rosetta or emulation. It compiles, runs, and plays. See
[`docs/MACOS_ARM64_PORT.md`](docs/MACOS_ARM64_PORT.md) for the full write-up.

### Build from source

```sh
brew install pkg-config sdl2 sdl2_image sdl2_mixer sdl2_net ffmpeg luajit \
    openal-soft libspng minizip miniupnpc libnatpmp zlib curl dylibbundler
./tools/build_macos_deps.sh                 # one-time: builds astronomy/centijson/enet6
make -f macos.mk -j"$(sysctl -n hw.ncpu)"   # -> bin/keeperfx (arm64 Mach-O)
```

CI builds and verifies this on every push (`.github/workflows/build-macos.yml`),
and uploads a ready-to-run `KeeperFX.app` as a build artifact.

### Package a self-contained `KeeperFX.app`

```sh
tools/make_macos_app.sh          # -> dist/KeeperFX.app
```

This bundles the engine's libraries (via `dylibbundler`, incl. the SDL3 that
`sdl2-compat` loads at runtime) and ad-hoc signs it, so the `.app` runs on any
Apple Silicon Mac with **no Homebrew installed**.

### Running: required game files & layout

Like every KeeperFX build, the macOS build needs a game directory populated with
the KeeperFX data **plus the original Dungeon Keeper files** listed in
[`docs/files_required_from_original_dk.txt`](docs/files_required_from_original_dk.txt).
The install gate is simply the presence of `data/bluepal.dat`.

`KeeperFX.app` is a *drop-in* engine: on startup it locates itself and changes
the working directory to the folder that contains the `.app`, so it finds the
game data sitting next to it. The layout is:

```
YourKeeperFX/            <- any folder (including Desktop/Documents)
├── KeeperFX.app         <- drop the app in here, next to the data
├── data/  sound/  ldata/  fxdata/  creatrs/  campgns/  levels/  music/  ...
└── keeperfx.cfg
```

Double-click `KeeperFX.app`. On first launch:
- If Gatekeeper blocks it (ad-hoc signed, not notarized), right-click → Open once.
- If the folder is privacy-protected (Desktop, Documents, Downloads), macOS asks
  to let KeeperFX access files there — click **Allow**. It then runs from any
  location.

> Tip: if you have GOG's *Dungeon Keeper Gold* installed, the required original
> files ship **uncompressed** inside the app bundle at
> `Contents/Resources/game/{DATA,SOUND}/` (and the `keeper0*.ogg` soundtrack in
> its game root) — no CD-image extraction needed. Copy them in with lowercase names.


## Development
To get started with KeeperFX development, refer to the [Development Guide](https://github.com/dkfans/keeperfx/wiki/Building-KeeperFX) for 
detailed instructions on setting up a development environment and building KeeperFX from source.

If you wish to discuss development, you can join the [Keeper Klan discord](https://discord.gg/hE4p7vy2Hb) and ask to 
be added to the KeeperFX development channel.


## Components
| Component | Language | Info |
|---|---|---|
| [KeeperFX](https://github.com/dkfans/keeperfx) | C, C++ | - |
| [Launcher](https://github.com/dkfans/keeperfx-launcherwx) | C++ | Official Launcher to edit settings and start the game with run options. |
| [FXGraphics](https://github.com/dkfans/FXGraphics) | - | Sources of KeeperFX graphics files. |
| [FXSounds](https://github.com/dkfans/FXsounds) | - | Sources of KeeperFX audio files. |
| [Masterserver](https://github.com/dkfans/keeperfx-masterserver) | PHP (CLI) | Multiplayer masterserver. Allows players to easily find public lobbies of others. |
| [Website](https://github.com/dkfans/keeperfx-website) | PHP | https://keeperfx.net |


## Tools
| Tool | Usage |
|---|---|
| sndbanker | Makes usable ingame sounds from SFX archives. |
| po2ngdat | Converts `.po` files (language) to `.dat`. |
| png2bestpal | Decides the best in-game color palette for an image and creates a `.pal` file. |
| png2ico | Converts `.png` files to `.ico`. |
| pngpal2raw | Creates a `.raw` image file that can be used by the game from a `.png` and a `.pal` (palette) file. The palette file can be created with _png2bestpal_. |
| rnctools | Handles the RNC compression of many original DK data files. |
| dkillconv | An unfinished tool to convert a map to a text based format. |


## Further Improvements
KeeperFX could be further improved in these key areas:
- Multiplayer performance and features
- Expand and improve AI / Computer player behavior
- Improve pathfinding performance
- Expand creative freedom for modders even further
- Native cross-platform support
- Improve code readability and maintainability
- Lua support
- ...


## Contributing
We welcome contributions from the community to improve and expand KeeperFX.
- Report bugs by opening [issues](https://github.com/dkfans/keeperfx/issues).
- Submit feature requests and discuss potential improvements.
- Contribute code by creating pull requests. 


## Code Signing Policy
Free code signing provided by [SignPath.io](https://about.signpath.io/), certificate by [SignPath Foundation](https://signpath.org/).


## License
This project is licensed under the [GNU General Public License v2.0](LICENSE).
Feel free to use, modify, and distribute it according to the terms of this license.
