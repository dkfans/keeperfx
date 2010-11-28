/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_lenses.c
 *     Eye lense algorithms config functions.
 * @par Purpose:
 *     Support of configuration files for eye lenses.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 07 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_lenses.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_doors.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_lenses_file[]="lenses.cfg";

const struct NamedCommand lenses_common_commands[] = {
  {"EYELENSESCOUNT",  1},
  {NULL,              0},
  };

const struct NamedCommand lenses_data_commands[] = {
  {"NAME",            1},
  {"MIST",            2},
  {"DISPLACEMENT",    3},
  {"PALETTE",         4},
  {NULL,              0},
  };

/******************************************************************************/
struct LensesConfig lenses_conf;
struct NamedCommand lenses_desc[LENS_ITEMS_MAX];
/******************************************************************************/

struct LensConfig *get_lens_config(long lens_idx)
{
    if ((lens_idx < 1) || (lens_idx > lenses_conf.lenses_count))
        return &lenses_conf.lenses[0];
    return &lenses_conf.lenses[lens_idx];
}

TbBool parse_lenses_common_blocks(char *buf,long len)
{
  static const char config_textname[] = "Lenses config";
  long pos;
  int k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize block data
  lenses_conf.lenses_count = 1;
  // Find the block
  sprintf(block_buf,"common");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
    return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(lenses_common_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,lenses_common_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // EYELENSESCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= LENS_ITEMS_MAX))
            {
              lenses_conf.lenses_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
              cmd_num,block_buf,config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
#undef COMMAND_TEXT
  return true;
}

TbBool parse_lenses_data_blocks(char *buf,long len)
{
  static const char config_textname[] = "Lenses config";
  long pos;
  int i,k,n;
  int cmd_num;
  char *fname;
  struct LensConfig *lenscfg;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the array
  int arr_size = sizeof(lenses_conf.lenses)/sizeof(lenses_conf.lenses[0]);
  for (i=0; i < arr_size; i++)
  {
    lenscfg = &lenses_conf.lenses[i];
    LbMemorySet(lenscfg->name, 0, COMMAND_WORD_LEN);
    LbMemorySet(lenscfg->mist_file, 0, DISKPATH_SIZE);
    lenscfg->mist_lightness = 0;
    lenscfg->mist_ghost = 0;
    lenscfg->displace_kind = 0;
    lenscfg->displace_magnitude = 0;
    lenscfg->displace_period = 1;
    LbMemorySet(lenscfg->palette, 0, PALETTE_SIZE*sizeof(TbPixel));
    lenscfg->flags = 0;
    if (i < lenses_conf.lenses_count)
    {
      lenses_desc[i].name = lenscfg->name;
      lenses_desc[i].num = i;
    } else
    {
      lenses_desc[i].name = NULL;
      lenses_desc[i].num = 0;
    }
  }
  arr_size = lenses_conf.lenses_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"lens%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      continue;
    }
    lenscfg = &lenses_conf.lenses[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(lenses_data_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,lenses_data_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,lenscfg->name,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
            break;
          }
          n++;
          break;
      case 2: // MIST
          if (get_conf_parameter_single(buf,&pos,len,lenscfg->mist_file,DISKPATH_SIZE) > 0)
          {
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k >= 0) && (k < 64))
            {
                lenscfg->mist_lightness = k;
                n++;
            }
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k >= 0) && (k < 256))
            {
                lenscfg->mist_ghost = k;
                n++;
            }
          }
          if (n < 3)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          } else
          {
              lenscfg->flags |= LCF_HasMist;
          }
          break;
      case 3: // DISPLACEMENT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k < 256))
            {
                lenscfg->displace_kind = k;
                n++;
            }
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k >= 0) && (k < 512))
            {
                lenscfg->displace_magnitude = k;
                n++;
            }
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k < 512))
            {
                lenscfg->displace_period = k;
                n++;
            }
          }
          if (n < 3)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          } else
          {
              lenscfg->flags |= LCF_HasDisplace;
          }
          break;
      case 4: // PALETTE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              fname = prepare_file_path(FGrp_StdData,word_buf);
              if ( LbFileLoadAt(fname, lenscfg->palette) == PALETTE_SIZE)
              {
                n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
          } else
          {
              lenscfg->flags |= LCF_HasPalette;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
              cmd_num,block_buf,config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
  }
  return true;
}

TbBool load_lenses_config(const char *conf_fname,unsigned short flags)
{
  char *fname;
  char *buf;
  long len;
  TbBool result;
  SYNCDBG(0,"Reading Lenses config file \"%s\".",conf_fname);
  fname = prepare_file_path(FGrp_FxData,conf_fname);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    WARNMSG("Lenses config file \"%s\" doesn't exist or is too small.",conf_fname);
    return false;
  }
  if (len > 65536)
  {
    WARNMSG("Lenses config file \"%s\" is too large.",conf_fname);
    return false;
  }
  buf = (char *)LbMemoryAlloc(len+256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  result = (len > 0);
  if (result)
  {
    result = parse_lenses_common_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing Lenses file \"%s\" common blocks failed.",conf_fname);
  }
  if (result)
  {
    result = parse_lenses_data_blocks(buf, len);
    if (!result)
      WARNMSG("Parsing Lenses file \"%s\" data blocks failed.",conf_fname);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return result;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
