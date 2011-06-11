/******************************************************************************/
// Utility routines.
/******************************************************************************/
/** @file FilelistChecker.cpp
 *     Checks integrity of list of files.
 * @par Purpose:
 *     Allows checking if files exist and are in correct version.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     08 Jun 2011 - 10 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "FilelistChecker.hpp"

#include <cstddef>
#include <cstring>
#include <sys/stat.h>
#include <stdexcept>
#include <wx/log.h>

/** Files for basic check which are supplied with KeeperFX. */
const struct CheckItem supplied_basic_check[] = {
    {L"data/tmapa007.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"ldata/dkmap01.raw",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"sound/sound.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"keeperfx.exe",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {NULL,CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
};

/** Files for basic check which have to be copied from original DK. */
const struct CheckItem additional_basic_check[] = {
    {L"data/creature.jty",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"ldata/front.raw",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"sound/atmos2.sbk",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"keeperfx.exe",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {NULL,CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
};

/** Files for complete check which have to be copied from original DK. */
const struct CheckItem additional_complete_check[] = {
    {L"data/creature.jty",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"ldata/front.raw",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"sound/atmos2.sbk",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"keeperfx.exe",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {NULL,CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
};

FilelistChecker::FilelistChecker()
{
    ntotal = 0;
    nfailed = 0;
}

FilelistChecker::~FilelistChecker()
{
}

bool FilelistChecker::verifyList(const wchar_t *folder, const struct CheckItem *ciList)
{
    const struct CheckItem *cItem;
    cItem = &ciList[0];
    while (cItem->filename != NULL)
    {
        int result;
        result = verifyItem(folder,*cItem);
        if (result != CIR_Correct)
        {
            nfailed++;
            failedFiles.push_back(cItem->filename);
        }
        ntotal++;
        cItem++;
    }
    return (nfailed == 0);
}

int FilelistChecker::verifyItem(const wchar_t *folder, const struct CheckItem &cItem)
{
    std::wstring fullname;
    fullname = folder; fullname += L"/"; fullname += cItem.filename;
    try {
        if (!fileExists(fullname.c_str(),false,false))
            return CIR_NotFound;
    }
    catch (std::exception &e) {
        wxLogMessage(wxT("Error while checking \"%s\": %s"),wxString(cItem.filename).c_str(),wxString::FromAscii(e.what()).c_str());
        return CIR_AccessFail;
    }
    return CIR_Correct;
}

/**
 * Checks if file of given name exists.
 * @param filename File name and path to be checked.
 * @param acceptFolders Selects whether the function will succeed when given filename exists, but represents a folder.
 * @param acceptSpecial Selects whether the function will succeed when given filename exists, but represents a special file.
 *     Special files are Sockets, FIFOs, block devices and other things that don't really exist on disk.
 * @return
 */
bool FilelistChecker::fileExists(const wchar_t * filename, bool acceptFolders, bool acceptSpecial)
{
  struct _stat fileInfo;
  int statRet;

  // Attempt to get the file attributes
  statRet = _wstat(filename,&fileInfo);
  if(statRet == 0) {
      // We were able to get the attributes so entry obviously exists.
      // Now, it it a file or a directory?
      if (!acceptFolders)
      {
          if ((fileInfo.st_mode & S_IFMT) == S_IFDIR)
              return false;
      }
      if (!acceptSpecial)
      {
          if ( ((fileInfo.st_mode & S_IFMT) != S_IFREG) && ((fileInfo.st_mode & S_IFMT) != S_IFDIR))
              return false;
      }
      return true;
  } else {
      // We were not able to get the file attributes.
      // This may mean that we don't have permission to
      // access the folder which contains this file.
      switch (errno)
      {
      case ENOENT:
      case ENOTDIR:
          // File really doesn't exist.
          break;
      case EACCES:
          // File may exist, we just don't have permission to it, or Search permission to its path.
          break;
      case EIO:
          // An error occurred while reading from the file system.
          break;
      default:
          throw new std::runtime_error(strerror(errno));
      }
      return false;
  }
}

void FilelistChecker::clearResults(void)
{
    ntotal = 0;
    nfailed = 0;
    failedFiles.clear();
}

int FilelistChecker::getNumTotal(void)
{
    return ntotal;
}

int FilelistChecker::getNumFailed(void)
{
    return nfailed;
}

const wchar_t * FilelistChecker::getFailedFilename(int idx)
{
    return failedFiles[idx].c_str();
}
