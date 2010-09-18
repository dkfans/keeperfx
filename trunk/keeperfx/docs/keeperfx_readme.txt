Dungeon Keeper Fan Expansion
------------------------------

KeeperFX is an extensive mod for Dungeon Keeper.

It is written by fans and not supported by original developer.
It contains some of original Dungeon Keeper data, but many files
are also modified or remade.

Installation of KeeperFX CCP:

KeeperFX CCP contains all the files necessary to play the game.
Decompress it anywhere, and it's ready to run. If you wish
to change language, edit "keeperfx.cfg" and set its shortcut
in the 'LANGUAGE=' line. Run "keeperfx.exe" to start the game.
If something doesn't work, see the 'troubleshooting' section.

Please note that there are two types of releases: complete
version and patch. You need a complete version to play the game;
a patch is only an additional feature, which you can use by
overwriting some files from complete version with new ones.

Available languages:

The following languages are currently functional:
 ENG ITA FRE SPA DUT GER POL SWE
The following languages are partially functional:
 RUS CHI CHT JAP
Note that some campaigns may not support your language.
In this case, default language will be used inside this
campaign.

Running KeeperFX:

To start the game, run "keeperfx.exe". If you want to report
any errors you encounter, you may run "keeperfx_dbg.exe"
instead. This will run a "debug version", which writes a lot of
information into "keeperfx.log". In case of the game hanging
on suddenly disappearing, you may send a last few lines of the
generated LOG to the author with your description of the bug.

Note that "keeperfx_dbg.exe" requires a lot more CPU than
standard version, and may be slow even on new computers.
Also, the generated LOG file may be very large, and after
a few hours of play it will have several hundreds megabytes.
This is why you should use standard "keeperfx.exe" if you're
not planning reporting any errors.

Both versions will recognize all command line options described
below.

Command line options:

-nointro
  The intro sequence won't play at startup.
-nocd
  The CD Sound tracks won't play.
-1player
  Allows playing multiplayer maps in skirmish mode.
  This is normally ON, so the option has no effect.
-nosound or -s
  Disables all the sounds.
-fps <num>
  Changes the game speed; default <num> is 20.
-usersfont
  Disable the AWE32/64 SoundFonts (.SBK files).
-alex
  Used to show the 'JLW' easter egg. And not only that.
-level <num>
  Brings you directly to level number <num>.
  After the level is finished, quits the game.
  Note that level number must be 1..65534.
-human <num>
  Changes human player to <num>. This option will
  work properly only in skirmish mode. Single player
  levels must be specially designed for this option
  to work. Also, the selected player must have
  heart on the map.
-q
  Works like '-level 1'.
-columnconvert
  I assume it converts the columns data. Watch out with
  this one or you may be unable to play the game again.
-lightconvert
  Same thing, but for lights.
-vidsmooth
  Smoothes the 3D view using 1-pixel bilinear blur.
-altinput
  Uses alternate input method. This changes the way of
  using mouse, keyboard and video driver. May be helpful
  if original method isn't working right (ie. mouse stops).
-packetsave <filename>
  Writes a packet file (replay file) when playing.
  After using this option, you must start a new level
  (or use '-level' parameter). Saved replay will work
  properly as long as you won't change any of the game
  files. Even a minor change in map or configuration
  may make the replay invalid.
-packetload <filename>
  Loads a previously created packet file. Starts the
  level for which packet file was created, and continues
  the gameplay. You may exit this mode by pressing
  Alt+X, or take over the control by pressing ALT+T.
  Note that this option is experimental, and packet files
  may sometimes not work as intended.

Troubleshooting:

Q: Colors are changed in menu or during gameplay,
    but no problem is mentioned in "keeperfx.log".
A: Select "Run in 256 colors" and "Run in 640x480" in the
    program properties, and change lines in KEEPERFX.CFG
    into those written in next answer.

Q: Intro doesn't play. LOG file says:
     Error: setup_game: Can't enter movies screen mode to play intro
A: The problem is that your drivers can't support 320x200 mode.
   Change the resolution config lines in KEEPERFX.CFG
    into those written in next answer.

Q: The game is pixelated/works in low resolution mode only.
   Can I make it work in higher resolutions?
A: To switch resolutions during the game, press Alt+R.
   If the screen blanks, but resolution doesn't change,
   then the video mode used for higher resolution is probably
   not supported by your video card/driver. In that case,
   change the resolution config line in KEEPERFX.CFG into:
  INGAME_RES=320x200x8 640x480x8
   You may also try other resolutions, but those over 640x480
   may be unstable.

