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
    //   - The map-fade world snapshot runs AFTER every world layer and BEFORE
    //     image-presents / overhead, which draw the parchment. Its parchment
    //     snapshot and composite run once the parchment is on screen.
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

    // World-space sprite / flat overlay layers.
    exec.FGDrawWorldSpriteLayer();
    exec.FGDrawWorldOverlayFlatLayer();

    // Map-fade world snapshot (after world, before presents / overhead).
    exec.FGCaptureMapFadeWorld();

    // Full-screen image presents (backgrounds / parchment / FMV).
    exec.FGExecuteImagePresents();

    // Overhead map / PiP captures.
    exec.FGDrawOverheadMap();
    exec.FGExecutePiPCaptures();

    // Game UI, then the map-fade parchment snapshot and composite.
    exec.FGDrawGameUI();
    exec.FGResolveMapFade();

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
