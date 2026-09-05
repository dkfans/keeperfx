#ifndef RENDERER_IR_UICOMMANDS_H
#define RENDERER_IR_UICOMMANDS_H

#include <cstdint>
#include <utility>
#include "kfx/renderer/SpriteHandle.h"
#include "kfx/renderer/DrawState.h"
#include "kfx/renderer/ir/IRCommandBuffer.h"

// Trimmed to the families Phase 3 actually routes today: sprites, solid
// boxes, slab background, cursor. World/image-present/post-process families
// join when their phase needs them.

// Beat 5: which composited layer a command belongs to, ported from develop's
// IRUILayer -- WorldOverlay/WorldOverlayFlat let world-positioned UI content
// (creature status, room flags, floating damage text) draw through this same
// IR/batching pipeline instead of a separate bucket-switch path, at the
// correct point in the frame (see RenderGraph::Execute()'s existing
// FGDrawWorldSpriteLayer/FGDrawWorldOverlayFlatLayer call sites). Trimmed to
// the 3 values this branch has a real consumer for; Overlay/Cursor from
// develop's 5-value enum are deliberately not added until something needs
// them (cursor stays on GLCursorLayer's own stash/snapshot path; nothing
// currently needs a distinct top-overlay layer).
enum class IRUILayer : uint8_t {
    WorldOverlay     = 0,  // World-space, depth-tested (creature status).
    WorldOverlayFlat = 1,  // World-space, NOT depth-tested (room flags, floating text).
    GameUI           = 2,  // All in-game 2D chrome (default).
};

struct IRUISpriteCmd {
    int32_t x = 0, y = 0;
    int32_t units_per_px = 16;
    SpriteHandle sprite = kInvalidSpriteHandle;
    TbDrawFlagsMask draw_flags = 0;
    IRUILayer layer = IRUILayer::GameUI;
    float ndc_z = 0.5f;
    uint32_t seq = 0;
};

struct IRUISpriteOneColourCmd {
    int32_t x = 0, y = 0;
    SpriteHandle sprite = kInvalidSpriteHandle;
    unsigned char colour = 0;
    TbDrawFlagsMask draw_flags = 0;
    IRUILayer layer = IRUILayer::GameUI;
    float ndc_z = 0.5f;
    uint32_t seq = 0;
};

struct IRUISpriteScaledCmd {
    int32_t x = 0, y = 0, w = 0, h = 0;
    SpriteHandle sprite = kInvalidSpriteHandle;
    TbDrawFlagsMask draw_flags = 0;
    IRUILayer layer = IRUILayer::GameUI;
    float ndc_z = 0.5f;
    uint32_t seq = 0;
};

struct IRUISpriteScaledOneColourCmd {
    int32_t x = 0, y = 0, w = 0, h = 0;
    SpriteHandle sprite = kInvalidSpriteHandle;
    unsigned char colour = 0;
    TbDrawFlagsMask draw_flags = 0;
    IRUILayer layer = IRUILayer::GameUI;
    float ndc_z = 0.5f;
    uint32_t seq = 0;
};

// cmap points into the static fade_tables array (never freed), so a raw
// pointer survives the deferred gap safely -- unlike a sprite pointer.
struct IRUISpriteScaledRemapCmd {
    int32_t x = 0, y = 0, w = 0, h = 0;
    SpriteHandle sprite = kInvalidSpriteHandle;
    const unsigned char* cmap = nullptr;
    TbDrawFlagsMask draw_flags = 0;
    IRUILayer layer = IRUILayer::GameUI;
    float ndc_z = 0.5f;
    uint32_t seq = 0;
};

struct IRUISolidBoxCmd {
    int32_t x = 0, y = 0, w = 0, h = 0;
    unsigned char colour = 0;
    TbDrawFlagsMask draw_flags = 0;
    IRUILayer layer = IRUILayer::GameUI;
    float ndc_z = 0.5f;
    uint32_t seq = 0;
};

struct IRUISlabBackgroundCmd {
    int32_t x = 0, y = 0, w = 0, h = 0;
    IRUILayer layer = IRUILayer::GameUI;
    float ndc_z = 0.5f;
    uint32_t seq = 0;
};

// No IR command type for the cursor: GLCursorLayer stashes the submitted
// pointer sprite directly (see its header comment) rather than going through
// this shared buffer -- it's a single value, not a stream of draws, and
// Clear() already has the right lifecycle hook (LbI_PointerHandler::Release).

struct UICommandBuffers {
    IRCommandBuffer<IRUISpriteCmd>                sprites;
    IRCommandBuffer<IRUISpriteOneColourCmd>       sprites_one_colour;
    IRCommandBuffer<IRUISpriteScaledCmd>          sprites_scaled;
    IRCommandBuffer<IRUISpriteScaledOneColourCmd> sprites_scaled_one_colour;
    IRCommandBuffer<IRUISpriteScaledRemapCmd>     sprites_scaled_remap;
    IRCommandBuffer<IRUISolidBoxCmd>              solid_boxes;
    IRCommandBuffer<IRUISlabBackgroundCmd>        slab_backgrounds;

    uint32_t  next_seq   = 0;
    uint32_t* shared_seq = nullptr;

    // Shared with TextCommandBuffers::NextSeq() when wired, so a merged
    // replay recovers true cross-family submission order.
    uint32_t NextSeq() { return shared_seq ? (*shared_seq)++ : next_seq++; }

    void Reset()
    {
        sprites.Reset();
        sprites_one_colour.Reset();
        sprites_scaled.Reset();
        sprites_scaled_one_colour.Reset();
        sprites_scaled_remap.Reset();
        solid_boxes.Reset();
        slab_backgrounds.Reset();
        next_seq = 0;
    }

    void Reserve(size_t sprites_n)
    {
        sprites.Reserve(sprites_n);
        sprites_one_colour.Reserve(sprites_n / 4);
        sprites_scaled.Reserve(sprites_n / 4);
        sprites_scaled_one_colour.Reserve(sprites_n / 8);
        sprites_scaled_remap.Reserve(sprites_n / 8);
        solid_boxes.Reserve(64);
        slab_backgrounds.Reserve(16);
    }

    void Swap(UICommandBuffers& other)
    {
        sprites.Swap(other.sprites);
        sprites_one_colour.Swap(other.sprites_one_colour);
        sprites_scaled.Swap(other.sprites_scaled);
        sprites_scaled_one_colour.Swap(other.sprites_scaled_one_colour);
        sprites_scaled_remap.Swap(other.sprites_scaled_remap);
        solid_boxes.Swap(other.solid_boxes);
        slab_backgrounds.Swap(other.slab_backgrounds);
        std::swap(next_seq, other.next_seq);
    }
};

#endif // RENDERER_IR_UICOMMANDS_H
