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
    {L"data/creature.jty",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/tmapa007.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"ldata/dkflag00.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"fxdata/gtext_eng.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"sound/sound.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"keeperfx.exe",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {NULL,CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
};

/** Files for basic check which have to be copied from original DK. */
const struct CheckItem additional_basic_check[] = {
    {L"data/bluepal.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/slab0-1.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/vampal.pal",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"sound/atmos2.sbk",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {NULL,CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
};

/** Files for complete check which have to be copied from original DK. */
const struct CheckItem additional_complete_check[] = {
    {L"data/bluepal.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/bluepall.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/redpal.col",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/redpall.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/whitepal.col",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/lightng.pal",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/hitpall.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/dogpal.pal",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/vampal.pal",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/slab0-0.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"data/slab0-1.dat",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"sound/atmos1.sbk",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"sound/atmos2.sbk",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
    {L"sound/bullfrog.sbk",CIF_Default,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} },
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

bool FilelistChecker::copyFilesList(const wchar_t *dstFolder, const struct CheckItem *dstList, const wchar_t *srcFolder, const struct CheckItem *srcList)
{
    const struct CheckItem *dstItem;
    const struct CheckItem *srcItem;
    dstItem = &dstList[0];
    srcItem = &srcList[0];
    while (dstItem->filename != NULL)
    {
        int result;
        result = copyFile(dstFolder,*dstItem,srcFolder,*srcItem);
        if (!result)
        {
            nfailed++;
            failedFiles.push_back(srcItem->filename);
        }
        ntotal++;
        dstItem++;
        srcItem++;
    }
    return (nfailed == 0);
}

/** Copy file - internal helper.
 *
 * @param fileIn Already opened source file.
 * @param fbuf
 * @param filenameDst Destination file with path.
 * @param overwrite
 * @return
 */
bool FilelistChecker::doCopyFile(wxFile& fileIn, const wxStructStat& fbuf, const wxString& filenameDst, bool overwrite)
{
    // reset the umask as we want to create the file with exactly the same
    // permissions as the original one
    wxCHANGE_UMASK(0);

    // create file2 with the same permissions than file1 and open it for
    // writing

    wxFile fileOut;
    if ( !fileOut.Create(filenameDst, overwrite, fbuf.st_mode & 0777) )
        return false;

    // copy contents of file1 to file2
    char buf[4096];
    for ( ;; )
    {
        ssize_t count = fileIn.Read(buf, WXSIZEOF(buf));
        if ( count == wxInvalidOffset )
            return false;

        // end of file?
        if ( !count )
            break;

        if ( fileOut.Write(buf, count) < (size_t)count )
            return false;
    }

    // we can expect fileIn to be closed successfully, but we should ensure
    // that fileOut was closed as some write errors (disk full) might not be
    // detected before doing this
    return fileIn.Close() && fileOut.Close();
}

bool FilelistChecker::copyFile(const wchar_t *dstFolder, const struct CheckItem &dstItem, const wchar_t *srcFolder, const struct CheckItem &srcItem)
{
    std::wstring tmpName;
    std::wstring dstFullName;
    std::wstring srcFullName;
    tmpName = L"/"; tmpName += dstItem.filename;
#ifdef __WIN32__
    pathReplaceAll(&tmpName, L"/", L"\\");
#endif
    dstFullName = dstFolder; dstFullName += tmpName;
    tmpName = L"/"; tmpName += srcItem.filename;
#ifdef __WIN32__
    pathReplaceAll(&tmpName, L"/", L"\\");
#endif
    srcFullName = srcFolder; srcFullName += tmpName;
    try {
        wxStructStat fbuf;
        // get permissions of srcFullName
        if ( wxStat( srcFullName, &fbuf) != 0 )
        {
            // the file probably doesn't exist or we haven't the rights to read
            // from it anyhow
            wxLogSysError(wxT("Couldn't get permissions for source file \"%s\", check if it exists."), srcItem.filename);
            return false;
        }

        // open srcFullName for reading
        wxFile fileIn(srcFullName, wxFile::read);
        if ( !fileIn.IsOpened() )
            return false;

        // remove dstFullName, if it exists. This is needed for creating
        // dstFullName with the correct permissions in the next step
        if ( wxFileExists(dstFullName)  && (!wxRemoveFile(dstFullName)))
        {
            wxLogSysError(wxT("Couldn't overwrite file \"%s\", it might be read-only."), dstItem.filename);
            return false;
        }

        if (!doCopyFile(fileIn, fbuf, dstFullName, true))
            return false;
    }
    catch (std::exception &e) {
        wxLogMessage(wxT("Error while copying \"%s\": %s"),wxString(srcItem.filename).c_str(),wxString::FromAscii(e.what()).c_str());
        return false;
    }
    return true;
}

/** Replace all instances of given substring with another substring.
 *
 * @param str The string to be modified.
 * @param from The string to be replaced.
 * @param to The new string to be replaced.
 */
void FilelistChecker::pathReplaceAll(std::wstring* str, const std::wstring& from, const std::wstring& to)
{
    std::wstring::size_type pos(0);
    while ((pos = str->find(from, pos)) != std::wstring::npos) {
        str->replace(pos, from.size(), to);
        pos += to.size();
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
/******************************************************************************/
