/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_fmvids.c
 *     Full Motion Videos displaying routines.
 * @par Purpose:
 *     Functions to show and maintain movie sequences.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 10 Jun 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "front_fmvids.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_sound.h"
#include "bflib_video.h"
#include "bflib_fmvids.h"
#include "bflib_mouse.h"
#include "bflib_fileio.h"

#include "frontend.h"
#include "front_simple.h"
#include "config.h"
#include "vidmode.h"
#include "vidfade.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const struct DemoItem demo_item[] = {
    {DIK_SwitchState, (char *)FeSt_CREDITS},
/*
    {DIK_LoadPacket, "PACKET1.SAV"},
    {DIK_LoadPacket, "PACKET2.SAV"},
*/
    {DIK_PlaySmkVideo, "intromix.smk"},
    {DIK_ListEnd, NULL},
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Plays a smacker animation file and sets frontend state to nstate.
 * @param nstate Frontend state; -1 means no change, -2 means don't even
 *    change screen mode.
 * @return Returns false if fatal error occurred and program execution should end.
 */
short play_smacker_file(char *filename, FrontendMenuState nstate)
{
  unsigned int movie_flags = 0;
  if (resize_movies_enabled())
  {
    movie_flags |= vid_scale_flags; // get new scaling settings from command line
  }
  if ( SoundDisabled )
    movie_flags |= 0x01;

  short result = 1;
  if ((result)&&(nstate>-2))
  {
    if ( setup_screen_mode_minimal(get_movies_vidmode()) )
    {
      LbMouseChangeSprite(NULL);
      LbScreenClear(0);
      LbScreenSwap();
    } else
    {
      ERRORLOG("Can't enter movies video mode to play a Smacker file");
      result=0;
    }
  }
  if (result)
  {
    // Fail in playing shouldn't set result=0, because result=0 means fatal error.
    if (play_smk_(filename, 0, movie_flags | 0x100) == 0)
    {
      ERRORLOG("Smacker play error");
      result=0;
    }
  }
  if (nstate>-2)
  {
    if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
    {
      ERRORLOG("Can't re-enter frontend video mode after playing Smacker file");
      FatalError = 1;
      exit_keeper = 1;
      return 0;
    }
  } else
  {
    memset(frontend_palette, 0, PALETTE_SIZE);
  }
  LbScreenClear(0);
  LbScreenSwap();
  LbPaletteSet(frontend_palette);
  if (nstate >= 0)
    frontend_set_state(nstate);
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
  lbDisplay.MiddleButton = 0;
  return result;
}

TbBool intro(void)
{
    char* fname = prepare_file_path(FGrp_LoData, "intromix.smk");
    SYNCDBG(0,"Playing intro movie \"%s\"",fname);
    return play_smacker_file(fname, FeSt_MAIN_MENU);
}

TbBool intro_replay(void)
{
    char* fname = prepare_file_path(FGrp_LoData, "intromix.smk");
    SYNCDBG(0,"Playing intro movie \"%s\"",fname);
    return play_smacker_file(fname, -2);
}

TbBool campaign_intro(void)
{
    if (campaign.movie_intro_fname[0] == '\0') {
        SYNCDBG(0,"No intro movie defined");
        frontend_set_state(FeSt_LAND_VIEW);
        return 0;
    }
    char* fname = prepare_file_path(FGrp_CmpgMedia, campaign.movie_intro_fname);
    SYNCDBG(0,"Playing intro movie \"%s\"",fname);
    return play_smacker_file(fname, FeSt_LAND_VIEW);
}

TbBool campaign_outro(void)
{
    if (campaign.movie_outro_fname[0] == '\0') {
        SYNCDBG(0,"No outro movie defined");
        return 0;
    }
    char* fname = prepare_file_path(FGrp_CmpgMedia, campaign.movie_outro_fname);
    SYNCDBG(0,"Playing outro movie \"%s\"",fname);
    return play_smacker_file(fname, FeSt_LEVEL_STATS);
}

TbBool moon_video(void)
{
    char* fname = prepare_file_path(FGrp_LoData, "bullfrog.smk");
    SYNCDBG(0,"Playing outro movie \"%s\"",fname);
    return play_smacker_file(fname, -2);
}

TbBool drag_video(void)
{
    char* fname = prepare_file_path(FGrp_LoData, "drag.smk");
    SYNCDBG(0,"Playing outro movie \"%s\"",fname);
    return play_smacker_file(fname, FeSt_TORTURE);
}

void demo(void)
{
    static long index = 0;
    char *fname;
    switch (demo_item[index].numfield_0)
    {
    case DIK_PlaySmkVideo:
        fname = prepare_file_path(FGrp_LoData,demo_item[index].fname);
        play_smacker_file(fname, FeSt_MAIN_MENU);
        break;
    case DIK_LoadPacket:
        fname = prepare_file_path(FGrp_FxData,demo_item[index].fname);
        wait_for_cd_to_be_available();
        if ( LbFileExists(fname) )
        {
          strcpy(game.packet_fname, fname);
          game.packet_load_enable = 1;
          game.turns_fastforward = 0;
          frontend_set_state(FeSt_PACKET_DEMO);
        }
        break;
    case DIK_SwitchState:
        frontend_set_state((long)demo_item[index].fname);
        break;
    }
    index++;
    if (demo_item[index].numfield_0 == DIK_ListEnd)
      index = 0;
}
/******************************************************************************/
