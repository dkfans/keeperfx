#include "pre_inc.h"
#include "kfx/renderer/backends/SoftwareCursorLayer.h"

#include "bflib_basics.h"
#include "bflib_video.h"       // scale_ui_value_lofi, MAX_SUPPORTED_SCREEN_*
#include "bflib_sprite.h"      // TbSprite
#include "bflib_vidraw.h"      // LbSpriteSetScaling*Array, LbSpriteDrawUsingScalingUpDataSolidLR
#include "bflib_mouse.h"       // LbMouseGetActivePointerSprite
#include "kfx/renderer/software/SwDrawTarget.h"
#include "engine_render.h"     // process_keeper_sprite
#include "post_inc.h"

/******************************************************************************/

namespace {

#define SW_CURSOR_XSTEPS (MAX_SUPPORTED_SCREEN_WIDTH  / 10)
#define SW_CURSOR_YSTEPS (MAX_SUPPORTED_SCREEN_HEIGHT / 10)

int32_t s_xsteps[2 * SW_CURSOR_XSTEPS];
int32_t s_ysteps[2 * SW_CURSOR_YSTEPS];

void set_scaling_w_clipped(long x, long sw, long dw, long gw)
{
    if (sw > SW_CURSOR_XSTEPS) sw = SW_CURSOR_XSTEPS;
    LbSpriteSetScalingWidthClippedArray(s_xsteps, x, sw, dw, gw);
}

void set_scaling_w_simple(long x, long sw, long dw)
{
    if (sw > SW_CURSOR_XSTEPS) sw = SW_CURSOR_XSTEPS;
    LbSpriteSetScalingWidthSimpleArray(s_xsteps, x, sw, dw);
}

void set_scaling_h_clipped(long y, long sh, long dh, long gh)
{
    if (sh > SW_CURSOR_YSTEPS) sh = SW_CURSOR_YSTEPS;
    LbSpriteSetScalingHeightClippedArray(s_ysteps, y, sh, dh, gh);
}

void set_scaling_h_simple(long y, long sh, long dh)
{
    if (sh > SW_CURSOR_YSTEPS) sh = SW_CURSOR_YSTEPS;
    LbSpriteSetScalingHeightSimpleArray(s_ysteps, y, sh, dh);
}

/** Draw the cursor sprite directly into the software draw target. */
void draw_pointer_sprite(int32_t x, int32_t y, const TbSprite* spr)
{
    TbPixel* outbuf = SwTargetWScreen();
    if (outbuf == nullptr)
        return;
    int32_t scanline = SwTargetScanline();

    int dw = scale_ui_value_lofi(spr->SWidth);
    int dh = scale_ui_value_lofi(spr->SHeight);
    if (dw <= 0 || dh <= 0) return;
    if (lbDisplay.MouseWindowWidth <= 0 || lbDisplay.MouseWindowHeight <= 0) return;

    if (x < 0 || (dw + spr->SWidth + x) >= lbDisplay.MouseWindowWidth)
        set_scaling_w_clipped(x, spr->SWidth, dw, lbDisplay.MouseWindowWidth);
    else
        set_scaling_w_simple(x, spr->SWidth, dw);

    if (y < 0 || (dh + spr->SHeight + y) >= lbDisplay.MouseWindowHeight)
        set_scaling_h_clipped(y, spr->SHeight, dh, lbDisplay.MouseWindowHeight);
    else
        set_scaling_h_simple(y, spr->SHeight, dh);

    outbuf = &outbuf[s_xsteps[0] + scanline * s_ysteps[0]];
    const TbSourceBuffer buf = {
        spr->Data, spr->SWidth, spr->SHeight, spr->SWidth,
    };
    LbSpriteDrawUsingScalingUpDataSolidLR(outbuf, scanline,
                                          lbDisplay.MouseWindowHeight,
                                          s_xsteps, s_ysteps, &buf);
}

} // namespace

/******************************************************************************/

void SoftwareCursorLayer::SubmitPointerSprite(const TbSprite* /*spr*/, int32_t /*x*/, int32_t /*y*/, int /*units_per_px*/)
{
    // No-op: software queries the live pointer state itself in Draw()
}

int SoftwareCursorLayer::SubmitKeeperHandSprite(short x, short y,
                                                unsigned short kspr_base,
                                                short angle,
                                                unsigned char sprgroup,
                                                int32_t scale,
                                                TbDrawFlagsMask draw_flags)
{
    (void)draw_flags; // no alpha-additive keeper path in this engine yet
    process_keeper_sprite(x, y, kspr_base, angle, sprgroup, (long)scale);
    return 1;  // always handled -- caller must not also call process_keeper_sprite()
}

void SoftwareCursorLayer::Draw()
{
    const TbSprite* spr = nullptr;
    int32_t x = 0;
    int32_t y = 0;
    int units_per_px = 16;
    if (LbMouseGetActivePointerSprite(&spr, &x, &y, &units_per_px))
        draw_pointer_sprite(x, y, spr);
}

void SoftwareCursorLayer::Clear()
{
    // No-op: nothing stashed to clear
}

/******************************************************************************/
