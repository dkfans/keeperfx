/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 27 Nov 2008

// Purpose:
//    Full Motion Videos (Smacker) decode & play library.

// Comment:
//   Sound and music routines to use in games.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#include "bflib_fmvids.h"

#include <string.h>
#include <stdio.h>

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_sndlib.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_fileio.h"
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// Constants and defines
#define FLI_PREFIX 0xF100u
#define FLI_COLOR256 4
#define FLI_SS2      7
#define FLI_COLOR   11
#define FLI_LC      12
#define FLI_BLACK   13
#define FLI_BRUN    15
#define FLI_COPY    16
#define FLI_PSTAMP  18

/******************************************************************************/
// Global variables
void *smack_draw_callback=NULL;

DLLIMPORT long _DK_anim_stop(void);
DLLIMPORT long _DK_anim_record(void);
DLLIMPORT int _DK_anim_open(char *fname, int arg1, short arg2, int width, int height, int arg5, int arg6);
DLLIMPORT long _DK_anim_make_next_frame(unsigned char *screenbuf, unsigned char *palette);
DLLIMPORT long _DK_anim_make_FLI_COLOUR256(unsigned char *palette);
DLLIMPORT long _DK_anim_make_FLI_BRUN(unsigned char *screenbuf);
DLLIMPORT long _DK_anim_make_FLI_SS2(unsigned char *src, unsigned char *dst);
DLLIMPORT long _DK_anim_make_FLI_LC(unsigned char *src, unsigned char *dst);

/******************************************************************************/
// Functions
typedef char (WINAPI *FARPROCP_C)(void *);
typedef unsigned long (WINAPI *FARPROCP_U)(void *);
typedef void (WINAPI *FARPROCP_V)(void *);
typedef void (WINAPI *FARPROCPU_V)(void *,unsigned long);
typedef void (WINAPI *FARPROCU_V)(unsigned long);
typedef struct SmackTag * (WINAPI *FARSMACKOPEN)(const char *,unsigned int,int);
typedef void (WINAPI *FARSMACKSUMMARY)(struct SmackTag *,struct SmackSumTag *);
typedef void (WINAPI *FARSMACKTOBUF)(struct SmackTag *,unsigned long,unsigned long,
    unsigned long,unsigned long,const void *,unsigned long);

char SmackSoundUseMSS(void* dig_driver)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackSoundUseMSS@4");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackSoundUseMSS function; skipped.\n"); return 0; }
    return ((FARPROCP_C)proc)(dig_driver);
}

struct SmackTag *SmackOpen(const char *name,unsigned int flags,int extrabuf)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackOpen@12");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackOpen function; skipped.\n"); return 0; }
    return ((FARSMACKOPEN)proc)(name,flags,extrabuf);
}

void SmackSummary(struct SmackTag *smk,struct SmackSumTag *sum)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackSummary@8");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackSummary function; skipped.\n"); return; }
    ((FARSMACKSUMMARY)proc)(smk,sum);
}

unsigned long SmackWait(struct SmackTag *smk)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackWait@4");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackWait function; skipped.\n"); return 0; }
    return ((FARPROCP_U)proc)(smk);
}

void SmackClose(struct SmackTag *smk)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackClose@4");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackClose function; skipped.\n"); return; }
    ((FARPROCP_V)proc)(smk);
}

unsigned long SmackDoFrame(struct SmackTag *smk)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackDoFrame@4");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackDoFrame function; skipped.\n"); return 0; }
    return ((FARPROCP_U)proc)(smk);
}

void SmackNextFrame(struct SmackTag *smk)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackNextFrame@4");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackNextFrame function; skipped.\n"); return; }
    ((FARPROCP_V)proc)(smk);
}

void SmackToBuffer(struct SmackTag *smk,unsigned long left,unsigned long top,
    unsigned long Pitch,unsigned long destheight,const void *buf,unsigned long Flags)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackToBuffer@28");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackToBuffer function; skipped.\n"); return; }
    ((FARSMACKTOBUF)proc)(smk,left,top,Pitch,destheight,buf,Flags);
}

void SmackGoto(struct SmackTag *smk,unsigned long frame)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackGoto@8");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackGoto function; skipped.\n"); return; }
    ((FARPROCPU_V)proc)(smk,frame);
}

