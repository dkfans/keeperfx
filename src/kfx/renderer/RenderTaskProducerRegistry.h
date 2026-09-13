#ifndef RENDERER_RENDERTASKPRODUCERREGISTRY_H
#define RENDERER_RENDERTASKPRODUCERREGISTRY_H

#include <cstdint>

class IRenderTaskProducer;

/// @brief Registers a producer. Any thread the producer lives on.
/// @param producer Producer to register.
void RenderTaskProducerRegistry_Register(IRenderTaskProducer* producer);

/// @brief Unregisters a producer.
/// @param producer Producer to unregister.
void RenderTaskProducerRegistry_Unregister(IRenderTaskProducer* producer);

/// @brief Calls ProduceFrame on every registered producer. Game thread only.
/// @param frame_number Frame currently being authored.
void RenderTaskProducerRegistry_ProduceAll(uint64_t frame_number);

#endif // RENDERER_RENDERTASKPRODUCERREGISTRY_H
