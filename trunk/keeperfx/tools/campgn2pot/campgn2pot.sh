#!/bin/bash
#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file campgn2pot.sh
#      A shell script for preparing new translations of KeeperFX campaigns.
#  @par Purpose:
#      Level creators often use QUICK_* commands to display in-game messages
#      during the level gameplay. While this method works, it is not recommended 
#      for KeeperFX because it doesn't allow translation to other languages.
#      This script reads QUICK_* commands from level scripts and consolidates them
#      into a file which can be easily modified to become .pot file - translations
#      template. When the .pot file is ready, it can be converted into .dat file
#      which the game uses for string translations.
#  @par Comment:
#      None
#  @author   Tomasz Lis
#  @date     01 Jul 2013 - 03 Nov 2013
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#******************************************************************************

# Find all QUICK_* commands in scripts
find . -name "map*.txt" | sort | xargs sed -n "s/^[[:space:]]*QUICK\_\(INFORMATION\|OBJECTIVE\)(.*\"\(.*\)\".*)/\2/p" > tmp_msg.txt
# Find all level names in .lif files
find . -name "map*.lif" | sort | xargs sed -n "s/^[[:space:]]*[0-9]*,[[:space:]]*\(.*\)$/\1/p" > tmp_lvn.txt

N=1
awk ' !x[$0]++' tmp_msg.txt | while read LINE ; do
    STR=`echo -n $LINE | tr -d "\n" | tr -d "\0" | tr -d "\015"`
    echo "#: guitext:${N}"
    echo "msgctxt \"In-game message\""
    echo "msgid \"${STR}\""
    echo "msgstr \"\""
    echo ""
    N=$(( $N + 1 ))
done

while [ $N -lt 201 ]; do
    echo "#: guitext:${N}"
    N=$(( $N + 1 ))
done
    echo "msgctxt \"In-game message\""
    echo "msgid \"Moo\""
    echo "msgstr \"\""
    echo ""

N=$(( $N + 1 ))
awk ' !x[$0]++' tmp_lvn.txt | while read LINE ; do
    STR=`echo -n $LINE | tr -d "\n" | tr -d "\0" | tr -d "\015"`
    echo "#: guitext:${N}"
    echo "msgctxt \"Level name\""
    echo "msgid \"${STR}\""
    echo "msgstr \"\""
    echo ""
    N=$(( $N + 1 ))
done

while [ $N -lt 226 ]; do
    echo "#: guitext:${N}"
    N=$(( $N + 1 ))
done
    echo "msgctxt \"Level name\""
    echo "msgid \"Unused\""
    echo "msgstr \"\""
    echo ""
