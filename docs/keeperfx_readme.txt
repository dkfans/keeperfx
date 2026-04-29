Dungeon Keeper Fan Expansion
------------------------------

KeeperFX is an extensive mod for Dungeon Keeper.

  It is written by fans and not supported by the original developer.
  It contains some of the original Dungeon Keeper data, but many files
  are also modified or remade.
  You can use your original disk, or the GOG/Steam/Origin versions of 
  the game to complete the installation. This fan expansion offers
  bug fixes, quality of live improvements and new content while 
  retaining the feel of the original game.

Installation of KeeperFX:

  You need an original version of Dungeon Keeper - a CD (or CD image) 
  or the installed GOG, Steam or Origin versions - to perform a 
  complete installation.

  To install KeeperFX, unpack the archive to your desired target location
  and run "keeperfx-launcher-qt.exe".
  It will automatically detect a source folder from which original DK files
  will be taken. Follow the on screen instructions.
  
  Users that do not use Windows 10 or newer will have to use "launcher.exe"
  instead. This legacy launcher will allow you to install
  by selecting a source folder. Click 'Installation' then find 
  the "keeper" folder with the required files.
  When using a CD version you must select the "keeper" folder from the CD 
  content list. Selecting the root folder installation folder on disk 
  won't be enough. 
  GOG: use the installation folder.
  Origin: select the 'data' sub-folder. 
  Steam: find it in steam/steamapps/common/Dungeon Keeper/.

  Changing the language of both the game and launcher is possible.

  Press the correct button inside Launcher to start the game.

  For information about running the game executable directly,
  see 'running KeeperFX' section. If something doesn't work,
  see the 'troubleshooting' section.

Available languages:

  The following languages are fully functional:
    ENG ITA FRE SPA DUT GER POL SWE JPN RUS CZE KOR CHI
  The following languages are partially functional:
    CHT LAT UKR POR
  Note that some campaigns may not support your language.
  In that case the default language will be used.


Running KeeperFX:

  To start the game, run the launcher and press 'Play'. Users of the 
  legacy launcher press 'Start game'. You can also run "keeperfx.exe"
  directly.

  If you want to report any errors you encounter, you may 
  run "keeperfx_hvlog.exe" instead. 
  This will run a "heavylog version", which writes a lot of information
  into "keeperfx.log". In case of the game hanging
  on suddenly disappearing, you may send the last few lines of the
  generated LOG to the authors with your description of the bug.

  Note that "keeperfx_hvlog.exe" may be slow even on new computers.

  Also, the generated LOG file may be very large. After
  a few hours of play it will be several hundred megabytes large.
  Use the standard "keeperfx.exe" if you're not experiencing any errors.
  Zip the log files before sending them.

  Both versions will recognize all command line options mentioned
  below.

Troubleshooting:

  There's an FAQ found on the Wiki page:
  https://github.com/dkfans/keeperfx/wiki/Troubleshooting
  
  If your problem is not listed there, open keeperfx.log with a text 
  editor and read the first and last few lines. Often the error is in
  plain English and tells you what to do.
  
  Otherwise look for solutions to your problem on:
  https://keeperklan.com/
  
  If this fails, ask for help on keeperklan, or the Discord:
  https://discord.gg/zKTjfDh

Command line options:

  KeeperFX accepts many command line options, which you can type
  while starting the game, or select from the launcher.
  All the options are listed and described on this Wiki page:
  https://github.com/dkfans/keeperfx/wiki/Command-Line-Options

Reporting a bug:

  If you've found a bug in the game, you may report it to the KeeperFX developers.
  But you will have to to do some tests to gather as much information as
  possible about the problem.

  First, if the game crashed, try looking into 'keeperfx.log'. If there are
  error messages in it, it's possible that you haven't properly installed or
  configured KeeperFX. In this case, check the 'Troubleshooting' section for a 
  description of your problem. Note that running the game again will overwrite
  the LOG file. If you want to keep it, you'll have to make a copy.

  The second step is to try reproducing the error and generate a more detailed log.
  Run 'keeperfx_hvlog.exe' and play the level again, doing similar things you did the first time to check if it crashes. If you can't reproduce the error, there is

  still a chance that the LOG file from the first crash is enough to locate the
  problem. Post the copy you've made on the issue tracker with your description
  of the problem and information that you couldn't reproduce it.

  If you were able to reproduce the error, please post a description of how
  to do it on the issue tracker. Remember to include the LOG file created by the 
  'heavylog' version of KeeperFX. Note that the log file will be huge - you
  shouldn't attach it directly. Instead, you can compress it, or just paste
  a few (ie. 20) lines from its beginning and its end. Remember to include the first
  and last line of the LOG - these are crucial, and doing it incorrectly would
  mislead the developers.
  If it is possible to reproduce the error by loading a specific saved game and
  doing a few simple actions, then attach the saved game to your report. You can
  recognize the file which contains a specific saved game by its number in the filename, which is always equal to the position of the saved game slot in the 'load game' menu.


New in-game commands:

  Most of the controls can be found in the official manual. The rest here:
  https://github.com/dkfans/keeperfx/wiki/New-Game-Controls-and-Commands

Changelog:

  The full changelog can be found here:
  https://github.com/dkfans/keeperfx/commits/master
  
  A summary can be read here:
  https://github.com/dkfans/keeperfx/wiki/History-of-KeeperFX-releases

Some data files are copyrighted by Bullfrog Productions.