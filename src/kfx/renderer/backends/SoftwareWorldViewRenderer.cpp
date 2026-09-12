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
