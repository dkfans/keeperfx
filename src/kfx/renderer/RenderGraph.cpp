#include "pre_inc.h"
#include "kfx/renderer/RenderGraph.h"
#include "kfx/renderer/IFrameGraphExecutor.h"
#include "kfx/profiling/KfxProfiling.h"
#include "post_inc.h"

// Ported verbatim (order + constraints) from develop's RenderGraph::Execute().
void RenderGraph::Execute(IFrameGraphExecutor& exec)
{
    KFX_ZONE_COLOR("RenderGraph::Execute", KFX_COLOR_RENDER_CPU);

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
    { KFX_ZONE_COLOR("FG::ClearFrame", KFX_COLOR_RENDER_CPU); exec.FGClearFrame(); }
    { KFX_ZONE_COLOR("FG::PopulateUI", KFX_COLOR_RENDER_CPU); exec.FGPopulateUI(); }

    // World, world-space sprite / flat overlay layers and swipe, inside the
    // lens scene-capture bracket.
    { KFX_ZONE_COLOR("FG::BeginWorldCapture", KFX_COLOR_RENDER_GPU); exec.FGBeginWorldCapture(); }
    { KFX_ZONE_COLOR("FG::ExecuteWorld", KFX_COLOR_RENDER_GPU); exec.FGExecuteWorld(); }
    { KFX_ZONE_COLOR("FG::DrawWorldSpriteLayer", KFX_COLOR_RENDER_GPU); exec.FGDrawWorldSpriteLayer(); }
    { KFX_ZONE_COLOR("FG::DrawWorldOverlayFlatLayer", KFX_COLOR_RENDER_GPU); exec.FGDrawWorldOverlayFlatLayer(); }
    { KFX_ZONE_COLOR("FG::FlushSwipeOverlay", KFX_COLOR_RENDER_GPU); exec.FGFlushSwipeOverlay(); }
    { KFX_ZONE_COLOR("FG::ResolveWorldCapture", KFX_COLOR_RENDER_GPU); exec.FGResolveWorldCapture(); }
    { KFX_ZONE_COLOR("FG::ApplyLensPaletteUIExclusion", KFX_COLOR_RENDER_GPU); exec.FGApplyLensPaletteUIExclusion(); }

    // Map-fade world snapshot (after world, before presents / overhead).
    { KFX_ZONE_COLOR("FG::CaptureMapFadeWorld", KFX_COLOR_RENDER_GPU); exec.FGCaptureMapFadeWorld(); }

    // Full-screen image presents (backgrounds / parchment / FMV).
    { KFX_ZONE_COLOR("FG::ExecuteImagePresents", KFX_COLOR_RENDER_GPU); exec.FGExecuteImagePresents(); }

    // Overhead map / PiP captures.
    { KFX_ZONE_COLOR("FG::DrawOverheadMap", KFX_COLOR_RENDER_GPU); exec.FGDrawOverheadMap(); }
    { KFX_ZONE_COLOR("FG::ExecutePiPCaptures", KFX_COLOR_RENDER_GPU); exec.FGExecutePiPCaptures(); }

    // Game UI, then the map-fade parchment snapshot and composite.
    { KFX_ZONE_COLOR("FG::DrawGameUI", KFX_COLOR_RENDER_GPU); exec.FGDrawGameUI(); }
    { KFX_ZONE_COLOR("FG::ResolveMapFade", KFX_COLOR_RENDER_GPU); exec.FGResolveMapFade(); }

    // Zoom-box tiles (on top of GameUI), front overlay, text.
    { KFX_ZONE_COLOR("FG::DrawZoomBoxes", KFX_COLOR_RENDER_GPU); exec.FGDrawZoomBoxes(); }
    { KFX_ZONE_COLOR("FG::DrawFrontOverlay", KFX_COLOR_RENDER_GPU); exec.FGDrawFrontOverlay(); }
    { KFX_ZONE_COLOR("FG::ExecuteText", KFX_COLOR_RENDER_GPU); exec.FGExecuteText(); }

    // Image-present overlay (e.g. landview window frame) -- after Game UI
    // and text so it draws on top of them, matching the immediate-mode
    // renderer's call-order-determined layering.
    { KFX_ZONE_COLOR("FG::ExecuteImagePresentOverlay", KFX_COLOR_RENDER_GPU); exec.FGExecuteImagePresentOverlay(); }

    // Cursor, dev overlay.
    { KFX_ZONE_COLOR("FG::ExecuteCursor", KFX_COLOR_RENDER_GPU); exec.FGExecuteCursor(); }
    { KFX_ZONE_COLOR("FG::DrawDevToolsOverlay", KFX_COLOR_RENDER_GPU); exec.FGDrawDevToolsOverlay(); }

    // Present-time captures (before the backend's buffer swap).
    { KFX_ZONE_COLOR("FG::CaptureScreenshot", KFX_COLOR_RENDER_GPU); exec.FGCaptureScreenshot(); }
    { KFX_ZONE_COLOR("FG::ApplyScrgbLift", KFX_COLOR_RENDER_GPU); exec.FGApplyScrgbLift(); }
}
