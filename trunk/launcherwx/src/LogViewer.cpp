/******************************************************************************/
// Utility routines.
/******************************************************************************/
/** @file LogViewer.cpp
 *     A frame which allows browsing large text files.
 * @par Purpose:
 *     Loads log file and allows viewing its lines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Jun 2011 - 24 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "LogViewer.hpp"

#include <cstddef>
#include <cstring>
#include <sys/stat.h>
#include <stdexcept>
#include <wx/log.h>
#include <wx/filefn.h>

LogViewer::LogViewer()
{
}

LogViewer::~LogViewer()
{
}
/******************************************************************************/
