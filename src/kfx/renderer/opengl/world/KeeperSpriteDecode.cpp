#include "pre_inc.h"
#include "kfx/renderer/opengl/world/KeeperSpriteDecode.h"
#include "bflib_basics.h"
#include "globals.h"
#include <cstddef>
#include <cstring>
#include "post_inc.h"

uint8_t kspr_decode_buf[k_kspr_decode_dim * k_kspr_decode_dim * 2];

bool decoded_sprite_darkens(int w, int h)
{
    for (int y = 0; y < h; ++y)
    {
        const uint8_t* row = kspr_decode_buf + y * k_kspr_decode_dim * 2;
        for (int x = 0; x < w; ++x)
            if (row[x * 2 + 1] != 0 && row[x * 2] >= 50 && row[x * 2] <= 56)
                return true;
    }
    return false;
}

void decode_keeper_rle(uint8_t* dst, const uint8_t* data, int w, int h)
{
    if (!data || w <= 0 || h <= 0) return;

    for (int y = 0; y < h; ++y)
        memset(dst + y * k_kspr_decode_dim * 2, 0, k_kspr_decode_dim * 2);

    const signed char* sp     = reinterpret_cast<const signed char*>(data);
    const signed char* sp_end = sp + (ptrdiff_t)w * h * 3 + h;  // generous worst-case bound
    for (int y = 0; y < h; ++y) {
        uint8_t* row = dst + y * k_kspr_decode_dim * 2;
        int x = 0;
        while (true) {
            if (sp >= sp_end) {
                WARNLOG("decode_keeper_rle: ran past expected data end at row %d", y);
                return;
            }
            signed char cmd = *sp++;
            if (cmd == 0) break;
            if (cmd < 0) {
                x += (int)(-cmd);
            } else {
                int count = (int)cmd;
                for (int i = 0; i < count; ++i) {
                    if (sp >= sp_end) {
                        WARNLOG("decode_keeper_rle: pixel data past expected end");
                        return;
                    }
                    if (x < w) {
                        row[x * 2]     = (uint8_t)(*sp);
                        row[x * 2 + 1] = 255;
                    }
                    ++sp;
                    ++x;
                }
            }
        }
    }
}
