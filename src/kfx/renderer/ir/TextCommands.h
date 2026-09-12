#ifndef RENDERER_IR_TEXTCOMMANDS_H
#define RENDERER_IR_TEXTCOMMANDS_H

#include <cstdint>
#include <cstring>
#include <utility>
#include "kfx/renderer/ir/IRCommandBuffer.h"

static constexpr size_t kIRTextMaxLen = 256;

// LbTextDrawResizedImmediate is a full layout pass over lbTextJustifyWindow/
// lbTextClipWindow/lbFontPtr; this snapshots that ambient state so a deferred
// replay reproduces the same layout. Safe only because submit and replay
// happen on the same thread within the same frame; a real
// cross-thread gap would need font_generation to actually be checked,
// not just carried.
struct IRTextDrawCmd
{
    int32_t  pos_x        = 0;
    int32_t  pos_y        = 0;
    int32_t  units_per_px = 16;
    unsigned char draw_colour = 0;
    uint32_t draw_flags   = 0;

    int32_t  justify_x = 0, justify_y = 0, justify_w = 0;
    int32_t  clip_x = 0, clip_y = 0, clip_w = 0, clip_h = 0;

    const void* font = nullptr; // TbSpriteSheet*, opaque here
    uint32_t    font_generation = 0;

    /* DBC/CJK font state, snapshotted the same way `font` is (see the struct
     * comment) rather than read live from bflib_sprfnt.c's active_dbcfont/
     * dbc_colour0/1 globals at replay time. dbc_font is null whenever DBC
     * text isn't active for this draw (is_dbc_language() false, or the
     * language pack didn't load), in which case GL draws the western
     * sprite-font path exactly as it already does. */
    const void* dbc_font    = nullptr; // struct AsianFont*, opaque here
    uint8_t     dbc_enabled = 0;
    long        dbc_colour0 = 0; // DBC face colour (palette index)
    long        dbc_colour1 = 0; // DBC shadow colour (palette index)

    uint32_t seq = 0; // shared with UICommandBuffers so a merged replay
                       // recovers true UI+text submission order

    char text[kIRTextMaxLen] = {};

    void SetText(const char* src)
    {
        if (src) {
            std::strncpy(text, src, kIRTextMaxLen - 1);
            text[kIRTextMaxLen - 1] = '\0';
        } else {
            text[0] = '\0';
        }
    }
};

struct TextCommandBuffers
{
    IRCommandBuffer<IRTextDrawCmd> draws;

    uint32_t  next_seq   = 0;
    uint32_t* shared_seq = nullptr;

    uint32_t NextSeq() { return shared_seq ? (*shared_seq)++ : next_seq++; }

    void Reset()   { draws.Reset(); next_seq = 0; }
    void Reserve(size_t n) { draws.Reserve(n); }
    void Swap(TextCommandBuffers& other) { draws.Swap(other.draws); std::swap(next_seq, other.next_seq); }
};

#endif // RENDERER_IR_TEXTCOMMANDS_H
