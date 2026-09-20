/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file ITextRenderer.cpp
 *     Software text drawing, plus the IR capture a deferring backend (GL) needs.
 * @par Comment:
 *     The layout and glyph rasterisation stay in bflib_sprfnt; this calls the
 *     same entry point the engine used before text was routed.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/ITextRenderer.h"
#include "kfx/renderer/ir/TextCommands.h"
#include "kfx/renderer/RendererManager.h"
#include "bflib_sprfnt.h"   // LbTextDrawResizedImmediate, window/font accessors
#include <cstring>
#include "post_inc.h"

/******************************************************************************/

TbBool ITextRenderer::DrawTextResized(int32_t x, int32_t y, int32_t units_per_px, const char* text)
{
    return LbTextDrawResizedImmediate(x, y, units_per_px, text);
}

IRTextDrawCmd* ITextRenderer::AppendTextCommand(int32_t x, int32_t y, int32_t units_per_px)
{
    if (!m_text_write_cmds)
        return nullptr;
    IRTextDrawCmd cmd;
    cmd.pos_x = x; cmd.pos_y = y; cmd.units_per_px = units_per_px;
    cmd.draw_colour = RendererGetDrawColour();
    cmd.draw_flags  = RendererGetDrawFlags();
    LbTextGetJustifyWindow(&cmd.justify_x, &cmd.justify_y, &cmd.justify_w);
    LbTextGetClipWindow(&cmd.clip_x, &cmd.clip_y, &cmd.clip_w, &cmd.clip_h);
    cmd.font = lbFontPtr;
    cmd.font_generation = LbTextGetFontGeneration();
    cmd.dbc_enabled = (dbc_initialized && dbc_enabled) ? 1 : 0;
    cmd.dbc_font    = cmd.dbc_enabled ? (const void*)active_dbcfont : nullptr;
    cmd.dbc_colour0 = dbc_colour0;
    cmd.dbc_colour1 = dbc_colour1;
    const unsigned char* remap = LbTextGetRemap();
    cmd.has_remap = (remap != nullptr);
    if (remap != nullptr)
        std::memcpy(cmd.remap.data(), remap, cmd.remap.size());
    cmd.seq = m_text_write_cmds->NextSeq();
    return &m_text_write_cmds->draws.Append(cmd);
}

void ITextRenderer::SetTextCommandBuffers(TextCommandBuffers* cmds)
{
    m_text_write_cmds = cmds;
}

/******************************************************************************/
