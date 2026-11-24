/** @file tileset_test_stubs.c
 *     Log and game-turn stubs so the tile set tests link without the game. */
#include "pre_inc.h"
#include "bflib_basics.h"
#include "globals.h"
#include "post_inc.h"

int LbErrorLog(const char *format, ...) { (void)format; return 0; }
int LbWarnLog(const char *format, ...)  { (void)format; return 0; }
int LbSyncLog(const char *format, ...)  { (void)format; return 0; }
int LbNetLog(const char *format, ...)   { (void)format; return 0; }
int LbJustLog(const char *format, ...)  { (void)format; return 0; }
GameTurn get_gameturn() { return 0; }

/* Same as bflib_basics.c; the RNC unpacker needs it. */
unsigned long lword(unsigned char *p)
{
    unsigned long n = p[1];
    n = (n << 8) + p[0];
    return n;
}
