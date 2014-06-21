Dungeon Keeper Fan Expansion
------------------------------

KeeperFX is an extensive mod for Dungeon Keeper.

It is written by fans and not supported by original developer.
It requires some of original Dungeon Keeper data, but many files
are also modified or remade.

Installation of KeeperFX CCP:

You need original Dungeon Keeper CD (or CD image) to perform
complete installation.

Please note that there are two types of releases: complete
version and patch. You need a complete version to play the game;
a patch is only an additional feature, which you can use by
overwriting some files from complete version with new ones.
Even with complete version, you still need original Dungeon
Keeper CD to prepare the game for playing.

To install KeeperFX CCP, you have to unpack the archive to
your desired target location, and then run "launcher.exe".
The Launcher utility will allow you to select source folder
from which original DK files will be taken. To select the
"keeper" folder and start copying files, press "Install" button.

You must select the "keeper" folder from the CD content list.
Selecting root folder of your CD, or selecting installation
folder on disk, won't be enough.

If you wish to change language, click "Settings" in the launcher
and select it from list. Remember to save your changes. You may
also edit "keeperfx.cfg" by hand, using text editor.

Press the correct button inside Launcher to start the game.
For information about running the game executable directly,
see 'running KeeperFX' section. If something doesn't work,
see the 'troubleshooting' section.

Supported DK releases to install from:

Here you can find a list of releases which you can use to get
files required by KeeperFX:
- Dungeon Keeper, english release
- Dungeon Keeper, multilingual release
- Dungeon Keeper Gold, english release
- Dungeon Keeper Gold, multilingual release
- Dungeon Keeper from GOG.com (the CD image "game.gog")
Remember that KeeperFX needs the original CD, or mounted
CD image. The files installed on disk by original DK setup
are not enough. Also, you must select the "keeper" folder from
the CD content list. Selecting root folder of your CD, or
selecting installation folder on disk, won't be enough.

Available languages:

The following languages are currently functional:
 ENG ITA FRE SPA DUT GER POL SWE JPN RUS CZE
The following languages are partially functional:
 CHI CHT LAT
Note that some campaigns may not support your language.
In this case, default language will be used inside this
campaign.

Available screen resolutions:

To change available screen modes, edit "keeperfx.cfg".
Screen mode can be in a form WIDTHxHEIGHTxBPP which defines
 fullscreen mode, or WIDTHxHEIGHTwBPP, which defines windowed
 mode. When defining windowed mode, the BPP should be equal to
 the colour depth on your desktop. It is not recommended to
 mix fullscreen and windowed modes in one config file. 
There are two lines which define resolutions: 'FRONTEND_RES='
 should have exactly 3 parameters and is used in the menu,
 while 'INGAME_RES=' can have 1-5 parameters, which define
 list of resolutions to switch between in the actual game.
Most stable modes are 640x400 and 640x480. Higher resolutions
 may be sometimes unstable, especially in possession.

Running KeeperFX:

To start the game, run "keeperfx.exe". If you want to report
any errors you encounter, you may run "keeperfx_hvlog.exe"
instead. This will run a "heavylog version", which writes a lot
of information into "keeperfx.log". In case of the game hanging
on suddenly disappearing, you may send a last few lines of the
generated LOG to the author with your description of the bug.

Note that "keeperfx_hvlog.exe" requires a lot more CPU than
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
  This consumes more CPU, and the effect is merely visible,
  so blurring is disabled by default.
-ppropoly <mode>
  Allows to control "pentium pro polygon rendering". Default
  value is 0, which means the game will detect if the CPU is
  modern enough and enable advanced rendering based on that.
  For processors below Pentium Pro (today such CPU would be
  ancient) the game disables advanced polygon lightning
  computing. Setting <mode> to 1 will make sure the advanced
  rendering is always enabled, and setting it to 2 disables
  Pentium Pro polygons regardless of the CPU.
-altinput
  Uses alternate mouse input method. This changes the way of
  computing mouse position; with this option, position is not
  reset to screen center every time movement is detected.
  May be helpful if original method isn't working right
  (ie. mouse stops).
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

