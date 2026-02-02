/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_filesdk1.c
 *     Level files reading routines fore standard DK1 levels.
 * @par Purpose:
 *     Allows reading level files in DK1 format.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "lvl_filesdk1.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_dernc.h"
#include "bflib_fileio.h"

#include "front_simple.h"
#include "config.h"
#include "config_campaigns.h"
#include "config_slabsets.h"
#include "config_strings.h"
#include "config_terrain.h"
#include "config_keeperfx.h"
#include "light_data.h"
#include "map_ceiling.h"
#include "map_utils.h"
#include "thing_factory.h"
#include "engine_textures.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "player_instances.h"

#include <toml.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** Global storage for level file version number.
 * Note that the version number is not stored anywhere on load/save.
 * It is only valid while the level is being loaded, and cannot be used
 * during gameplay. Remember not to use it within script_process_value(),
 * or any other function used beyond first initialization of a level.
  */
long level_file_version = 0;
char *level_strings[STRINGS_MAX+1];
char *level_strings_data;
/******************************************************************************/
#pragma pack(1)

// all these structs have a fixed size to remain compatible with the files out there
struct LegacyCoord3d {
    union {
      unsigned short val;
    } x;
    union {
      unsigned short val;
    } y;
    union {
      unsigned short val;
    } z;
};

struct LegacyCoord2d {
    union {
      unsigned short val;
    } x;
    union {
      unsigned short val;
    } y;
};

struct LegacyInitThing { // sizeof=0x15
    struct LegacyCoord3d mappos;
    unsigned char oclass;
    unsigned char model; // Converted to ThingModel on read
    unsigned char owner;
    unsigned short range;
    unsigned short index;
    unsigned char params[8];
};

struct LegacyInitActionPoint { // sizeof = 8
    struct LegacyCoord2d mappos;
    unsigned short range;
    unsigned short num;
};

struct LegacyInitLight { // sizeof=0x14
    short radius;
    unsigned char intensity;
    unsigned char flags;
    short field_4_unused;
    short field_6_unused;
    short field_8_unused;
    struct LegacyCoord3d mappos;
    unsigned char field_10_unused;
    unsigned char is_dynamic;
    short attached_slb;
};

#pragma pack()

/******************************************************************************/


/**
 * Loads map file with given level number and file extension.
 * @return Returns NULL if the file doesn't exist or is smaller than ldsize;
 * on success, returns a buffer which should be freed after use,
 * and sets ldsize into its size.
 */
unsigned char *load_single_map_file_to_buffer(LevelNumber lvnum,const char *fext,int32_t *ldsize,unsigned short flags)
{
  short fgroup = get_level_fgroup(lvnum);
  char* fname = prepare_file_fmtpath(fgroup, "map%05u.%s", lvnum, fext);
  long fsize = LbFileLengthRnc(fname);
  if (fsize < *ldsize)
  {
      if ((flags & LMFF_Optional) == 0)
          WARNMSG("Map file \"map%05u.%s\" doesn't exist or is too small.", lvnum, fext);
      else
          SYNCMSG("Optional file \"map%05u.%s\" doesn't exist or is too small.", lvnum, fext);
      return NULL;
  }
  unsigned char* buf = calloc(fsize + 16, 1);
  if (buf == NULL)
  {
    if ((flags & LMFF_Optional) == 0)
      WARNMSG("Can't allocate %ld bytes to load \"map%05u.%s\".",fsize,lvnum,fext);
    else
      SYNCMSG("Can't allocate %ld bytes to load \"map%05u.%s\".",fsize,lvnum,fext);
    return NULL;
  }
  fsize = LbFileLoadAt(fname,buf);
  if (fsize < *ldsize)
  {
    if ((flags & LMFF_Optional) == 0)
      WARNMSG("Reading map file \"map%05u.%s\" failed.",lvnum,fext);
    else
      SYNCMSG("Reading optional file \"map%05u.%s\" failed.",lvnum,fext);
    free(buf);
    return NULL;
  }
  *ldsize = fsize;
  SYNCDBG(7,"Map file \"map%05u.%s\" loaded.",lvnum,fext);
  return buf;
}

long get_level_number_from_file_name(const char *fname)
{
  if (strnicmp(fname,"map",3) != 0)
    return SINGLEPLAYER_NOTSTARTED;
  // Get level number
  long lvnum = strtol(&fname[3], NULL, 10);
  if (lvnum <= 0)
    return SINGLEPLAYER_NOTSTARTED;
  return lvnum;
}

/**
 * Analyzes one line of .LIF file buffer. The buffer must be null-terminated.
 * @return Length of the parsed line.
 */
