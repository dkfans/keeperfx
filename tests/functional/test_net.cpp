/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file test_net.c
 *     Main file for network testing utility
 * @par Purpose:
 *     To test Mid level TCP service API (retransmission etc).
 * @par Comment:
 *     None.
 * @author   The KeeperFX Team
 * @date     20 Oct 2020 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include <stdio.h>
#include <stdarg.h>

#include <bflib_basics.h>
#include <bflib_datetm.h>
#include <bflib_network.h>
#include <bflib_netsession.h>
#include <game_legacy.h>
#include <packets.h>

#define   NET_DELAY  30
extern "C" {

int LbJustLog(const char *format, ...)
{
    va_list val;
    va_start(val, format);
    vfprintf(stdout, format, val);
    va_end(val);
    return 0;
}

int LbNetLog(const char *format, ...)
{
    va_list val;
    va_start(val, format);
    vfprintf(stdout, format, val);
    va_end(val);
    return 0;
}

int LbWarnLog(const char *format, ...)
{
    va_list val;
    va_start(val, format);
    vfprintf(stdout, format, val);
    va_end(val);
    return 0;
}

int LbErrorLog(const char *format, ...)
{
    va_list val;
    va_start(val, format);
    vfprintf(stdout, format, val);
    va_end(val);
    return 0;
}

int LbSyncLog(const char *format, ...)
{
    va_list val;
    va_start(val, format);
    vfprintf(stdout, format, val);
    va_end(val);
    return 0;
}

TbBool is_onscreen_msg_visible()
{
    return 0;
}

TbBool show_onscreen_msg(int nturns, const char *format, ...)
{
    va_list val;
    va_start(val, format);
    vfprintf(stdout, format, val);
    va_end(val);
    return true;
}

}

struct TbNetworkPlayerInfo playerinfo[4];

static TbBool client_packet_cb(void *context, unsigned long turn, int plyr_idx, unsigned char kind, void *packet_data, short size)
{
    unsigned short *data = (unsigned short *)packet_data;
    fprintf(stderr, "*** cb kind:%d from:%d data:%d %d %d\n",
        kind, plyr_idx, data[0], data[1], data[2]);
    return true;
}

static TbBool server_packet_cb(void *context, unsigned long turn, int plyr_idx, unsigned char kind, void *packet_data, short size)
{
    return client_packet_cb(context, turn, plyr_idx, kind, packet_data, size);
}

struct ClientAction
{
    unsigned long turn;
    long rep;
    void (*action)(ptrdiff_t idx, long rep);
};

ClientAction actions[16384] = {0};

static void process_actions()
{
    // Dirty!
    static ClientAction *next_action = &actions[0];
    while (next_action->action && (game.play_gameturn >= next_action->turn))
    {
        for (long i = 0; i < next_action->rep; i++)
        {
            next_action->action(next_action - actions, i);
        }
        next_action++;
    }
}

void run_server()
{
    struct ServiceInitData idata;
    unsigned long plyr_num = 0;
    char nsname[16] = "nsname";
    char clientname[16] = "server";

    TbError err = LbNetwork_Init(TbNetworkService::NS_TCP_IP, 
        4,  playerinfo,  &idata);
    fprintf(stderr, "LbNetwork_Init -> %d\n", err);

    err = LbNetwork_Create(nsname, clientname, &plyr_num, NULL);
    fprintf(stderr, "LbNetwork_Create -> %d\n", err);

    //while (LbTimerClock() < 60 * 1000)
    for (int j = 0; j < 60*1000; j++)
    {
        game.play_gameturn++;
        process_actions();
        LbNetwork_Exchange(NULL, &server_packet_cb);
        LbSleepFor(NET_DELAY);
    }
    LbNetwork_Stop();
}

void client_add_packet(ptrdiff_t idx, long rep)
{
    fprintf(stderr, "add turn:%ld packet:%d\n", game.play_gameturn, (char)idx);
    unsigned short *data = (unsigned short*)LbNetwork_AddPacket(PckA_PlyrMsgChar, game.play_gameturn, 6);
    data[0] = (unsigned short)game.play_gameturn;
    data[1] = (unsigned short)idx;
    data[2] = (unsigned short)rep;
}

