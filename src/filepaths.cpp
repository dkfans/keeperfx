/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file filepaths.cpp
 *     File path resolution support.
 * @par Purpose:
 *     Resolving file paths for various file groups.
 * @par Comment:
 *     None.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "pre_inc.h"
#include "post_inc.h"

enum file_priority {
    FP_Base = 0,
    FP_Mod_after_base,
    FP_Campaign,
    FP_Mod_after_campaign,
    FP_Map,
    FP_Mod_after_map,
};

struct FileInfo {
    std::string fpath;
    file_priority priority;
    
};


std::array<std::map<std::string, std::vector<FileInfo>>, FGrp_MpLevels> file_map;


static void load_dir(const char *dir_path,short fgroup)
{
    SYNCDBG(8, "Starting");
    if (dir_path == NULL || dir_path[0] == 0) {
        return;
    }
    char full_path[1024] = {0};
    sprintf(full_path, "%s/*", dir_path);
    struct TbFileEntry fe;
    struct TbFileFind *ff = LbFileFindFirst(full_path, &fe);
    if (ff) {
        do {
            sprintf(full_path, "%s/%s", dir_path, fe.Filename);
            if (fe.Attributes & TbFileAttr_Directory) {
                continue;
            }
            struct FileInfo fi;
            fi.fpath = full_path;
            fi.priority = FP_Base;
            file_map[fgroup][tolower(fe.Filename)].push_back(fi);
        } while (LbFileFindNext(ff, &fe) >= 0);
        LbFileFindEnd(ff);
    }
}


void build_file_map()
{
    file_map.clear();


}

char *prepare_file_path_m(short fgroup, const char *fname)
{
  struct FileInfo *fi = &file_map[fgroup][tolower(fname)][0];
  
  return fi->fpath.c_str();

}

char *prepare_file_fmtpath_m(short fgroup, int flags, const char *fmt_str, ... )
{
  char fname[255] = "";
  va_list arg;
  va_start(arg, fmt_str);
  vsnprintf(fname, sizeof(fname), fmt_str, arg);
  va_end(arg);

  return prepare_file_path_m(fgroup, fname);

}

/******************************************************************************/
