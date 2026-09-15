/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file SoftwareWorldViewRenderer.cpp
 *     CPU software implementation of IWorldViewRenderer.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/backends/SoftwareWorldViewRenderer.h"
#include "engine_render.h"  // software_execute_world_from_ir
#include "keeperfx.hpp"       // EngineSpriteDrawUsingAlpha
#include "bflib_vidraw.h"   // LbSpriteDrawUsingScalingData, DrawAlphaSpriteUsingScalingData
#include "post_inc.h"

/******************************************************************************/

void SoftwareWorldViewRenderer::BeginWorldPass(int w, int h, int vp_x, int vp_y)
{
    m_win_x = vp_x;
    m_win_y = vp_y;
    m_win_w = w;
    m_win_h = h;
}

void SoftwareWorldViewRenderer::DrawIsometricView()
{
    m_valid     = true;
    m_frontview = false;
    m_lens      = false;   // no lens redirect in software
    m_cam       = nullptr;
    // Execute now, not at frame-graph resolve time: callers such as
    // draw_creature_view() (thing_creature.c) temporarily swap
    // lbDisplay.WScreen to an offscreen lens-capture buffer around this call
    // and restore it immediately after -- deferring the rasterize to a later
    // phase would run it against whatever buffer/window happens to be live
    // by then, not the one that was live here.
    ExecuteRecordedWorld();
}

void SoftwareWorldViewRenderer::DrawFrontView(struct Camera* cam)
{
    m_valid     = true;
    m_frontview = true;
    m_lens      = false;
    m_cam       = cam;
    ExecuteRecordedWorld();
}

void SoftwareWorldViewRenderer::ExecuteRecordedWorld()
{
    software_execute_world_from_ir(m_win_x, m_win_y, m_win_w, m_win_h,
                                   m_frontview ? 1 : 0, m_cam);
}

int SoftwareWorldViewRenderer::ResolveDeferredWorld()
{
    // Software already executed synchronously in Draw*View() above; nothing
    // is ever left pending here. True deferral (record now, resolve at
    // frame-graph time) is GL-only, added when GL needs to batch
    // world geometry through IR.
    return 0;
}

void SoftwareWorldViewRenderer::ReexecuteDeferredWorld()
{
    if (m_valid)
        ExecuteRecordedWorld();
}

// Draws through the scaling data the caller set for the sprite's frame; the
// draw flags and remap table are read from the ambient draw state.
void SoftwareWorldViewRenderer::SubmitKeeperSprite(int32_t frame_x, int32_t frame_y,
                                                   int32_t /*dst_x*/, int32_t /*dst_y*/,
                                                   int32_t /*dst_w*/, int32_t /*dst_h*/,
                                                   const unsigned char* data, int src_w, int /*src_h*/,
                                                   int32_t content_h,
                                                   unsigned int /*draw_flags*/, const unsigned char* /*remap*/,
                                                   int32_t /*sprite_id*/)
{
    const struct TbSourceBuffer buffer = {
        data,
        (unsigned long)src_w,
        (unsigned long)content_h,
        (unsigned long)src_w,
    };
    if (EngineSpriteDrawUsingAlpha)
        DrawAlphaSpriteUsingScalingData(frame_x, frame_y, &buffer);
    else
        LbSpriteDrawUsingScalingData(frame_x, frame_y, &buffer);
}
