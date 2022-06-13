taskkill /f /fi "windowtitle eq Dungeon Keeper FX"
taskkill /fi "windowtitle eq KeeperFX Debugging Window"
title KeeperFX Debugging Window
wsl cd "/mnt/d/keeperfx-source"; make standard
if not %errorlevel% equ 0 (echo Compilation failed && pause && exit)
COPY /Y "D:\keeperfx-source\bin\"* "D:\DungeonKeeper\"
cd /D "D:\DungeonKeeper\"
start keeperfx.exe -level 00001 -campaign personal -nointro -alex
break>"D:\DungeonKeeper\keeperfx.log"
wsl less +F /mnt/d/DungeonKeeper/keeperfx.log