Q: When I click "Install" and select my CD-ROM where original DK
    is, the game tells there are no required files.
A: You need to select "KEEPER" folder on the CD-ROM, not the
    whole CD.

Q: When I click "Install", select "KEEPER" folder, then I click
    that I want to copy files - the loader shows "Access denied"
    error message.
A: In that case, you have to run "loader.exe" with administrative
    privileges. Another solution is to give all users write access
    to the folder where KeeperFX is, and its sub-folders.

Q: Colors are changed in menu or during gameplay,
    but no problem is mentioned in "keeperfx.log".
A: Try using different color modes  in KEEPERFX.CFG. For example,
    you could try 24-bit or 32-bit colour (ie. 640x480x24).
    It is best to use the same colour mode for all resolutions.

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
  INGAME_RES=640x480x32
   You may also try other resolutions, but those over 640x480
   may be unstable.

Q: Game stops when loading a map. LOG file says:
     Error: setup_screen_mode: Unable to setup screen resolution
            640x400x8 (mode 10) 
A: The problem is that your drivers can't support 640x400 mode.
   Change the resolution config lines in KEEPERFX.CFG into:
  FRONTEND_RES=640x480x32 640x480x32 640x480x32
  INGAME_RES=640x480x32

Q: The game doesn't run. LOG file says:
     Error: setup_strings_data: Strings file couldn't be loaded
            or is too small
A: Check if there's a language file in 'FXDATA' folder for the
   language which you've selected in KEEPERFX.CFG.

Q: The game starts up to main menu, but when I try to load a level,
   it exits back to desktop.
A: There may be many reasons for that. Try checking your LOG file.
   Also, try adding keeperfx.exe and keeperfx_hvlog.exe to DEP list
   in Windows - that's the solution to most common problem.
   DEP is a Data Execution prevention mechanism; search the net
   for details about it.

Q: Mouse stops/teleports/moves incorrectly during the game.
A: Try the '-altinput' command line switch. If the mouse moves too
   fast or too slow, try changing "POINTER_SENSITIVITY" option
   in "keeperfx.cfg"

Q: I get a message 'Cannot initialize' when I try to enter network game.
A: KeeperFX does not support serial cable, modem and IPX multiplayer.
   Use standard, retail version of DK if you wish to play serial or
   modem game. Use standard DK with IPX fix (or DK Gold) if you wish
   to play IPX game. KeeperFX supports only TCP/IP protocol. 

Q: I'm having problems with TCP/IP multiplayer.
A: See 'tcp_readme.txt' for more information about multiplayer.

Q: There are no special eye effects when I possess Beetle, Fly,
    Dragon, Tentacle etc.!
A: Lens effect only work if the game detects over 16MB RAM.
   Also, make sure that "fxdata/lenses.cfg" is present and not damaged.

Q: After some time of gameplay, many in-game elements stops working.
    Creature spells and some keeper spells (ie. Create Imp spell) ain't
    working. Same with placing traps/doors, and creating gold piles.
A: The game has a limit for amount of 'things' - these 'things' are
    all the objects that generate sprites (graphic representations as
    an image). They are limited to 2047, and if all slots are taken,
    it's no longer possible to create spell shots, gold piles or doors.
    Creatures have additional limit - even if there are still free
    'thing' slots, they are limited to 255.

Q: What's the difference between 'keeperfx.exe' and 'keeperfx_hvlog.exe'?
A: These files are identical except of one thing: 'keeperfx_hvlog.exe'
    writes A LOT of messages into 'keeperfx.log', allowing to trace
    any bugs and problems during the game. Because of the amount of data
    being written, the heavylog version, 'keeperfx_hvlog.exe', is a few
    times slower than standard version, 'keeperfx.exe'. If you're
    not planning to report any bugs, you should use 'keeperfx.exe'.

Q: I've found a cheat menu, but it doesn't work!
A: The three cheat menus are only partially functional.

Reporting a bug:

If you've found a bug in the game, you may report it to KeeperFX developers.
But you will have to to do some tests to gather as much information as
possible about the problem.