void run_client()
{
    struct ServiceInitData idata;
    TbError err = LbNetwork_Init(TbNetworkService::NS_TCP_IP, 
        4,  playerinfo,  &idata);
    fprintf(stderr, "LbNetwork_Init -> %d\n", err);

    struct TbNetworkSessionNameEntry entry = {
      1, 42, 0, "127.0.0.1:5555", 0
    };
    unsigned long plyr_num = 1234;

    err = LbNetwork_Join(&entry, "client", &plyr_num, NULL);
    fprintf(stderr, "LbNetwork_Join -> %d; plyr_num:%ld\n", err, plyr_num);

    if (err == 0)
    {
        //while (LbTimerClock() < 60 * 1000)
        for (int j = 0; j < 600; j++)
        {
            for (int k = 0; k < 100; k++)
            {
                game.play_gameturn++;

                process_actions();

                LbNetwork_Exchange(NULL, &client_packet_cb);
                LbSleepFor(NET_DELAY);
            }
            fprintf(stderr, "tick:%d\n", j * 100);
            fprintf(stdout, "tick:%d\n", j * 100);
        }
    }

    LbNetwork_Stop();
}

int main(int argc, char **argv)
{
    for (int j=0; j<argc; j++)
    {
        //printf("%d %s\n", j, argv[j]);
    }
    if (argc > 1)
    {
        LbTimerInit();
        game.play_gameturn = 0;

        ClientAction *next_action = &actions[0];
        ClientAction *end_action = next_action + (sizeof(actions) / sizeof(actions[0]));
        for (int i=1; i < argc; i++)
        {
            if (next_action >= end_action)
            {
                printf("too many actions %s\n", argv[i]);
                break;
            }
            if (argv[i][0] == 'p')
            {
                long rep = 1;
                char *p = argv[i] + 1;
                while (*p == '_')
                    p++;
                unsigned long turn;
                int ok = sscanf(p, "%ldx%ld", &turn, &rep);
                if (ok == 0)
                  continue;
                if (turn < 1)
                {
                    printf("unexpected action time for %s\n", argv[i]);
                }
                if (next_action >= end_action)
                {
                    printf("too many actions %s\n", argv[i]);
                    break;
                }
                next_action->turn = turn;
                next_action->action = &client_add_packet;
                next_action->rep = rep;
                next_action++;
            } else if (argv[i][0] == 'M')
            {
                long rep = 1;
                char *p = argv[i] + 1;
                while (*p == '_')
                    p++;
                unsigned long turn;
                int ok = sscanf(p, "%ldx%ld", &turn, &rep);
                if (ok == 0)
                  continue;
                if (turn < 1)
                {
                    printf("unexpected action time for %s\n", argv[i]);
                }
                for (int j = 0; j < rep; j++)
                {
                    if (next_action >= end_action)
                    {
                        printf("too many actions %s\n", argv[i]);
                        break;
                    }
                    next_action->turn = turn + j;
                    next_action->action = &client_add_packet;
                    next_action->rep = rep;
                    next_action++;
                }
            } else if (argv[i][0] == 'Q')
            {
                long rep = 1;
                char *p = argv[i] + 1;
                while (*p == '_')
                    p++;
                unsigned long turn = 1;
                int ok = sscanf(p, "%ld", &rep);
                if (ok == 0)
                  continue;
                if (turn < 1)
                {
                    printf("unexpected action time for %s\n", argv[i]);
                }
                int j = 0;
                for (;rep > 0; rep -= 32, j++)
                {
                    if (next_action >= end_action)
                    {
                        printf("too many actions %s\n", argv[i]);
                        break;
                    }
                    next_action->turn = turn + j;
                    next_action->action = &client_add_packet;
                    next_action->rep = rep > 32 ? 32 : rep;
                    next_action++;
                }
            }
        }
        /*
          with MAX_STORED_ACTIONS_TH set to three
          Good tests:
          -s | -c M5x4
          -s | -c M5x32
          -s | -c p1x1 Q65536 p2060x3
          -s Q200 p100 p200 | wait for p100 on serv then connect -c
        */

        if (0 == strcmp(argv[1], "-s"))
        {
            run_server();
        }
        else if (0 == strcmp(argv[1], "-c"))
        {
            run_client();
        }
        else
        {
            printf("use -s or -c\n");
        }
    }
    return 0;
}