Q: Game stops when loading a map. LOG file says:
     Error: setup_screen_mode: Unable to setup screen resolution
            640x400x8 (mode 10) 
A: The problem is that your drivers can't support 640x400 mode.
   Change the resolution config lines in KEEPERFX.CFG into:
  FRONTEND_RES=640x480x8 640x480x8 640x480x8
  INGAME_RES=640x480x8

Q: The game doesn't run. LOG file says:
     Error: setup_strings_data: Strings file couldn't be loaded
            or is too small
A: Check if there's a language file in 'FXDATA' folder for the
     language which you've selected in KEEPERFX.CFG.

Q: Mouse stops/teleports/moves incorrectly during the game.
A: Try the '-altinput' command line switch.

Q: I get a message 'Cannot initialize' when I try to enter network game.
A: There's no fix; use stanard DK with IPX fix for multiplayer.

Q: There are no special eye effects when I posses Beetle, Fly,
    Dragon, Tentacle etc.!
A: Lense effect only work if the game detects over 16MB RAM.
   Also, make sure that "fxdata/lenses.cfg" is present and not damaged.

Q: I've found a cheat menu, but it doesn't work!
A: The three cheat menus are only partially functional.

Config file details:

FRONTEND_RES
  Allows you to select front-end resolution (used inside
   menu and for playing movies, but not in actual game)
  FRONTEND_RES=<failsafe mode> <movies mode> <menu mode>

INGAME_RES
  Allows you to select up to five in-game resolutions.
  Resolution has the form of WIDTHxHEIGHTxBPP.
  Standard modes are: 320x200x8, 320x240x8, 512x384x8,
   640x400x8, 640x480x8, 800x600x8, 1024x768x8,
   1280x1024x8, 1600x1200x8. Different modes (ie.
   widescreen) may be used too, if only they are
   supported by your graphics card and video driver.
  You can switch between those resolutions during the
   gameplay by pressing Alt+R. Modes over 640x480 are
   experimental, and not completely stable.
  INGAME_RES=<mode1> <mode2> <mode3> ....

SCREENSHOT
  Selects the format in which screenshots will be written.
    You can choose between BMP and HSI bitmap format.
  SCREENSHOT=<type>

LANGUAGE
  This option is used to select language file, used for
  displaying texts. It also changes language in mentor
  speeches. Note that if the specific campaign doesn't
  have support for your language, the default language
  will be used.
  
New in-game commands:

 Record a movie
  To record a FLC movie, press Shift+M during the game.
  A text "REC" will appear to inform you that recording
  is on (the text will not be visible in recorded movie).
  Note that only video is recorded, no sound. The movie
  will be placed in 'SCRSHOTS' folder; you may play it
  with "mplayer" or its clones. Note that the generated
  file may be large.

 Make a screenshot
  Use the Shift+C keys to make screenshot. The image may
  be written in 'mhwanh' HSI/RAW format, or Windows BMP
  format. Format is chosen in KEEPERFX.CFG, 'SCREENSHOT='
  option, which may be set to 'HSI' or 'BMP'.

 Make and replay packet file
  These functions can be only enabled by command line
  parameters. You can make a packet file which contains
  the replay with '-packetsave' command, and then play it
  with '-packetload'. When in the replay, you may always
  take over control by pressing Alt+T, or exit with Alt+X.

 Release speed mode
  This mode is also available in original DK, but here it's
  a bit enhanced. Normally, the engine limits amount of
  game turns per second. This function allows to stop
  controlling speed - the game will work at its maximum speed,
  which highly depends on your computer hardware.
  To enter released speed mode, press Ctrl+'+' from numpad
  section of your keyboard. Pressing it more than once will
  increase frameskip - some of the frames will not be drawn,
  which will result in even greater increase in speed.
  To cancel the effect, press Ctrl+'-' as many times as you
  pressed Ctrl+'+' before.