void SmackSimulate(unsigned long sim)
{
    HMODULE hModule;
    hModule=GetModuleHandle("SMACKW32");
    FARPROC proc;
    proc=GetProcAddress(hModule,"_SmackSimulate@4");
    if (proc==NULL)
    { LbErrorLog("Can't get address of SmackSimulate function; skipped.\n"); return; }
    ((FARPROCU_V)proc)(sim);
}

short play_smk_via_buffer(char *fname, int smkflags, int plyflags)
{
  static const char *func_name="play_smk_via_buffer";
  //LbSyncLog("Starting %s.\n",func_name);
  //return _DK_play_smk_via_buffer(fname, smkflags, plyflags);
  static unsigned char palette[768];
  void *snd_driver=GetSoundDriver();
  if ( snd_driver )
    SmackSoundUseMSS(snd_driver);
  else
    plyflags |= 0x01;
  int opnflags = -((plyflags & 0x01) < 1);
  struct SmackTag *smktag = SmackOpen(fname, opnflags & 0xFE000, -1);
  if ( !smktag )
    return 0;
  unsigned long nframe = 1;
  unsigned char *buf = (unsigned char *)LbMemoryAlloc(smktag->Width*smktag->Height);
  if (buf==NULL)
  {
    SmackClose(smktag);
    return 0;
  }
  SmackToBuffer(smktag, 0, 0, smktag->Width, smktag->Height, buf, 0);
  while ( (plyflags & 0x0400) || (smktag->Frames >= nframe) )
  {
      short reset_pal = 0;
      int idx;
      if ( smktag->NewPalette )
      {
        reset_pal = 1;
        for (idx=0;idx<768;idx++)
        {
          unsigned char chr;
          chr = smktag->Palette[idx];
          palette[idx] = chr>>2;
        }
      }
      SmackDoFrame(smktag);
      if ( LbScreenLock() == 1 )
      {
        copy_to_screen(buf, smktag->Width, smktag->Height, plyflags);
        LbScreenUnlock();
        //LbDoMultitasking();
        if ( reset_pal )
        {
          LbScreenWaitVbi();
          LbPaletteSet(palette);
        }
        LbScreenSwap();
      }
      SmackNextFrame(smktag);

      do {
        if (!LbWindowsControl())
        {
            SmackClose(smktag);
            return 2;
        }
        if (((plyflags & 0x02)==0) && (_DK_lbKeyOn[KC_ESCAPE] || _DK_lbKeyOn[KC_RETURN]
            || _DK_lbKeyOn[KC_SPACE] || _DK_lbDisplay.LeftButton) )
        {
            SmackClose(smktag);
            LbMemoryFree(buf);
            return 2;
        }
      } while ( SmackWait(smktag) );
      ++nframe;
  }
  LbMemoryFree(buf);
  SmackClose(smktag);
  return 1;
}

/**
 * Plays Smacker file more directly.
 * @return Returns 0 on error, 1 if file was played, 2 if the play was interrupted.
 */
