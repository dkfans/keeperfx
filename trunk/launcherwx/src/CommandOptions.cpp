/******************************************************************************/
// Utility routines.
/******************************************************************************/
/** @file CommandOptions.cpp
 *     A frame which allows changing command line options.
 * @par Purpose:
 *     Generates command line string for running the game.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Jun 2011 - 12 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "CommandOptions.hpp"

#include <cstddef>
#include <cstring>
#include <sys/stat.h>
#include <stdexcept>
#include <wx/log.h>
#include <wx/filefn.h>

CommandOptions::CommandOptions()
{
}

CommandOptions::~CommandOptions()
{
}
/******************************************************************************/
