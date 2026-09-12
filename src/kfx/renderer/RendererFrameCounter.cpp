#include "pre_inc.h"
#include "kfx/renderer/RendererFrameCounter.h"
#include "post_inc.h"

#include <atomic>

static std::atomic<uint64_t> s_frame{0};

uint64_t RendererFrameCounter_Advance()
{
    return ++s_frame;
}

uint64_t RendererFrameCounter_Current()
{
    return s_frame.load(std::memory_order_relaxed);
}
