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

    // -- Map-fade compose (after world, before presents/overhead) -----------
    virtual void FGExecuteMapFade() {}

    // -- World-space sprite / flat overlay layers ----------------------------
    virtual void FGDrawWorldSpriteLayer() {}
    virtual void FGDrawWorldOverlayFlatLayer() {}

    // -- Full-screen image presents (backgrounds / parchment / FMV) ---------
    virtual void FGExecuteImagePresents() {}

    // -- Overhead map / PiP / zoom box ---------------------------------------
    virtual void FGDrawOverheadMap() {}
    virtual void FGExecutePiPCaptures() {}

    // -- Game UI, then deferred world-view capture ---------------------------
    virtual void FGDrawGameUI() {}
    virtual void FGCaptureWorldFrameIfPending() {}

    // -- Zoom-box tiles (on top of GameUI), front overlay, text -------------
    virtual void FGDrawZoomBoxes() {}
    virtual void FGDrawFrontOverlay() {}
    virtual void FGExecuteText() {}

    // -- Full-screen tint, cursor, dev overlay -------------------------------
    virtual void FGDrawScreenTint() {}
    virtual void FGExecuteCursor() {}
    virtual void FGDrawDevToolsOverlay() {}

    // -- Present-time captures ------------------------------------------------
    virtual void FGCaptureScreenshot() {}
    virtual void FGApplyScrgbLift() {}
};

#endif // RENDERER_IFRAMEGRAPHEXECUTOR_H