New and modified level script commands:

 ADD_GOLD_TO_PLAYER
  Allows to add some off-map gold as a reward to a player.
  Example: ADD_GOLD_TO_PLAYER(PLAYER0,5000)
 BONUS_LEVEL_TIME
  Sets time to be displayed on "bonus timer" - on-screen
  time field, used mostly for bonus levels.
  Like in original DK, this command accepts one parameter
  - number of game turns to start the countdown from.
  But now this command can be used to show bonus timer in
  any level. Setting game turns to 0 will hide the timer.
  Example: BONUS_LEVEL_TIME(12000)
 DISPLAY_OBJECTIVE
  The 2nd parameter can now have the following values:
  - 'PLAYERx' - zoom to player's dungeon heart
  - positive integer - zoom to Action Point of given number
  - negative integer - zoom to Hero Gate of given number
  - 'ALL_PLAYERS' - zoom button will be inactive
 LEVEL_VERSION
  Lets the game know if the level was designed specially for
  KeeperFX. To use new script commands, you must start the
  script with LEVEL_VERSION(1). Without it, the new commands
  will not work properly.
 PLAY_MESSAGE
  Allows to play any SOUND or SPEECH from the game.
  Example: PLAY_MESSAGE(PLAYER0,SPEECH,107)
 QUICK_INFORMATION
  These works same as in Deeper Dungeons, but allows message
  length up to 1024 characters. There are 50 quick message
  slots.
 QUICK_OBJECTIVE
  Same as in DD, but allows longer messages and more control
  over zoom button (like in DISPLAY_OBJECTIVE).
 QUICK_INFORMATION_WITH_POS
 QUICK_OBJECTIVE_WITH_POS
  Accepts additional XY coordinates of the zoom place.
 SET_CREATURE_TENDENCIES
  Allows to set tendencies: IMPRISON and FLEE, for a player's
  creatures. Example: SET_CREATURE_TENDENCIES(PLAYER2,FLEE,1)
  Note that a player must have prison when IMPRISON command
  is trigered; otherwise it won't make any change.
 REVEAL_MAP_RECT
  Reveals rectangular map area for given player. Requires
  coordinates of area center point, and rectangle dimensions.
  Numbers are scaled in subtiles (range is 1..254).
  Example: REVEAL_MAP_RECT(PLAYER0,132,96,13,11)
 REVEAL_MAP_LOCATION
  Reveals square area of subtiles around given location.
  Location meaning is identical to the one in DISPLAY_OBJECTIVE.
  For example, to reveal Hero Gate no.1:
  REVEAL_MAP_LOCATION(PLAYER0,-1,11)
 RESEARCH
  Changes amount of research points needed to discover an item
  in library. It doesn't affect research order, only amount
  of points. If the item never was in research list, it's added
  at end. Example: RESEARCH(PLAYER1,MAGIC,POWER_CHICKEN,10000)
 RESEARCH_ORDER
  When this command is first called, the research list for
  specified players is cleared. Using it you may create
  a research list from beginning. Note that if you won't place
  an item on the list, it will not be possible to research it.
  So if you're using this command, you must add all items
  available on the level to the research list. Example:
   RESEARCH_ORDER(ALL_PLAYERS,ROOM,SCAVENGER,50000)
   [...] - more RESEARCH_ORDER commands should follow.
 RANDOM
  It's not a command, but may be used instead of most parameters.
  If used instead of a number, then should look like:
   RANDOM(min,max)
  but may also be used instead of any other value. Examples:
    MAX_CREATURES(PLAYER0,RANDOM(12,19))
    ADD_CREATURE_TO_POOL(RANDOM,20)
  Note that when used instead of player name, RANDOM may return
  ALL_PLAYERS. Also, the command shouldn't be used in multiplayer
  maps, as it will lead to synchronization problems.
  Value represented by RANDOM is selected at start of a map,
  and never changes during the gameplay.

Changelog:

Version: 0.38
  Fixed duplicates level when using Multiply special
  Created some 64-bit math, similarly to original code
  Rewritten more of pathfinding

Version: 0.37c
  Fixed dungeon heart blinking if under mouse
  It is now easier to target a creature for pick up
  Fixed disappearing in-game speeches
  Computer player config is reloaded on saved game loading
  Fixed possible hang when computer player moves creatures
  Added new creature property, "NEVER_CHICKENS"
  Rewritten more of pathfinding
  Rewritten creature training code
  Renamed and rescaled "PartnerTraining" (was "RealTraining")
  Fixed linked list storing creatures who work in a room
  Updated room selling code

