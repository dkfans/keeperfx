#ifndef RENDERER_IFRAMEGRAPHEXECUTOR_H
#define RENDERER_IFRAMEGRAPHEXECUTOR_H

// Ordered per-frame phase hooks. The call order is owned by whichever backend
// drives it (RendererSoftware::PresentFrame()
class IFrameGraphExecutor {
public:
    virtual ~IFrameGraphExecutor() = default;

    // -- Frame setup ---------------------------------------------------------
    virtual void FGClearFrame() {}
    virtual void FGPopulateUI() {}

    // -- World inside the lens scene-capture bracket -------------------------
    virtual void FGBeginWorldCapture() {}
    virtual void FGExecuteWorld() {}
    virtual void FGFlushSwipeOverlay() {}
    virtual void FGResolveWorldCapture() {}
    virtual void FGApplyLensPaletteUIExclusion() {}

    // -- World-space sprite / flat overlay layers ----------------------------
    virtual void FGDrawWorldSpriteLayer() {}
    virtual void FGDrawWorldOverlayFlatLayer() {}

    // -- Map-fade world snapshot (after all world layers, before the parchment)
    virtual void FGCaptureMapFadeWorld() {}

    // -- Full-screen image presents (backgrounds / parchment / FMV) ---------
    virtual void FGExecuteImagePresents() {}

    // -- Overhead map / PiP / zoom box ---------------------------------------
    virtual void FGDrawOverheadMap() {}
    virtual void FGExecutePiPCaptures() {}

    // -- Game UI, then the map-fade parchment snapshot and composite --------
    virtual void FGDrawGameUI() {}
    virtual void FGResolveMapFade() {}

    // -- Zoom-box tiles (on top of GameUI), front overlay, text -------------
    virtual void FGDrawZoomBoxes() {}
    virtual void FGDrawFrontOverlay() {}
    virtual void FGExecuteText() {}

    // -- Full-screen image-present overlay (e.g. landview window frame) --
    // -- runs after Game UI/text so it draws on top of them, unlike the
    // -- early background present in FGExecuteImagePresents() above. -----
    virtual void FGExecuteImagePresentOverlay() {}

    // -- Cursor, dev overlay -------------------------------------------------
    virtual void FGExecuteCursor() {}
    virtual void FGDrawDevToolsOverlay() {}

    // -- Present-time captures ------------------------------------------------
    virtual void FGCaptureScreenshot() {}
    virtual void FGApplyScrgbLift() {}
};

#endif // RENDERER_IFRAMEGRAPHEXECUTOR_H
