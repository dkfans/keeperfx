#ifndef RENDERER_RENDERERFRAMECOUNTER_H
#define RENDERER_RENDERERFRAMECOUNTER_H

#include <cstdint>

/// @brief Advances the shared frame counter. Producer thread only.
/// @return The new value.
uint64_t RendererFrameCounter_Advance();

/// @brief Reads the shared frame counter. Any thread.
/// @return The current value.
uint64_t RendererFrameCounter_Current();

#endif // RENDERER_RENDERERFRAMECOUNTER_H
