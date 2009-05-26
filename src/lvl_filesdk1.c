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
#include "lvl_filesdk1.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_dernc.h"
#include "bflib_memory.h"
#include "bflib_bufrw.h"
#include "bflib_fileio.h"

#include "front_simple.h"
#include "config.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char slabclm_fname[] = "slabs.clm";
const char slabdat_fname[] = "slabs.dat";
long level_file_version = 0;
/******************************************************************************/
DLLIMPORT long _DK_convert_old_column_file(unsigned long lv_num);
DLLIMPORT unsigned char _DK_load_map_slab_file(unsigned long lv_num);
DLLIMPORT long _DK_load_column_file(unsigned long lv_num);
DLLIMPORT void _DK_load_slab_file(void);
DLLIMPORT long _DK_load_map_data_file(unsigned long lv_num);
DLLIMPORT void _DK_load_thing_file(unsigned long lv_num);
DLLIMPORT long _DK_load_action_point_file(unsigned long lv_num);
DLLIMPORT void _DK_load_level_file(long lvnum);
DLLIMPORT void _DK_initialise_extra_slab_info(unsigned long lv_num);
/******************************************************************************/

/*
 * Loads map file with given level number and file extension.
 * @return Returns NULL if the file doesn't exist or is smaller than ldsize;
 * on success, returns a buffer which should be freed after use,
 * and sets ldsize into its size.
 */
unsigned char *load_single_map_file_to_buffer(unsigned long lvnum,const char *fext,long *ldsize)
{
  unsigned char *buf;
  char *fname;
  long fsize;
  short fgroup;
  fgroup = get_level_fgroup(lvnum);
  fname = prepare_file_fmtpath(fgroup,"map%05lu.%s",lvnum,fext);
  wait_for_cd_to_be_available();
  fsize = LbFileLengthRnc(fname);
  if (fsize < *ldsize)
  {
    LbWarnLog("Map file \"map%05lu.%s\" doesn't exist or is too small.\n",lvnum,fext);
    return NULL;
  }
  if (fsize > ANY_MAP_FILE_MAX_SIZE)
  {
    LbWarnLog("Map file \"map%05lu.%s\" exceeds max size of %d; loading failed.\n",lvnum,fext,ANY_MAP_FILE_MAX_SIZE);
    return NULL;
  }
  buf = LbMemoryAlloc(fsize+16);
  if (buf == NULL)
  {
    LbWarnLog("Can't allocate %ld bytes to load \"map%05lu.%s\".\n",fsize,lvnum,fext);
    return NULL;
  }
  fsize = LbFileLoadAt(fname,buf);
  if (fsize < *ldsize)
  {
    LbWarnLog("Reading map file \"map%05lu.%s\" failed.\n",lvnum,fext);
    LbMemoryFree(buf);
    return NULL;
  }
  *ldsize = fsize;
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("Map file \"map%05lu.%s\" loaded.\n",lvnum,fext);
#endif
  return buf;
}

/*
 * Analyses one line of .LIF file buffer. The buffer must be null-terminated.
 * @return Returns length of the parsed line.
 */
