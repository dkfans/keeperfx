#ifndef RENDERER_IRENDERTASKPRODUCER_H
#define RENDERER_IRENDERTASKPRODUCER_H

#include <cstdint>

/// A source of per-frame render content beyond the core game-code call graph.
class IRenderTaskProducer
{
public:
    virtual ~IRenderTaskProducer() = default;

    /// @brief Submits this producer's content for one frame.
    /// @param frame_number Frame currently being authored.
    virtual void ProduceFrame(uint64_t frame_number) = 0;
};

#endif // RENDERER_IRENDERTASKPRODUCER_H