short play_smk_direct(char *fname, int smkflags, int plyflags)
{
  static const char *func_name="play_smk_direct";
  //LbSyncLog("Starting %s.\n",func_name);
  //return _DK_play_smk_direct(fname, smkflags, plyflags);

  static unsigned char palette[768];
  void *snd_driver=GetSoundDriver();
  if ( snd_driver )
    SmackSoundUseMSS(snd_driver);
  else
    plyflags |= 0x01;
  int opnflags = -((plyflags & 0x01) < 1);
  struct SmackTag *smktag = SmackOpen(fname, opnflags & 0xFE000, -1);
  if ( !smktag )
    return 0;
  unsigned long nframe = 1;
  while ( (plyflags & 0x0400) || (smktag->Frames-1 >= nframe) )
  {
      short reset_pal = 0;
      int idx;
      if ( smktag->NewPalette )
      {
        reset_pal = 1;
        for (idx=0;idx<768;idx++)
        {
          unsigned char chr;
          chr = smktag->Palette[idx];
          palette[idx] = chr>>2;
        }
      }
      if ( LbScreenLock() == 1 )
      {
        int left = 0;
        if ( smktag->Width < _DK_lbDisplay.PhysicalScreenWidth )
          left = (_DK_lbDisplay.PhysicalScreenWidth-smktag->Width) >> 1;
        int top = 0;
        if ( smktag->Height < _DK_lbDisplay.PhysicalScreenHeight )
          top = (_DK_lbDisplay.PhysicalScreenHeight-smktag->Height) >> 1;
        SmackToBuffer(smktag,left,top,_DK_lbDisplay.GraphicsScreenWidth,
            _DK_lbDisplay.GraphicsScreenHeight,_DK_lbDisplay.WScreen,0);
        SmackDoFrame(smktag);
        LbScreenUnlock();
        //LbDoMultitasking();
        if ( reset_pal )
        {
          LbScreenWaitVbi();
          LbPaletteSet(palette);
        }
        LbScreenSwap();
      }
      SmackNextFrame(smktag);

      do {
        if (!LbWindowsControl())
        {
            SmackClose(smktag);
            return 2;
        }
        if (((plyflags & 0x02)==0) && (_DK_lbKeyOn[KC_ESCAPE] || _DK_lbKeyOn[KC_RETURN]
            || _DK_lbKeyOn[KC_SPACE] || _DK_lbDisplay.LeftButton) )
        {
            SmackClose(smktag);
            return 2;
        }
      } while ( SmackWait(smktag) );
      ++nframe;
  }
  if ( plyflags & 0x40 )
  {
      struct SmackSumTag smksum;
      SmackSummary(smktag, &smksum);
      FILE *ssp = fopen("smacksum.txt", "w");
      if ( ssp )
      {
          fprintf(ssp, "TotalTime         = %ld\n", smksum.TotalTime);
          fprintf(ssp, "MS100PerFrame     = %ld\n", smksum.MS100PerFrame);
          fprintf(ssp, "TotalOpenTime     = %ld\n", smksum.TotalOpenTime);
          fprintf(ssp, "TotalFrames       = %ld\n", smksum.TotalFrames);
          fprintf(ssp, "SkippedFrames     = %ld\n", smksum.SkippedFrames);
          fprintf(ssp, "TotalBlitTime     = %ld\n", smksum.TotalBlitTime);
          fprintf(ssp, "TotalReadTime     = %ld\n", smksum.TotalReadTime);
          fprintf(ssp, "TotalDecompTime   = %ld\n", smksum.TotalDecompTime);
          fprintf(ssp, "TotalBackReadTime = %ld\n", smksum.TotalBackReadTime);
          fprintf(ssp, "TotalReadSpeed    = %ld\n", smksum.TotalReadSpeed);
          fprintf(ssp, "SlowestFrameTime  = %ld\n", smksum.SlowestFrameTime);
          fprintf(ssp, "Slowest2FrameTime = %ld\n", smksum.Slowest2FrameTime);
          fprintf(ssp, "SlowestFrameNum   = %ld\n", smksum.SlowestFrameNum);
          fprintf(ssp, "Slowest2FrameNum  = %ld\n", smksum.Slowest2FrameNum);
          fprintf(ssp, "AverageFrameSize  = %ld\n", smksum.AverageFrameSize);
          fprintf(ssp, "Highest1SecRate   = %ld\n", smksum.Highest1SecRate);
          fprintf(ssp, "Highest1SecFrame  = %ld\n", smksum.Highest1SecFrame);
          fprintf(ssp, "HighestMemAmount  = %ld\n", smksum.HighestMemAmount);
          fprintf(ssp, "TotalExtraMemory  = %ld\n", smksum.TotalExtraMemory);
          fprintf(ssp, "HighestExtraUsed  = %ld\n", smksum.HighestExtraUsed);
          fclose(ssp);
      }
      FILE *svp = fopen(fname, "rb");
      FILE *sgp = fopen("smkgraph.raw", "wb");
      if ( (sgp) && (svp) )
      {
        int idx;
        fseek(svp, 104, 0);
        for (idx=0;idx<smksum.TotalFrames;idx++)
        {
          unsigned long rawval;
          unsigned short val;
          fread(&rawval, 4u, 1u, svp);
          if ( rawval < 65535 )
            val = rawval-12000;
          else
            val = 32767;
          fwrite(&val, 2u, 1u, sgp);
        }
      }
      if ( sgp )
        fclose(sgp);
      if ( svp )
        fclose(svp);
  }
  SmackClose(smktag);
  return 1;
}