long level_lif_entry_parse(char *fname, char *buf)
{
  long lvnum;
  char *cbuf;
  long i;
  if (buf[0] == '\0')
    return 0;
  i = 0;
  // Skip spaces and control chars
  while (buf[i] != '\0')
  {
    if (!isspace(buf[i]) && (buf[i] != ',') && (buf[i] != ';') && (buf[i] != ':'))
      break;
    i++;
  }
  // Get level number
  lvnum = strtol(&buf[i],&cbuf,0);
  // If can't read number, return
  if (cbuf == &buf[i])
  {
    LbWarnLog("Can't read level number from '%s'\n", fname);
    return 0;
  }
  // Skip spaces and blank chars
  while (cbuf[0] != '\0')
  {
    if (!isspace(cbuf[0]) && (cbuf[0] != ',') && (cbuf[0] != ';') && (cbuf[0] != ':'))
      break;
    cbuf++;
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
    LbWarnLog("Level name from '%s' truncated from %d to %d characters\n", fname,i,LINEMSG_SIZE);
    i = LINEMSG_SIZE-1;
    cbuf[i] = '\0';
  }
  if (cbuf[0] == '\0')
  {
    LbWarnLog("Can't read level name from '%s'\n", fname);
    return 0;
  }
  // check if the level isn't added as other type of level
  if (is_campaign_level(lvnum))
    return (cbuf-buf)+i;
  // Store level name
  if (add_freeplay_level_to_campaign(&campaign,lvnum) < 0)
  {
    LbWarnLog("Can't add freeplay level from '%s' to campaign\n", fname);
    return 0;
  }
  if (!set_level_info_text_name(lvnum,cbuf,LvOp_IsFree))
  {
    LbWarnLog("Can't set name of level from file '%s'\n", fname);
    return 0;
  }
  return (cbuf-buf)+i;
}

/*
 * Analyses given .LIF file buffer. The buffer must be null-terminated.
 */
short level_lif_file_parse(char *fname, char *buf, long buflen)
{
  short result;
  long i;
  long pos;
  if (buf == NULL)
    return false;
  result = false;
  pos = 0;
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

short find_and_load_lif_files(void)
{
  struct TbFileFind fileinfo;
  unsigned char *buf;
  char *fname;
  short result;
  int rc;
  long i;
  buf = LbMemoryAlloc(MAX_LIF_SIZE);
  if (buf == NULL)
  {
    LbErrorLog("Can't allocate memory for .LIF files parsing.\n");
    return false;
  }
  result = false;
  fname = prepare_file_path(FGrp_VarLevels,"*.lif");
  rc = LbFileFindFirst(fname, &fileinfo, 0x21u);
  while (rc != -1)
  {
    fname = prepare_file_path(FGrp_VarLevels,fileinfo.Filename);
    i = LbFileLength(fname);
    if ((i < 0) || (i >= MAX_LIF_SIZE))
    {
      LbWarnLog("File '%s' too long (Max size %d)\n", fileinfo.Filename, MAX_LIF_SIZE);

    } else
    if (LbFileLoadAt(fname, buf) != i)
    {
      LbWarnLog("Unable to read .LIF file, '%s'\n", fileinfo.Filename);
    } else
    {
      buf[i] = '\0';
      if (level_lif_file_parse(fileinfo.Filename, (char *)buf, i))
        result = true;
    }
    rc = LbFileFindNext(&fileinfo);
  }
  LbFileFindEnd(&fileinfo);
  LbMemoryFree(buf);
  return result;
}

long convert_old_column_file(unsigned long lv_num)
{
  _DK_convert_old_column_file(lv_num);
}

short load_column_file(unsigned long lv_num)
{
  //return _DK_load_column_file(lv_num);
  struct Column *col;
  unsigned long i;
  long k;
  unsigned short n;
  long total;
  unsigned char *buf;
  long fsize;
  if (game.numfield_C & 0x08)
  {
    convert_old_column_file(lv_num);
    set_flag_byte(&game.numfield_C,0x08,false);
  }
  fsize = 8;
  buf = load_single_map_file_to_buffer(lv_num,"clm",&fsize);
  if (buf == NULL)
    return false;
  clear_columns();
  i = 0;
  total = llong(&buf[i]);
  i += 4;
  // Validate total amount of columns
  if ((total < 0) || (total > (fsize-8)/sizeof(struct Column)))
  {
    total = (fsize-8)/sizeof(struct Column);
    LbWarnLog("Bad amount of columns in CLM file; corrected to %ld.\n",total);
  }
  if (total > COLUMNS_COUNT)
  {
    LbWarnLog("Only %d columns supported, CLM file has %ld.\n",COLUMNS_COUNT,total);
    total = COLUMNS_COUNT;
  }
  // Read and validate second amount
  game.field_14AB3F = llong(&buf[i]);
  if (game.field_14AB3F >= COLUMNS_COUNT)
  {
    game.field_14AB3F = COLUMNS_COUNT-1;
  }
  i += 4;
  // Fill the columns
  for (k=0; k < total; k++)
  {
    col = &game.columns[k];
    LbMemoryCopy(col, &buf[i], sizeof(struct Column));
    n = find_column_height(col);
    col->bitfileds = (n<<4) ^ ((n<<4) ^ (col->bitfileds)) & 0xF;
    i += sizeof(struct Column);
  }
  LbMemoryFree(buf);
  return true;
}

long load_map_data_file(unsigned long lv_num)
{
  //return _DK_load_map_data_file(lv_num);
  struct Map *map;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  unsigned long n;
  unsigned short *wptr;
  long fsize;
  clear_map();
  fsize = 2*(map_subtiles_y+1)*(map_subtiles_x+1);
  buf = load_single_map_file_to_buffer(lv_num,"dat",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      n = -lword(&buf[i]);
      map->data = map->data ^ (map->data ^ n) & 0x7FF;
      i += 2;
    }
  LbMemoryFree(buf);
  // Clear some bits and do some other setup
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      wptr = &game.field_46157[y*(map_subtiles_x+1) + x];
      *wptr = 32;
      map->data &= 0xFFC007FFu;
      map->data &= 0xF0FFFFFFu;
      map->data &= 0x0FFFFFFFu;
    }
  return true;
}

