#ifndef RENDERER_RENDERGRAPH_H
#define RENDERER_RENDERGRAPH_H

class IFrameGraphExecutor;

// Canonical per-frame phase order, shared by every IFrameGraphExecutor
// implementation (RendererSoftware, RendererOpenGL, and any future backend).
class RenderGraph {
public:
    static void Execute(IFrameGraphExecutor& exec);
};

#endif // RENDERER_RENDERGRAPH_H
