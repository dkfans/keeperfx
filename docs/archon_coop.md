# Archon co-op mode

## What it is

Archon mode lets two or more networked humans control ONE keeper. Everyone
shares the same dungeon, gold, rooms, creatures, research and spells. Each human
has their own camera, cursor, hand cursor, spell targeting and possession; the
contents of the hand are the keeper's and shared.

Internally the game separates a human (`NetUserId`, state in
`game.user_states[user]`) from a keeper faction (`PlayerNumber`, `PlayerInfo`).
In archon mode every active user is mapped to the same player number: the host's
slot in a normal multiplayer map, or the campaign's human player (default
PLAYER0, the red keeper) for a campaign level.

The game is lockstep: all clients run the same simulation from the same packets.
Per-user state (cursor, hand cursor, work state, view type, possession, roomspace,
dungeon camera) lives in `UserState` and is checksummed for desync detection
(`src/net_checksums.c`).

## How to start

Archon is decided by the host only; the flag is sent to joiners in the startup
sync packet, so joiners do not need it (passing it is harmless).

Default network port: 5556 (UDP, ENet). Override on the host with
`-server <port>` or `MULTIPLAYER_PORT` in `keeperfx.cfg`. LAN discovery uses UDP
5557 (broadcast). Internet play needs the host to forward UDP 5556 (or use the
built-in NAT traversal when starting through the online lobby).

Command line flags (all existing flags are in `src/main.cpp`):