First, if the game crashed, try looking into 'keeperfx.log'. If there are
error messages in it, it's possible that you haven't properly installed or
configured KeeperFX. In this case, check the 'Troubleshooting' section for
description of your problem. Note that running the game again will overwrite
the LOG file, so if you want to keep it, you'll have to make a copy.

Second step is to try reproducing the error, and generate more detailed log.
Run 'keeperfx_hvlog.exe' and play the level again, doing similar things you did
first time, to check if it crashes. If you can't reproduce the error, there is
still a chance that the LOG file from first crash is enough to locate the
problem - so post the copy you've made on issue tracker, with your description
of the problem, and information that you couldn't reproduce it.

If you was able to reproduce the error, then post detailed description of how
to do it on the issue tracker. Remember to include LOG file created by
'heavylog' version of KeeperFX. Note that the log file will be huge - you
shouldn't attach it directly. Instead, you can compress it, or just paste
a few (ie. 20) lines from its beginning and its end. Remember to include first
and last line of the LOG - these are crucial, and doing it incorrectly would
mislead the developers.
If it is possible to reproduce the error by loading specific saved game and
doing a few simple actions, then attach the saved game to your report. You can
recognize file which contains specific saved game by number in filename, which
is always equal po position of the saved game slot in 'load' menu.


Config file details:

FRONTEND_RES
  Allows you to select front-end resolution (used inside
   menu and for playing movies, but not in actual game)
  FRONTEND_RES=<failsafe mode> <movies mode> <menu mode>

INGAME_RES
  Allows you to select up to five in-game resolutions.
  Resolution has the form of WIDTHxHEIGHTxBPP.
  Standard modes are: 320x200x8, 320x240x8, 512x384x8,
   640x400x8, 640x480x8, 800x600x8, 1024x768x8.
  Different modes (ie. widescreen, higher res or
   higher BPP) may be used too, if only they are
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

POINTER_SENSITIVITY
  Allows you to adjust the speed of mouse movement.
  Use this option only if you have serious issues with
  moving your mouse, or if the speed inside KeeperFX is
  completely different to the one in your OS. On most
  problems, you should change mouse speed in your OS
  preferences instead of changing this option. 
  
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

  The commands list has been moved to Wiki, to make it easier to maintain:
  https://code.google.com/p/keeperfx/wiki/NewLevelScriptCommands

Changelog:

