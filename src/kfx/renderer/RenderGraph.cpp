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
    //   - World, its sprite/flat overlay layers and the swipe must run INSIDE
    //     the lens FBO bracket (FGBeginWorldCapture ... FGResolveWorldCapture):
    //     software draws all of them into the lens buffer, so the lens
    //     distorts them together.
    //   - The map-fade world snapshot runs AFTER every world layer and BEFORE
    //     image-presents / overhead, which draw the parchment. Its parchment
    //     snapshot and composite run once the parchment is on screen.
    //   - Screenshot / scRGB-lift run last, after all draws, before the swap
    //     (which stays in the backend's EndFrame/present).

    // Frame setup.
    exec.FGClearFrame();
    exec.FGPopulateUI();

    // World, world-space sprite / flat overlay layers and swipe, inside the
    // lens scene-capture bracket.
    exec.FGBeginWorldCapture();
    exec.FGExecuteWorld();
    exec.FGDrawWorldSpriteLayer();
    exec.FGDrawWorldOverlayFlatLayer();
    exec.FGFlushSwipeOverlay();
    exec.FGResolveWorldCapture();
    exec.FGApplyLensPaletteUIExclusion();

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

    // Cursor, dev overlay.
    exec.FGExecuteCursor();
    exec.FGDrawDevToolsOverlay();

    // Present-time captures (before the backend's buffer swap).
    exec.FGCaptureScreenshot();
    exec.FGApplyScrgbLift();
}
