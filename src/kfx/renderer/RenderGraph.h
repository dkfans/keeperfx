#ifndef RENDERER_RENDERGRAPH_H
#define RENDERER_RENDERGRAPH_H

class IFrameGraphExecutor;

// Canonical per-frame phase order, shared by every IFrameGraphExecutor
// implementation (RendererSoftware, RendererOpenGL, and any future backend).
// Ported from develop's RenderGraph::Execute() -- unlike develop's own
// RenderGraph, this branch's IR command buffers stay backend-private (each
// backend already owns its own double-buffering), so this class is scoped to
// just the one thing that needs to be shared and centrally owned: the order
// phases run in. Backends must not call their own FG* methods directly
// outside this function -- that's exactly the "dispatch consolidated into
// one backend's monolithic function" mistake this exists to prevent.
class RenderGraph {
public:
    static void Execute(IFrameGraphExecutor& exec);
};

#endif // RENDERER_RENDERGRAPH_H