Version: 0.4.5
  Rewritten drawing the pannel minimap.
  Rewritten a lot of network GUI routines.
  Rewritten a few functions related to traps and shots.
  Rewritten some of moods and needs processing. Allowed diggers to have moods and needs if their config file permits that.
  Made creatures unable to teleport just after a battle.
  Improved recognition between KINKY_TORTURE and PAINFUL_TORTURE jobs.
  Rewritten payday processing.
  Fixed restoring Flight spell state when creature is being dropped, or ends other state which prevented it from flying.
  Started renaming 'job stress' to 'going postal'. Also rewritten some of scavenging.
  Introduced ONE_OF_KIND creature property, for Avatar.
  Introduced NO_HAND_PURGE_ON_DEFEAT classic bug.
  Creature job first initialization remade to be configurable in creature.cfg.
  Moved creature unconscious time to config file.
  Changed concept behind job assigning to be based on job selection, not on room selection.
  Work around for issues with multiplayer level number in level selection screen. The number is exchanged as 8-bit int, which should be correctly fixed when possible.
  Unifications in creature jobs system. Also added definitions of a few more jobs.
  Fixed directory listing to correctly use MS Windows API.
  Fix for loading damaged saved game during another game.
  Updated attraction score computations to include scores from all 3 rooms required to attract.
  Updated Chinese translation.
  Rewritten keeper powers update function.
  Removed area of effect damage from lightning creature spell.
  Modified ForceVisibility option to be in game turns.
  Rewritten code of destroying a room with CTA. Modified it to get more random order of destroyed tiles.
  Made new "room unreachable" event for lair.
  Updated events when a spellbook or dungeon special is discovered or stolen.
  Neutral boulder traps will now activate for any creatures other than neutral.
  Moved several creatures from "United Kingdom" level to being triggered by script.
  Added classic latin as recognized language.
  Added friendly fire on area damage as config parameter.
  Updated gold pots scaling algorithm. Pots are now generally smaller, and those with lots of gold are growing slower than small ones.
  Rewritten boulder trap activation check. Spectators can no longer trigger boulder traps.
  Added config option to select whether neutral creatures can be scavenged.
  Fixed problem with creature strength not being increased with experience.
  Armageddon no longer teleports neutrals by default.
  Added rules option to disable affecting neutral creatures by armageddon.
  Progressed the implementation of going through locked doors property.
  Rewritten drawing plane of engine columns in isometric and clueo mode.
  Added two new creature properties - FEMALE and INSECT.
  Retwritten some GUI functions and creation of creature own name.
  Added SDL_mixer to pre-compiled libraries and to linking process. The library isn't used yet.
  Rewritten the check if a creature can move directly to a place.
  Rewritten the function which destroys creature lair.
  Modified uses of apply_damage_to_thing() so that the function is always informed about the type of damage being inflicted.
  Rewritten and highly modified poison gas effect affecting creatures.
  Made some improvements to creature movement system. Creatures will now reset their routes after being teleported.
  Added fixing travel speed if the next travel point returned by ariadne is too far.
  Rewritten and fixed picking up creatures based only on their job and not their model.
  Removed some of references to manual from english translation.
  Modified area damage to be applied on detonation of every shot.
  Fixed problem with line of sight computation which caused word of power trap to not do any damage.
  Started integrating OGG music support made by Lukas Niemeier.
  Added Czech characters to European conversion table.
  Made more advanced scaling of parchment view.
  Renamed spells to start with SPELL_ and shots to start with SHOT_.
  Traps placed on strange terrain are now destroyed when depleted.
  Added support of off-map traps and off-map doors.
  Selling traps change - only get a refund if armed trap was sold.
  Made computer player cancel any defend drops while his heart is ongoing destruction.
  Introduced types of damage, ie. physical, magical, electric, combustion.
  Rewritten some code related to gold and paydays.
  Updated language names to meet ISO 639-2 standard. Japanese is now JPN.
  Rewritten a lot of code related to workshop selling, to fix a bug in counting workshop items which are being sold.
  Added states which cannot be blocked by spells - currently there's one, the being dropped state.
  Modified battle event support to ignore the event if there are no enemies nearby.
  Modified battle events to properly react on heart being attacked.
  Added a new state which allows to attack doors and enemies while moving to attack room.
  Rewritten door collision detection.
  Rewritten some code around creature fights with doors.
  Some minor updates to polish translations.
  Fixed problem with magic door being recognized as sacrificial ground.
  Updated russian fonts to be correctly included in multilingual font files.
  Fixed problem with finding a creature dragging given thing.
  Made kinky torture to not remove creature control.
  Fixed problem with imps escaping from creatures behind doors.
  Removed possible infinite loop in pathfinding, and enabled some rewritten routines.
  Rewritten dungeon devastation when heart is destroyed.
  Some serious changes in line of sight computation.
  Computer players can now sell traps where they intend to place room.
  Fixed imps transporting enemy creatures to drop them when enemy is defeated.
  Fixed some possible problems related to creatures attack.
  Rewritten some functions related to fight with doors and objects.
  Forced creatures to become visible when dying or losing consciousness.
  Fixed inconsistency in resetting states of creatures working in a room taken over.
  Updated leaving or dying function (for defeated player creatures) to not wake unconscious creatures.
  Rewritten some code related to dragging unconscious creatures.
  Added regaining comp control when creature is called to arms.
  Updated computer player gold digging code.
  Updated room building code to prevent placing rooms on slabs with traps.
  Fixed lava trap activation on room area to not break list of room slabs.
  Added cleaning combat when computer player picks up creature.
  Updated computer player digging path routine.
  Updated Ancient Keeper translations, and added German translation.