long level_lif_entry_parse(const char *fname, char *buf)
{
  if (buf[0] == '\0')
    return 0;
  long i = 0;
  // Skip spaces and control chars
  while (buf[i] != '\0')
  {
    // Check for commented-out lines
    if (buf[i] == ';')
    {
      // Loop through the entire commented-out line
      while (buf[i] != '\0')
      {
        if ((buf[i] == '\n') || (buf[i] == '\r'))
        {
          break;
        }
        i++;
        if (i >= 10000) // arbritarily big number to prevent an infinte loop if last line is a comment that doesn't have a new line at the end
        {
          WARNMSG("commented-out line from \"%s\" is too long at %ld characters", fname,i);
          return 0;
        }
      }
    }
    if (!isspace(buf[i]) && (buf[i] != ',') && (buf[i] != ';') && (buf[i] != ':') && (buf[i] != '\n') && (buf[i] != '\r'))
      break;
    i++;
  }
  // when the last line of a .lif is a comment, we check here if the end of the file has been reached (and we should exit the function)
  if (buf[i] == '\0')
    return 0;
  // Get level number
  char* cbuf;
  long lvnum = strtol(&buf[i], &cbuf, 10);
  // If can't read number, return
  if (cbuf == &buf[i])
  {
    WARNMSG("Can't read level number from \"%s\"", fname);
    return 0;
  }
  // Skip spaces and blank chars
  while (cbuf[0] != '\0')
  {
    if (!isspace(cbuf[0]) && (cbuf[0] != ',') && (cbuf[0] != ';') && (cbuf[0] != ':'))
      break;
    cbuf++;
  }
  // IF the next field starts with a "#" then treat it as a string ID for the level's name
    if (cbuf[0] == '#')
    {
      cbuf++;
      if (!set_level_info_string_index(lvnum,cbuf,LvKind_IsFree))
      {
        WARNMSG("Can't set string index of level %ld from file \"%s\"", lvnum, fname);
      }
      cbuf--;
    }
  // Find length of level name; make it null-terminated
  i = 0;
  while (cbuf[i] != '\0')
  {
    if ((cbuf[i] == '\n') || (cbuf[i] == '\r'))
    {
      cbuf[i] = '\0';
      break;
    }
    i++;
  }
  if (i >= LINEMSG_SIZE)
  {
    WARNMSG("Level name from \"%s\" truncated from %ld to %d characters", fname,i,LINEMSG_SIZE);
    i = LINEMSG_SIZE-1;
    cbuf[i] = '\0';
  }
  if (cbuf[0] == '\0')
  {
    WARNMSG("Can't read level name from \"%s\"", fname);
    return 0;
  }
  // Store level name
  if (add_freeplay_level_to_campaign(&campaign,lvnum) < 0)
  {
    WARNMSG("Can't add freeplay level from \"%s\" to campaign \"%s\"", fname, campaign.name);
    return 0;
  }
  if (!set_level_info_text_name(lvnum,cbuf,LvKind_IsFree))
  {
    WARNMSG("Can't set name of level from file \"%s\"", fname);
    return 0;
  }
  return (cbuf-buf)+i;
}

/** Analyzes given LIF file buffer.
 *
 * @param fname Name of the LIF file which we've loaded.
 * @param buf The buffer; must be null-terminated.
 * @param buflen Length of the buffer.
 * @return
 */
short level_lif_file_parse(const char *fname, char *buf, long buflen)
{
  if (buf == NULL)
    return false;
  short result = false;
  long pos = 0;
  long i;
  do
  {
    i = level_lif_entry_parse(fname, &buf[pos]);
    if (i > 0)
    {
      result = true;
      pos += i+1;
      if (pos+1 >= buflen)
        break;
    }
  } while (i > 0);
  return result;
}

/**
 * Searches levels folder for LIF files and adds them to campaign levels list.
 */
TbBool find_and_load_lif_files(void)
{
    unsigned char* buf = calloc(MAX_LIF_SIZE, 1);
    if (buf == NULL)
    {
        ERRORLOG("Can't allocate memory for .LIF files parsing.");
        return false;
  }
  short result = false;
  char* fname = prepare_file_path(FGrp_CmpgLvls, "*.lif");
  struct TbFileEntry fe;
  struct TbFileFind * ff = LbFileFindFirst(fname, &fe);
  if (ff) {
    do {
      fname = prepare_file_path(FGrp_CmpgLvls, fe.Filename);
      long i = LbFileLength(fname);
      if ((i < 0) || (i >= MAX_LIF_SIZE)) {
        WARNMSG("File \"%s\" too long (Max size %d)", fe.Filename, MAX_LIF_SIZE);
      } else if (LbFileLoadAt(fname, buf) != i) {
        WARNMSG("Unable to read .LIF file, \"%s\"", fe.Filename);
      } else {
        buf[i] = '\0';
        if (level_lif_file_parse(fe.Filename, (char *)buf, i)) {
          result = true;
        }
      }
    } while (LbFileFindNext(ff, &fe) >= 0);
    LbFileFindEnd(ff);
  }
  free(buf);
  return result;
}

/**
 * Analyzes given LOF file buffer. The buffer must be null-terminated.
 */
