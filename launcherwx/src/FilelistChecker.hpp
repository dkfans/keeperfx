/******************************************************************************/
// Utility routines.
/******************************************************************************/
/** @file FilelistChecker.hpp
 *     Header file for FilelistChecker.cpp.
 * @par Purpose:
 *     Checks integrity of list of files.
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
#ifndef FILELISTCHECKER_HPP_
#define FILELISTCHECKER_HPP_

#include <string>
#include <vector>
#include <wx/file.h>

enum CheckItemFlags {
    CIF_Default = 0x00,
    CIF_NotMandatory = 0x01,
    CIF_VerifyChecksum = 0x02,
};

enum CheckItemResult {
    CIR_Correct = 0,
    CIR_NotFound,
    CIR_AccessFail,
    CIR_BadChecksum,
};

struct CheckItem {
    const wchar_t *filename;
    unsigned short flags;
    unsigned char md5sum[16];
};

class wxFile;

class FilelistChecker
{
public:
    FilelistChecker();
    virtual ~FilelistChecker();
    bool verifyList(const wchar_t *folder, const struct CheckItem *ciList);
    void clearResults(void);
    int getNumTotal(void);
    int getNumFailed(void);
    const wchar_t * getFailedFilename(int idx);
    bool copyFilesList(const wchar_t *dstFolder, const struct CheckItem *dstList, const wchar_t *srcFolder, const struct CheckItem *srcList);
    bool copyFile(const wchar_t *dstFolder, const struct CheckItem &dstItem, const wchar_t *srcFolder, const struct CheckItem &srcItem);
private:
    int verifyItem(const wchar_t *folder, const struct CheckItem &cItem);
    static bool fileExists(const wchar_t * filename, bool acceptFolders, bool acceptSpecial);
    static bool doCopyFile(wxFile& fileIn, const wxStructStat& fbuf, const wxString& filenameDst, bool overwrite);
    static void pathReplaceAll(std::wstring* str, const std::wstring& from, const std::wstring& to);
    int ntotal;
    int nfailed;
    std::vector<std::wstring> failedFiles;
};

extern const struct CheckItem supplied_basic_check[];
extern const struct CheckItem additional_basic_check[];
extern const struct CheckItem additional_complete_check[];

/******************************************************************************/
#endif /* FILELISTCHECKER_HPP_ */
