#ifndef RENDERER_SPRITEHANDLE_H
#define RENDERER_SPRITEHANDLE_H

#include <cstdint>

// Generation-checked handle a deferred (IR) command can safely outlive the
// current frame with -- unlike a raw TbSprite*, which can dangle if the
// sprite sheet reloads between submission and replay.
using SpriteHandle = uint32_t;
static constexpr SpriteHandle kInvalidSpriteHandle = UINT32_MAX;

#endif // RENDERER_SPRITEHANDLE_H
