/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file RendererBridge_UI.cpp
 *     C-callable UIRenderer wrappers (split from RendererManager.cpp).
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
#include "kfx/renderer/RendererManager_Internal.h"
#include "kfx/renderer/IUIRenderer.h"
#include "bflib_sprite.h"   // TbSprite
#include "bflib_vidraw.h"   // LbSpriteDraw*Immediate, LbDrawBoxImmediate
#include "post_inc.h"

/******************************************************************************/

/* The ambient draw state is what the caller set before the call, so it travels with
 * the submission and is reapplied if the draw is replayed later. */
static KfxDrawState ambient_draw_state(void)
{
    return draw_state_make(RendererGetDrawFlags(), RendererGetDrawColour());
}

void RendererDrawSlabBackground(int32_t x, int32_t y, int32_t width, int32_t height)
{
    RendererGetActiveUIRenderer()->SubmitSlabBackground(x, y, width, height);
}

TbResult RendererDrawBox(int32_t x, int32_t y, uint32_t width, uint32_t height, unsigned char colour)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) return LbDrawBoxImmediate(x, y, width, height, colour);
    ui->SubmitSolidBox(x, y, (int32_t)width, (int32_t)height, colour, ambient_draw_state());
    return Lb_SUCCESS;
}

TbResult RendererSpriteDraw(int32_t x, int32_t y, const struct TbSprite *spr)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) return LbSpriteDrawImmediate(x, y, spr);
    return ui->SubmitRawSprite(x, y, spr, ambient_draw_state());
}

TbResult RendererSpriteDrawOneColour(int32_t x, int32_t y, const struct TbSprite *spr, unsigned char colour)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) return LbSpriteDrawOneColourImmediate(x, y, spr, colour);
    return ui->SubmitRawSpriteOneColour(x, y, spr, colour, ambient_draw_state());
}

TbResult RendererSpriteDrawScaled(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) return LbSpriteDrawScaledImmediate(x, y, spr, w, h);
    return ui->SubmitRawSpriteScaled(x, y, spr, w, h, ambient_draw_state());
}

TbResult RendererSpriteDrawScaledOneColour(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h, unsigned char colour)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) return LbSpriteDrawScaledOneColourImmediate(x, y, spr, w, h, colour);
    return ui->SubmitRawSpriteScaledOneColour(x, y, spr, w, h, colour, ambient_draw_state());
}

int RendererSpriteDrawScaledRemap(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h, const unsigned char *cmap)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) return LbSpriteDrawScaledRemapImmediate(x, y, spr, w, h, cmap);
    return ui->SubmitRawSpriteScaledRemap(x, y, spr, w, h, cmap, ambient_draw_state());
}

void UIRenderer_SubmitSolidBox(int32_t x, int32_t y, int32_t w, int32_t h, unsigned char colour)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui != nullptr) ui->SubmitSolidBox(x, y, w, h, colour, draw_state_default());
}

void UIRenderer_SubmitOutlineBox(int32_t x, int32_t y, int32_t w, int32_t h, unsigned char colour)
{
    if (w < 2 || h < 2) return;
    UIRenderer_SubmitSolidBox(x,         y,         w, 1, colour);  // top
    UIRenderer_SubmitSolidBox(x,         y + h - 1, w, 1, colour);  // bottom
    UIRenderer_SubmitSolidBox(x,         y,         1, h, colour);  // left
    UIRenderer_SubmitSolidBox(x + w - 1, y,         1, h, colour);  // right
}

void UIRenderer_SubmitCircle(int32_t x, int32_t y, int32_t radius, unsigned char colour)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) return;
    int32_t d = radius * 2 + 1;
    ui->SubmitSolidBox(x - radius, y - radius, d, d, colour, draw_state_default());
}

// Both scaled-sprite bridges below round the same way LbSpriteDrawResized*'s
// macros do (see bflib_vidraw.h): (dim * units_per_px + 8) / 16.

void UIRenderer_SubmitPanelSpriteRaw(int32_t x, int32_t y, int units_per_px, const struct TbSprite *spr, TbDrawFlagsMask draw_flags)
{
    if (spr == nullptr) return;
    int32_t w = ((int32_t)spr->SWidth  * units_per_px + 8) / 16;
    int32_t h = ((int32_t)spr->SHeight * units_per_px + 8) / 16;
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) { LbSpriteDrawScaledImmediate(x, y, spr, w, h); return; }
    ui->SubmitRawSpriteScaled(x, y, spr, w, h, draw_state_make(draw_flags, 0));
}

void UIRenderer_SubmitPanelSpriteRawColored(int32_t x, int32_t y, int units_per_px, const struct TbSprite *spr, unsigned char colour, TbDrawFlagsMask draw_flags)
{
    if (spr == nullptr) return;
    int32_t w = ((int32_t)spr->SWidth  * units_per_px + 8) / 16;
    int32_t h = ((int32_t)spr->SHeight * units_per_px + 8) / 16;
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) { LbSpriteDrawScaledOneColourImmediate(x, y, spr, w, h, colour); return; }
    ui->SubmitRawSpriteScaledOneColour(x, y, spr, w, h, colour, draw_state_make(draw_flags, 0));
}

void UIRenderer_SubmitButtonSprite(int32_t x, int32_t y, int units_per_px, const struct TbSprite *spr)
{
    UIRenderer_SubmitPanelSpriteRaw(x, y, units_per_px, spr, 0);
}

void UIRenderer_SubmitScaledSprite(int32_t x, int32_t y, int32_t w, int32_t h, const struct TbSprite *spr, TbDrawFlagsMask draw_flags)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) { LbSpriteDrawScaledImmediate(x, y, spr, w, h); return; }
    ui->SubmitRawSpriteScaled(x, y, spr, w, h, draw_state_make(draw_flags, 0));
}

void UIRenderer_BeginTopOverlay(void)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui != nullptr) ui->SetTopOverlay();
}

void UIRenderer_EndTopOverlay(void)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui != nullptr) ui->ClearTopOverlay();
}

void UIRenderer_BeginZoomBoxOverlay(int32_t x, int32_t y, int32_t w, int32_t h)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui != nullptr) ui->BeginZoomBoxOverlay(x, y, w, h);
}

void UIRenderer_EndZoomBoxOverlay(int32_t x, int32_t y, int32_t w, int32_t h)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui != nullptr) ui->EndZoomBoxOverlay(x, y, w, h);
}

unsigned char* UIRenderer_AcquireMinimapBuffer(int32_t size)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) return nullptr;
    return ui->AcquireMinimapBuffer(size);
}

void UIRenderer_SubmitMinimap(int32_t screen_x, int32_t screen_y, int32_t size,
                              const int32_t *shape_start, const int32_t *shape_end)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui != nullptr) ui->SubmitMinimap(screen_x, screen_y, size, shape_start, shape_end);
}

int32_t RendererClearSpriteHandleCache(void)
{
    IUIRenderer* ui = RendererGetActiveUIRenderer();
    if (ui == nullptr) return 0;
    return ui->ClearSpriteHandleCache();
}

/******************************************************************************/