TbBool level_lof_file_parse(const char *fname, char *buf, long len)
{
    struct LevelInformation *lvinfo;
    int32_t pos;
    char word_buf[32];
    long lvnum;
    int cmd_num;
    int k;
    int n;
    word_buf[0] = 0;
    SYNCDBG(8,"Starting for \"%s\"",fname);
    if (buf == NULL)
        return false;
    lvnum = get_level_number_from_file_name(fname);
    if (lvnum < 1)
    {
        WARNLOG("Incorrect .LOF file name \"%s\", skipped.",fname);
        return false;
    }
    lvinfo = get_or_create_level_info(lvnum, LvKind_None);
    if (lvinfo == NULL)
    {
        WARNMSG("Can't get LevelInformation item to store level %ld data from LOF file.",lvnum);
        return 0;
    }
    lvinfo->location = LvLc_Custom;
    pos = 0;
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(cmpgn_map_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,cmpgn_map_commands);
        // Now store the config item in correct place
        if (cmd_num == ccr_endOfBlock) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // NAME_TEXT
            if (get_conf_parameter_whole(buf,&pos,len,lvinfo->name,LINEMSG_SIZE) <= 0)
            {
              WARNMSG("Couldn't read \"%s\" parameter in LOF file '%s'.",
                  COMMAND_TEXT(cmd_num),fname);
              break;
            }
            break;
        case 2: // NAME_ID
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
              {
                lvinfo->name_stridx = k;
                n++;
              }
            }
            if ((n < 1) && (strlen(word_buf) != 0))
            {
              WARNMSG("Couldn't recognize \"%s\" number in LOF file '%s'.",
                  COMMAND_TEXT(cmd_num),fname);
            }
            break;
        case 3: // ENSIGN_POS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->ensign_x = k;
                  n++;
                }
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->ensign_y = k;
                  n++;
                }
            }
            if ((n < 2) && (strlen(word_buf) != 0))
            {
              WARNMSG("Couldn't recognize \"%s\" coordinates in LOF file '%s'.",
                  COMMAND_TEXT(cmd_num),fname);
            }
            break;
        case 4: // ENSIGN_ZOOM
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->ensign_zoom_x = k;
                  n++;
                }
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->ensign_zoom_y = k;
                  n++;
                }
            }
            if ((n < 2) && (strlen(word_buf) != 0))
            {
              WARNMSG("Couldn't recognize \"%s\" coordinates in LOF file '%s'.",
                  COMMAND_TEXT(cmd_num),fname);
            }
            break;
        case 5: // PLAYERS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
              {
                lvinfo->players = k;
                n++;
              }
            }
            if ((n < 1) && (strlen(word_buf) != 0))
            {
              WARNMSG("Couldn't recognize \"%s\" number in LOF file '%s'.",
                  COMMAND_TEXT(cmd_num),fname);
            }
            break;
        case 6: // ENSIGN
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = get_id(cmpgn_map_ensign_flag_options, word_buf);
                if (k >= 0)
                {
                    lvinfo->ensign = k;
                }
                else
                {
                    WARNMSG("Invalid value '%s' for \"%s\" in '%s' file.", word_buf,
                        COMMAND_TEXT(cmd_num), fname);
                }
            }
            break;
        case 7: // SPEECH
            if (get_conf_parameter_single(buf,&pos,len,lvinfo->speech_before,DISKPATH_SIZE) > 0)
            {
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,lvinfo->speech_after,DISKPATH_SIZE) > 0)
            {
              n++;
            }
            if ((n < 2) && (strlen(word_buf) != 0))
            {
              WARNMSG("Couldn't recognize \"%s\" file names in LOF file '%s'.",
                  COMMAND_TEXT(cmd_num),fname);
            }
            break;
        case 8: // LAND_VIEW
            if (get_conf_parameter_single(buf,&pos,len,lvinfo->land_view,DISKPATH_SIZE) > 0)
            {
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,lvinfo->land_window,DISKPATH_SIZE) > 0)
            {
              n++;
            }
            if ((n < 2) && (strlen(word_buf) != 0))
            {
              WARNMSG("Couldn't recognize \"%s\" file names in LOF file '%s'.",
                  COMMAND_TEXT(cmd_num),fname);
            }
            break;
        case 9: // KIND
            while ((k = recognize_conf_parameter(buf,&pos,len,cmpgn_map_cmnds_kind)) > 0)
            {
              switch (k)
              {
              case LvKind_IsSingle:
                if ((lvinfo->level_type & LvKind_IsSingle) == 0)
                {
                    if (add_single_level_to_campaign(&campaign,lvinfo->lvnum) >= 0)
                        n++;
                    lvinfo->level_type |= LvKind_IsSingle;
                } else
                    n++;
                break;
              case LvKind_IsMulti:
                if ((lvinfo->level_type & LvKind_IsMulti) == 0)
                {
                    if (add_multi_level_to_campaign(&campaign,lvinfo->lvnum) >= 0)
                        n++;
                    lvinfo->level_type |= LvKind_IsMulti;
                } else
                    n++;
                break;
              case LvKind_IsBonus:
                if ((lvinfo->level_type & LvKind_IsBonus) == 0)
                {
                    if (add_bonus_level_to_campaign(&campaign,lvinfo->lvnum) >= 0)
                        n++;
                    lvinfo->level_type |= LvKind_IsBonus;
                } else
                    n++;
                break;
              case LvKind_IsExtra:
                if ((lvinfo->level_type & LvKind_IsExtra) == 0)
                {
                    if (add_extra_level_to_campaign(&campaign,lvinfo->lvnum) >= 0)
                        n++;
                    lvinfo->level_type |= LvKind_IsExtra;
                } else
                    n++;
                break;
              case LvKind_IsFree:
                if ((lvinfo->level_type & LvKind_IsFree) == 0)
                {
                    if (add_freeplay_level_to_campaign(&campaign,lvinfo->lvnum) >= 0)
                        n++;
                    lvinfo->level_type |= LvKind_IsFree;
                } else
                    n++;
                break;
              }
              if (n < 1)
              {
                  WARNMSG("Level %ld defined in '%s' wasn't added to any list; "
                      "kind is wrong or there's no space.",(long)lvinfo->lvnum,fname);
              }
            }
            break;
        case 10: // AUTHOR
        case 11: // DESCRIPTION
        case 12: // DATE
        case 14: // MAP_FORMAT_VERSION
            // As for now, ignore these
            break;
        case 13: // MAPSIZE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->mapsize_x = k;
                  n++;
                }
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                  lvinfo->mapsize_y = k;
                  n++;
                }
            }
            if ((n < 2) && (strlen(word_buf) != 0))
            {
              WARNMSG("Couldn't recognize \"%s\" mapsize in LOF file '%s'.",
                  COMMAND_TEXT(cmd_num),fname);
            }
            break;
        case ccr_comment:
            break;
        case ccr_endOfFile:
            break;
        default:
            WARNMSG("Unrecognized command (%d) in LOF file '%s', starting on byte %d.",cmd_num,fname,pos);
            break;
        }
        skip_conf_to_next_line(buf,&pos,len);
    }
    SYNCDBG(18,"Level %ld ensign (%d,%d) zoom (%d,%d)",(long)lvinfo->lvnum,(int)lvinfo->ensign_x,(int)lvinfo->ensign_y,(int)lvinfo->ensign_zoom_x,(int)lvinfo->ensign_zoom_y);