short play_smk_(char *fname, int smkflags, int plyflags)
{
  short result;
  _DK_lbDisplay.LeftButton = 0;
  if ( (smack_draw_callback) || (plyflags & 0x8C) )
    result = play_smk_via_buffer(fname, smkflags, plyflags);
  else
    result = play_smk_direct(fname, smkflags, plyflags);
  return result;
}

/**
 * Writes the data into FLI animation.
 * @return Returns false on error, true on success.
 */
short anim_write_data(void *buf, long size)
{
  if (LbFileWrite(animation.outfhndl,buf,size) == size)
  {
    return true;
  }
  return false;
}

/**
 * Stores data into FLI buffer.
 * @return Returns false on error, true on success.
 */
short anim_store_data(void *buf, long size)
{
  memcpy(animation.field_C, buf, size);
  animation.field_C += size;
  return true;
}

/**
 * Reads the data from FLI animation.
 * @return Returns false on error, true on success.
 */
short anim_read_data(void *buf, long size)
{
  if (buf == NULL)
  {
    LbFileSeek(animation.inpfhndl,size,Lb_FILE_SEEK_CURRENT);
    return true;
  } else
  if (LbFileRead(animation.inpfhndl,buf,size) == size)
  {
    return true;
  }
  return false;
}

long anim_make_FLI_COPY(unsigned char *screenbuf)
{
  int scrpoints = animation.header.height * animation.header.width;
  memcpy(animation.field_C, screenbuf, scrpoints);
  animation.field_C += scrpoints;
  return scrpoints;
}

long anim_make_FLI_BLACK(unsigned char *screenbuf)
{
  return 0;
}

long anim_make_FLI_COLOUR256(unsigned char *palette)
{
  return _DK_anim_make_FLI_COLOUR256(palette);
}

/**
 * Compress data into FLI's BRUN block (8-bit Run-Length compression).
 * @return Returns unpacked size of the block which was compressed.
 */
long anim_make_FLI_BRUN(unsigned char *screenbuf)
{
  //return _DK_anim_make_FLI_BRUN(screenbuf);
  unsigned char *blk_begin = animation.field_C;
  short w,h,k,count;
  unsigned char *sbuf = screenbuf;
  for ( h = animation.header.height; h>0; h-- )
  {
    animation.field_C++;
    for (w=animation.header.width; w>0; )
    {
      count = 0;
      // Counting size of RLE block
      for ( k=1; w>1; k++ )
      {
        if (sbuf[k] != sbuf[0]) break;
        if (count == 127) break;
        w--;
        count++;
      }
      // If RLE block would be valid
      if ( count>0 )
      {
        if ( count < 127 )
          { count++; w--; }
        *animation.field_C = (char)count;
        animation.field_C++;
        *animation.field_C = sbuf[0];
        animation.field_C++;
        sbuf += count;
      } else
      {
        if ( w > 1 )
        {
          count=0;
          // Find the next block of at least 4 same pixels
          for ( k = 0; w>0; k++ )
          {
            if ( (sbuf[k+1]==sbuf[k]) && (sbuf[k+2]==sbuf[k]) && (sbuf[k+3]==sbuf[k]) )
              break;
            if ( count == -127 )
              break;
            count--;
            w--;
          }
        } else
        { count=-1; w--; }
        if ( count!=0 )
        {
          *animation.field_C = (char)count;
          animation.field_C++;
          memcpy(animation.field_C, sbuf, -count);
          sbuf -= count;
          animation.field_C -= count;
        }
      }
    }
  }
  // Make the block size even
  if ((int)animation.field_C & 1)
  {
    *animation.field_C='\0';
    animation.field_C++;
  }
  return (animation.field_C - blk_begin);
}

/**
 * Compress data into FLI's SS2 block.
 * @return Returns unpacked size of the block which was compressed.
 */
long anim_make_FLI_SS2(unsigned char *src, unsigned char *dst)
{
  return _DK_anim_make_FLI_SS2(src, dst);
}

/**
 * Compress data into FLI's LC block.
 * @return Returns unpacked size of the block which was compressed.
 */
