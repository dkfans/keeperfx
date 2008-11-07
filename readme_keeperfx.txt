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
-q
  Works like "-level 1".
-columnconvert
  I assume it converts the columns data. Beware with
  this one or you may be unable to play the game again.
-lightconvert
  Same thing, but for lights.

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
  Made exit possibility when playing with "-level" option

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


Programming:
 Tomasz Lis aka Mefistotelis

Graphics:
 Madkill

Part of the code closed into DLL is copyrighted
 by Bullfrog.