Version: 0.4.4
  Added auto-generating possession swipe sprites.
  Update to foe identification algorithm; creatures being dropped can no longer be treated as enemies.
  Fixed problem with battles not being dissolved.
  Modified creatures with high fear to no longer start combat when invisible.
  Modified imps to be less interested in dead bodies laying near living enemies.
  Teleporting creatures are now a little more interested in staying in combat.
  Unified coordinates computation and added scaling to parchment map screen.
  Rewritten drawing things on round minimap.
  Fixed the problem with backward move into negative Y direction.
  Fixed problem with bitmap font while recording a movie which lasts several levels.
  Changed the way trap-to-crate and door-to-crate association is stored.
  Fixed mistake in code which caused creatures which are doing nothing to wander the dungeon.
  Replaced the non-existing GROUP job with BARRACK for skeletons.
  Rewritten drawing of list of things in hand.
  Allowed computer player to move creatures to lair if they don't have one yet.
  Rewritten large part of computer player room building code.
  Updated ComputerTask structure - created unions which store task-specific parameters for each task.
  Forced computer player to drop things from his "fake hand" when it's being disabled or defeated.
  Created a function which fills dungeon position using room coordinates if no soul container is present.
  Divided the map event 'fight' into two distinct events - friendly fight and enemy fight.
  Rewritten computer player digging room. Modified checking if the room has been dug.
  Changed order of land view narrations, and the folder where narrations are stored.
  Modified workshop job to be stronger used when gold is low.
  Rewritten computing ceiling height above set of subtiles.
  Rewritten some of computer player CTA casting code.
  Rewritten generation of map fade ghost color file.
  Improved room expansion to never create long, narrow rooms.
  Rewritten computer player selecting best digging path to entrance.
  Added support of computer player selling deployed doors.
  Completely changed the way of tracking amount of traps and doors available to be placed.
  Fixed computer player expanding rooms to not include slabs with traps on.
  Implemented selling placed traps by computer player when out of money.
  Rewritten computer player placing bridge on dig path.
  Rewritten some of computer player attack code and hatred computation.
  Modified gold throwing code to never create too many pots, and to distribute the gold equally amongst pots.
  Computer player now can use temple to heal disease, and puts much higher priority to training his creatures.
  Fixed spell casting and slapping by computer player. Computer is now aware that imps may die when slapped.
  Improved checking creature job availability when moving creatures by computer player.
  Computer player will now less likely put his creatures to work in rooms with enemies.
  Updated casting keeper powers on things - a specific thing is now selected based on castability settings.
  Fixed computer player to stop dropping creatures in rooms which are in their not_do_job list.
  Modified CREATURE_AVAILABLE command to allow force-attracting creatures.
  Fixed selection of random wandering position while exploring the dungeon.
  Rewritten computer player digging to entrance. Made unifications to other digging operations.
  Added checking for spell availability before they're used.
  Rewritten computer player picking up imps in danger.
  Several unifications in manufacture system and objects data.
  Rewritten some functions related to computer player casting SoE.
  Rewritten computer player casting spells on attackers.
  Rewritten computer player casting speed spell on fighters.
  Rewritten some of the sulking code.
  Fixed distinction between AP number and AP index.
  Original Campaign - fixed Mistle script to give the player only 3 boulder traps.
  Rewritten drawing trap buttons; also renamed some variables.
  Japanese DKMaps8 pack - Added messages to first map.
  Rewritten a few functions related to battles and unconscious creatures.
  Modified tunnelers ant tunneler parties to use the same method for storing spawn place and tunnel target.
  Updated all .po translations of campaigns with their associated .pot template files.
  Updated spell descriptions in translations and resaved them with POEdit.
  Fixed unconscious creatures to no longer be attacked due to job stress.
  Added possibility to load per-campaign creature.cfg file.
  Added stats increase with experience as config parameters. Added different default values for Ancient Keeper.
  Rewritten Anger Job selection.
  Rewritten some of "persuade" anger job.
  Rewritten some of Mad Killing code; updated mood routine to not allow mad creature to stop being angry.
  Original Campaign - Updates to scripts of some more Deeper Dungeon levels.
  Fixed creatures in livid state - they now stop working when very angry, as they should.
  Original Campaign - reviewed and updated scripts for all levels.
  Original Campaign - fixed bonus levels to use correct text messages.
  Updated some more of multiplayer levels script files; only irrelevant changes.
  Updated some of script files, mostly from multiplayer levels, but also from original campaign.
  Generated .pot translation templates for 3 campaigns - pstunded, questfth, twinkprs.
  Prepared translation files for "Conquest of the Arctic" campaign.
  Documented some tools. Added a script which helps in creating .pot files for translations.
  Rewritten traps creation routines.
  Rewritten destroying a door.
  Rewritten some of freezing prisoners code.
  Fixed a mistake in trap placing conditions.
  Rewritten computer player digging routine.
  Rewritten some of picking up things code.
  Rewritten searching for interesting neutral places by computer player.
  Improved high score table input.
  Rewritten color matching function which finds best color index for given RGB values.
  Rewritten some of working in barracks and placing traps code.
  Divided computer player door placement function to make it more clear.
  Rewritten computer player search for enemy entrances and placing doors around rooms.
  Some unifications in functions relatedto moving in rooms.
  Rewritten a torture-related function.
  Renamed job TEMPLE to TEMPLE_PRAY.
  Fixed bug with not being able to manufacture resulting in not being able to pray.
  Fixed problem with sell button being shown incorrectly when mouse button is kept down on it.
  Minor renaming which affect quite lot of files.
  Rewritten a missing part of FLI files creation; also updated memory management routines.
  Activated some of rewritten sound handling code.
  Rewritten some GUI-related routines.
  Fixed some potential problems with config files parsing.
  Fixed problem with invalid value casting causing long wait in creature doing nothing state.
  PngPal2Raw - Finished support of JTY files, fixed for better error handling.