#undef COMMAND_TEXT
    return true;
}

/**
 * Searches levels folder for LOF files and adds them to campaign levels list.
 */
TbBool find_and_load_lof_files(void)
{
    SYNCDBG(16,"Starting");
    unsigned char* buf = calloc(MAX_LIF_SIZE, 1);
    if (buf == NULL)
    {
      ERRORLOG("Can't allocate memory for .LOF files parsing.");
      return false;
    }
    short result = false;
    char* fname = prepare_file_path(FGrp_CmpgLvls, "*.lof");
    struct TbFileEntry fe;
    struct TbFileFind * ff = LbFileFindFirst(fname, &fe);
    if (ff) {
        do {
            fname = prepare_file_path(FGrp_CmpgLvls, fe.Filename);
            long i = LbFileLength(fname);
            if ((i < 0) || (i >= MAX_LIF_SIZE)) {
              WARNMSG("File '%s' too long (Max size %d)", fe.Filename, MAX_LIF_SIZE);

            } else if (LbFileLoadAt(fname, buf) != i) {
              WARNMSG("Unable to read .LOF file, '%s'", fe.Filename);
            } else {
              buf[i] = '\0';
              if (level_lof_file_parse(fe.Filename, (char *)buf, i))
                result = true;
            }
        } while (LbFileFindNext(ff, &fe) >= 0);
        LbFileFindEnd(ff);
    }
    free(buf);
    return result;
}

TbBool load_column_file(LevelNumber lv_num)
{
    int32_t fsize = 8;
    unsigned char* buf = load_single_map_file_to_buffer(lv_num, "clm", &fsize, LMFF_None);
    if (buf == NULL)
      return false;
    clear_columns();
    unsigned long i = 0;
    long total = llong(&buf[i]);
    i += 4;
    // Validate total amount of columns
    if ((total < 0) || (total > (fsize-8)/sizeof(struct Column)))
    {
      total = (fsize-8)/sizeof(struct Column);
      WARNMSG("Bad amount of columns in CLM file; corrected to %ld.",total);
    }
    if (total > COLUMNS_COUNT)
    {
      WARNMSG("Only %d columns supported, CLM file has %ld.",COLUMNS_COUNT,total);
      total = COLUMNS_COUNT;
    }
    // The second lot of 4 bytes here are ignored.
    i += 4;
    // Fill the columns
    for (long k = 0; k < total; k++)
    {
        struct Column* colmn = &game.columns_data[k];
        memcpy(colmn, &buf[i], sizeof(struct Column));
        //Update top cube in the column
        unsigned short n = find_column_height(colmn);
        set_column_floor_filled_subtiles(colmn, n);
        i += sizeof(struct Column);
    }
    free(buf);
    return true;
}

TbBool load_map_data_file(LevelNumber lv_num)
{
    struct Map *mapblk;
    unsigned long x;
    unsigned long y;
    clear_map();
    int32_t fsize = 2 * (game.map_subtiles_y + 1) * (game.map_subtiles_x + 1);
    unsigned char* buf = load_single_map_file_to_buffer(lv_num, "dat", &fsize, LMFF_None);
    if (buf == NULL)
        return false;
    unsigned long i = 0;
    for (y=0; y < (game.map_subtiles_y+1); y++)
    {
        for (x=0; x < (game.map_subtiles_x+1); x++)
        {
            mapblk = get_map_block_at(x,y);
            mapblk->col_idx = -lword(&buf[i]);
            i += 2;
        }
    }
    free(buf);
    // Clear some bits and do some other setup
    for (y=0; y < (game.map_subtiles_y+1); y++)
    {
        for (x=0; x < (game.map_subtiles_x+1); x++)
        {
            mapblk = get_map_block_at(x,y);
            unsigned short* wptr = &game.lish.subtile_lightness[get_subtile_number(x, y)];
            *wptr = 32;
            mapblk->mapwho = 0;
            mapblk->filled_subtiles = 0;
            mapblk->revealed = 0;
        }
    }
    return true;
}

static TbBool load_thing_file(LevelNumber lv_num)
{
    SYNCDBG(5,"Starting");
    int32_t fsize = 2;
    unsigned char* buf = load_single_map_file_to_buffer(lv_num, "tng", &fsize, LMFF_None);
    if (buf == NULL)
      return false;
    unsigned long i = 0;
    long total = lword(&buf[i]);
    i += 2;
    // Validate total amount of things
    if ((total < 0) || (total > (fsize-2)/sizeof(struct LegacyInitThing)))
    {
        total = (fsize-2)/sizeof(struct LegacyInitThing);
        WARNMSG("Bad amount of things in TNG file; corrected to %d.",(int)total);
    }
    if (total > THINGS_COUNT-2)
    {
        WARNMSG("Only %d things supported, TNG file has %d.",(int)(THINGS_COUNT-2),(int)total);
        total = THINGS_COUNT-2;
    }
    // Create things
    for (long k = 0; k < total; k++)
    {
        struct LegacyInitThing litng;
        struct InitThing itng;
        memcpy(&litng, &buf[i], sizeof(struct LegacyInitThing));
        itng.mappos.x.val = litng.mappos.x.val;
        itng.mappos.y.val = litng.mappos.y.val;
        itng.mappos.z.val = litng.mappos.z.val;
        itng.oclass = litng.oclass;
        itng.model  = litng.model;
        itng.owner  = litng.owner;
        itng.range  = litng.range;
        itng.index  = litng.index;
        memcpy(&itng.params, &litng.params, 8);

        thing_create_thing(&itng);
        i += sizeof(struct LegacyInitThing);
    }
    free(buf);
    return true;
}

