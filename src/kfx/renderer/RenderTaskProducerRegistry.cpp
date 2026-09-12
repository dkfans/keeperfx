#include "pre_inc.h"
#include "kfx/renderer/RenderTaskProducerRegistry.h"
#include "kfx/renderer/IRenderTaskProducer.h"
#include "kfx/renderer/RendererThread.h"
#include "post_inc.h"

#include <algorithm>
#include <vector>
#include <mutex>

static std::vector<IRenderTaskProducer*> s_producers;
static std::mutex s_mutex;

void RenderTaskProducerRegistry_Register(IRenderTaskProducer* producer)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_producers.push_back(producer);
}

void RenderTaskProducerRegistry_Unregister(IRenderTaskProducer* producer)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = std::find(s_producers.begin(), s_producers.end(), producer);
    if (it != s_producers.end())
    {
        s_producers.erase(it);
    }
}

void RenderTaskProducerRegistry_ProduceAll(uint64_t frame_number)
{
    ASSERT_GAME_THREAD();
    std::lock_guard<std::mutex> lock(s_mutex);
    for (auto producer : s_producers)
    {
        producer->ProduceFrame(frame_number);
    }
}
