#!/bin/bash
#******************************************************************************
#  Free implementation of Bullfrog's Dungeon Keeper strategy game.
#******************************************************************************
#   @file keeperfx_build.sh
#      A shell script for preparing nightly builds of KeeperFX.
#  @par Purpose:
#      Makes SVN checkout and prepares release of KeeperFX. If invoked every
#      night, gives a nightly build system with releases accessible through
#      HTML server.
#  @par Comment:
#      Usage with 'cron' - to load the job:
#        crontab keeperfx_build.cron
#      To remove the job:
#        crontab -r
#  @author   Tomasz Lis
#  @date     01 Jul 2011 - 04 Jul 2015
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#******************************************************************************

REPO_TRUNK=https://github.com/mefistotelis/keeperfx.git
# Set values of env. variables to default if they're not set
HOMEDIR=${HOMEDIR:-~}
WORKDIR=${WORKDIR:-${HOMEDIR}/nightly/keeperfx}
RESEASEDIR=${RESEASEDIR:-${HOMEDIR}/public_html/keeper/nightly}
CROSS_COMPILE_TOOLCHAIN=${CROSS_COMPILE_TOOLCHAIN:-i586-mingw32msvc-}

LANG=C
HASH="$1"
REV=0
RUN_DATE=$(date -u '+%Y-%m-%d %T')

# Clean products of previous build
function keeperfxclean {
    if [ -d "$WORKDIR" ]; then
        echo "Cleaning previous build"
        make --directory=$WORKDIR clean CROSS_COMPILE=${CROSS_COMPILE_TOOLCHAIN}
        echo "Cleaning finished"
        return 0
    fi
    return 1
}

# Updates local KeeperFX sources
function keeperfxcheckout {
    # Check out or update the svn checkout
    if [ -d "$WORKDIR" ]; then
            echo "Fetching any new commits from remote repository"
            GITLOG=`cd "$WORKDIR" && git fetch origin`
            if [ $? != 0 ]; then
                echo "Problem with git fetch"
                return 1
            fi
            if [ -z "${HASH}" ]; then
                echo "Updating local copy to HEAD"
                #GITLOG=`cd "$WORKDIR" && git merge FETCH_HEAD`
                GITLOG=`cd "$WORKDIR" && git reset --hard FETCH_HEAD`
            else
                echo "Updating local copy to commit ${HASH}"
                GITLOG=`cd "$WORKDIR" && git reset --hard ${HASH}`
            fi
            if [ $? != 0 ]; then
                echo "${GITLOG}"
                echo "Problem with git reset"
                return 1
            fi
    else
            echo "Clonong remote git repo to $WORKDIR"
            GITLOG=`git clone --no-checkout $REPO_TRUNK "$WORKDIR"`
            if [ $? != 0 ]; then
                echo "Problem with git clone"
                return 1
            fi
            if [ -z "${HASH}" ]; then
                echo "Checking out HEAD to $WORKDIR"
                GITLOG=`cd "$WORKDIR" && git reset --hard HEAD`
            else
                echo "Checking out commit ${HASH} to $WORKDIR"
                GITLOG=`cd "$WORKDIR" && git reset --hard ${HASH}`
            fi
            if [ $? != 0 ]; then
                echo "$GITLOG"
                echo "Problem with git reset"
                return 1
            fi
    fi
    echo "$GITLOG"
    HASH=`cd "$WORKDIR" && git rev-parse HEAD`
    REV=`cd "$WORKDIR" && git rev-list ${HASH} --count`
    echo "GIT Hash ${HASH} Revision ${REV}"
    return 0
}

function keeperfxcancel {
    PKGFILES="${RESEASEDIR}/keeperfx*_${REV}-patch.7z"
    NOTEXISTS=1; for FILE in ${PKGFILES}; do test -f "${FILE}"; NOTEXISTS=$?; break; done
    if [ ! $NOTEXISTS -eq 0 ] ; then
        echo "No package like '${PKGFILES}', proceeding with build."
        return 0
    fi
    echo "Package of r${REV} already exists."
    return 1
}

