/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file RendererBridge_UI.h
 *     C-callable UIRenderer wrappers (split from RendererManager.cpp).
 */
/******************************************************************************/
#ifndef RENDERER_RENDERERBRIDGE_UI_H
#define RENDERER_RENDERERBRIDGE_UI_H

#include <stdint.h>
#include "bflib_basics.h"
#include "kfx/renderer/DrawState.h"

struct TbSprite;

#ifdef __cplusplus
extern "C" {
#endif

TbResult RendererDrawBox(int32_t x, int32_t y, uint32_t width, uint32_t height, unsigned char colour);
void RendererDrawSlabBackground(int32_t x, int32_t y, int32_t width, int32_t height);
TbResult RendererSpriteDraw(int32_t x, int32_t y, const struct TbSprite *spr);
TbResult RendererSpriteDrawOneColour(int32_t x, int32_t y, const struct TbSprite *spr, unsigned char colour);
TbResult RendererSpriteDrawScaled(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h);
TbResult RendererSpriteDrawScaledOneColour(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h, unsigned char colour);
int      RendererSpriteDrawScaledRemap(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h, const unsigned char *cmap);

/** Filled rect at an explicit position/size, default (no outline) draw state. */
void UIRenderer_SubmitSolidBox(int32_t x, int32_t y, int32_t w, int32_t h, unsigned char colour);

/** 1-pixel-thick rectangular outline (Lb_SPRITE_OUTLINE), decomposed into
 *  four SubmitSolidBox() strips -- no dedicated outline primitive exists. */
void UIRenderer_SubmitOutlineBox(int32_t x, int32_t y, int32_t w, int32_t h, unsigned char colour);

/** Circle approximated as its bounding square (no circle primitive exists
 *  anywhere in this renderer's IR either -- matches develop's own
 *  UIRenderer_SubmitCircle, which is the same approximation). */
void UIRenderer_SubmitCircle(int32_t x, int32_t y, int32_t radius, unsigned char colour);

/** Sprite drawn at an explicit size (spr->SWidth/SHeight scaled by
 *  units_per_px), with an explicit draw-flags parameter instead of reading
 *  ambient RendererGetDrawFlags() state. */
void UIRenderer_SubmitPanelSpriteRaw(int32_t x, int32_t y, int units_per_px, const struct TbSprite *spr, TbDrawFlagsMask draw_flags);

/** One-colour variant of UIRenderer_SubmitPanelSpriteRaw(). */
void UIRenderer_SubmitPanelSpriteRawColored(int32_t x, int32_t y, int units_per_px, const struct TbSprite *spr, unsigned char colour, TbDrawFlagsMask draw_flags);

/** Button sprite drawn at its own size, scaled by units_per_px, default draw state. */
void UIRenderer_SubmitButtonSprite(int32_t x, int32_t y, int units_per_px, const struct TbSprite *spr);

/** Sprite drawn at an explicit destination size (w,h), with an explicit
 *  draw-flags parameter -- matches develop's UIRenderer_SubmitScaledSprite. */
void UIRenderer_SubmitScaledSprite(int32_t x, int32_t y, int32_t w, int32_t h, const struct TbSprite *spr, TbDrawFlagsMask draw_flags);

/** Top-overlay: subsequent UIRenderer_Submit* calls land in IRUILayer::Overlay,
 *  drawn dead-last over GameUI. No-op when no renderer is active. */
void UIRenderer_BeginTopOverlay(void);
void UIRenderer_EndTopOverlay(void);

/** Brackets the zoom-box's thing-sprite submissions -- see
 *  IUIRenderer::BeginZoomBoxOverlay()'s own comment. */
void UIRenderer_BeginZoomBoxOverlay(int32_t x, int32_t y, int32_t w, int32_t h);
void UIRenderer_EndZoomBoxOverlay(int32_t x, int32_t y, int32_t w, int32_t h);

/** Renderer-owned size*size palette-index scratch buffer for the minimap
 *  (zero-filled; index 0 = "nothing drawn here"). Caller writes into it, then
 *  calls UIRenderer_SubmitMinimap(). Never null for size > 0 with a renderer
 *  active; null when no renderer is active. */
unsigned char* UIRenderer_AcquireMinimapBuffer(int32_t size);

/** Display the buffer filled via UIRenderer_AcquireMinimapBuffer() at
 *  (screen_x, screen_y). shape_start/shape_end are the per-row circular-mask
 *  bounds (size entries each) -- see IUIRenderer::SubmitMinimap()'s comment. */
void UIRenderer_SubmitMinimap(int32_t screen_x, int32_t screen_y, int32_t size,
                              const int32_t *shape_start, const int32_t *shape_end);

/** Drops every cached sprite/glyph handle. Call whenever any
 *  sprite sheet frees -- see IUIRenderer::ClearSpriteHandleCache(). Returns
 *  the number of entries cleared. */
int32_t RendererClearSpriteHandleCache(void);

#ifdef __cplusplus
}
#endif

#endif // RENDERER_RENDERERBRIDGE_UI_H