long anim_make_FLI_LC(unsigned char *src, unsigned char *dst)
{
  return _DK_anim_make_FLI_LC(src, dst);
  //TODO: finish the encoding code
  unsigned char *blk_begin = animation.field_C;
  unsigned char *sbuf;
  unsigned char *dbuf;
  short w,h,cntr;
  short blockh;
  int blksize;
  sbuf = src;
  dbuf = dst;
  for (h = animation.header.height; h>0; h--)
  {
    cntr = 0;
    for (w = animation.header.width; w>0; w--)
    {
      if (sbuf[cntr] != dbuf[cntr]) break;
      cntr++;
    }
    if (cntr != animation.header.width)
      break;
    sbuf += lbDisplay.GraphicsScreenWidth;
    dbuf += lbDisplay.GraphicsScreenWidth;
  }

  if ( h>0 )
  {
    blockh = animation.header.height-h;
    blksize = animation.header.width*(animation.header.height-1);
    sbuf = src+blksize;
    dbuf = dst+blksize;
    for (h = animation.header.height; h>0; h--)
    {
      cntr = 0;
      for (w = animation.header.width; w>0; )
      {
        if (sbuf[cntr] != dbuf[cntr]) break;
        w--;
        cntr++;
      }
      if (cntr != animation.header.width)
        break;
      sbuf -= lbDisplay.GraphicsScreenWidth;
      dbuf -= lbDisplay.GraphicsScreenWidth;
    }

    *(short *)animation.field_C = blockh;
    animation.field_C += 2;

    blockh = h-blockh;
    blksize = animation.header.width * blockh;
    sbuf = src+blksize;
    dbuf = dst+blksize;

    *(short *)animation.field_C = blockh;
    animation.field_C += 2;

    h = blockh;
    while (h > 0)
    {
//TODO
        h--;
        sbuf += lbDisplay.GraphicsScreenWidth;
        dbuf += lbDisplay.GraphicsScreenWidth;
    }
  } else
  {
    *(short *)animation.field_C = 0;
    animation.field_C += 2;
    *(short *)animation.field_C = 1;
    animation.field_C += 2;
    *(char *)animation.field_C = 0;
    animation.field_C++;
  }
  // Make the block size even
  animation.field_C = (unsigned char *)(((unsigned int)animation.field_C+1) & 0xFFFFFFFE);
  return (animation.field_C - blk_begin);
}

short anim_stop(void)
{
  static const char *func_name="anim_stop";
  LbSyncLog("Finishing movie recording.\n");
  //return _DK_anim_stop();
  if ( ((animation.field_0 & 0x01)==0) || (animation.outfhndl==0))
  {
    error(func_name, 44563, "Can't stop recording movie");
    return false;
  }
  LbFileSeek(animation.outfhndl, 0, Lb_FILE_SEEK_BEGINNING);
  animation.header.frames--;
  LbFileWrite(animation.outfhndl, &animation.header, sizeof(struct AnimFLIHeader));
  if ( LbFileClose(animation.outfhndl) == -1 )
  {
      error(func_name, 44564, "Can't close movie file");
      return false;
  }
  animation.outfhndl = 0;
  animation.field_0 = 0;
  return true;
}

