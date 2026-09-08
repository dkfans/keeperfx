#ifndef RENDERER_ICURSORLAYER_H
#define RENDERER_ICURSORLAYER_H

#include <stdint.h>
#include "kfx/renderer/DrawState.h" // TbDrawFlagsMask

struct TbSprite;

// Cursor rendering abstraction — owns both the OS pointer sprite and the
// in-game keeper-hand sprite so both can be guaranteed to draw on top of
// everything else, without special-casing them in the other renderers.

class ICursorLayer {
public:
    virtual ~ICursorLayer() = default;

    // Submit the OS pointer sprite for deferred rendering at end-of-frame.
    virtual void SubmitPointerSprite(const struct TbSprite* spr,
                                     int32_t x, int32_t y,
                                     int units_per_px) = 0;
                                     
    // Returns 1 if this layer handled the sprite (caller must not also draw
    // it), 0 to fall back to the caller's own immediate draw.
    virtual int SubmitKeeperHandSprite(short x, short y,
                                       unsigned short kspr_base,
                                       short angle,
                                       unsigned char sprgroup,
                                       int32_t scale,
                                       TbDrawFlagsMask draw_flags) = 0;

    // Draw all queued cursor sprites. Called as the absolute last draw step
    // before the buffer swap.
    virtual void Draw() = 0;

    // Clear queued state. Called at the start of each frame.
    virtual void Clear() = 0;

    virtual const char* GetName() const = 0;
};

#endif // RENDERER_ICURSORLAYER_H