static TbBool load_kfx_toml_file(LevelNumber lv_num, const char *ext, const char *msg_name,
                                 const char *sections, const char *count_field, const char *section_fmt,
                                 int max_count, TbBool (*section_loader)(VALUE *arg))
{
    SYNCDBG(5,"Starting");
    int32_t fsize = 0;
    unsigned char* buf = load_single_map_file_to_buffer(lv_num, ext, &fsize, LMFF_None);
    if (buf == NULL)
        return false;
    VALUE file_root, *root_ptr = &file_root;
    char err[255] = "";
    char key[64] = "";

    if (toml_parse((char*)buf, err, sizeof(err), root_ptr))
    {
        WARNMSG("Unable to load %s file\n %s", msg_name, err);
        free(buf);
        return false;
    }
    VALUE *common_section = value_dict_get(root_ptr, "common");
    if (!common_section)
    {
        WARNMSG("No [common] in %s for level %d", msg_name, lv_num);
        value_fini(root_ptr);
        free(buf);
        return false;
    }
    int32_t total;

    VALUE *item_arr = value_dict_get(root_ptr, sections);
    if (value_type(item_arr) == VALUE_ARRAY)
    {
        total = value_array_size(item_arr);
    }
    else
    {
        item_arr = NULL; // Against bad values
        total = value_int32(value_dict_get(common_section, count_field));
    }
    // Validate total amount of sections
    if (total < 0)
    {
        WARNMSG("Bad amount of secions in %s file", msg_name);
        value_fini(root_ptr);
        free(buf);
        return false;
    }
    if (total >= max_count)
    {
        WARNMSG("Only %d things supported, file has %d.", max_count,total);

    }
    // Create sections
    for (int k = 0; k < total; k++)
    {
        VALUE *section;
        if (item_arr)
        {
            // Array form
            section = value_array_get(item_arr, k);
        }
        else
        {
            snprintf(key, sizeof(key), section_fmt, k);
            section = value_dict_get(root_ptr, key);
        }
        if (value_type(section) != VALUE_DICT)
        {
            WARNMSG("Invalid %s section %d", msg_name, k);
        }
        else
        {
            if (!section_loader(section))
            {
                WARNMSG("Failed to load section %d from %s", k, msg_name);
            }
        }
    }
    value_fini(root_ptr);
    free(buf);
    return true;
}

static TbBool load_tngfx_file(LevelNumber lv_num)
{
    return load_kfx_toml_file(lv_num, "tngfx", "TNGFX",
                              "thing", "ThingsCount", "thing%d", THINGS_COUNT - 2,
                              &thing_create_thing_adv);
}

TbBool load_action_point_file(LevelNumber lv_num)
{
  SYNCDBG(5,"Starting");
  int32_t fsize = 4;
  unsigned char* buf = load_single_map_file_to_buffer(lv_num, "apt", &fsize, LMFF_None);
  if (buf == NULL)
    return false;
  unsigned long i = 0;
  long total = llong(&buf[i]);
  i += 4;
  // Validate total amount of action points
  if ((total < 0) || (total > (fsize-4)/sizeof(struct LegacyInitActionPoint)))
  {
    total = (fsize-4)/sizeof(struct LegacyInitActionPoint);
    WARNMSG("Bad amount of action points in APT file; corrected to %ld.",total);
  }
  if (total > ACTN_POINTS_COUNT-1)
  {
    WARNMSG("Only %d action points supported, APT file has %ld.",ACTN_POINTS_COUNT-1,total);
    total = ACTN_POINTS_COUNT-1;
  }
  // Create action points
  for (long k = 0; k < total; k++)
  {
      struct LegacyInitActionPoint legiapt;
      struct InitActionPoint iapt;
      memcpy(&legiapt, &buf[i], sizeof(struct LegacyInitActionPoint));
      iapt.mappos.x.val = legiapt.mappos.x.val;
      iapt.mappos.y.val = legiapt.mappos.y.val;
      iapt.num          = legiapt.num;
      iapt.range        = legiapt.range;
      if (actnpoint_create_actnpoint(&iapt) == INVALID_ACTION_POINT)
          ERRORLOG("Cannot allocate action point %ld during APT load", k);
    i += sizeof(struct LegacyInitActionPoint);
  }
  free(buf);
  return true;
}

TbBool load_aptfx_file(LevelNumber lv_num)
{
    return load_kfx_toml_file(lv_num, "aptfx", "APTFX",
                              "actionpoint", "ActionPointsCount", "actionpoint%d", ACTN_POINTS_COUNT - 1,
                              &actnpoint_create_actnpoint_adv);
}

/**
 * Updates "use" property of given columns set, using given SlabSet entries.
 */
TbBool update_columns_use(struct Column *cols,long ccount,struct SlabSet *sset,long scount)
{
    long i;
    long k;
    long ncol;
    for (i = 0; i < ccount; i++)
    {
        cols[i].use = 0;
    }
    for (i=0; i < scount; i++)
        for (k=0; k < 9; k++)
        {
            ncol = -sset[i].col_idx[k];
            if ((ncol >= 0) && (ncol < ccount))
                cols[ncol].use++;
        }
    return true;
}

TbBool columns_add_static_entries(void)
{
    short c[3];

    for (long i=0; i < 3; i++)
      c[i] = 0;
    struct Column lcolmn;
    memset(&lcolmn, 0, sizeof(struct Column));
    short* wptr = &game.col_static_entries[0];
    for (long i=0; i < 3; i++)
    {
        memset(&lcolmn, 0, sizeof(struct Column));
        lcolmn.floor_texture = c[i];
        for (long k = 0; k < 6; k++)
        {
          lcolmn.cubes[0] = player_cubes[k];
          make_solidmask(&lcolmn);
          long ncol = find_column(&lcolmn);
          if (ncol == 0)
            ncol = create_column(&lcolmn);
          struct Column* colmn = get_column(ncol);
          colmn->bitfields |= CLF_ACTIVE;
          *wptr = -(short)ncol;
          wptr++;
        }
    }
    return true;
}