| Flag | Meaning |
|------|---------|
| `-archon` | Host: request archon mode. |
| `-server [port]` | Host a session directly (optional port). |
| `-connect <ip[:port]>` | Join a session directly (also fills the session list). |
| `-sessions <ip[:port]>` | Add a manual session entry to the join list. |
| `-steamhost` | Host through a Steam lobby instead of a UDP port (see "Playing over Steam"). Combine with `-archon -campaign ... -waitusers 2` like `-server`. |
| `-steamjoin [lobby id]` | Join a Steam lobby. Without an id the game waits at the main menu for an accepted Steam invite. |
| `+connect_lobby <id>` | What Steam passes when it starts the game from an invite or "Join Game"; same as `-steamjoin <id>`. |
| `-nick <name>` | Player name. |
| `-waitusers <n>` | Host auto-starts once n users are connected (needs `-level`/`-campaign`). |
| `-level <n>` | Auto-start level number (multiplayer map or campaign level). |
| `-campaign <name>` | Campaign to use (e.g. `keeporig`); with `-archon` this is an archon co-op campaign: without `-level` everyone goes to the shared campaign map, with `-level` that level starts directly and the session continues on the shared map afterwards. |
| `-spectate` | Join as spectator (watch with the host's synced state). |
| `-packetsave <file>` / `-packetload <file>` | Record / replay the packet stream. |
| `-mplog` | Detailed multiplayer logging. |
| `-netstats` | Show network statistics. |
| `-nointro` | Skip intro videos (handy for testing). |

### Host (menu steps)

Co-op campaign:
1. Main menu -> Multiplayer -> LAN (or Online) -> Create session.
2. In the session settings set "Campaigns" to the campaign to play together
   (click cycles Off -> each installed campaign; right click goes back). This
   turns archon mode on.
3. Wait for the joiner(s) in the lobby, then Start Game: everyone goes to the
   shared campaign map (see below).

Archon on a multiplayer map: start with `-archon`, e.g.
`keeperfx.exe -nointro -archon -nick Alice`, create a session as above (leave
"Campaigns" Off), choose the map and start.

### Joiner

1. Start normally: `keeperfx.exe -nointro -nick Bob`.
2. Main menu -> Multiplayer -> LAN (session list fills by broadcast) or
   Online -> select the host's session -> Join.
3. Or skip the menus: `keeperfx.exe -nointro -connect 192.168.1.10:5556`.

### One-shot test launch (two machines or two instances)

```
host:   keeperfx.exe -nointro -archon -nick Alice -campaign keeporig -waitusers 2 -server
joiner: keeperfx.exe -nointro -nick Bob -connect <host-ip>:5556
```

Add `-level <n>` on the host to skip the map and start level n directly.

## Shared campaign flow (between levels)

An archon co-op campaign session stays together from level to level; nobody
relaunches or goes back to the main menu.

1. Shared campaign map: the host opens it (autostart, or the lobby's Start Game).
   Every user sees the campaign's land view as the host's progress shows it:
   the land picture and the ensigns of the levels the host may play (the
   continue level, revealed bonus levels, moon levels). Every user's hand is
   shown with their name, as on the multiplayer map.
2. The host picks: a click on an ensign selects the level and shows its briefing
   to every user (the level narration and the level text box: name, plus the
   description if the campaign has one). A second click on the same ensign
   starts the level as soon as every user has the briefing. Right click clears
   the selection. Joiners cannot pick; their selection follows the host's.
3. The level: all users share the keeper. Script events (objectives, speech,
   zoom-to-event, the heart zoom at the start) reach every user.
4. Level end: when the level is won or lost, any user's Finish key (the
   continue after victory/defeat) ends the level for everyone. The host's
   progress is saved (only the host's). After the campaign's last level every
   user sees the outro movie.
5. Statistics: every user sees the level statistics. Rule: the host continues.
   Its Ok button is enabled once every user has reached the statistics screen;
   joiners' Ok is disabled. Everyone then goes back to the shared map: after a
   victory with the next level, after a defeat with the same level to retry.

Escape on the shared map takes every user back to the lobby (the session stays
open; Start Game reopens the map). If a user leaves on the map the others return
to the lobby, as on the multiplayer map; if the host leaves during the
statistics, joiners return to the main menu.

The flow is frontend-only (not lockstep): the host opens the map with a chat
message `<campaign>:@<land view level>,<level>,...`; users report their screen
(on the map / on the statistics screen) in the frontend screen packets.

Both instances need identical game files (same mods, same `fxdata` sprite zips).
The startup checks refuse mismatching maps and required sprite zips.

## Playing over Steam

Instead of a UDP port the session can run through Steam: the host makes a Steam
lobby, the friend joins it, and Steam relays the packets, so there are no IP
addresses, port forwarding or NAT problems. Both players need Steam running and
logged in. It uses Valve's free test app Spacewar (AppID 480), so Steam shows
"Spacewar" as the game being played; that is expected.

Files next to `keeperfx.exe`: `steam_api.dll` (32 bit) and `steam_appid.txt`
containing `480` (the game creates the txt itself if it is missing). The tested
redistributable is `Windows-x86/steam_api.dll` from the Steamworks.NET
"Standalone" release 2025.164.1 (github.com/rlabrecque/Steamworks.NET/releases),
SHA-256 `fdafb7c65ec7d4182b42bf92cc8671c68e4dc547cc6e613899d2618a47eb65da`; it exports
`SteamAPI_InitFlat`, `SteamAPI_SteamNetworkingMessages_SteamAPI_v002`,
`SteamAPI_SteamMatchmaking_v009` and the other entry points listed in `net_steam.cpp`.

```
host:   keeperfx.exe -nointro -archon -campaign keeporig -waitusers 2 -steamhost
joiner: keeperfx.exe -nointro -steamjoin 109775240917155841     (the lobby id the host sees)
joiner: keeperfx.exe -nointro -steamjoin                         (wait for a Steam invite)
```

Host: starting with `-steamhost` creates a friends-only lobby and opens the Steam
overlay invite dialog. The lobby id is shown in the lobby screen's message
list, written to `keeperfx.log` ("Steam: lobby created, id ...") and copied to
the clipboard. The game also publishes the rich presence key `connect`
(`+connect_lobby <id>`), so the host shows a "Join Game" entry in the friends list.

Joiner: start the game with `-steamjoin` (no id) and accept the invite (overlay
popup, or right-click the host -> Join Game); the game then joins at once. If the
game is not running when "Join Game" is clicked, Steam would start Spacewar
itself, so start KeeperFX first. As a fallback give the lobby id on the command line.

Implementation (`src/net_steam.cpp`, a `struct NetSP` next to the ENet one):

- The Steamworks SDK is not needed to build. The flat C API entry points that
  are used are looked up in `steam_api.dll` with `GetProcAddress`; callbacks are
  polled with the manual dispatch API (`SteamAPI_ManualDispatch_*`).
- A `NetUserId` maps to a Steam ID: the host is user 0, joiners get ids from
  `OnNewUser` when they first contact the host (the
  `SteamNetworkingMessagesSessionRequest_t` callback, or their first message).
- Traffic is `ISteamNetworkingMessages`. Channel 0 carries game messages: reliable
  and ordered for `sendmsg_single/all`, unreliable for `sendmsg_single_unsequenced`.
  Messages over 60000 bytes (resync snapshots) are split and reassembled.
  Channel 1 carries control messages: kick with a join-rejection reason, goodbye
  when leaving, and a 1 second keep-alive.
- `ISteamNetworkingMessages` gives no notice when a peer closes its session, so
  leaving sends a goodbye (drop at once), a vanished peer is dropped after 20 s of
  silence (`NETDROP_ERROR`, like an ENet timeout), and a lobby member leaving also
  drops the peer. Silence is not counted while the local game was not looking at
  the network (level loading).
- The lobby id is shown in the lobby screen (host) and spectators are not
  supported over Steam.
- Struct layouts follow the Windows 32 bit SDK (callback structs packed to 8,
  `SteamNetworkingMessage_t` identity at offset 16).
- Failure handling: a missing `steam_api.dll`, Steam not running or not logged in
  shows an error box with the reason and returns to the menu; the ENet code paths
  are untouched.

### Testing without Steam

Two Steam clients cannot run as two accounts on one PC, so
`kfx-tools/mock-steam/mock_steam_api.c` builds a mock 32 bit `steam_api.dll`
(`kfx-tools/mock-steam/build.sh`) that exports the same flat functions. It derives
the Steam ID from `-nick`, moves messages between the processes over localhost
UDP, fakes lobbies (lobby id = `0x0186000000000000` + the owner's number, so the
lobby of the nick `Host` is 109775240917155841) and, like Steam, holds the first
message of an unknown user until the session is accepted and sends no notice when a
peer disappears. `kfx-tools/run-pair-steam.ps1` and `run-campaign-steam.ps1` run
the bot tests through it (`-killclient N` kills the client process after N seconds).
What only a real two-account Steam test can show is listed in the hand-over notes
of this feature: overlay invites, "Join Game" from the friends list, relay
behaviour and the real callback timing.

## What is per user and what is shared

Per user (separate for each human of the keeper):
- Dungeon camera position/zoom/rotation, first-person camera, parchment map view
- View type (top-down, possession, passenger, map screen)
- Cursor and hand cursor state (`hand_idx`, `hand_thing_idx`, `thing_under_hand`)
- Work state (dig, build room, cast spell, sell, ...), room-space (build rectangle) mode and size
- Possession (`controlled_thing_idx`), query/influenced thing, instance in progress
- Preferences (wibble, front view, highlight mode, ...), chat input
- A creature can be possessed by only one user at a time.

Shared (the keeper's):
- Gold, mana, research, rooms, doors, traps, creatures, imp dig tasks, scripts
- Hand contents: creatures and gold held in the hand (`dungeon->things_in_hand`) are one shared pile for the keeper;
  any user of the keeper can drop what another user picked up
- Victory/defeat state and campaign progress
- Alliances, creature stats, the dungeon heart

Scripts and Lua: `VIEW_TYPE` and `CONTROLLED_THING` report the possessing user if
any user of the keeper is possessing, otherwise the first user.

## Partner hand

Each machine also draws the partner's hand: the real hand sprite in its current pose
(open, grabbing, holding, slapping), semi-transparent, at the map position the partner's
cursor points at, with the partner's nickname under it. The partner's position is marked
on the minimap as a blinking cross. Nothing is drawn while the partner is possessing a creature,
is on the map screen or the cursor is over the partner's panels (no valid map position in the packet),
or when the position is off screen on this machine. Spectators and the observer camera do not draw it.

It is rendering only, so it cannot desync. It reads state that already reaches both machines
through lockstep: the partner's last packet (`pos_x`/`pos_y`, `PCtr_MapCoordsValid`, `PCtr_Gui`,
camera context) and the partner's `UserState` (`hand_thing_idx` and with it the hand animation,
`thing_under_hand`, `work_state`, `instance_num`, `hand_busy_until_turn`, `view_type`,
`controlled_thing_idx`). What it holds is the keeper's shared `things_in_hand`. No new synced field
is needed and no game state is written (`draw_partner_hands()` in `src/power_hand.c`; the pose
drawing is the same code as for the own hand, `draw_user_hand()`).
The projection to the screen uses this machine's own camera, so each player sees the partner's hand
over the spot it points at, wherever the partner's camera is.

Turn it off with `show_partner_hand = false` in the `[display]` section of the settings file,
or start with `-nopartnerhand`.

## Known limitations

- A script/Lua API addressed by player number acts on the keeper, not a specific
  user; where several users drive it the first (lowest id) user is used unless
  stated above.
- Shared campaign flow: the torture screen after capturing the Lord of the Land
  is skipped; the campaign intro movie is not played (the map opens directly);
  movies are played by each machine on its own (a very long movie could let the
  network time out). Bonus ensigns follow the host's progress but no "reveal"
  animation is shown. The joiner's map hand moves where its own mouse is; both
  hands sit on the selected ensign.
- A few script and map location paths use `my_player_number`; these are safe
  only because all archon users share the host's player slot.
- Rival keepers in campaign levels depend on the host's campaign config
  (`assignCpuKeepers` is synced). Joiners should have the same campaign installed.
- Save games: archon sessions do not support the regular multiplayer save/load
  path; saves are not compatible with older versions.
- Spectators receive the same per-user state but have no packets of their own.
- Cannot be tested without game data in CI; behaviour below is from code
  reading and needs the manual test plan.
- Local-only lookups still using the legacy `get_player_user_state()` (lowest
  user of the keeper) may remain in rarely used GUI code; these only affect
  presentation, never the simulation.

## Manual test plan (two players)

Run each section first over LAN (two machines or two instances on one machine
with different `-nick`, ports via `-server <port>` / `-connect ip:port`), then
over the internet (host forwards UDP 5556 or uses the online lobby). Always
launch both with `-mplog`. Pass `-packetsave <file>` on both for every session
so a desync can be replayed.

Setup check (all runs):
1. Both players spawn at the same heart; each camera starts centred on the heart.
2. Both can dig, build, cast, slap, pick up with their own cursor at the same time.
3. Player names show combined in the player list ("Alice & Bob").

### 1. Possession conflicts
- A possesses an imp. B tries to possess the same imp: must be refused.
- A possesses creature X; B possesses creature Y at the same time; both play
  (move, attack, cast, imp dig/pretty/convert). Imp tasks while possessed
  (drag/drop with the imp action) must work for each user independently.
- A leaves possession; B then possesses X.
- A is possessing X while B casts a spell on X, slaps it, or picks it up (hand):
  behaviour must be identical on both screens, no desync.
- Kill A's possessed creature: A returns to top-down view, B unaffected.
- Script `VIEW_TYPE`/`CONTROLLED_THING` correctness (use a test map that prints them).

### 2. Simultaneous room building
- A and B each drag out a different room at the same time (different shapes).
- A and B drag the same area simultaneously: no desync, gold deducted once per
  slab, exactly one room results.
- Both dig the same wall block; both sell the same room slab at once.
- Different room-space sizes per user (cursor box size / L-shape) must not leak
  between users.

### 3. Hand and spells
- A picks up a creature; B tries to pick up the same one: only one succeeds.
- Both hold different creatures and drop them; drop onto the same slab.
- A casts a spell (e.g. speed, lightning) while B holds a creature: mana drawn
  once, effects identical on both machines.
- Slap, sell items, pick up gold, pick-up-all gold toggle per user.
- Each user's own hand is drawn at the mouse; the partner's hand is drawn too, see "Partner hand" below.

### 4. Chat
- Both players send chat: text shows once, with the right sender name.
- Chat while the other possesses a creature and while on the map screen.

### 5. Leaving mid-game
- Joiner quits: host continues, keeper stays alive, the leaver's hand cursor is
  removed (held things stay in the keeper's shared hand), possession released,
  no crash and no desync on the host.
- Host quits: joiner gets the usual disconnect handling.
- Joiner force-closes (kill process) and reconnect attempt (if supported).
- A spectator (`-spectate`) joins and leaves while the two players play.

### 6. Campaign victory progress
- Start an archon co-op campaign (session option "Campaigns", or
  `-archon -campaign keeporig`): both players are on the shared map.
- The joiner cannot pick; the host picks: both see the briefing; the host starts.
- Win the level: both players see the victory, then the statistics; the host's
  campaign progress is saved and advanced; the joiner's own progress is not
  touched. The host's Ok takes both back to the map with the next level.
- Lose the level (heart destroyed): both see defeat and the statistics; progress
  not advanced; the map offers the same level again.
- Start the next level and verify carried-over creatures and campaign flags are
  taken from the host.
- Level with rival keepers (`assignCpuKeepers` campaigns): rivals exist on both
  machines.

### 7. Desync checks
- Run each scenario for 10+ minutes of real play at game speed and at fast
  forward; keep the game running through an alt-tab.
- On any desync dialog, collect the logs below before closing.

## Automated test (`-archontest`)

`-archontest <seconds>` replaces each client's mouse and keyboard input with a
scripted bot (`src/archon_test.c`). The bot only reads game state and fills the
local packet the way human input does (packet actions, cursor position and
context, mouse button flags; camera scrolling through the local camera input),
so everything it does goes through lockstep. Host = user 0, joiner = user 1.

Scenario (by game turn, 20 turns per second):
1. Turn 60: check both users drive the same keeper; user 0 grants Possess and
   all rooms through the regular cheat packets (needs `-alex` on both clients).
2. Both users tag 4 diggable slabs next to the heart at the same time, user 0 to
   the east, user 1 to the west.
3. Both build a room at the same time: user 0 a Treasure Room, user 1 a Lair.
4. Turn 400 / 560: user 0, then user 1 picks up an imp and drops it on own floor
   (the hand contents are shared, so they take turns); user 1 slaps an imp.
5. Turn 800: user 0 possesses an imp; user 1 tries to possess the same imp (must
   be refused), then possesses another one; both possess at once; user 0
   releases, user 1 releases and then possesses the imp user 0 released.
6. Near the end each user scrolls its own camera (user 0 right, user 1 down).

The two users rest their cursors 3 slabs left (user 0) and right (user 1) of the heart. To see
the partner hand, each bot brings the other user's cursor on screen with its own local camera
and takes a screenshot at turn 320 (idle), while the other user holds an imp, and while the other
user slaps ("partner hand idle/holding/slapping" in the log, `scrshots/`).

Each client writes `archontest_<user>.log` in its working folder: `STATE` lines
every 100 turns, `CKSUM` lines every 20 turns, `CHECK <name> PASS|FAIL` lines,
`ARCHONTEST DESYNC turn N` on any checksum mismatch or resync, then
`ARCHONTEST DONE result=PASS` (or `result=FAIL failed=<checks>`), after which the
joiner and then the host quit through the normal quit packet. It also takes
in-game screenshots (`scrshots/`) at key moments.

```
host:   keeperfx.exe -nointro -skipheartzoom -mplog -archon -nick Host -campaign keeporig -level 1 -waitusers 2 -server -archontest 120 -alex
joiner: keeperfx.exe -nointro -skipheartzoom -mplog -nick Client -connect 127.0.0.1:5556 -archontest 120 -alex
python tools/archontest/compare_logs.py <host folder> <joiner folder>
```

`compare_logs.py` checks both results and that the per-turn checksums of the
two clients match.

### Campaign progression (`-archontestcampaign`)

With `-archontestcampaign` (on both clients) the bot, after its checks, wins the
level through the win-level cheat packet (`CHECK victory_both`), then the host
sends the finish-level packet, which ends the level for everyone. After the
level, the host checks that its campaign progress file (`save/<campaign>.cid`)
now continues at the next level (`host_progress_updated`) and the joiner checks
that its own progress file did not change (`client_progress_untouched`); then
both exit. Start the host without `-level` so the launch continues from the
host's saved progress (on the shared campaign map the host bot picks and starts
the level its progress continues at); one launch per level:

```
host:   keeperfx.exe -nointro -skipheartzoom -mplog -archon -nick Host -campaign keeporig -waitusers 2 -server -archontest 70 -alex -archontestcampaign
joiner: keeperfx.exe -nointro -skipheartzoom -mplog -nick Client -connect 127.0.0.1:5556 -archontest 70 -alex -archontestcampaign
```


### Shared campaign flow test (`-archonflow <levels>`)

With `-archonflow <levels>` added to `-archontest <seconds>` (both machines, no
`-level`), the bot plays the shared campaign flow in one launch instead: shared
map -> host picks the continue level (the joiner tries to pick first and must
be refused) -> briefing on both -> level -> win through the cheat packet (the
first attempt at the second level is lost on purpose) -> Finish -> statistics
on both (the joiner's continue must not leave) -> host continues -> back on the
shared map, until `<levels>` levels are won. `<seconds>` is then a wall-clock
limit. Checks per round: `briefing_both`, `host_pick_seen_by_client`,
`level_loaded_same_player`, `heart_zoom_both`, `objective_both`,
`victory_both`/`defeat_both`, `no_desync`, `stats_both`, `back_to_map_both`,
`host_progress_advanced`/`host_progress_kept` (host), `client_progress_untouched`
(joiner, its own progress file), `retry_after_defeat`; once: `map_shared`,
`client_cannot_pick`, `client_cannot_continue`, `host_continues`. Screenshots
are taken of the shared map, the briefing, the objective, victory/defeat and
the statistics.

```
host:   keeperfx.exe -nointro -mplog -archon -nick Host -campaign keeporig -waitusers 2 -server 5566 -archontest 300 -archonflow 3 -alex
joiner: keeperfx.exe -nointro -mplog -nick Client -connect 127.0.0.1:5566 -archontest 300 -archonflow 3 -alex
```

Delete the host's `save/keeporig.cid` first for a run from level 1. Started as a
plain session instead (host without `-archon`/`-campaign`), the host bot first
sets the co-op campaign through the session settings' "Campaigns" option and
presses Start Game, which tests the menu path.

## Capturing desync logs and replays

1. Launch both sides with `-mplog -packetsave <name>.pck` (use different file
   names per machine).
2. `keeperfx.log` in the game folder is written by each client. A desync prints
   `=== DESYNC` lines with the first turn where checksums diverged and a per-
   category comparison (Creatures, Things, Rooms, Players, Users...). The Users
   section lists, per user, the camera pos, instance, controlled thing,
   view_type, work_state, continue_work_state, hand_thing_idx and
   thing_under_hand for host vs client.
3. Copy `keeperfx.log` and the `.pck` file from BOTH machines, plus the host's
   `fxdata` and mod list, and note the build, the map/campaign, the exact action
   (e.g. "A possessed an imp and picked up gold at turn ~1200").
4. Replay: `keeperfx.exe -packetload <name>.pck` reproduces the session offline
   with frame stepping; if the replay desyncs vs the recorded checksums,
   the bug is in the simulation. If it replays cleanly, suspect a
   local-state-dependent write (anything keyed by `get_local_user()` /
   `my_player_number` inside packet processing).
