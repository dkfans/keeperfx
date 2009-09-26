Dungeon Keeper Fan Expansion
------------------------------

KeeperFX is a executable mod for Dungeon Keeper.

It is written by fans and not supported by original developer.
Still, it requires original Dungeon Keeper to work.

Installation over original DK:

Copy all the files into your Dungeon Keeper directory.
If you've installed DK from CD, then you will also have to copy
folders DATA, LDATA and LEVELS from CD into this directory.
Then open 'KEEPERFX.CFG' with Notepad, and set proper language
shortcut in the 'LANGUAGE=' line.
Run "keeperfx.exe" to start the game.

Note that automatic builds of KeeperFX do not contain all of
neccessary files, you need to use a released version first.

Command line options:

-nointro
  The intro sequence won't play at startup.
-nocd
  The CD Sound tracks won't play.
-1player
  Allows playing multiplayer maps in skirmish mode.
-nosound or -s
  Disables all the sounds.
-fps <num>
  Changes the game speed; default <num> is 20.
-usersfont
  Disable the AWE32/64 SoundFonts (.SBK files).
-alex
  Used to show the 'JLW' easter egg.
-level <num>
  Brings you directly to level number <num>.
  After the level is finished, quits the game.
  Note that level number must be 1..255.
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
    but no problem is mentioned in KEEPERFX.LOG.
A: Select "Run in 256 colors" and "Run in 640x480" in the
    program properties, and change lines in KEEPERFX.CFG
    into those written in next answer.

Q: Intro doesn't play. LOG file says:
     Error: In source setup_game:
     1500 - Can't enter movies screen mode to play intro
A: The problem is that your drivers can't support 320x200 mode.
   Change the resolution config lines in KEEPERFX.CFG into:
  FRONTEND_RES=MODE_640_480_8 MODE_640_480_8 MODE_640_480_8
  INGAME_RES=MODE_640_480_8

Q: The game doesn't run. LOG file says:
     Error: In source setup_strings_data:
     1501 - Strings file couldn't be loaded or is too small
A: Check if there's a language file in 'FXDATA' folder for the
     language which you've selected in KEEPERFX.CFG.

Q: Mouse doesn't work properly.
A: There's no fix for this yet.

Q: I get a message 'Cannot initialize' when I try to enter network game.
A: There's no fix; use stanard DK with IPX fix for multiplayer.

Q: Switching resolution/Taking over control in map view disables
   all rooms and spells!
A: Press TAB key twice to bring back working menu.

Q: There are no special eye effects when I posses Beetle, Fly,
    Dragon, Tentacle etc.!
A: Lense effect only work if you have over 16MB RAM and in screen
     resolutions: 320x200, 640x400 and 640x480.

Q: I've found a cheat menu, but it doesn't work!
A: The three cheat menus are only partially functional.

Config file details:

FRONTEND_RES
  Allows you to select front-end resolution (used inside
   menu and for playing movies, but not in actual game)
  FRONTEND_RES=<failsafe mode> <movies mode> <menu mode>

INGAME_RES
  Allows you to select up to five in-game resolutions.
  Valid modes are: MODE_320_200_8, MODE_320_240_8,
   MODE_512_384_8, MODE_640_400_8, MODE_640_480_8,
   MODE_800_600_8, MODE_1024_768_8, MODE_1280_1024_8,
   MODE_1600_1200_8.
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
  displaying texts. Note that changing this option will not
  completely switch the language - only text messages
  will be changed. If the language setting doesn't match
  the one selected during installation, then national
  letters may be displayed incorrectly. Also, this option
  doesn't change the speech language.
  
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

Version: 0.3.2
  Added CPU identification
  Rewritten part of spell casting code
  Rewritten some of 'Transfer creature' code
  Rewritten some possession-related code
  RANDOM can now be used instead of most values in script
  Remade part of 'player instances' code
  More cheat options, like 'Everything is free', now works
  Created .LOF Level Overview Files, levels.txt no longer used
  Started replacing network support

Version: 0.3.1
  Fixed disappearing creatures in zoom box
  Updated zoom level for various resolutions
  Rewritten even more of Hand Of Evil support
  Rewritten heap support for sound and speech samples
  Replaced CREATURE.TXT with multiple .CFG files.
  Rewritten network GUI functions

Version: 0.3.0
  Fixed Hand Of Evil support code
  Added range to 'REVEAL_MAP_LOCATION'
  Campaign files improved (file locations, options)
  Some more unification in accessing array elements
  Rewritten some network support functions
  Rewritten credits screen, added credits file