short anim_open(char *fname, int arg1, short arg2, int width, int height, int bpp, unsigned int flags)
{
  static const char *func_name="anim_open";
  //return _DK_anim_open(fname, arg1, arg2, width, height, bpp, flags);

  if ( flags & animation.field_0 )
  {
    error(func_name, 44567, "Cannot record movie");
    return false;
  }
  if (flags & 0x01)
  {
      LbSyncLog("Starting to record new movie, \"%s\".\n",fname);
      memset(&animation, 0, sizeof(struct Animation));
      animation.field_0 |= flags;
      animation.videobuf = LbMemoryAlloc(2 * height*width);
      if (animation.videobuf==NULL)
      {
        error(func_name, 44568, "Cannot allocate video buffer.");
        return false;
      }
      animation.chunkdata = LbMemoryAlloc(2 * height*width);
      if (animation.chunkdata==NULL)
      {
        error(func_name, 44569, "Cannot allocate chunk buffer.");
        return false;
      }
      animation.outfhndl = LbFileOpen(fname, Lb_FILE_MODE_NEW);
      if (animation.outfhndl == -1)
      {
        error(func_name, 44570, "Can't open movie file.");
        return false;
      }
      animation.header.dsize = 128;
      animation.header.magic = 0xAF12;
      animation.header.depth = bpp;
      animation.header.flags = 3;
      animation.header.speed = 57;
      animation.header.created = 0;
      animation.header.frames = 0;
      animation.header.width = width;
      animation.header.updated = 0;
      animation.header.aspectx = 6;
      animation.header.height = height;
      animation.header.reserved2 = 0;
      animation.header.creator = 0x464C4942;//'BILF'
      animation.header.aspecty = 5;
      animation.header.updater = 0x464C4942;
      memset(animation.header.reserved3, 0, sizeof(animation.header.reserved3));
      animation.header.oframe1 = 0;
      animation.header.oframe2 = 0;
      memset(animation.header.reserved4, 0, sizeof(animation.header.reserved4));
      animation.field_18 = arg2;
      if ( !anim_write_data(&animation.header, sizeof(struct AnimFLIHeader)) )
      {
        error(func_name, 44571, "Movie write error.");
        LbFileClose(animation.outfhndl);
        return false;
      }
      animation.field_31C = 0;
      animation.field_320 = height*width + 1024;
      memset(animation.palette, -1, sizeof(animation.palette));
  }
  if (flags & 0x02)
  {
      LbSyncLog("Resuming movie recording, \"%s\".\n",fname);
      animation.field_0 |= flags;
      animation.chunkdata = LbMemoryAlloc(2 * height*width);
      if (animation.chunkdata==NULL)
        return false;
      animation.inpfhndl = LbFileOpen(fname, 2);
      if ( animation.inpfhndl == -1 )
        return false;

      if (!anim_read_data(&animation.header, sizeof(struct AnimFLIHeader)))
      {
        error(func_name, 44572, "Movie header read error.");
        LbFileClose(animation.inpfhndl);
        return false;
      }
      if (!anim_read_data(&animation.chunk, sizeof(struct AnimFLIChunk)))
      {
        error(func_name, 44573, "Movie chunk read error.");
        LbFileClose(animation.inpfhndl);
        return false;
      }
      if ( animation.chunk.ctype == FLI_PREFIX )
      {
        if (!anim_read_data(animation.chunkdata, animation.chunk.csize-sizeof(struct AnimFLIChunk)))
        {
          error(func_name, 44574, "Movie data read error.");
          LbFileClose(animation.inpfhndl);
          return false;
        }
      } else
      {
        LbFileSeek(animation.inpfhndl, -sizeof(struct AnimFLIChunk), Lb_FILE_SEEK_CURRENT);
      }
      animation.field_31C = 0;
  }
  return true;
}

