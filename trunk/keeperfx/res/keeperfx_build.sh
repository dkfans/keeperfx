#!/bin/bash

#
# Script for building KeeperFX packages from an SVN checkout.
#
#

REV="$1"
DIR=/home/tomasz/nightly/keeperfx
REPO_TRUNK=https://keeperfx.googlecode.com/svn/trunk/keeperfx/
VERSION="0.0.0.0"

# Updates local KeeperFX sources
function keeperfxclean {
    if [ -d "$DIR" ]; then
        echo "Cleaning previous build"
        make --directory=$DIR clean
        echo "Cleaning finished"
    fi
}

# Updates local KeeperFX sources
function keeperfxcheckout {
    # Check out or update the svn checkout
    if [ -d "$DIR" ]; then
            if [ -z "$REV" -o 0"$REV" -lt 1 ]; then
                echo "Updating svn checkout"
                REL=`svn update "$DIR"`
            else
                echo "Updating svn checkout to r$REV"
                REL=`svn update -r "$REV" "$DIR"`
            fi
    else
            if [ -z "$REV" -o 0"$REV" -lt 1 ]; then
                echo "Checking out svn to $DIR"
                REL=`svn checkout $REPO_TRUNK "$DIR"`
            else
                echo "Checking out svn r$REV to $DIR"
                REL=`svn checkout -r "$REV" $REPO_TRUNK "$DIR"`
            fi
    fi
    if [ $? != 0 ]; then
        echo "Problem with svn checkout"
        return $?
    fi
    REL=`echo "$REL" | grep -i revision | sed -e 's/[^0-9]\\+//g'`
    echo "SVN Revision $REL"
}

# Builds the KeeperFX package
function keeperfxbuild {
    if [ ! -d "$DIR" ]; then
        echo "There's no code to build"
        return
    fi

    echo "Update build version to r$REL"
    sed -i \
        -e "s,VER_BUILD=.\+,VER_BUILD=$REL," \
        $DIR/version.mk

    # Build
    echo "Building KeeperFX debug"
    make --directory=$DIR debug CROSS_COMPILE=i586-mingw32msvc-
    # Error?
    if [ "$?" -ne 0 ]; then
        echo "Build error."
        return
    fi
}

function keeperfxpublish {
    echo "Creating archive"
    make --directory=$DIR package
    echo "Publishing archive"
}

# Actually build the packages.
    keeperfxcheckout
    keeperfxbuild
    keeperfxpublish
