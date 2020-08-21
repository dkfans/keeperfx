/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_fmvids.h
 *     Header file for bflib_fmvids.c.
 * @par Purpose:
 *     Full Motion Videos (Smacker,FLIC) decode & play library.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     27 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_FMVIDS_H
#define BFLIB_FMVIDS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum SmackerPlayFlags {
    SMK_NoStopOnUserInput  = 0x02,
    SMK_PixelDoubleLine    = 0x04,
    SMK_InterlaceLine      = 0x08,
    SMK_WriteStatusFile    = 0x40,
    SMK_PixelDoubleWidth   = 0x80,
    SMK_FullscreenFit      = 0x10,
    SMK_FullscreenStretch  = 0x20,
    SMK_FullscreenCrop     = 0x40,
};

/******************************************************************************/
#pragma pack(1)

// Type definitions
struct SmackTag {
  unsigned long Version;           // SMK2 only right now
  unsigned long Width;             // Width (1 based, 640 for example)
  unsigned long Height;            // Height (1 based, 480 for example)
  unsigned long Frames;            // Number of frames (1 based, 100 = 100 frames)
  unsigned long MSPerFrame;        // Frame Rate
  unsigned long SmackerType;       // bit 0 set=ring frame
  unsigned long LargestInTrack[7]; // Largest single size for each track
  unsigned long tablesize;         // Size of the init tables
  unsigned long codesize;          // Compression info
  unsigned long absize;            // ditto
  unsigned long detailsize;        // ditto
  unsigned long typesize;          // ditto
  unsigned long TrackType[7];      // high byte=0x80-Comp,0x40-PCM data,0x20-16 bit,0x10-stereo
  unsigned long extra;             // extra value (should be zero)
  unsigned long NewPalette;        // set to one if the palette changed
  unsigned char Palette[772];      // palette data
  unsigned long PalType;           // type of palette
  unsigned long FrameNum;          // Frame Number to be displayed
  unsigned long FrameSize;         // The current frame's size in bytes
  unsigned long SndSize;           // The current frame sound tracks' size in bytes
  long LastRectx;         // Rect set in from SmackToBufferRect (X coord)
  long LastRecty;         // Rect set in from SmackToBufferRect (Y coord)
  long LastRectw;         // Rect set in from SmackToBufferRect (Width)
  long LastRecth;         // Rect set in from SmackToBufferRect (Height)
  unsigned long OpenFlags;         // flags used on open
  unsigned long LeftOfs;           // Left Offset used in SmackTo
  unsigned long TopOfs;            // Top Offset used in SmackTo
  unsigned long LargestFrameSize;  // Largest frame size
  unsigned long Highest1SecRate;   // Highest 1 sec data rate
  unsigned long Highest1SecFrame;  // Highest 1 sec data rate starting frame
  unsigned long ReadError;         // Set to non-zero if a read error has ocurred
  unsigned long addr32;            // translated address for 16 bit interface
};

struct SmackSumTag {
  unsigned long TotalTime;         // total time
  unsigned long MS100PerFrame;     // MS*100 per frame (100000/MS100PerFrame=Frames/Sec)
  unsigned long TotalOpenTime;     // Time to open and prepare for decompression
  unsigned long TotalFrames;       // Total Frames displayed
  unsigned long SkippedFrames;     // Total number of skipped frames
  unsigned long SoundSkips;        // Total number of sound skips
  unsigned long TotalBlitTime;     // Total time spent blitting
  unsigned long TotalReadTime;     // Total time spent reading
  unsigned long TotalDecompTime;   // Total time spent decompressing
  unsigned long TotalBackReadTime; // Total time spent reading in background
  unsigned long TotalReadSpeed;    // Total io speed (bytes/second)
  unsigned long SlowestFrameTime;  // Slowest single frame time
  unsigned long Slowest2FrameTime; // Second slowest single frame time
  unsigned long SlowestFrameNum;   // Slowest single frame number
  unsigned long Slowest2FrameNum;  // Second slowest single frame number
  unsigned long AverageFrameSize;  // Average size of the frame
  unsigned long Highest1SecRate;
  unsigned long Highest1SecFrame;
  unsigned long HighestMemAmount;  // Highest amount of memory allocated
  unsigned long TotalExtraMemory;  // Total extra memory allocated
  unsigned long HighestExtraUsed;  // Highest extra memory actually used
};

struct AnimFLIHeader { // sizeof=0x80
    unsigned long dsize;
    unsigned short magic;
    unsigned short frames;
    short width;
    short height;
    unsigned short depth;
    unsigned short flags;
    unsigned long speed;
    short reserved2;
    unsigned long created;
    unsigned long creator;
    unsigned long updated;
    unsigned long updater;
    short aspectx;
    short aspecty;
    char reserved3[38];
    unsigned long oframe1;
    unsigned long oframe2;
    char reserved4[40];
};

struct AnimFLIChunk { //sizeof=0x6
    long csize;
    unsigned short ctype;
};

struct AnimFLIPrefix { //sizeof=0x6
    long csize;
    unsigned short ctype;
    short nchunks;
    char reserved[8];
};

struct Animation { // sizeof=0x3D0
long field_0;
    unsigned char *videobuf;
    unsigned char *chunkdata;
    unsigned char *field_C;
    long inpfhndl;
    long outfhndl;
short field_18;
short field_1A;
    unsigned char palette[768];
long field_31C;
long field_320;
long field_324;
    struct AnimFLIHeader header;
    struct AnimFLIChunk chunk;
    struct AnimFLIPrefix prefix;
    struct AnimFLIChunk subchunk;
char field_3C4[12];
};

typedef void (*SmackDrawCallback)(unsigned char *frame_data, long width, long height);

/******************************************************************************/
// Exported variables

DLLIMPORT struct Animation _DK_animation;
#define animation _DK_animation

#pragma pack()
/******************************************************************************/
// Exported functions - SMK related
short play_smk_(char *fname, int smkflags, int plyflags);
short play_smk_direct(char *fname, int smkflags, int plyflags);
short play_smk_via_buffer(char *fname, int smkflags, int plyflags);

// Exported functions - FLI related
short anim_open(char *fname, int arg1, short arg2, int width, int height, int arg5, unsigned int flags);
short anim_stop(void);
short anim_record(void);
TbBool anim_record_frame(unsigned char *screenbuf, unsigned char *palette);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