short anim_make_next_frame(unsigned char *screenbuf, unsigned char *palette)
{
  static const char *func_name="anim_make_next_frame";
  //return _DK_anim_make_next_frame(screenbuf, palette);
  unsigned long bufsize;
  unsigned char *dataptr;
  long brun_size,lc_size,ss2_size,size;
  int width = MyScreenWidth/pixel_size;
  int height = MyScreenHeight/pixel_size;
  animation.field_C = animation.chunkdata;
  bufsize = height*width + 32768;
  memset(animation.chunkdata, 0, bufsize);
  animation.prefix.ctype = 0xF1FAu;
  animation.prefix.nchunks = 0;
  animation.prefix.csize = 0;
  memset(animation.prefix.reserved, 0, sizeof(animation.prefix.reserved));
  struct AnimFLIPrefix *prefx = (struct AnimFLIPrefix *)animation.field_C;
  anim_store_data(&animation.prefix, sizeof(struct AnimFLIPrefix));
  animation.subchunk.ctype = 0;
  animation.subchunk.csize = 0;
  struct AnimFLIChunk *subchnk = (struct AnimFLIChunk *)animation.field_C;
  anim_store_data(&animation.subchunk, sizeof(struct AnimFLIChunk));
  if ( animation.field_31C == 0 )
  {
      animation.header.oframe1 = animation.header.dsize;
  } else
  if ( animation.field_31C == 1 )
  {
      animation.header.oframe2 = animation.header.dsize;
  }
  if ( anim_make_FLI_COLOUR256(palette) )
  {
    prefx->nchunks++;
    subchnk->ctype = 4;
    subchnk->csize = animation.field_C-(unsigned char *)subchnk;
    animation.subchunk.ctype = 0;
    animation.subchunk.csize = 0;
    subchnk = (struct AnimFLIChunk *)animation.field_C;
    anim_store_data(&animation.subchunk, sizeof(struct AnimFLIChunk));
  }
  int scrpoints = animation.header.height * animation.header.width;
  if (animation.field_31C == 0)
  {
    if ( anim_make_FLI_BRUN(screenbuf) )
    {
      prefx->nchunks++;
      subchnk->ctype = FLI_BRUN;
    } else
    {
      anim_make_FLI_COPY(screenbuf);
      prefx->nchunks++;
      subchnk->ctype = FLI_COPY;
    }
  } else
  {
    // Determining the best compression method
    dataptr = animation.field_C;
    brun_size = anim_make_FLI_BRUN(screenbuf);
    memset(animation.field_C, 0, size);
    animation.field_C = dataptr;
    ss2_size = anim_make_FLI_SS2(screenbuf, animation.videobuf);
    memset(dataptr, 0, ss2_size);
    animation.field_C = dataptr;
    lc_size = anim_make_FLI_LC(screenbuf, animation.videobuf);
    if ((lc_size < ss2_size) && (lc_size < brun_size))
    {
        // Store the LC compressed data
        prefx->nchunks++;
        subchnk->ctype = FLI_LC;
    } else
    if (ss2_size < brun_size)
    {
      // Clear the LC compressed data
      memset(dataptr, 0, lc_size);
      animation.field_C = dataptr;
      // Compress with SS2 method
      anim_make_FLI_SS2(screenbuf, animation.videobuf);
      prefx->nchunks++;
      subchnk->ctype = FLI_SS2;
    } else
    if ( brun_size < scrpoints+16 )
    {
      // Clear the LC compressed data
      memset(dataptr, 0, lc_size);
      animation.field_C = dataptr;
      // Compress with BRUN method
      anim_make_FLI_BRUN(screenbuf);
      prefx->nchunks++;
      subchnk->ctype = FLI_BRUN;
    } else
    {
      // Clear the LC compressed data
      memset(dataptr, 0, lc_size);
      animation.field_C = dataptr;
      // Store uncompressed frame data
      anim_make_FLI_COPY(screenbuf);
      prefx->nchunks++;
      subchnk->ctype = FLI_COPY;
    }
  }
  subchnk->csize = animation.field_C-(unsigned char *)subchnk;
  prefx->csize = animation.field_C - animation.chunkdata;
  if ( !anim_write_data(animation.chunkdata, animation.field_C-animation.chunkdata) )
  {
    return false;
  }
  memcpy(animation.videobuf, screenbuf, height*width);
  memcpy(animation.palette, palette, sizeof(animation.palette));
  animation.header.frames++;
  animation.field_31C++;
  animation.header.dsize += animation.field_C-animation.chunkdata;
  return true;
}

short anim_record_frame(unsigned char *screenbuf, unsigned char *palette)
{
  if ((animation.field_0 & 0x01)==0)
    return false;
  struct TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(lbDisplay.ScreenMode);
  if ( mdinfo->BitsPerPixel != animation.header.depth )
    return false;
  if ((MyScreenWidth/pixel_size != animation.header.width) ||
      (MyScreenHeight/pixel_size != animation.header.height))
    return false;
  return anim_make_next_frame(screenbuf, palette);
}

short anim_record(void)
{
  static const char *func_name="anim_record";
  //return _DK_anim_record();
  static char finalname[255];
  TbFileHandle fh;
  struct TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(lbDisplay.ScreenMode);
  if ( mdinfo->BitsPerPixel != 8 )
  {
    error(func_name, 179, "Cannot record movie in non-8bit screen mode");
    return 0;
  }
  int idx;
  for (idx=0;idx<10000;idx++)
  {
      sprintf(finalname, "%s/game%04d.flc","scrshots",idx);
      if (LbFileExists(finalname))
        continue;
      return anim_open(finalname, 0, 0, MyScreenWidth/pixel_size,MyScreenHeight/pixel_size,8, 1);
  }
  error(func_name, 180, "No free file name for recorded movie");
  return 0;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