# Builds the KeeperFX package
function keeperfxbuild {
    if [ ! -d "$WORKDIR" ]; then
        echo "There's no code to build"
        return 1
    fi

    echo "Update build version to r${REV}"
    sed -i \
        -e "s,VER_BUILD=.\+,VER_BUILD=${REV}," \
        $WORKDIR/version.mk

    # For old versions of GCC (< 4.7)
    #sed -i \
    #    -e "s,-std=c[+]+11,-std=c++0x," \
    #    $WORKDIR/tools/png2bestpal/Makefile
    #sed -i \
    #    -e "s,-std=c[+]+11,-std=c++0x," \
    #    $WORKDIR/tools/pngpal2raw/Makefile
    #sed -i \
    #    -e "s,-std=c[+]+11,-std=c++0x," \
    #    $WORKDIR/tools/po2ngdat/Makefile

    # Build
    echo "Building KeeperFX standard"
    make --directory=$WORKDIR standard CROSS_COMPILE=${CROSS_COMPILE_TOOLCHAIN}
    if [ "$?" -ne 0 ]; then
        echo "Build 'standard' error."
        return 1
    fi
    echo "Building KeeperFX heavylog"
    make --directory=$WORKDIR heavylog CROSS_COMPILE=${CROSS_COMPILE_TOOLCHAIN}
    if [ "$?" -ne 0 ]; then
        echo "Build 'heavylog' error."
        return 1
    fi
    return 0
}

function keeperfxpublish {
    echo "Creating archive"
    make --directory=$WORKDIR package CROSS_COMPILE=${CROSS_COMPILE_TOOLCHAIN}
    if [ "$?" -ne 0 ]; then
        echo "Build 'package' error."
        return 1
    fi
    echo "Searching for archives"
    PKGFILES="${WORKDIR}/pkg/*.7z"
    for PKGFILE in $PKGFILES
    do
        PKGCOPY="${RESEASEDIR}/${PKGFILE##*/}"
        echo "Publishing '$PKGFILE' as '$PKGCOPY'"
        # Copy package to website
        cp -n "$PKGFILE" "${PKGCOPY}"
        if [ "$?" -ne 0 ]; then
            echo "Package copy error; it might already exist."
            return 1
        fi
        if [ ! -f "$PKGCOPY" ]; then
            echo "Package copy not found even tho no 'cp' error."
            return 1
        fi
        # update website packages list
        if [ ! -f "${RESEASEDIR}/list.txt" ]; then
            echo "" > "${RESEASEDIR}/list.txt"
        fi
        # Add new line to list file
        sed "1i${REV} ${RUN_DATE} ${PKGFILE##*/}" "${RESEASEDIR}/list.txt" > "${RESEASEDIR}/list_new.txt"
        # Remove lines over 16 (disabled - don't remove any lines)
        #sed -n -e '1,16p' "${RESEASEDIR}/list_new.txt" > "${RESEASEDIR}/list.txt"
        cp -f "${RESEASEDIR}/list_new.txt" "${RESEASEDIR}/list.txt"
    done
    return 0
}

# Actually build the packages.
echo "Starting KeeperFX build on ${RUN_DATE}"
    keeperfxclean
    keeperfxcheckout
    if [ "$?" -ne 0 ]; then
        echo "Automatic build failed to checkout repository!"
        exit 2
    fi
    keeperfxcancel
    if [ "$?" -ne 0 ]; then
        echo "Automatic build cancelled."
        exit 0
    fi
    keeperfxbuild
    if [ "$?" -ne 0 ]; then
        echo "Automatic build failed to make executables!"
        exit 3
    fi
    keeperfxpublish
    if [ "$?" -ne 0 ]; then
        echo "Automatic build failed to publish package!"
        exit 4
    fi
