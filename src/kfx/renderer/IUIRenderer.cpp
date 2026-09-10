/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file IUIRenderer.cpp
 *     Software draw path for the shared UI submission API, plus the IR
 *     bind/replay machinery a deferring backend (GL) needs.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/IUIRenderer.h"
#include "kfx/renderer/ITextRenderer.h"
#include "kfx/renderer/ir/UICommands.h"
#include "kfx/renderer/ir/TextCommands.h"
#include "kfx/renderer/RendererManager.h"
#include "bflib_vidraw.h"   // the raster primitives
#include "bflib_sprite.h"   // TbSprite, num_sprites, get_sprite
#include "bflib_video.h"    // Lb_SPRITE_* draw flags
#include "gui_draw.h"       // draw_slab64k_background_immediate
#include <algorithm>        // std::sort for the seq merge
#include <vector>
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

void IUIRenderer::ReplayMergedFromIR(const UICommandBuffers& ui,
                                     const TextCommandBuffers& text,
                                     ITextRenderer* text_renderer)
{
    // Commands are grouped by type, so gather them into one list ordered by
    // the shared submission seq to recover the order they were issued in.
    struct Ref { uint32_t seq; uint8_t kind; uint32_t idx; };
    enum { K_Sprite = 0, K_OneColour, K_Scaled, K_ScaledOneColour, K_ScaledRemap, K_Box, K_Slab, K_Text };

    std::vector<Ref> order;
    order.reserve(ui.sprites.Size() + ui.sprites_one_colour.Size() +
                  ui.sprites_scaled.Size() + ui.sprites_scaled_one_colour.Size() +
                  ui.sprites_scaled_remap.Size() + ui.solid_boxes.Size() +
                  ui.slab_backgrounds.Size() + text.draws.Size());

    for (uint32_t i = 0; i < (uint32_t)ui.sprites.Size(); ++i)
        order.push_back({ ui.sprites.Data()[i].seq, K_Sprite, i });
    for (uint32_t i = 0; i < (uint32_t)ui.sprites_one_colour.Size(); ++i)
        order.push_back({ ui.sprites_one_colour.Data()[i].seq, K_OneColour, i });
    for (uint32_t i = 0; i < (uint32_t)ui.sprites_scaled.Size(); ++i)
        order.push_back({ ui.sprites_scaled.Data()[i].seq, K_Scaled, i });
    for (uint32_t i = 0; i < (uint32_t)ui.sprites_scaled_one_colour.Size(); ++i)
        order.push_back({ ui.sprites_scaled_one_colour.Data()[i].seq, K_ScaledOneColour, i });
    for (uint32_t i = 0; i < (uint32_t)ui.sprites_scaled_remap.Size(); ++i)
        order.push_back({ ui.sprites_scaled_remap.Data()[i].seq, K_ScaledRemap, i });
    for (uint32_t i = 0; i < (uint32_t)ui.solid_boxes.Size(); ++i)
        order.push_back({ ui.solid_boxes.Data()[i].seq, K_Box, i });
    for (uint32_t i = 0; i < (uint32_t)ui.slab_backgrounds.Size(); ++i)
        order.push_back({ ui.slab_backgrounds.Data()[i].seq, K_Slab, i });
    if (text_renderer)
        for (uint32_t i = 0; i < (uint32_t)text.draws.Size(); ++i)
            order.push_back({ text.draws.Data()[i].seq, K_Text, i });

    std::sort(order.begin(), order.end(),
              [](const Ref& a, const Ref& b) { return a.seq < b.seq; });

    // Detach the write buffer so the Submit* bodies below draw instead of
    // appending the commands back onto it.
    UICommandBuffers* saved = m_ui_write_cmds;
    m_ui_write_cmds = nullptr;

    for (const Ref& r : order)
    {
        switch (r.kind)
        {
        case K_Sprite: {
            const IRUISpriteCmd& c = ui.sprites.Data()[r.idx];
            auto it = m_handle_to_sprite.find(c.sprite);
            if (it != m_handle_to_sprite.end())
                SubmitRawSprite(c.x, c.y, it->second, draw_state_make(c.draw_flags, 0));
            break;
        }
        case K_OneColour: {
            const IRUISpriteOneColourCmd& c = ui.sprites_one_colour.Data()[r.idx];
            auto it = m_handle_to_sprite.find(c.sprite);
            if (it != m_handle_to_sprite.end())
                SubmitRawSpriteOneColour(c.x, c.y, it->second, c.colour, draw_state_make(c.draw_flags, 0));
            break;
        }
        case K_Scaled: {
            const IRUISpriteScaledCmd& c = ui.sprites_scaled.Data()[r.idx];
            auto it = m_handle_to_sprite.find(c.sprite);
            if (it != m_handle_to_sprite.end())
                SubmitRawSpriteScaled(c.x, c.y, it->second, c.w, c.h, draw_state_make(c.draw_flags, 0));
            break;
        }
        case K_ScaledOneColour: {
            const IRUISpriteScaledOneColourCmd& c = ui.sprites_scaled_one_colour.Data()[r.idx];
            auto it = m_handle_to_sprite.find(c.sprite);
            if (it != m_handle_to_sprite.end())
                SubmitRawSpriteScaledOneColour(c.x, c.y, it->second, c.w, c.h, c.colour, draw_state_make(c.draw_flags, 0));
            break;
        }
        case K_ScaledRemap: {
            const IRUISpriteScaledRemapCmd& c = ui.sprites_scaled_remap.Data()[r.idx];
            auto it = m_handle_to_sprite.find(c.sprite);
            if (it != m_handle_to_sprite.end())
                SubmitRawSpriteScaledRemap(c.x, c.y, it->second, c.w, c.h, c.cmap, draw_state_make(c.draw_flags, 0));
            break;
        }
        case K_Box: {
            const IRUISolidBoxCmd& c = ui.solid_boxes.Data()[r.idx];
            SubmitSolidBox(c.x, c.y, c.w, c.h, c.colour, draw_state_make(c.draw_flags, 0));
            break;
        }
        case K_Slab: {
            const IRUISlabBackgroundCmd& c = ui.slab_backgrounds.Data()[r.idx];
            SubmitSlabBackground(c.x, c.y, c.w, c.h);
            break;
        }
        case K_Text:
            text_renderer->ReplayTextCommand(text.draws.Data()[r.idx]);
            break;
        }
    }

    m_ui_write_cmds = saved;
}

/******************************************************************************/