Version: 0.4.3
  Rewritten some of creature tunneling code and position computation when tunneling.
  Rewritten some of heroes attacking rooms and dropping gold code.
  Rewritten moving creatures in workshop.
  Rewritten the function which controls sending creatures to rooms, moved options to config files.
  Rewritten the function which controls creature behaviour while it's idle.
  Rewritten waiting for combat (random jumps) code.
  Rewritten the function which creates Action Points.
  Added spaces to Japanese translation.
  Updated the function which defines game keys to accept ALT modifier.
  Rewritten "define keys" screen.
  Rewritten drawing the creatures list in Transfer Creature special.
  Fixed a possible crash while drawing a sprite with very large scale.
  Rewritten the function which controls revealing terrain by a creature.
  Updated power hand pickability code. Added function which checks if a creature is dying.
  Fixed problem with floating spirit spell being inactive and controlling floating spirit.
  Rewritten some functions related to computer player tasks.
  Rewritten some of creature manufacture task code.
  Rewritten creating a creature at dungeon heart.
  Allowed turning alliances on and off.
  Rewritten selecting imps for pickup by computer player.
  Rewritten selecting creatures for defensive drop by computer player.
  Upgraded the code used for killing creatures. Created cases when not adding to resurrect list.
  Rewritten a bit more of computer checks code; fixed a coding mistake which caused a crash.
  Improved damage projection. Also, made better Dexterity and Defence explanation in config files.
  Rewritten some of keeper sprites loading and handling code.
  Fixed tunneller being unwilling to attack sometimes.
  Fixed the problem with fairies being stucked in the ceiling. They will now lower the flight.
  Replaced binary cubes config file with text one.
  Modified some config files to make the game more similar to original DK.
  Introduced per-campaign config files for Ancient Keeper. Also removed unused credits file.
  Modified checking if creature will attack another to make sure creatures in prison won't be attacked.
  Fixed the problem with computer player not building new room if it already has such room with low capacity.
  Modified fear computation to prevent excessive fluctuation of behavior.
  Fixed problem with digging gems consuming most all of computer player workforce.
  Rewritten some of computer player moving creatures code.
  Improved searching for a hatchery when creature is hungry.
  Rewritten several computer player routines related to room building.
  Added automatic creation of a few RAW files from PNGs.
  Rewritten some of computer building rooms code.
  Rewritten searching for food. Added another condition of "no food" event.
  Own creatures fight is no longer causing casting CTA and moving creatures by computer.
  Combat with unconscious creature is no longer a valid combat.
  Rewritten creature moving routine and some of prettying code.
  Workaround for allowing to spawn special workers with fly ability.
  Rewritten loading frontend sprites. Modified general sprites loading function.
  Updated spanish language, and building of graphics files.
  Updated building of GUI DAT/TAB files. Also removed casting Destroy Walls on rock.
  Modified config to allow casting speed spell on creatures held in custody.
  Rewritten part of creature fighting code.
  Rewritten two important functions in pathfinding.
  Rewritten some creature instance callbacks.
  Launchwx - Removed a few unused or auto-generated files from KeeperFX installation
  PngPal2Raw - created loading of TXT animation lists for JSPR(JTY) format support.

