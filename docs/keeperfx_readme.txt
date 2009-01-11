Dungeon Keeper Fan Expansion
------------------------------

KeeperFX is a executable mod for Dungeon Keeper.

It is written by fans and not supported by original developer.
Still, it requires original Dungeon Keeper to work.

Installation:

Just copy all the files into your Dungeon Keeper directory.
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
  Disable the AWE32/64 SoundFonts.
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
  to work properly with it. Also, the selected player
  must have heart on the map.
-q
  Works like '-level 1'.
-columnconvert
  I assume it converts the columns data. Watch out with
  this one or you may be unable to play the game again.
-lightconvert
  Same thing, but for lights.

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
  Fixed load slots counting error from v0.19
  Rewritten GUI initialization arrays
  Rewritten and fixed movie recording
  Allowed to record movie in high resolution

Version: 0.1.9
  Rewritten GUI events system
  Rewritten some easter eggs
  Fixed music support problem from v0.18
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
 Madkill
 Chagui

Part of the code closed into DLL is copyrighted
 by Bullfrog.
