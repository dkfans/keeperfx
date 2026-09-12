/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file ITextRenderer.cpp
 *     Software text drawing, plus the IR bind/replay machinery a deferring
 *     backend (GL) needs.
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
#include "post_inc.h"

/******************************************************************************/

TbBool ITextRenderer::DrawTextResized(int32_t x, int32_t y, int32_t units_per_px, const char* text)
{
    if (m_text_write_cmds) {
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
        cmd.SetText(text);
        cmd.seq = m_text_write_cmds->NextSeq();
        m_text_write_cmds->draws.Append(cmd);
        return true;
    }
    return LbTextDrawResizedImmediate(x, y, units_per_px, text);
}

void ITextRenderer::SetTextCommandBuffers(TextCommandBuffers* cmds)
{
    m_text_write_cmds = cmds;
}

void ITextRenderer::ReplayTextCommand(const IRTextDrawCmd& cmd)
{
    if (cmd.font_generation != LbTextGetFontGeneration())
        return; // font reloaded since submission; stale snapshot, skip rather than misdraw

    int save_jx, save_jy, save_jw;
    int save_cx, save_cy, save_cw, save_ch;
    LbTextGetJustifyWindow(&save_jx, &save_jy, &save_jw);
    LbTextGetClipWindow(&save_cx, &save_cy, &save_cw, &save_ch);
    const struct TbSpriteSheet* save_font = lbFontPtr;
    unsigned char save_colour = RendererGetDrawColour();
    unsigned short save_flags = RendererGetDrawFlags();

    LbTextSetJustifyWindow(cmd.justify_x, cmd.justify_y, cmd.justify_w);
    LbTextSetClipWindow(cmd.clip_x, cmd.clip_y, cmd.clip_w, cmd.clip_h);
    LbTextSetFont((const struct TbSpriteSheet*)cmd.font);
    RendererSetDrawColour(cmd.draw_colour);
    RendererSetDrawFlags((unsigned short)cmd.draw_flags);

    LbTextDrawResizedImmediate(cmd.pos_x, cmd.pos_y, cmd.units_per_px, cmd.text);

    LbTextSetJustifyWindow(save_jx, save_jy, save_jw);
    LbTextSetClipWindow(save_cx, save_cy, save_cw, save_ch);
    LbTextSetFont(save_font);
    RendererSetDrawColour(save_colour);
    RendererSetDrawFlags(save_flags);
}

/******************************************************************************/