Version: 0.4.2
  Launcher updated with additional options
  More graphics data files are created from PNGs
  Made Slap spell do be added to a player by default
  Created new options for preserving classic bugs
  Fixed invisible mouse cursor when leaving multiplayer game
  Fixed fading while traveling through the menu
  Fixed error in defensive spells code
  Also increased WIND instance reload time
  Rewritten creature hatchery search while hungry
  Improved accuracy of angle-to-position calculation
  Unified computing whether a thing can be affected by a creature spell
  Rewritten the Destroy Walls spell
  Removed the possibility of ice explosion death for creatures which do not bleed
  Added more config options in rules file
  Improved adding re-arm tasks to imp queue
  Rewritten engine perspective rotation
  Retwritten movement while working in temple
  Rewritten some data structures related to possession swipe effects
  Fixed not allowing 2 trap boxes on same subtile
  Increased the push effect caused by explosion
  Reduced explosion range for friendly creatures to 1/3 of normal range
  Rewritten spell casting from within battles list message
  Rewritten and unified magic cast checking
  Moved some job properties to config files
  Rewritten footstep sounds playing
  Rewritten functions which draw scaled sprites
  Rewritten some of checking for imp jobs
  Fixed and re-enabled the new pathfinding code
  Rewritten some imps tasks which make use of pathfinding routines

Version: 0.4.1
  Updates to land view screen
  Integrated Post Undead Keeper campaign
  Integrated Conquest of the Arctic campaign
  Fixed a problem with making non-existing player an ally
  Improved line of sight computing
  Many updates to digger tasks code
  Remade magic maintenance
  Introduced a new way of determining where a spell can be casted
  Hero player isn't allowed to be set up as computer player
  Integrated Undead Keeper campaign
  Remade dungeon heart fight selection
  Fixed the problem with computer player placing traps at invalid places
  Rewritten displaying the heart flower and anger level above a creature
  Rewritten a part of computer player tasks code
  Fixed the definition of keeping creature in enemy custody
  Any player now automatically drops all things in hand while his heart is exploding.
  Fixed issues with spell being in Library at start of a level
  Rewritten lot of code around room maintenance
  Introduced tools which allow to easily create land views from PNG files (Png2bestPal,PngPal2raw)
  Introduced tools to translate campaigns with use of .po/.pot files (Po2ngdat)
  Prepared .po/.pot files for all the campaigns which have national messages
  Rewritten and modified some lightning-related routines
  Rewritten the function which computes amount of hate computer player has towards other players
  Rewritten large part of scavenging code
  Rewritten some code related to needs of creatures
  Moved some of room and slab properties into terrain config file
  Rewritten line of sight computation
  Some modifications to fighting doors
  Rewritten part of color tables generation
  Rewritten part of temple summoning code
  Rewritten and unified the way of searching for things around given coordinates
  Fixed playing level intro/victory speech multiple times
  Creatures are now picked by level only with CTRL, and normally they're picked unordered
  Rewritten hero tunnelling code
  Statistics screen will now show asian languages correctly
  Fixed error with invalid screen showing after playing some of DD levels
  Association between lair object and creature is now moved to config file
  Added new creature properties
  Rewritten dropping creature from hand
  Improved config files for objects and for magic
  Updated clipping of map coordinates
  Rewritten the function which handles trap update
  Updated functions used for updating positions of things
  Rewritten putting traps by computer player
  Rewritten the function used for claiming enemy rooms
  Improved WOP trap to use the same explosion routine as WOP spell
  Prepared a mechanism to preserve some of classic DK bugs
  Replaced fear computation algorithm, introduced 3 factors related to fear
  Rewritten a lair creation routine
  Rewritten the research process routine
  Rewritten and improved large part of battle mechanics.
  Fixed problem with line pitch setting for some rare video modes
  Rewritten a few routines related to gold gathering
  Integrated The Destiny of Ninja campaign
  Fixed truncated water drip effect
  Fixed multiplayer landview to use ENSIGN_ZOOM
  Fixed one of rendering routines to be more stable in high resolutions
  Rewritten spawning heroes
  Fixed error with drawing creature who is being sacrificed
  Fixed max zoom problem in very high resolutions
  Rewritten the keeper sprite drawing function
  Rewritten creation of shot hit effects
  