TbBool update_slabset_column_indices(struct Column *cols, long ccount)
{
    struct Column lcolmn;
    memset(&lcolmn,0,sizeof(struct Column));
    for (long i = 0; i < game.slabset_num; i++)
    {
        struct SlabSet* sset = &game.slabset[i];
        for (long k = 0; k < 9; k++)
        {
            long n = sset->col_idx[k];
            long ncol;
            if (n >= 0)
            {
                lcolmn.floor_texture = n;
                ncol = find_column(&lcolmn);
                if (ncol == 0)
                {
                    ncol = create_column(&lcolmn);
                    struct Column* colmn = get_column(ncol);
                    colmn->bitfields |= CLF_ACTIVE;
                }
            } else
            {
                if (-n < ccount)
                    ncol = find_column(&cols[-n]);
                else
                    ncol = 0;
                if (ncol == 0)
                {
                    ERRORLOG("column:%ld referenced in slabset.toml but not present in columnset.toml",-n);
                    continue;
                }
            }
            sset->col_idx[k] = -ncol;
        }
    }
    return true;
}

TbBool create_columns_from_list(struct Column *cols, long ccount)
{
    for (long i = 1; i < ccount; i++)
    {
        if (cols[i].use)
        {
            long ncol = find_column(&cols[i]);
            if (ncol == 0)
                ncol = create_column(&cols[i]);
            struct Column* colmn = get_column(ncol);
            colmn->bitfields |= CLF_ACTIVE;
        }
    }
    return true;
}

TbBool load_slab_datclm_files(void)
{
    SYNCDBG(5,"Starting");

    long slbset_tot = game.conf.slab_conf.slab_types_count * SLABSETS_PER_SLAB;
    game.slabset_num = slbset_tot;

    update_columns_use(game.conf.column_conf.cols,game.conf.column_conf.columns_count,game.slabset,slbset_tot);
    create_columns_from_list(game.conf.column_conf.cols,game.conf.column_conf.columns_count);
    update_slabset_column_indices(game.conf.column_conf.cols,game.conf.column_conf.columns_count);
    return true;
}

TbBool load_slab_file(void)
{
    TbBool result = true;
    if (!load_slab_datclm_files())
        result = false;
    if (!columns_add_static_entries())
        result = false;
    return result;
}

long load_map_wibble_file(unsigned long lv_num)
{
    struct Map *mapblk;
    unsigned long stl_x;
    unsigned long stl_y;
    unsigned char *buf;
    unsigned long i;
    unsigned long k;
    int32_t fsize;
    fsize = (game.map_subtiles_y+1)*(game.map_subtiles_x+1);
    buf = load_single_map_file_to_buffer(lv_num,"wib",&fsize,LMFF_None);
    if (buf == NULL)
      return false;
    i = 0;
    for (stl_y=0; stl_y < (game.map_subtiles_y+1); stl_y++)
      for (stl_x=0; stl_x < (game.map_subtiles_x+1); stl_x++)
      {
        mapblk = get_map_block_at(stl_x,stl_y);
        k = buf[i];
        set_mapblk_wibble_value(mapblk, k);
        i++;
      }
    free(buf);
    return true;
}

short load_map_ownership_file(LevelNumber lv_num)
{
    unsigned long x;
    unsigned long y;
    unsigned char *buf;
    unsigned long i;
    int32_t fsize;
    fsize = (game.map_subtiles_y+1)*(game.map_subtiles_x+1);
    buf = load_single_map_file_to_buffer(lv_num,"own",&fsize,LMFF_None);
    if (buf == NULL)
      return false;
    i = 0;
    for (y=0; y < (game.map_subtiles_y+1); y++)
      for (x=0; x < (game.map_subtiles_x+1); x++)
      {
        if ((x < game.map_subtiles_x) && (y < game.map_subtiles_y))
        {
            set_slab_owner(subtile_slab(x),subtile_slab(y),buf[i]);
        }
        else
            set_slab_owner(subtile_slab(x),subtile_slab(y),PLAYER_NEUTRAL);
        i++;
      }
    free(buf);
    return true;
}

TbBool initialise_map_wlb_auto(void)
{
    struct SlabMap *slb;
    struct SlabConfigStats *slabst;
    unsigned long x;
    unsigned long y;
    unsigned long n;
    unsigned long nbridge;
    nbridge = 0;
    for (y = 0; y < game.map_tiles_y; y++)
    {
        for (x = 0; x < game.map_tiles_x; x++)
        {
            slb = get_slabmap_block(x, y);
            if (slb->kind == SlbT_BRIDGE)
            {
                if (slabs_count_near(x, y, 1, SlbT_LAVA) > slabs_count_near(x, y, 1, SlbT_WATER))
                    n = SlbT_LAVA;
                else
                    n = SlbT_WATER;
                nbridge++;
            }
            else
            {
                n = slb->kind;
            }
            slabst = get_slab_kind_stats(n);
            slb->wlb_type = slabst->wlb_type;
        }
    }
    SYNCMSG("Regenerated WLB flags, unsure for %d bridge blocks.",(int)nbridge);
    return true;
}

