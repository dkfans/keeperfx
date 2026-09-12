/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file IUIRenderer.cpp
 *     Software draw path for the shared UI submission API, plus the IR
 *     bind machinery a deferring backend (GL) needs.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/IUIRenderer.h"
#include "kfx/renderer/ir/UICommands.h"
#include "kfx/renderer/RendererManager.h"
#include "bflib_vidraw.h"   // the raster primitives
#include "bflib_sprite.h"   // TbSprite, num_sprites, get_sprite
#include "bflib_video.h"    // Lb_SPRITE_* draw flags
#include "gui_draw.h"       // draw_slab64k_background_immediate
#include <algorithm>        // std::fill (AcquireMinimapBuffer)
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
// Sprite handle registry

void IUIRenderer::RegisterSpriteHandle(SpriteHandle h, const struct TbSprite* spr)
{
    std::lock_guard<std::mutex> guard(m_handle_mutex);
    m_handle_to_sprite[h] = spr;
    m_sprite_to_handle[spr] = h;
}

SpriteHandle IUIRenderer::ResolveSprite(const struct TbSprite* spr)
{
    if (!spr) return kInvalidSpriteHandle;
    std::lock_guard<std::mutex> guard(m_handle_mutex);
    auto it = m_sprite_to_handle.find(spr);
    if (it != m_sprite_to_handle.end())
        return it->second;
    SpriteHandle h = m_next_handle++;
    m_handle_to_sprite[h] = spr;
    m_sprite_to_handle[spr] = h;
    return h;
}

int32_t IUIRenderer::ClearSpriteHandleCache()
{
    std::lock_guard<std::mutex> guard(m_handle_mutex);
    const int32_t count = (int32_t)m_sprite_to_handle.size();
    m_sprite_to_handle.clear();
    m_handle_to_sprite.clear();
    return count;
}

void IUIRenderer::RegisterSpriteSheet(const struct TbSpriteSheet* sheet)
{
    if (!sheet) return;
    const int32_t n = (int32_t)num_sprites(sheet);
    for (int32_t i = 0; i < n; ++i)
        ResolveSprite(get_sprite(sheet, i));
}

/******************************************************************************/
// Submission

TbResult IUIRenderer::SubmitRawSprite(int32_t x, int32_t y, const struct TbSprite* spr,
                                      KfxDrawState state)
{
    if (!spr) return Lb_FAIL;
    if (m_ui_write_cmds) {
        IRUISpriteCmd cmd;
        cmd.layer = ComputeCurrentLayer();
        ApplyGameViewportOffset(cmd.layer, x, y);
        cmd.x = x; cmd.y = y;
        cmd.sprite = ResolveSprite(spr);
        cmd.draw_flags = state.flags;
        cmd.ndc_z = ComputeCurrentNdcZ();
        cmd.seq = m_ui_write_cmds->NextSeq();
        m_ui_write_cmds->sprites.Append(cmd);
        return Lb_SUCCESS;
    }
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawImmediate(x, y, spr);
}

TbResult IUIRenderer::SubmitRawSpriteOneColour(int32_t x, int32_t y, const struct TbSprite* spr,
                                               unsigned char colour, KfxDrawState state)
{
    if (!spr) return Lb_FAIL;
    if (m_ui_write_cmds) {
        IRUISpriteOneColourCmd cmd;
        cmd.layer = ComputeCurrentLayer();
        ApplyGameViewportOffset(cmd.layer, x, y);
        cmd.x = x; cmd.y = y;
        cmd.sprite = ResolveSprite(spr);
        cmd.colour = colour;
        cmd.draw_flags = state.flags;
        cmd.ndc_z = ComputeCurrentNdcZ();
        cmd.seq = m_ui_write_cmds->NextSeq();
        m_ui_write_cmds->sprites_one_colour.Append(cmd);
        return Lb_SUCCESS;
    }
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawOneColourImmediate(x, y, spr, colour);
}

TbResult IUIRenderer::SubmitRawSpriteScaled(int32_t x, int32_t y, const struct TbSprite* spr,
                                            int32_t w, int32_t h, KfxDrawState state)
{
    if (!spr) return Lb_FAIL;
    if (m_ui_write_cmds) {
        IRUISpriteScaledCmd cmd;
        cmd.layer = ComputeCurrentLayer();
        ApplyGameViewportOffset(cmd.layer, x, y);
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.sprite = ResolveSprite(spr);
        cmd.draw_flags = state.flags;
        cmd.ndc_z = ComputeCurrentNdcZ();
        cmd.seq = m_ui_write_cmds->NextSeq();
        m_ui_write_cmds->sprites_scaled.Append(cmd);
        return Lb_SUCCESS;
    }
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawScaledImmediate(x, y, spr, w, h);
}

TbResult IUIRenderer::SubmitRawSpriteScaledOneColour(int32_t x, int32_t y, const struct TbSprite* spr,
                                                     int32_t w, int32_t h, unsigned char colour,
                                                     KfxDrawState state)
{
    if (!spr) return Lb_FAIL;
    if (m_ui_write_cmds) {
        IRUISpriteScaledOneColourCmd cmd;
        cmd.layer = ComputeCurrentLayer();
        ApplyGameViewportOffset(cmd.layer, x, y);
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.sprite = ResolveSprite(spr);
        cmd.colour = colour;
        cmd.draw_flags = state.flags;
        cmd.ndc_z = ComputeCurrentNdcZ();
        cmd.seq = m_ui_write_cmds->NextSeq();
        m_ui_write_cmds->sprites_scaled_one_colour.Append(cmd);
        return Lb_SUCCESS;
    }
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawScaledOneColourImmediate(x, y, spr, w, h, colour);
}

