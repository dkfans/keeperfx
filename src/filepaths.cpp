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
#include "filepaths.h"

#include <array>
#include <map>
#include <string>
#include <vector>
#include <stdarg.h>
#include <cstring>
#include <cstdio>

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "config.h"
#include "config_campaigns.h"
#include "config_keeperfx.h"
#include "config_mods.h"
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

static const char* fgroup_names[] = {
        NULL,       //FGrp_None,
        "data",     //FGrp_StdData,
        "fxdata",   //FGrp_FxData,
        "ldata",    //FGrp_LoData,
        "levels",   //FGrp_VarLevels,
        "save",     //FGrp_Save,
        "scrshots", //FGrp_SShots,
        "sound",    //FGrp_LrgSound,
        NULL,       //FGrp_AtlSound,
        NULL,       //FGrp_Main,
        NULL,       //FGrp_Campgn,
        NULL,       //FGrp_CmpgLvls,
        NULL,       //FGrp_LandView,
        "creatrs",  //FGrp_CrtrData,
        NULL,       //FGrp_CmpgCrtrs,
        NULL,       //FGrp_CmpgConfig,
        NULL,       //FGrp_CmpgMedia,
        "music",    //FGrp_Music,
        NULL,       //FGrp_MpLevels,
};


static const short base_fgroups[] = {
    FGrp_StdData,
    FGrp_FxData,
    FGrp_LoData,
    FGrp_VarLevels,
    FGrp_Save,
    FGrp_SShots,
    FGrp_LrgSound,
    FGrp_CrtrData,
    FGrp_Music,
};

// +1 so that FGrp_MpLevels itself is a valid index
static std::array<std::map<std::string, std::vector<FileInfo>>, FGrp_MpLevels + 1> file_map;

static std::string to_lower_str(const char *s)
{
    std::string result(s ? s : "");
    for (auto &c : result)
        c = (char)tolower((unsigned char)c);
    return result;
}

static void load_dir(const char *dir_path, short fgroup, file_priority priority)
{
    SYNCDBG(8, "Starting");
    if (dir_path == NULL || dir_path[0] == '\0') {
        return;
    }
    char full_path[2048];
    snprintf(full_path, sizeof(full_path), "%s/*", dir_path);
    struct TbFileEntry fe;
    struct TbFileFind *ff = LbFileFindFirst(full_path, &fe);
    if (ff) {
        do {
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, fe.Filename);
            struct FileInfo fi;
            fi.fpath = full_path;
            fi.priority = priority;
            file_map[fgroup][to_lower_str(fe.Filename)].push_back(fi);
        } while (LbFileFindNext(ff, &fe) >= 0);
        LbFileFindEnd(ff);
    }
}

static void load_dirs(const char *base_dir_path, file_priority priority)
{
    for (int i = 0; i < sizeof(base_fgroups)/sizeof(base_fgroups[0]); i++) {
        char dir[2048];
        snprintf(dir, sizeof(dir), "%s/%s", base_dir_path, fgroup_names[base_fgroups[i]]);
        load_dir(dir, base_fgroups[i], priority);
    }
}

void build_file_map(void)
{
    char dir[2048];
    const char *base = install_info.inst_path[0] ? install_info.inst_path : keeper_runtime_directory;

    load_dirs(base, FP_Base);

    for (int i = 0; i < mods_conf.after_base_cnt; i++)
        load_dirs(&mods_conf.after_base_item[i].name, FP_Mod_after_base);
    for (int i = 0; i < mods_conf.after_campaign_cnt; i++)
        load_dirs(&mods_conf.after_campaign_item[i].name, FP_Mod_after_campaign, FP_Mod_after_campaign);
    for (int i = 0; i < mods_conf.after_map_cnt; i++)
        load_dirs(&mods_conf.after_map_item[i].name, FP_Mod_after_map, FP_Mod_after_map);
}

static const FileInfo *find_in_map(short fgroup, const std::string &key)
{
    if (fgroup < 0 || fgroup > FGrp_MpLevels)
        return NULL;
    auto &grp = file_map[(size_t)fgroup];
    auto it = grp.find(key);
    if (it == grp.end() || it->second.empty())
        return NULL;
    const FileInfo *best = &it->second[0];
    for (const auto &fi : it->second) {
        if (fi.priority > best->priority)
            best = &fi;
    }
    return best;
}


char *prepare_file_path_prio(short fgroup, short fgroup_cmp, TbBool check_level, const char *fname)
{
    static char fp_result_buf[2048];
    std::string key = to_lower_str(fname);
    const FileInfo *fi = find_in_map(fgroup, to_lower_str(fname));


    if (!fi || fi->priority < FP_Map) {
        if (check_level) {
            LevelNumber lvnum = get_selected_level_number();
            if (lvnum != 0) {
                char level_fname[255];
                snprintf(level_fname, sizeof(level_fname), "map%05lu.%s", lvnum, fname);
                fi = find_in_map(fgroup, to_lower_str(level_fname));
            }
        }
    }

    if (fgroup_cmp != FGrp_None) {
        fi = find_in_map(fgroup_cmp, key);
        if (fi) {
            snprintf(fp_result_buf, sizeof(fp_result_buf), "%s", fi->fpath.c_str());
            return fp_result_buf;
        }
    }

    return fp_result_buf;
}

char *prepare_file_fmtpath_prio(short fgroup, short fgroup_cmp, TbBool check_level, const char *fmt_str, ...)
{
    char fname[255] = "";
    va_list arg;
    va_start(arg, fmt_str);
    vsnprintf(fname, sizeof(fname), fmt_str, arg);
    va_end(arg);



    return prepare_file_path_prio(fgroup, fgroup_cmp, check_level, fname);
}

/******************************************************************************/

