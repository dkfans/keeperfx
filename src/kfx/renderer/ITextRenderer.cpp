/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file ITextRenderer.cpp
 *     Software text drawing.
 * @par Comment:
 *     The layout and glyph rasterisation stay in bflib_sprfnt; this calls the
 *     same entry point the engine used before text was routed.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/ITextRenderer.h"
#include "bflib_sprfnt.h"   // LbTextDrawResizedImmediate
#include "post_inc.h"

/******************************************************************************/

TbBool ITextRenderer::DrawTextResized(int32_t x, int32_t y, int32_t units_per_px, const char* text)
{
    return LbTextDrawResizedImmediate(x, y, units_per_px, text);
}

/******************************************************************************/
