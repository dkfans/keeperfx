/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file SoftwareWorldViewRenderer.h
 *     CPU software implementation of IWorldViewRenderer.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/IWorldViewRenderer.h"

/******************************************************************************/

class SoftwareWorldViewRenderer : public IWorldViewRenderer {
public:
    SoftwareWorldViewRenderer()  = default;
    ~SoftwareWorldViewRenderer() = default;

    void BeginWorldPass(int w, int h, int vp_x, int vp_y) override;
    void DrawIsometricView() override;
    void DrawFrontView(struct Camera* cam) override;

    int  ResolveDeferredWorld() override;
    void ReexecuteDeferredWorld() override;
    void MarkDeferredWorldAsLensCapture() override { m_lens = true; }

    const char* GetName() const override { return "SOFTWARE"; }

private:
    // Rasterise the recorded descriptor. Called synchronously from
    // DrawIsometricView()/DrawFrontView() because software has no
    // ILensRenderer to redirect a later-resolved draw into a lens-capture
    // buffer, so it cannot actually defer past callers that swap
    // lbDisplay.WScreen around the Draw*View() call -- see draw_creature_view()
    // in thing_creature.c. True deferral is GL-only.
    void ExecuteRecordedWorld();

    // Engine-window rect of the current pass.
    int  m_win_x = 0;
    int  m_win_y = 0;
    int  m_win_w = 0;
    int  m_win_h = 0;

    // Last-recorded world descriptor, kept for ReexecuteDeferredWorld().
    bool m_valid     = false;  // a world has been recorded at least once (re-execute)
    bool m_frontview = false;  // front view vs isometric/1st-person
    bool m_lens      = false;  // possession lens capture flag (unused by software; only GL sets this)
    struct Camera* m_cam = nullptr; // front-view camera (null for iso)
};

/******************************************************************************/
