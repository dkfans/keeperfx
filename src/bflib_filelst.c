/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_filelst.c
 *     Reading/freeing of file lists.
 * @par Purpose:
 *     Allows to allocate memory and read multiple files at once.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     02 Mar 2008 - 08 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "globals.h"
#include "bflib_filelst.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/

ModifyDataLoadFnameFunc *modify_data_load_filename_function = defaultModifyDataLoadFilename;

/******************************************************************************/

short LbDataFree(struct TbLoadFiles *load_file)
{
  if (load_file == NULL)
    return 0;
  unsigned char** data = load_file->SEnd;
  if (load_file->FName[0] == '!')
      return 1;
  if (data != NULL)
  {
      (*data) = NULL;
  }
  data = load_file->Start;
  if (data != NULL)
    if ((*data) != NULL)
    {
//#ifdef __DEBUG
        LbJustLog("LbDataFree: freeing \"%s\"...",load_file->FName);
//#endif
        free(*data);
        (*data) = NULL;
//#ifdef __DEBUG
        LbJustLog("done\n");
//#endif
    }
  return 1;
}

void LbDataFreeAll(struct TbLoadFiles load_files[])
{
    struct TbLoadFiles* t_lfile = &load_files[0];
    // note that t_lfile->Start is not NULL even if the buffer is allocated
    while (t_lfile->Start != NULL)
    {
        LbDataFree(t_lfile);
        t_lfile++;
  }
}

void LbDataFreeAllV2(struct TbLoadFilesV2 load_files[])
{
    struct TbLoadFilesV2* t_lfile = &load_files[0];
    struct TbLoadFiles tmp = {0};
    // note that t_lfile->Start is not NULL even if the buffer is allocated
    while (t_lfile->Start != NULL)
    {
        strncpy(tmp.FName, t_lfile->FName, sizeof(tmp.FName) - 1);
        tmp.Start = t_lfile->Start;
        LbDataFree(&tmp);
        t_lfile++;
    }
}

int LbDataLoad(struct TbLoadFiles *load_file, LoadFilesGetSizeFunc get_size_fn, LoadFilesUnpackFunc unpack_fn)
{
  LbDataFree(load_file);
  const char *fname = modify_data_load_filename_function(load_file->FName);
  TbBool is_static = (fname[0] == '!');
  if (is_static)
      fname++;
  if (fname[0] == '*')
  {
#ifdef __DEBUG
      LbJustLog("LbDataLoad: * in fname \"%s\"\n",fname);
#endif
    *(load_file->Start) = calloc(load_file->SLength, 1);
    if ( (*(load_file->Start)) == NULL )
        return -100;
  } else
  {
    long slength = LbFileLengthRnc(fname);
#ifdef __DEBUG
    LbJustLog("LbDataLoad: filelength %ld for file \"%s\"\n",slength,fname);
#endif
    load_file->SLength = get_size_fn? get_size_fn(slength): slength;
    if (slength <= 0)
        return -101;
    if (!is_static)
    {
        *(load_file->Start) = calloc(load_file->SLength + 512, 1);
    }
    if ((*(load_file->Start)) == NULL)
        return -100;
    if (LbFileLoadAt(fname, *(load_file->Start)) != slength)
    {
        *(load_file->Start) = 0;
        if (load_file->SEnd != NULL)
          *(load_file->SEnd) = 0;
        load_file->SLength = 0;
        return -101;
    }
    if (unpack_fn)
        unpack_fn(*(load_file->Start), slength);
  }
  if (load_file->SEnd != NULL)
    *(load_file->SEnd) = *(load_file->Start) + load_file->SLength;
  return 1;
}

/*
 * Loads a list of files. Allocates memory and loads new data.
 * ! - prefix means memory already allocated
 * * - prefix means no file to open
 * @return Returns amount of entries failed, or 0 on success.
 */
int LbDataLoadAll(struct TbLoadFiles load_files[])
{
  LbDataFreeAll(load_files);
  int ferror = 0;
  int i = 0;
  struct TbLoadFiles* t_lfile = &load_files[i];
  while (t_lfile->Start != NULL)
  {
        int ret_val = LbDataLoad(t_lfile, NULL, NULL);
        if (ret_val == -100)
        {
          ERRORLOG("Can't allocate memory for \"%s\"", t_lfile->FName);
          ferror++;
        }
        else if ( ret_val == -101 )
        {
          ERRORLOG("Can't load file \"%s\"", t_lfile->FName);
          ferror++;
        }
        i++;
        t_lfile = &load_files[i];
  }
  return ferror;
}

int LbDataLoadAllV2(struct TbLoadFilesV2 load_files[])
{
    LbDataFreeAllV2(load_files);
    int ferror = 0;
    struct TbLoadFilesV2* t_lfile = &load_files[0];
    while (t_lfile->Start != NULL)
    {
        struct TbLoadFiles tmp = {.Start = t_lfile->Start, .SLength = t_lfile->SLength, 0};
        strncpy(tmp.FName, t_lfile->FName, sizeof(tmp.FName) - 1);

        int ret_val = LbDataLoad(&tmp, t_lfile->GetSizeFunc, t_lfile->UnpackFunc);
        if (ret_val == -100)
        {
            ERRORLOG("Can't allocate memory for \"%s\"", t_lfile->FName);
            ferror++;
        }
        else if ( ret_val == -101 )
        {
            ERRORLOG("Can't load file \"%s\"", t_lfile->FName);
            ferror++;
        }
        t_lfile++;
    }
    return ferror;
}

ModifyDataLoadFnameFunc *LbDataLoadSetModifyFilenameFunction(ModifyDataLoadFnameFunc *newfunc)
{
  modify_data_load_filename_function = newfunc;
  return newfunc;
}

const char * defaultModifyDataLoadFilename(const char * input)
{
     return input;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
