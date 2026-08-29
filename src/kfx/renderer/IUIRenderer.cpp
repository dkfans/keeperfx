/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file IUIRenderer.cpp
 *     Software UI drawing.
 * @par Design:
 *     Each body applies the submitted draw state to the ambient state, calls the
 *     existing raster primitive, then restores it — the same sequence the engine
 *     performed around these primitives before they were routed, so a submission
 *     draws exactly what a direct call did.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/IUIRenderer.h"
#include "kfx/renderer/RendererManager.h"
#include "bflib_vidraw.h"   // the raster primitives
#include "bflib_sprite.h"   // TbSprite
#include "bflib_video.h"    // Lb_SPRITE_* draw flags
#include "gui_draw.h"       // draw_slab64k_background_immediate
#include "post_inc.h"

/******************************************************************************/

namespace {

/** Applies a submitted draw state for the duration of one draw. */
class ScopedDrawState {
public:
    explicit ScopedDrawState(TbDrawFlagsMask flags)
        : m_flags(RendererGetDrawFlags())
    {
        RendererSetDrawFlags((unsigned short)flags);
    }
    ~ScopedDrawState() { RendererSetDrawFlags(m_flags); }
private:
    unsigned short m_flags;
};

} // namespace

/******************************************************************************/

TbResult IUIRenderer::SubmitRawSprite(int32_t x, int32_t y, const struct TbSprite* spr,
                                      KfxDrawState state)
{
    if (!spr) return Lb_FAIL;
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawImmediate(x, y, spr);
}

TbResult IUIRenderer::SubmitRawSpriteOneColour(int32_t x, int32_t y, const struct TbSprite* spr,
                                               unsigned char colour, KfxDrawState state)
{
    if (!spr) return Lb_FAIL;
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawOneColourImmediate(x, y, spr, colour);
}

TbResult IUIRenderer::SubmitRawSpriteScaled(int32_t x, int32_t y, const struct TbSprite* spr,
                                            int32_t w, int32_t h, KfxDrawState state)
{
    if (!spr) return Lb_FAIL;
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawScaledImmediate(x, y, spr, w, h);
}

TbResult IUIRenderer::SubmitRawSpriteScaledOneColour(int32_t x, int32_t y, const struct TbSprite* spr,
                                                     int32_t w, int32_t h, unsigned char colour,
                                                     KfxDrawState state)
{
    if (!spr) return Lb_FAIL;
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawScaledOneColourImmediate(x, y, spr, w, h, colour);
}

int IUIRenderer::SubmitRawSpriteScaledRemap(int32_t x, int32_t y, const struct TbSprite* spr,
                                            int32_t w, int32_t h, const unsigned char* cmap,
                                            KfxDrawState state)
{
    if (!spr || !cmap) return Lb_FAIL;
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawScaledRemapImmediate(x, y, spr, w, h, cmap);
}

void IUIRenderer::SubmitSolidBox(int32_t x, int32_t y, int32_t w, int32_t h,
                                 uint8_t colour_idx, KfxDrawState state)
{
    if (w <= 0 || h <= 0) return;
    // LbDrawBox reads the outline flag itself, so the whole state just goes ambient.
    ScopedDrawState guard(state.flags);
    LbDrawBoxImmediate(x, y, (unsigned long)w, (unsigned long)h, colour_idx);
}

void IUIRenderer::SubmitSlabBackground(int32_t x, int32_t y, int32_t w, int32_t h)
{
    draw_slab64k_background_immediate(x, y, w, h);
}

/******************************************************************************/