TbBool load_map_wlb_file(unsigned long lv_num)
{
    struct SlabMap *slb;
    unsigned long x;
    unsigned long y;
    unsigned char *buf;
    unsigned long i;
    unsigned long n;
    unsigned long nfixes;
    int32_t fsize;
    SYNCDBG(7,"Starting");
    nfixes = 0;
    fsize = game.map_tiles_y*game.map_tiles_x;
    buf = load_single_map_file_to_buffer(lv_num,"wlb",&fsize,LMFF_Optional);
    if (buf == NULL)
      return false;
    i = 0;
    for (y=0; y < game.map_tiles_y; y++)
      for (x=0; x < game.map_tiles_x; x++)
      {
        slb = get_slabmap_block(x,y);
        n = buf[i];
        slb->wlb_type = buf[i];
        if ((n != WlbT_Water) || (slb->kind != SlbT_WATER))
          if ((n != WlbT_Lava) || (slb->kind != SlbT_LAVA))
            if (((n == WlbT_Water) || (n == WlbT_Lava)) && (slb->kind != SlbT_BRIDGE))
            {
                nfixes++;
                slb->wlb_type = WlbT_None;
            }
        i++;
      }
    free(buf);
    if (nfixes > 0)
    {
      ERRORLOG("WLB file is muddled - Fixed values for %lu tiles",nfixes);
    }
    return true;
}

TbBool initialise_extra_slab_info(unsigned long lv_num)
{
    initialise_map_rooms();
    TbBool result = load_map_wlb_file(lv_num);
    if (!result)
      result = initialise_map_wlb_auto();
    return result;
}

short load_map_slab_file(unsigned long lv_num)
{
    SYNCDBG(7,"Starting");
    struct SlabMap *slb;
    unsigned long x;
    unsigned long y;
    unsigned char *buf;
    unsigned long i;
    unsigned long n;
    int32_t fsize;
    fsize = 2*game.map_tiles_y*game.map_tiles_x;
    buf = load_single_map_file_to_buffer(lv_num,"slb",&fsize,LMFF_None);
    if (buf == NULL)
      return false;
    i = 0;
    for (y = 0; y < game.map_tiles_y; y++)
    {
        for (x = 0; x < game.map_tiles_x; x++)
        {
            slb = get_slabmap_block(x, y);
            n = lword(&buf[i]);
            if (n > game.conf.slab_conf.slab_types_count)
            {
                WARNMSG("Found invalid Slab Type %d at Tile %ld,%ld, exceeds limit of %d", (int)n, x, y, game.conf.slab_conf.slab_types_count);
                n = SlbT_ROCK;
            }
            slb->kind = n;
            i += 2;
        }
    }
    free(buf);
    initialise_map_collides();
    initialise_map_health();
    initialise_extra_slab_info(lv_num);
    return true;
}

short load_map_flag_file(unsigned long lv_num)
{
    SYNCDBG(5,"Starting");
    int32_t fsize = 2 * (game.map_subtiles_y + 1) * (game.map_subtiles_x + 1);
    unsigned char* buf = load_single_map_file_to_buffer(lv_num, "flg", &fsize, LMFF_Optional);
    if (buf == NULL)
        return false;
    unsigned long i = 0;
    for (unsigned long stl_y = 0; stl_y < (game.map_subtiles_y + 1); stl_y++)
    {
        for (unsigned long stl_x = 0; stl_x < (game.map_subtiles_x + 1); stl_x++)
        {
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            mapblk->flags = buf[i];
            i += 2;
        }
    }
    free(buf);
    return true;
}

static TbBool load_static_light_file(unsigned long lv_num)
{
    int32_t fsize = 4;
    unsigned char* buf = load_single_map_file_to_buffer(lv_num, "lgt", &fsize, LMFF_Optional);
    if (buf == NULL)
        return false;
    light_initialise();
    unsigned long i = 0;
    long total = llong(&buf[i]);
    i += 4;
    // Validate total amount of lights
    if ((total < 0) || (total > (fsize-4)/sizeof(struct LegacyInitLight)))
    {
        total = (fsize-4)/sizeof(struct LegacyInitLight);
        WARNMSG("Bad amount of static lights in LGT file; corrected to %ld.",total);
    }
    if (total >= LIGHTS_COUNT)
    {
        WARNMSG("Only %d static lights supported, LGT file has %ld.",LIGHTS_COUNT,total);
        total = LIGHTS_COUNT-1;
    } else
    if (total >= LIGHTS_COUNT/2)
    {
        WARNMSG("More than %ld%% of light slots used by static lights.",100*total/LIGHTS_COUNT);
    }
    // Create the lights
    for (long k = 0; k < total; k++)
    {
        struct LegacyInitLight legilght;
        struct InitLight ilght;
        memcpy(&legilght, &buf[i], sizeof(struct LegacyInitLight));
        ilght.attached_slb = legilght.attached_slb;
        ilght.flags      = legilght.flags;
        ilght.intensity    = legilght.intensity;
        ilght.is_dynamic   = legilght.is_dynamic;
        ilght.radius       = legilght.radius;
        ilght.mappos.x.val = legilght.mappos.x.val;
        ilght.mappos.y.val = legilght.mappos.y.val;
        ilght.mappos.z.val = legilght.mappos.z.val;

        if (light_create_light(&ilght) == 0)
        {
            WARNLOG("Couldn't allocate static light %d",(int)k);
        }
        i += sizeof(struct LegacyInitLight);
    }
    free(buf);
    return true;
}

static TbBool load_lgtfx_file(unsigned long lv_num)
{
    TbBool ret = load_kfx_toml_file(lv_num, "lgtfx", "LGTFX",
                             "light", "LightsCount", "light%d", LIGHTS_COUNT - 1,
                             &light_create_light_adv);
    if (light_count_lights() > LIGHTS_COUNT / 2)
    {
        WARNMSG("More than %d%% of light slots used by static lights.", 100*light_count_lights()/LIGHTS_COUNT);
    }
    return ret;
}