int IUIRenderer::SubmitRawSpriteScaledRemap(int32_t x, int32_t y, const struct TbSprite* spr,
                                            int32_t w, int32_t h, const unsigned char* cmap,
                                            KfxDrawState state)
{
    if (!spr || !cmap) return Lb_FAIL;
    if (m_ui_write_cmds) {
        IRUISpriteScaledRemapCmd cmd;
        cmd.layer = ComputeCurrentLayer();
        ApplyGameViewportOffset(cmd.layer, x, y);
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.sprite = ResolveSprite(spr);
        cmd.cmap = cmap;
        cmd.draw_flags = state.flags;
        cmd.ndc_z = ComputeCurrentNdcZ();
        cmd.seq = m_ui_write_cmds->NextSeq();
        m_ui_write_cmds->sprites_scaled_remap.Append(cmd);
        return Lb_SUCCESS;
    }
    ScopedDrawState guard(state.flags);
    return LbSpriteDrawScaledRemapImmediate(x, y, spr, w, h, cmap);
}

void IUIRenderer::SubmitSolidBox(int32_t x, int32_t y, int32_t w, int32_t h,
                                 uint8_t colour_idx, KfxDrawState state)
{
    if (w <= 0 || h <= 0) return;
    if (m_ui_write_cmds) {
        IRUISolidBoxCmd cmd;
        cmd.layer = ComputeCurrentLayer();
        ApplyGameViewportOffset(cmd.layer, x, y);
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.colour = colour_idx;
        cmd.draw_flags = state.flags;
        cmd.ndc_z = ComputeCurrentNdcZ();
        cmd.seq = m_ui_write_cmds->NextSeq();
        m_ui_write_cmds->solid_boxes.Append(cmd);
        return;
    }
    // LbDrawBox reads the outline flag itself, so the whole state just goes ambient.
    ScopedDrawState guard(state.flags);
    LbDrawBoxImmediate(x, y, (unsigned long)w, (unsigned long)h, colour_idx);
}

void IUIRenderer::SubmitSlabBackground(int32_t x, int32_t y, int32_t w, int32_t h)
{
    if (m_ui_write_cmds) {
        IRUISlabBackgroundCmd cmd;
        cmd.layer = ComputeCurrentLayer();
        ApplyGameViewportOffset(cmd.layer, x, y);
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.ndc_z = ComputeCurrentNdcZ();
        cmd.seq = m_ui_write_cmds->NextSeq();
        m_ui_write_cmds->slab_backgrounds.Append(cmd);
        return;
    }
    draw_slab64k_background_immediate(x, y, w, h);
}

uint8_t* IUIRenderer::AcquireMinimapBuffer(int size)
{
    if (size <= 0) return nullptr;
    const size_t needed = (size_t)size * (size_t)size;
    if (m_minimap_cpu_size != size)
    {
        m_minimap_cpu_buf.assign(needed, 0);
        m_minimap_cpu_size = size;
    }
    else
    {
        std::fill(m_minimap_cpu_buf.begin(), m_minimap_cpu_buf.end(), (uint8_t)0);
    }
    return m_minimap_cpu_buf.data();
}

void IUIRenderer::SubmitMinimap(int screen_x, int screen_y, int size,
                                const int32_t* shape_start, const int32_t* shape_end)
{
    // CPU default: blit the acquired buffer straight into the framebuffer,
    // masked to the circular shape so panel art outside the minimap circle
    // survives (mirrors what the old direct-WScreen-write code did).
    if (size <= 0 || (int)m_minimap_cpu_buf.size() < size * size || lbDisplay.WScreen == NULL)
        return;
    const long stride = RendererScreenWidth();
    TbPixel* out_line = &lbDisplay.WScreen[screen_x + stride * screen_y];
    const uint8_t* src_line = m_minimap_cpu_buf.data();
    for (int h = 0; h < size; h++)
    {
        int w0 = shape_start ? shape_start[h] : 0;
        int w1 = shape_end   ? shape_end[h]   : size;
        if (w0 < 0) w0 = 0;
        if (w1 > size) w1 = size;
        for (int w = w0; w < w1; w++)
            out_line[w] = (TbPixel)src_line[w];
        out_line += stride;
        src_line += size;
    }
}

void IUIRenderer::BeginZoomBoxOverlay(int32_t x, int32_t y, int32_t w, int32_t h)
{
    LbScreenSetGraphicsWindow(x, y, w, h);
    SetTopOverlay();
}

void IUIRenderer::EndZoomBoxOverlay(int32_t x, int32_t y, int32_t w, int32_t h)
{
    (void)x; (void)y; (void)w; (void)h;
    ClearTopOverlay();
    LbScreenSetGraphicsWindow(0, 0, RendererScreenWidth(), RendererScreenHeight());
}

/******************************************************************************/
// Frame wiring

void IUIRenderer::SetUICommandBuffers(UICommandBuffers* cmds)
{
    m_ui_write_cmds = cmds;
}

void IUIRenderer::SetGameViewport(int32_t x, int32_t y, int32_t w, int32_t h)
{
    m_game_vp_x = x; m_game_vp_y = y; m_game_vp_w = w; m_game_vp_h = h;
    m_game_vp_set = true;
    if (m_ui_write_cmds) m_ui_write_cmds->game_vp = { x, y, w, h, true };
}

/******************************************************************************/