Version: 0.37b
  Fixed crash on freeing swipe sprites at end of mission
  Fixed SEEK_THE_ENEMY job (Hellhound)
  Fixed crash when zooming in isometric (non rotable) view
  Fixed Imps aimless walking around bug
  Fixed invalid celebration sprite bug
  Video modes in config file are no longer pre-defined 

Version: 0.37a
  Fixed gems appearance bug
  Fixed selling bug
  Fixed per-campaign creatures config bug

Version: 0.37
  Rewritten computer tasks list
  Added palette stealing protection to video driver
  Rewritten some of creature fighting code
  Rewritten enemy seeking code for heroes
  Fixed green volume box height in clueo (low walls) mode
  Rewritten gold stealing code
  Remade some creature spells code
  Remade some of imps AI code
  Remade and fixed some of Ariadne pathfinding system
  Campaign list is now sorted
  Imp tasks selection rewritten
  Creature sprite indexes are now in .CFG files
  Campaigns can now have their own creature config files

Version: 0.36
  Rewritten some of room sound playing
  Modified saved games format (old saves won't work anymore)
  Saved game can now be loaded even if loading campaign file fails
  Improved handling of player index errors
  Improved BONUS_LEVEL_TIME() script command
  Remade part of creature states system

Version: 0.35
  Rewritten shot throwing code (for both melee combat and spells)
  Rewritten some of Dungeon Heart behaviour
  Programming IDE switched to Eclipse for C++
  New Makefile and project structure - easier to recompile
  Rewritten and fixed Word of Power damage code
  Rewritten missing function for new way of storing bonus levels
  Rewritten more of eye lenses code, updated lenses config file
  Eye lenses now supported in all resolutions
  Rewritten part of scavenging code
  Rewritten part of the polygon rendering code

Version: 0.34
  Some fixups in CPU Identification code
  Fixed Imps sacrifice code
  Text drawing functions remade
  Fixed CTRL+Arrows keys support
  Fixed transfer creature bug
  New way of storing bonus levels availability
  Fixed spell cursors when playing player > 0
  Fixed sacrifices when playing player > 0
  Asian languages support reached alpha stage
  Shift+M key now works in high scores screen
  Fixed incorrect memory write on creature suicide

Version: 0.33
  Rewritten creature killing code
  Improved creature config files
  Tooltip drawing code improved
  Remade another part of network support
  Improved logging system
  Creature states are now defined outside DLL
  Fixed checking for gold bug (gold digging by computer)
  Changed Video and Mouse driver - EXPERIMENTAL
  Added '-altinput' command line parameter
  Rewritten digging and claiming effects
  Rewritten part of imp tasks management
  Rewritten and extended creature sacrifice recipes
  Censorship no longer bounded to german language

Version: 0.32
  Added CPU identification
  Rewritten part of spell casting code
  Rewritten some of 'Transfer creature' code
  Rewritten some possession-related code
  RANDOM can now be used instead of most values in script
  Remade part of 'player instances' code
  More cheat options, like 'Everything is free', now works
  Created .LOF Level Overview Files, levels.txt no longer used
  Started replacing network support

Version: 0.31
  Fixed disappearing creatures in zoom box
  Updated zoom level for various resolutions
  Rewritten even more of Hand Of Evil support
  Rewritten heap support for sound and speech samples
  Replaced CREATURE.TXT with multiple .CFG files.
  Rewritten network GUI functions
  Changed RESEARCH and added RESEARCH_ORDER commands

Version: 0.30
  Fixed Hand Of Evil support code
  Added range to 'REVEAL_MAP_LOCATION'
  Campaign files improved (file locations, options)
  Some more unification in accessing array elements
  Rewritten some network support functions
  Rewritten credits screen, added credits file

Version: 0.29
  Added new script command, 'REVEAL_MAP_RECT'
  Added new script command, 'REVEAL_MAP_LOCATION'
  Hand of Evil code has been rewritten
  Fixed memory leak in keeper AI module
  Fixed problem with 'IF_AVAILABLE' command
  Added support of multiple campaigns (not tested)

Version: 0.28b
  Rewritten more of script support, warnings added
  Added new script command, 'PLAY_MESSAGE'
  Added new script command, 'ADD_GOLD_TO_PLAYER'
  Added new script command, 'SET_CREATURE_TENDENCIES'
  Finished work on QUICK_* script commands support
  Fixed spells visibility in zoom box of map view
  DISPLAY_INFORMATION now requires two parameters (added zoom location)
  QUICK_INFORMATION now requires three parameters (added zoom location)
  When selling multiple traps on same tile, total cost is displayed
  Text file is now selected based on language setting in "keeperfx.cfg"
  Fixed room selling (v0.28a)
  Fixed wage and luck value displayed in creature info panel (v0.28a)
  Fixed typing mistake in hero party objectives (v0.28b)
  Fixed ALL_DUNGEONS_DESTROYED implementation error (v0.28b)

Version: 0.27a
  Rewritten most of the world view screen
  Allowed map numbers over 255
  Fixed mistake in function create_room
  All levels are available when whole campaign is passed
  Campaign configuration file improved
  LIF files support adopted from Deeper Dungeons
  Free level selection screen adopted from Deeper Dungeons
  New High Scores support code
  Strings (text.dat) replaced with those from DD (dd1text.dat)
  Script command QUICK_OBJECTIVE is now supported
  Compiled under new MinGW environment
  Some functions of Cheat Mode are now functional
  Continue file creation code unified and improved
  Started work on support of QUICK_INFORMATION command
  Fixed mistake in trap manufacture code (v0.27a)

Version: 0.26
  Rewritten minimap control code, fixed for 640x480
  Smarter WLB flags regeneration
  Rewritten part of 'player instances' code
  Better memory cleaning after a level is finished
  Bonus levels are preserved in 'Continue game'
  Transferred creature is preserved (for one use only)
  Fixed disappearing menu content problem
  Fixed ending statistics when finishing loaded game
  Computer player config file created

Version: 0.25
  Fixed information button blinking
  Rewritten part of the rendering engine
  Fixed mouse cursor shift and scrolling at 640x480
  Added options to save and load replay (packet file)
  Alt+X can now be used to quit the game
  Files created by the game are no longer read-only
  Rewritten loading of map files

Version: 0.24
  Rewritten level script analysis
  Added warning and error messages on script loading
  Allowed longer messages in script
  Patched DISPLAY_INFORMATION_WITH_POS
  Fixed problem with CFG file location
  Rewritten 'landscape affecting creature'
  Added comments to CFG file

Version: 0.23a
  Fixed time counter sound in bonus levels
  Fixed bad memory read in creatures tab code
  Created a campaign file which stores level numbers
  More error-safe config file support
  More inputs left when lost (screnshot, minimap zoom, etc.)
  Renamed campaign file to load properly (the .23a release)

Version: 0.22
  Reworked more of the packets processing system
  Prepared screen resolution ring
  Fixed problem with floating spirit on lost level
  Screenshots now work not only in game, but also in menu
  Improved quality of in-game map screen
  Fixed a 'sudden speedup' problem when playing > 50 minutes

Version: 0.21
  Video compression improved for high resolution
  Added command line option to change player in skirmish
  Added function of writing screenshots in BMP format
  Reworked part of the packets processing system
  Config file separated from original DK

Version: 0.20
  Message will show if a command line option is wrong
  Updated delay function to not use 'hlt' command
  Fixed object tooltips to show when mouse is on something
  Fixed load slots counting error from v0.19
  Rewritten GUI initialization arrays
  Rewritten and fixed movie recording
  Allowed to record movie in high resolution

Version: 0.19
  Rewritten GUI events system
  Rewritten some easter eggs
  Fixed music support problem from v0.18
  Game will try to continue if can't enter low-res mode
  Compiled using new version of GCC

Version: 0.18
  Rewritten frontend state control and menu speed
  Added high resolution loading screen
  Renewed video playing code and fading

Version: 0.17
  Rewritten main loop and game speed control
  Fixed screen object destructor
  Rewritten part of mouse support
  Made exit possibility when playing with '-level' option

Version: 0.16
  Finished logging module and log file creation

Version: 0.15
  Reworked the main initiation function
  Added introduction screen by Madkill

Version: 0.14
  Added some command line options from Beta
  Reworked moon phase calculation

Version: 0.13
  LbBullfrogMain() reworked
  Command line parameters reworked and fixed a bit

Version: 0.11
  Managed to make the DLL working (at last!)

Known problems:
  Mouse stops when trying to use multiplayer
  Movie encoding doesn't properly support high res

Programming:
 Tomasz Lis aka Mefistotelis
 Petter Hansson

Graphics:
 Chagui
 Madkill
 Synesthesia

Part of the code closed into DLL and some data files
 are copyrighted by Bullfrog Productions.