Version: 0.2.9
  Added new script command, 'REVEAL_MAP_RECT'
  Added new script command, 'REVEAL_MAP_LOCATION'
  Hand of Evil code has been rewritten
  Fixed memory leak in computer player module
  Fixed problem with 'IF_AVAILABLE' command
  Added support of multiple campaigns (not tested)

Version: 0.2.8b
  Rewritten more of script support, warnings added
  Added new script command, 'PLAY_MESSAGE'
  Added new script command, 'ADD_GOLD_TO_PLAYER'
  Added new script command, 'SET_CREATURE_TENDENCIES'
  Finished work on QUICK_* script commands support
  Fixed spells visibility in zoom box of map view
  DISPLAY_INFORMATION now requires two parameters (added zoom location)
  QUICK_INFORMATION now requires three parameters (added zoom location)
  When selling multiple traps on same tile, total cost is displayed
  Text file is now selected based on language setting in 'keeperfx.cfg'
  Fixed room selling (v0.2.8a)
  Fixed wage and luck value displayed in creature info panel (v0.2.8a)
  Fixed typing mistake in hero party objectives (v0.2.8b)
  Fixed ALL_DUNGEONS_DESTROYED implementation error (v0.2.8b)

Version: 0.2.7a
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
  Fixed mistake in trap manufacture code (v0.2.7a)

Version: 0.2.6
  Rewritten minimap control code, fixed for 640x480
  Smarter WLB flags regeneration
  Rewritten part of 'player instances' code
  Better memory cleaning after a level is finished
  Bonus levels are preserved in 'Continue game'
  Transferred creature is preserved (for one use only)
  Fixed disappearing menu content problem
  Fixed ending statistics when finishing loaded game
  Computer player config file created

Version: 0.2.5
  Fixed information button blinking
  Rewritten part of the rendering engine
  Fixed mouse cursor shift and scrolling at 640x480
  Added options to save and load replay (packet file)
  Alt+X can now be used to quit the game
  Files created by the game are no longer read-only
  Rewritten loading of map files

Version: 0.2.4
  Rewritten level script analysis
  Added warning and error messages on script loading
  Allowed longer messages in script
  Patched DISPLAY_INFORMATION_WITH_POS
  Fixed problem with CFG file location
  Rewritten 'landscape affecting creature'
  Added comments to CFG file

Version: 0.2.3a
  Fixed time counter sound in bonus levels
  Fixed bad memory read in creatures tab code
  Created a campaign file which stores level numbers
  More error-safe config file support
  More inputs left when lost (screnshot, minimap zoom, etc.)
  Renamed campaign file to load properly (the .2.3a release)

Version: 0.2.2
  Reworked more of the packets processing system
  Prepared screen resolution ring
  Fixed problem with floating spirit on lost level
  Screenshots now work not only in game, but also in menu
  Improved quality of in-game map screen
  Fixed a 'sudden speedup' problem when playing > 50 minutes

Version: 0.2.1
  Video compression improved for high resolution
  Added command line option to change player in skirmish
  Added function of writing screenshots in BMP format
  Reworked part of the packets processing system
  Config file separated from original DK

Version: 0.2.0
  Message will show if a command line option is wrong
  Updated delay function to not use 'hlt' command
  Fixed object tooltips to show when mouse is on something
  Fixed load slots counting error from v0.1.9
  Rewritten GUI initialization arrays
  Rewritten and fixed movie recording
  Allowed to record movie in high resolution

Version: 0.1.9
  Rewritten GUI events system
  Rewritten some easter eggs
  Fixed music support problem from v0.1.8
  Game will try to continue if can't enter low-res mode
  Compiled using new version of GCC

Version: 0.1.8
  Rewritten frontend state control and menu speed
  Added high resolution loading screen
  Renewed video playing code and fading

Version: 0.1.7
  Rewritten main loop and game speed control
  Fixed screen object destructor
  Rewritten part of mouse support
  Made exit possibility when playing with '-level' option

Version: 0.1.6
  Finished logging module and log file creation

Version: 0.1.5
  Reworked the main initiation function
  Added introduction screen by Madkill

Version: 0.1.4
  Added some command line options from Beta
  Reworked moon phase calculation

Version: 0.1.3
  LbBullfrogMain() reworked
  Command line parameters reworked and fixed a bit

Version: 0.1.1
  Managed to make the DLL working (at last!)

Known problems:
  Mouse stops when trying to use multiplayer
  Movie encoding doesn't properly support high res

Programming:
 Tomasz Lis aka Mefistotelis

Graphics:
 Chagui
 Madkill
 Synesthesia

Part of the code closed into DLL is copyrighted
 by Bullfrog Productions.
