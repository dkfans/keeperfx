#ifndef RENDERER_RENDERERSOFTWARE_H
#define RENDERER_RENDERERSOFTWARE_H

#include "kfx/renderer/IRenderer.h"

// Software backend. A thin shell for now: the legacy bflib_video / bflib_vidraw
// path still performs the actual rasterisation. Drawing migrates behind this
// seam over subsequent increments.
class RendererSoftware : public IRenderer {
public:
    bool Init() override;
    void Shutdown() override;
    const char* GetName() const override { return "software"; }
};

#endif // RENDERER_RENDERERSOFTWARE_H
