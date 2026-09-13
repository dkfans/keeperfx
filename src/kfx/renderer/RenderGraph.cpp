#include "pre_inc.h"
#include "kfx/renderer/RenderGraph.h"
#include "kfx/renderer/IFrameGraphExecutor.h"
#include "post_inc.h"

// Ported verbatim (order + constraints) from develop's RenderGraph::Execute().
void RenderGraph::Execute(IFrameGraphExecutor& exec)
{
    // This function OWNS the per-frame execution order. Each call
    // is one ordered phase realised by the backend.
    // Ordering constraints:
    //   - World must run INSIDE the lens FBO bracket
    //     (FGBeginWorldCapture ... FGResolveWorldCapture) so post-process passes
    //     operate on the captured scene before it reaches the screen.
    //   - Map-fade must run AFTER world but BEFORE image-presents / overhead so
    //     those queues are still available for the parchment FBO capture.
    //   - The deferred world-view capture runs AFTER GameUI so the sidebar is
    //     part of the crossfaded snapshot.
    //   - Screenshot / scRGB-lift run last, after all draws, before the swap
    //     (which stays in the backend's EndFrame/present).

    // Frame setup.
    exec.FGClearFrame();
    exec.FGPopulateUI();

    // World inside the lens scene-capture bracket.
    exec.FGBeginWorldCapture();
    exec.FGExecuteWorld();
    exec.FGFlushSwipeOverlay();
    exec.FGResolveWorldCapture();
    exec.FGApplyLensPaletteUIExclusion();

    // Map-fade compose (after world, before presents / overhead).
    exec.FGExecuteMapFade();

    // World-space sprite / flat overlay layers.
    exec.FGDrawWorldSpriteLayer();
    exec.FGDrawWorldOverlayFlatLayer();

    // Full-screen image presents (backgrounds / parchment / FMV).
    exec.FGExecuteImagePresents();

    // Overhead map / PiP captures.
    exec.FGDrawOverheadMap();
    exec.FGExecutePiPCaptures();

    // Game UI, then the deferred world-view capture.
    exec.FGDrawGameUI();
    exec.FGCaptureWorldFrameIfPending();

    // Zoom-box tiles (on top of GameUI), front overlay, text.
    exec.FGDrawZoomBoxes();
    exec.FGDrawFrontOverlay();
    exec.FGExecuteText();

    // Full-screen tint, cursor, dev overlay.
    exec.FGDrawScreenTint();
    exec.FGExecuteCursor();
    exec.FGDrawDevToolsOverlay();

    // Present-time captures (before the backend's buffer swap).
    exec.FGCaptureScreenshot();
    exec.FGApplyScrgbLift();
}