short load_and_setup_map_info(unsigned long lv_num)
{
    int32_t fsize = 1;
    unsigned char* buf = load_single_map_file_to_buffer(lv_num, "inf", &fsize, LMFF_None);
    if (buf == NULL)
    {
        game.texture_id = 0;
        return false;
    }
    game.texture_id = buf[0];
    free(buf);
    return true;
}

static void load_ext_slabs(LevelNumber lvnum)
{
    short fgroup = get_level_fgroup(lvnum);
    char* fname = prepare_file_fmtpath(fgroup, "map%05lu.slx", (unsigned long)lvnum);
    if (LbFileExists(fname))
    {
        if (game.map_tiles_x * game.map_tiles_y != LbFileLoadAt(fname, game.slab_ext_data))
        {
            JUSTLOG("Invalid ExtSlab data from %s", fname);
            memset(game.slab_ext_data, 0, sizeof(game.slab_ext_data));
        }
        SYNCDBG(1, "ExtSlab file:%s ok", fname);
    }
    else
    {
        SYNCDBG(1, "No ExtSlab file:%s", fname);
        memset(game.slab_ext_data, 0, sizeof(game.slab_ext_data));
    }
    memcpy(&game.slab_ext_data_initial,&game.slab_ext_data, sizeof(game.slab_ext_data));
}

void load_map_string_data(struct GameCampaign *campgn, LevelNumber lvnum, short fgroup)
{
    char* fname = prepare_file_fmtpath(fgroup, "map%05lu.%s.dat", (unsigned long)lvnum, get_language_lwrstr(install_info.lang_id));
    if (!LbFileExists(fname))
    {
        SYNCMSG("Map string file %s doesn't exist.", fname);
        char buf[2048];
        buf[0] = 0;
        memcpy(&buf, fname, 2048);
        fname = prepare_file_fmtpath(fgroup, "map%05lu.%s.dat", (unsigned long)lvnum, get_language_lwrstr(campgn->default_language));
        if (strcasecmp(fname, buf) == 0)
        {
            return;
        }
        if (!LbFileExists(fname))
        {
            SYNCMSG("Map string file %s doesn't exist.", fname);
            return;
        }
    }
    long filelen = LbFileLengthRnc(fname);
    if (filelen <= 0)
    {
        ERRORLOG("Map Strings file %s does not exist or can't be opened", fname);
        return;
    }
    size_t size = filelen + 256;
    level_strings_data = calloc(size, sizeof(char)); // we're allocating extra memory, which we can't just assume is already clear; this could cause weird issues
    if (level_strings_data == NULL)
    {
        ERRORLOG("Can't allocate memory for Map Strings data");
        return;
    }
    long loaded_size = LbFileLoadAt(fname, level_strings_data);
    if (loaded_size < 16)
    {
        ERRORLOG("Map Strings file couldn't be loaded or is too small");
        return;
    }
    unsigned long loaded_strings_count = count_strings(level_strings_data, loaded_size);
    char* strings_data_end = level_strings_data + size;
    // Resetting all values to empty strings
    reset_strings(level_strings, STRINGS_MAX);
    // Analyzing strings data and filling correct values
    TbBool result = fill_strings_list(level_strings, level_strings_data, strings_data_end, STRINGS_MAX);
    if (result)
    {
        SYNCMSG("Loaded %lu strings from %s", loaded_strings_count, fname);
    }
    SYNCDBG(19, "Finished");
}

static TbBool load_level_file(LevelNumber lvnum)
{
    TbBool result;
    TbBool new_format = true;
    short fgroup = get_level_fgroup(lvnum);
    char* fname = prepare_file_fmtpath(fgroup, "map%05lu.slb", (unsigned long)lvnum);
    if (LbFileExists(fname))
    {
        result = true;
        struct GameCampaign *campgn = &campaign;
        load_map_string_data(campgn, lvnum, fgroup);
        load_map_data_file(lvnum);
        load_map_flag_file(lvnum);
        load_column_file(lvnum);
        init_whole_blocks();
        load_slab_file();
        init_columns();
        result = load_lgtfx_file(lvnum);
        if (!result)
        {
            new_format = false;
            result = load_static_light_file(lvnum);
        }
        if (!load_map_ownership_file(lvnum))
          result = false;
        load_map_wibble_file(lvnum);
        load_and_setup_map_info(lvnum);
        load_texture_map_file(game.texture_id, lvnum, fgroup);
        if (new_format)
        {
            load_aptfx_file(lvnum);
        }
        else
        {
            load_action_point_file(lvnum);
        }
        if (!load_map_slab_file(lvnum))
          result = false;
        if (new_format)
        {
            result = load_tngfx_file(lvnum);
        }
        else
        {
            result = load_thing_file(lvnum);
        }
        reinitialise_map_rooms();
        ceiling_init();
        init_keys();
        if (result)
        {
            load_ext_slabs(lvnum);
        }
    } else
    {
        ERRORLOG("The level \"map%05u\" doesn't exist; creating empty map.",lvnum);
        init_whole_blocks();
        load_slab_file();
        init_columns();
        game.texture_id = 0;
        load_texture_map_file(game.texture_id, lvnum, fgroup);
        init_top_texture_to_cube_table();
        result = false;
    }
    return result;
}

TbBool load_map_file(LevelNumber lvnum)
{
    TbBool result = load_level_file(lvnum);
    if (result)
        set_loaded_level_number(lvnum);
    else
        set_loaded_level_number(SINGLEPLAYER_NOTSTARTED);
    return result;
}

void free_level_strings_data()
{
  // Resetting all values to empty strings
  reset_strings(level_strings, STRINGS_MAX);
  // Freeing memory
  free(level_strings_data);
  level_strings_data = NULL;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
