Dungeon Keeper Fan Expansion
------------------------------

KeeperFX is a executable mod for Dungeon Keeper.

It is written by fans and not supported by original developer.
Still, it requires original Dungeon Keeper to work.

Installation:

Copy all the files into your Dungeon Keeper directory.
Then open "KEEPERFX.CFG" with Notepad, and make the line
which starts with "INSTALL_PATH=" identical as in original
"KEEPER.CFG" file.
Run "keeperfx.exe" to start the game.

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
     Error: In source setup_game:
     1501 - Strings data too small
A: Copy a file TEXT.DAT from CD into DATA folder in
    DK installation directory.

Q: Mouse doesn't work properly.
A: There's no fix for this yet.

Q: Mouse disappears when I try to enter network game.
A: There's no fix; use stanard DK with IPX fix for multiplayer.

Q: Switching resolution/Taking over control in map view disables
   all rooms and spells!
A: Press TAB key twice to bring back working menu.

Q: There are no special eye effects when I posses Beetle, Fly,
    Dragon, Tentacle etc.!
A: Lense effect only work if you have over 16MB RAM and in screen
     resolutions: 320x200, 640x400 and 640x480.

Q: I've found a cheat menu, but it doesn't work!
A: All three cheat menus are still unfinished.

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
   gameplay by pressing ALT+R. Modes over 640x480 are
   experimental, and not completely stable.
  INGAME_RES=<mode1> <mode2> <mode3> ....

SCREENSHOT
  Selects the format in which screenshots will be written.
    You can choose between BMP and HSI bitmap format.
  SCREENSHOT=<type>

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
  parameters. You can made a packet file which contains
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

Changelog:

Version: 0.2.6
  Rewritten minimap control code, fixed for 640x480
  Smarter WLB flags regeneration
  Rewritten part of 'player instances' code
  Better memory cleaning after a level is finished
  Bonus levels are preserved in "Continue game".
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