short load_thing_file(unsigned long lv_num)
{
  static const char *func_name="load_thing_file";
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_load_thing_file(lv_num); return true;
  struct InitThing itng;
  unsigned long i;
  long k;
  long total;
  unsigned char *buf;
  long fsize;
  fsize = 2;
  buf = load_single_map_file_to_buffer(lv_num,"tng",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  total = lword(&buf[i]);
  i += 2;
  // Validate total amount of things
  if ((total < 0) || (total > (fsize-2)/sizeof(struct InitThing)))
  {
    total = (fsize-2)/sizeof(struct InitThing);
    LbWarnLog("Bad amount of things in TNG file; corrected to %ld.\n",total);
  }
  if (total > THINGS_COUNT-2)
  {
    LbWarnLog("Only %d things supported, TNG file has %ld.\n",THINGS_COUNT-2,total);
    total = THINGS_COUNT-2;
  }
  // Create things
  for (k=0; k < total; k++)
  {
    LbMemoryCopy(&itng, &buf[i], sizeof(struct InitThing));
    thing_create_thing(&itng);
    i += sizeof(struct InitThing);
  }
  LbMemoryFree(buf);
  return true;
}

long load_action_point_file(unsigned long lv_num)
{
  static const char *func_name="load_action_point_file";
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //return _DK_load_action_point_file(lv_num);
  struct InitActionPoint iapt;
  unsigned long i;
  long k;
  long total;
  unsigned char *buf;
  char *text;
  long fsize;
  fsize = 4;
  buf = load_single_map_file_to_buffer(lv_num,"apt",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  total = llong(&buf[i]);
  i += 4;
  // Validate total amount of action points
  if ((total < 0) || (total > (fsize-4)/sizeof(struct InitActionPoint)))
  {
    total = (fsize-4)/sizeof(struct InitActionPoint);
    LbWarnLog("Bad amount of action points in APT file; corrected to %ld.\n",total);
  }
  if (total > ACTN_POINTS_COUNT-1)
  {
    LbWarnLog("Only %d action points supported, APT file has %ld.\n",ACTN_POINTS_COUNT-1,total);
    total = ACTN_POINTS_COUNT-1;
  }
  // Create action points
  for (k=0; k < total; k++)
  {
    LbMemoryCopy(&iapt, &buf[i], sizeof(struct InitActionPoint));
    if (actnpoint_create_actnpoint(&iapt) == &game.action_points[0])
    {
      text = buf_sprintf("Cannot allocate action point %d during APT load",k);
      error(func_name, 472, text);
    }
    i += sizeof(struct InitActionPoint);
  }
  LbMemoryFree(buf);
  return true;
}

short load_slabdat_file(struct SlabSet *slbset, long *scount)
{
  static const char *func_name="load_slabdat_file";
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  long total;
  unsigned char *buf;
  long fsize;
  long i,k,n;
  fsize = 2;
  buf = load_data_file_to_buffer(&fsize, FGrp_StdData, "slabs.dat");
  if (buf == NULL)
    return false;
  i = 0;
  total = lword(&buf[i]);
  i += 2;
  // Validate total amount of indices
  if ((total < 0) || (total > (fsize-2)/(9*sizeof(short))))
  {
    total = (fsize-2)/(9*sizeof(short));
    LbWarnLog("Bad amount of indices in Slab Set file; corrected to %ld.\n",total);
  }
  if (total > *scount)
  {
    LbWarnLog("Only %d slabs supported, Slab Set file has %ld.\n",SLABSET_COUNT,total);
    total = *scount;
  }
  for (n=0; n < total; n++)
    for (k=0; k < 9; k++)
    {
      slbset[n].col_idx[k] = lword(&buf[i]);
      i += 2;
    }
  *scount = total;
  LbMemoryFree(buf);
  return true;
}

/*
 * Updates "use" property of given columns set, using given SlabSet entries.
 */
short update_columns_use(struct Column *cols,long ccount,struct SlabSet *sset,long scount)
{
  long i,k;
  long ncol;
  for (i=0; i < ccount; i++)
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

short load_slabclm_file(struct Column *cols, long *ccount)
{
  static const char *func_name="load_slabclm_file";
  long total;
  unsigned char *buf;
  long fsize;
  long i,k;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  fsize = 4;
  buf = load_data_file_to_buffer(&fsize, FGrp_StdData, "slabs.clm");
  if (buf == NULL)
    return false;
  i = 0;
  total = llong(&buf[i]);
  i += 4;
  // Validate total amount of columns
  if ((total < 0) || (total > (fsize-4)/sizeof(struct Column)))
  {
    total = (fsize-4)/sizeof(struct Column);
    LbWarnLog("Bad amount of columns in Column Set file; corrected to %ld.\n",total);
  }
  if (total > *ccount)
  {
    LbWarnLog("Only %d columns supported, Column Set file has %ld.\n",*ccount,total);
    total = *ccount;
  }
  for (k=0; k < total; k++)
  {
    LbMemoryCopy(&cols[k],&buf[i],sizeof(struct Column));
    i += sizeof(struct Column);
  }
  *ccount = total;
  LbMemoryFree(buf);
  return true;
}

short columns_add_static_entries(void)
{
  struct Column col;
  short *wptr;
  short c[3];
  long ncol;
  long i,k;

  for (i=0; i < 3; i++)
    c[i] = 0;
  LbMemorySet(&col,0,sizeof(struct Column));
  wptr = &game.field_14A818[0];
  for (i=0; i < 3; i++)
  {
    memset(&col, 0, sizeof(struct Column));
    col.baseblock = c[i];
    for (k=0; k < 6; k++)
    {
      col.cubes[0] = player_cubes[k];
      make_solidmask(&col);
      ncol = find_column(&col);
      if (ncol == 0)
        ncol = create_column(&col);
      game.columns[ncol].bitfileds |= 0x01;
      *wptr = -(short)ncol;
      wptr++;
    }
  }
  return true;
}

short update_slabset_column_indices(struct Column *cols, long ccount)
{
  static const char *func_name="update_slabset_column_indices";
  struct Column col;
  struct SlabSet *sset;
  long ncol;
  long i,k,n;
  LbMemorySet(&col,0,sizeof(struct Column));
  for (i=0; i < game.slabset_num; i++)
  {
    sset = &game.slabset[i];
    for (k=0; k < 9; k++)
    {
        n = sset->col_idx[k];
        if (n >= 0)
        {
          col.baseblock = n;
          ncol = find_column(&col);
          if (ncol == 0)
          {
            ncol = create_column(&col);
            game.columns[ncol].bitfileds |= 0x01;
          }
        } else
        {
          if (-n < ccount)
            ncol = find_column(&cols[-n]);
          else
            ncol = 0;
          if (ncol == 0)
          {
            error(func_name,3374,"E14R432Q#222564-3; I should be able to find a column here");
            continue;
          }
        }
        sset->col_idx[k] = -ncol;
    }
  }
  return true;
}

short create_columns_from_list(struct Column *cols, long ccount)
{
  long ncol;
  long i;
  for (i=1; i < ccount; i++)
  {
      if (cols[i].use)
      {
        ncol = find_column(&cols[i]);
        if (ncol == 0)
          ncol = create_column(&cols[i]);
        game.columns[ncol].bitfileds |= 0x01;
      }
  }
  return true;
}

short load_slab_datclm_files(void)
{
  static const char *func_name="load_slab_datclm_files";
  struct Column *cols;
  long cols_tot;
  struct SlabSet *slbset;
  long slbset_tot;
  struct SlabSet *sset;
  long i,k,n;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  // Load Column Set
  cols_tot = COLUMNS_COUNT;
  cols = (struct Column *)LbMemoryAlloc(cols_tot*sizeof(struct Column));
  if (cols == NULL)
  {
    LbWarnLog("Can't allocate memory for %d column sets.\n",cols_tot);
    return false;
  }
  if (!load_slabclm_file(cols, &cols_tot))
  {
    LbMemoryFree(cols);
    return false;
  }
  // Load Slab Set
  slbset_tot = SLABSET_COUNT;
  slbset = (struct SlabSet *)LbMemoryAlloc(slbset_tot*sizeof(struct SlabSet));
  if (slbset == NULL)
  {
    LbWarnLog("Can't allocate memory for %d slab sets.\n",slbset_tot);
    return false;
  }
  if (!load_slabdat_file(slbset, &slbset_tot))
  {
    LbMemoryFree(cols);
    LbMemoryFree(slbset);
    return false;
  }
  // Update the structure
  for (i=0; i < slbset_tot; i++)
  {
      sset = &game.slabset[i];
      LbMemoryCopy(sset, &slbset[i], sizeof(struct SlabSet));
  }
  game.slabset_num = slbset_tot;
  update_columns_use(cols,cols_tot,slbset,slbset_tot);
  LbMemoryFree(slbset);
  create_columns_from_list(cols,cols_tot);
  update_slabset_column_indices(cols,cols_tot);
  LbMemoryFree(cols);
  return true;
}

short load_slab_tng_file(void)
{
  char *fname;
  static const char *func_name="load_slab_tng_file";
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  fname = prepare_file_fmtpath(FGrp_StdData,"slabs.tng");
  wait_for_cd_to_be_available();
  if ( LbFileExists(fname) )
    LbFileLoadAt(fname, &game.slabobjs_num);
  else
    error(func_name, 3439, "Could not load slab object set");
  return true;
}

short load_slab_file(void)
{
  static const char *func_name="load_slab_file";
  short result;
  //_DK_load_slab_file(); return true;
  result = true;
  if (!load_slab_datclm_files())
    result = false;
  if (!columns_add_static_entries())
    result = false;
  if (!load_slab_tng_file())
    result = false;
  return result;
}

long load_map_wibble_file(unsigned long lv_num)
{
  struct Map *map;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i,k;
  char *fname;
  long fsize;
  fsize = (map_subtiles_y+1)*(map_subtiles_x+1);
  buf = load_single_map_file_to_buffer(lv_num,"wib",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      k = buf[i];
      k = map->data ^ (k << 22);
      map->data = map->data ^ k & 0xC00000;
      i++;
    }
  LbMemoryFree(buf);
  return true;
}

short load_map_ownership_file(unsigned long lv_num)
{
  struct SlabMap *slbmap;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  char *fname;
  long fsize;
  fsize = (map_subtiles_y+1)*(map_subtiles_x+1);
  buf = load_single_map_file_to_buffer(lv_num,"own",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      slbmap = &game.slabmap[map_to_slab[y]*map_tiles_x + map_to_slab[x]];
      if ((x < map_subtiles_x) && (y < map_subtiles_y))
        slbmap->field_5 ^= (slbmap->field_5 ^ buf[i]) & 7;
      else
        // TODO: This should be set to 5, but some errors prevent it (hang on map 9)
        slbmap->field_5 ^= (slbmap->field_5 ^ 0) & 7;
      i++;
    }
  LbMemoryFree(buf);
  return true;
}

short initialise_map_wlb_auto(void)
{
  static const char *func_name="initialise_map_wlb_auto";
  struct SlabMap *slb;
  unsigned long x,y;
  unsigned long n,nbridge;
  nbridge = 0;
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slb = &game.slabmap[y*map_tiles_x + x];
      if (slb->slab == SlbT_BRIDGE)
      {
        if (slabs_count_near(x,y,1,SlbT_LAVA) > slabs_count_near(x,y,1,SlbT_WATER))
          n = SlbT_LAVA;
        else
          n = SlbT_WATER;
        nbridge++;
      } else
      {
        n = slb->slab;
      }
      n = (slab_attrs[n%SLAB_TYPES_COUNT].field_15 << 3);
      slb->field_5 ^= (slb->field_5 ^ n) & 0x18;
    }
  LbSyncLog("Regenerated WLB flags, unsure for %lu bridge blocks.\n",nbridge);
  return true;
}

short load_map_wlb_file(unsigned long lv_num)
{
  static const char *func_name="load_map_wlb_file";
  struct SlabMap *slb;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  unsigned long n;
  unsigned long nfixes;
  char *text;
  long fsize;
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  nfixes = 0;
  fsize = map_tiles_y*map_tiles_x;
  buf = load_single_map_file_to_buffer(lv_num,"wlb",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slb = &game.slabmap[y*map_tiles_x + x];
      n = (buf[i] << 3);
      n = slb->field_5 ^ ((slb->field_5 ^ n) & 0x18);
      slb->field_5 = n;
      n &= 0x18;
      if ((n != 16) || (slb->slab != SlbT_WATER))
        if ((n != 8) || (slb->slab != SlbT_LAVA))
          if (((n == 16) || (n == 8)) && (slb->slab != SlbT_BRIDGE))
          {
              nfixes++;
              slb->field_5 &= 0xE7u;
          }
      i++;
    }
  LbMemoryFree(buf);
  if (nfixes > 0)
  {
    text = buf_sprintf("WLB file is muddled - Fixed values for %lu tiles",nfixes);
    error(func_name, 4696, text);
  }
  return true;
}

short initialise_extra_slab_info(unsigned long lv_num)
{
  short result;
  //_DK_initialise_extra_slab_info(lv_num); return true;
  initialise_map_rooms();
  result = load_map_wlb_file(lv_num);
  if (!result)
    result = initialise_map_wlb_auto();
  return result;
}

short load_map_slab_file(unsigned long lv_num)
{
  static const char *func_name="load_map_slab_file";
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //return _DK_load_map_slab_file(lv_num);
  struct SlabMap *slbmap;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  unsigned long n;
  long fsize;
  fsize = 2*map_tiles_y*map_tiles_x;
  buf = load_single_map_file_to_buffer(lv_num,"slb",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slbmap = &game.slabmap[y*map_tiles_x + x];
      n = lword(&buf[i]);
      if (n > SLAB_TYPES_COUNT)
      {
        LbWarnLog("Slab Type %d exceeds limit of %d\n",n,SLAB_TYPES_COUNT);
        n = SlbT_ROCK;
      }
      slbmap->slab = n;
      i += 2;
    }
  LbMemoryFree(buf);
  initialise_map_collides();
  initialise_map_health();
  initialise_extra_slab_info(lv_num);
  return true;
}

short load_map_flag_file(unsigned long lv_num)
{
  static const char *func_name="load_map_flag_file";
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  struct Map *map;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  long fsize;
  fsize = 2*(map_subtiles_y+1)*(map_subtiles_x+1);
  buf = load_single_map_file_to_buffer(lv_num,"flg",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      map->flags = buf[i];
      i += 2;
    }
  LbMemoryFree(buf);
  return true;
}

long load_static_light_file(unsigned long lv_num)
{
  unsigned long i;
  long k;
  long total;
  unsigned char *buf;
  struct InitLight ilght;
  long fsize;
  fsize = 4;
  buf = load_single_map_file_to_buffer(lv_num,"lgt",&fsize);
  if (buf == NULL)
    return false;
  light_initialise();
  i = 0;
  total = llong(&buf[i]);
  i += 4;
  // Validate total amount of lights
  if ((total < 0) || (total > (fsize-4)/sizeof(struct InitLight)))
  {
    total = (fsize-4)/sizeof(struct InitLight);
    LbWarnLog("Bad amount of static lights in LGT file; corrected to %ld.\n",total);
  }
  if (total >= LIGHTS_COUNT)
  {
    LbWarnLog("Only %d static lights supported, LGT file has %ld.\n",LIGHTS_COUNT,total);
    total = LIGHTS_COUNT-1;
  } else
  if (total >= LIGHTS_COUNT/2)
  {
    LbWarnLog("More than %d%% of light slots used by static lights.\n",100*total/LIGHTS_COUNT);
  }
  // Create the lights
  for (k=0; k < total; k++)
  {
    LbMemoryCopy(&ilght, &buf[i], sizeof(struct InitLight));
    light_create_light(&ilght);
    i += sizeof(struct InitLight);
  }
  LbMemoryFree(buf);
  return true;
}

short load_and_setup_map_info(unsigned long lv_num)
{
  char *fname;
  unsigned char *buf;
  long fsize;
  fsize = 1;
  buf = load_single_map_file_to_buffer(lv_num,"inf",&fsize);
  if (buf == NULL)
  {
    game.texture_id = 0;
    return false;
  }
  game.texture_id = buf[0];
  LbMemoryFree(buf);
  return true;
}

short load_level_file(LevelNumber lvnum)
{
  static const char *func_name="load_level_file";
  char *fname;
  short fgroup;
  short result;
  //_DK_load_level_file(lvnum); return true;
  fgroup = get_level_fgroup(lvnum);
  fname = prepare_file_fmtpath(fgroup,"map%05lu.slb",(unsigned long)lvnum);
  wait_for_cd_to_be_available();
  if (LbFileExists(fname))
  {
    result = true;
    load_map_data_file(lvnum);
    load_map_flag_file(lvnum);
    load_column_file(lvnum);
    init_whole_blocks();
    load_slab_file();
    init_columns();
    load_static_light_file(lvnum);
    if (!load_map_ownership_file(lvnum))
      result = false;
    load_map_wibble_file(lvnum);
    load_and_setup_map_info(lvnum);
    load_texture_map_file(game.texture_id, 2);
    load_action_point_file(lvnum);
    if (!load_map_slab_file(lvnum))
      result = false;
    if (!load_thing_file(lvnum))
      result = false;
    reinitialise_treaure_rooms();
    ceiling_init(0, 1);
  } else
  {
    LbErrorLog("The level 'map%05lu' doesn't exist; creating empty map.\n",lvnum);
    init_whole_blocks();
    load_slab_file();
    init_columns();
    game.texture_id = 0;
    load_texture_map_file(game.texture_id, 2);
    init_top_texture_to_cube_table();
    result = false;
  }
  return result;
}

short load_map_file(LevelNumber lvnum)
{
  short result;
  result = load_level_file(lvnum);
  if (result)
    set_loaded_level_number(lvnum);
  else
    set_loaded_level_number(SINGLEPLAYER_NOTSTARTED);
  return result;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
