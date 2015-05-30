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

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>

#define SDL_AUDIO_BUFFER_SIZE 1024
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

#define FF_REFRESH_EVENT (SDL_USEREVENT)
// #define FF_QUIT_EVENT (SDL_USEREVENT + 1)

#define VIDEO_PICTURE_QUEUE_SIZE 1

typedef AVPacketList AVPacketNode;
/******************************************************************************/
enum SmackerPlayFlags
{
    SMK_NoSound = 0x01,
};

typedef struct PacketQueue 
{
    AVPacketNode *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

typedef struct VideoPicture
{
    SDL_Texture *texture;
    int width, height; /* source height & width */
    int allocated;
} VideoPicture;

typedef struct VideoState 
{
    double          video_clock; // pts of last decoded frame / predicted pts of next decoded frame

    AVFormatContext *p_format_ctx; // released
    int             video_stream_idx, audio_stream_idx;

// Audio
    int             mute;
    AVStream        *audio_stream; // released
    AVCodecContext  *audio_ctx; // released
    PacketQueue     audio_queue; // released

// Video
    AVStream        *video_stream; // released
    AVCodecContext  *video_ctx; // released
    PacketQueue     video_queue; // released

// Picture
    struct SwsContext *sws_ctx; // released
    VideoPicture    pict_queue[VIDEO_PICTURE_QUEUE_SIZE]; // released
    int             pict_queue_size, pict_queue_read_idx, pict_queue_write_idx;
    SDL_mutex       *pict_queue_mutex;  // released
    SDL_cond        *pict_queue_cond;  // released

// Thread
    SDL_Thread      *decode_thread_id;  // released
    SDL_Thread      *video_thread_id;   // released

    char            filename[1024];
    int             quit;
    int             no_more_packet;
} VideoState;

#pragma pack(1)

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

#pragma pack()
/******************************************************************************/
// Exported variables

DLLIMPORT struct Animation _DK_animation;
#define animation _DK_animation

/******************************************************************************/
// Audio helpers
int _audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size);
void _audio_callback(void *userdata, Uint8 *stream, int len);

// packet operation
void _packet_queue_init(PacketQueue *pq);
void _packet_queue_destroy(PacketQueue *pq);
int _packet_queue_put(PacketQueue *pq, AVPacket *pkt);
int _packet_queue_get(PacketQueue *pq, AVPacket *pkt, int block);

// Timer helpers
double _synchronize_video(VideoState *is, AVFrame *src_frame, double pts);

// Threads
int _decode_thread(void *arg);
int _video_thread(void *arg); 
int _open_smacker_video(VideoState *videoState);
int _open_stream_component(VideoState *is, int stream_index);

// Pictures helpers
void _alloc_picture(void *userdata);
int _prepa_prepare_scaled_framereFrame(AVFrame** ppFrameRGB, uint8_t** pBuffer);
int _queue_picture(VideoState *is, AVFrame *pFrame, double pts);

// Display
void _video_display(VideoPicture *videoPicture);
void video_refresh_timer_callback(void *userdata);

short play_smk_direct(char *fname, int smkflags, int plyflags);

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
