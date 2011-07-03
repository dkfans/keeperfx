#!/bin/bash

#
# Script for building KeeperFX packages from an SVN checkout.
#
#

REV="$1"
WORKDIR=/home/tomasz/nightly/keeperfx
RESEASEDIR=/home/tomasz/nightly/releases
REPO_TRUNK=https://keeperfx.googlecode.com/svn/trunk/keeperfx/
RUN_DATE=$(date -u '+%Y.%m.%d %T')
CROSS_COMPILE_TOOLCHAIN=i586-mingw32msvc-

# Clean products of previous build
function keeperfxclean {
    if [ -d "$WORKDIR" ]; then
        echo "Cleaning previous build"
        make --directory=$WORKDIR clean CROSS_COMPILE=${CROSS_COMPILE_TOOLCHAIN}
        echo "Cleaning finished"
    fi
}

# Updates local KeeperFX sources
function keeperfxcheckout {
    # Check out or update the svn checkout
    if [ -d "$WORKDIR" ]; then
            if [ -z "${REV}" -o 0"${REV}" -lt 1 ]; then
                echo "Updating svn checkout"
                SVNLOG=`svn update "$WORKDIR"`
            else
                echo "Updating svn checkout to r${REV}"
                SVNLOG=`svn update -r "${REV}" "$WORKDIR"`
            fi
            if [ $? != 0 ]; then
                echo "${SVNLOG}"
                echo "Problem with svn update"
                return $?
            fi
            svn revert -R "$WORKDIR"
            if [ $? != 0 ]; then
                echo "Problem with svn revert"
                return $?
            fi
    else
            if [ -z "${REV}" -o 0"${REV}" -lt 1 ]; then
                echo "Checking out svn to $WORKDIR"
                SVNLOG=`svn checkout $REPO_TRUNK "$WORKDIR"`
            else
                echo "Checking out svn r${REV} to $WORKDIR"
                SVNLOG=`svn checkout -r "$REV" $REPO_TRUNK "$WORKDIR"`
            fi
            if [ $? != 0 ]; then
                echo "$SVNLOG"
                echo "Problem with svn checkout"
                return $?
            fi
    fi
    echo "$SVNLOG"
    REV=`echo "$SVNLOG" | grep -i revision | sed -e 's/[^0-9]\\+//g'`
    echo "SVN Revision ${REV}"
}

# Builds the KeeperFX package
function keeperfxbuild {
    if [ ! -d "$WORKDIR" ]; then
        echo "There's no code to build"
        return
    fi

    echo "Update build version to r${REV}"
    sed -i \
        -e "s,VER_BUILD=.\+,VER_BUILD=${REV}," \
        $WORKDIR/version.mk

    # Build
    echo "Building KeeperFX standard"
    make --directory=$WORKDIR standard CROSS_COMPILE=${CROSS_COMPILE_TOOLCHAIN}
    if [ "$?" -ne 0 ]; then
        echo "Build 'standard' error."
        return
    fi
    echo "Building KeeperFX heavylog"
    make --directory=$WORKDIR heavylog CROSS_COMPILE=${CROSS_COMPILE_TOOLCHAIN}
    if [ "$?" -ne 0 ]; then
        echo "Build 'heavylog' error."
        return
    fi
}

function keeperfxpublish {
    echo "Creating archive"
    make --directory=$WORKDIR package CROSS_COMPILE=${CROSS_COMPILE_TOOLCHAIN}
    if [ "$?" -ne 0 ]; then
        echo "Build 'package' error."
        return
    fi
    echo "Searching for archives"
    PKGFILES="${WORKDIR}/pkg/*.7z"
    for PKGFILE in $PKGFILES
    do
        PKGCOPY="${RESEASEDIR}/${PKGFILE##*/}"
        echo "Publishing '$PKGFILE' as '$PKGCOPY'"
        # Copy package to website
        cp "$PKGFILE" "${PKGCOPY}"
        if [ "$?" -ne 0 ]; then
            echo "Package copy error; it might already exist."
            return
        fi
        if [ ! -f "$PKGCOPY" ]; then
            echo "Package copy not found even tho no 'cp' error."
            return
        fi
        # update website packages list
        if [ ! -f "${RESEASEDIR}/list.txt" ]; then
            touch "${RESEASEDIR}/list.txt"
        fi
        # Add new line to list file
        sed "1i${REV} ${RUN_DATE} ${PKGFILE##*/}" "${RESEASEDIR}/list.txt" > "${RESEASEDIR}/list_new.txt"
        # Remove lines over 16
        sed -n -e '1,16p' "${RESEASEDIR}/list_new.txt" > "${RESEASEDIR}/list.txt"
    done
}

# Actually build the packages.
echo "Starting KeeperFX build on ${RUN_DATE}"
    keeperfxclean
    keeperfxcheckout
    keeperfxbuild
    keeperfxpublish