Version: 0.4.0a
  Renamed 'debug version' to 'heavylog version'
  Allowed making 'debug' version which is really with debug info

Version: 0.4.0
  Prepared game launcher with installation function
  Updated some internal mechanisms, ie. columns finding
  Made small revolution in the zooming system
  Rewritten the green/red cube (map volume box) drawing
  Fixed engine window center to be on screen center
  Introduced new config file - creature states config
  Improved room efficiency calculation
  Rewritten a few more routines related to workshop
  Fixed the sound emitter cleanup code
  Rewritten revealing map due to torture
  
Version: 0.3.9a
  Updated compound eye effect for high resolution
  Fixed the problem with Dungeon Heart background sound
  Updated reading keyboard in front view, also named some constants
  Fixed creature death kind "ice explosion" when creature is frozen
  Fixed shadows and lights affecting things
  Fixed distance computing required for certain shots to hit target
  Fixed imps to continue their jobs after they finish a part of it

Version: 0.3.9
  Fixed the v0.3.8c bug with freeze on computer digging for gold
  Updated frontend menus code, some changes in network related menus
  Updated creature statistics in Ancient Keeper
  Disabled another pathfinding routine
  Rewritten picking up trap box to fill trap
  Rewritten routine which draws things on the parchment map screen
  Added zombie players; uninitialized players are initied as zombies
  Rewritten dead bodies rotting code
  Rewritten searching for dead bodies by imps

Version: 0.3.8c
  Fixed the v0.38b bug with computer keeper not digging for gold
  Rewritten some of imps and workshop related code
  Rewritten some of imp tasks code and gold counter

Version: 0.3.8b
  Updated scripts for some maps 
  Fixed killing a creature which has an armour spell casted on
  Improved some debugging messages
  Rewritten some of entrance generation code
  Fixed bug in hero attacking creature code
  Fixed losing things due to lights overflow
  Rewritten shot reaching its target
  Added boulder immunity to creature config files

Version: 0.3.8a
  Added mouse sensitivity modification option
  Fixed crash on invalid movies video mode
  Fixed problem with creatures standing next to lair
  Fixed error in the new rooms merging algorithm
  Linked with MinGW libraries as static
  Fixed problem with no price when selling traps
  Changed tunneler "relax" sprite to the one with sleeping

Version: 0.3.8
  Switched video support library to SDL
  Fixed duplicates level when using Multiply special
  Created some 64-bit math, similarly to original code
  New pathfinding code has been disabled (needs debugging)
  Dutch land introductions were added to 3 campaigns
  New graphic modes - any colour depth is now supported
  New graphic modes - windowed mode is now supported
  Rewritten more of creature state machine
  Added more options to CFG files
  Experimental TCP/IP multiplayer support

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

Programming:
 Tomasz Lis aka Mefistotelis
 Petter Hansson

Graphics:
 Chagui
 Madkill
 Synesthesia

Part of the code closed into DLL and some data files
 are copyrighted by Bullfrog Productions.
