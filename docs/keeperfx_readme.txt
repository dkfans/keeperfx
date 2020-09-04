Dungeon Keeper Fan Expansion
------------------------------

KeeperFX is an extensive mod for Dungeon Keeper.

  It is written by fans and not supported by the original developer.
  It contains some of original Dungeon Keeper data, but many files
  are also modified or remade.
  You can use your original disk, or the GOG/Origins versions of the 
  game to complete the installation. This fan expansion offers  bug 
  fixes, quality of live improvements and new content, while 
  retaining the feel of the original game.

Installation of KeeperFX CCP:

  You need an original version of Dungeon Keeper - a CD (or CD image) 
  or the installed GOG or Origin versions - to perform a 
  complete installation.

  To install KeeperFX CCP, you have to unpack the archive to
  your desired target location, and then run "launcher.exe".
  The Launcher utility will allow you to select source folder
  from which original DK files will be taken. To select the
  "keeper" folder and start copying files, press "Install" button.

  When using a CD version you must select the "keeper" folder from the CD 
  content list. Selecting the root folder installation folder on disk, 
  won't be enough. For the version from GOG.com, the installation folder
  will be usable. On the E.A. Origin version you need to select the 
  'data' sub-folder

  If you wish to change language, click "Settings" in the launcher
  and select it from list. Remember to save your changes. You may
  also edit "keeperfx.cfg" by hand, using text editor.

  Press the correct button inside Launcher to start the game.
  For information about running the game executable directly,
  see 'running KeeperFX' section. If something doesn't work,
  see the 'troubleshooting' section.

Available languages:

  The following languages are currently functional:
    ENG ITA FRE SPA DUT GER POL SWE JPN RUS CZE KOR
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
  not planning reporting any errors. Zip the log files before
  sending them.

  Both versions will recognize all command line options mentioned
  below.

Command line options:

  KeeperFX accepts many command line options, which you can type
  while starting the game, or select from launcher.
  All the options are listed and described on Wiki page:
  https://github.com/dkfans/keeperfx/wiki/Command-Line-Options

Troubleshooting:

  There's a faq found on Wiki page:
  https://github.com/dkfans/keeperfx/wiki/Troubleshooting
  
  If your problem is not listed there, open keeperfx.log with a text 
  editor and read the first and last few lines. Often the error is in
  plain english and tells you what to do.
  
  Otherwise look for solutions to your problem at:
  https://keeperklan.com/
  
  If this fails, ask for help on keeperklan, or the discord:
  https://discord.gg/zKTjfDh

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

New in-game commands:

  Most of the controls can be found in the official manual. The rest here:
  https://github.com/dkfans/keeperfx/wiki/New-In-Game-Commands

New and modified level script commands:

  The commands list has been moved to Wiki, to make it easier to maintain:
  https://github.com/dkfans/keeperfx/wiki/New-and-Modified-Level-Script-Commands

Changelog:

  The full changelog can be found here:
  https://github.com/dkfans/keeperfx/commits/master
  
  A summary can be read on keeperklan:
  https://keeperklan.com/threads/627-Releases-and-news

Part of the code closed into DLL and some data files
 are copyrighted by Bullfrog Productions.
