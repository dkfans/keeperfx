#ifndef RENDERER_IR_TEXTCOMMANDS_H
#define RENDERER_IR_TEXTCOMMANDS_H

#include <array>
#include <cstdint>
#include <utility>
#include "kfx/renderer/ir/IRCommandBuffer.h"
#include "kfx/renderer/SpriteHandle.h"

enum class IRTextGlyphKind : uint8_t
{
    PaletteSprite, // glyph sampled through the palette
    ColourSprite,  // glyph drawn flat in `colour`
    SolidRect,     // underline segment drawn flat in `colour`
};

// One glyph or underline segment, laid out when the text is submitted so a
// replay on another thread never reads font memory the game may have freed.
struct IRTextGlyph
{
    IRTextGlyphKind kind   = IRTextGlyphKind::PaletteSprite;
    uint8_t         colour = 0; // palette index, for ColourSprite and SolidRect
    SpriteHandle    sprite = kInvalidSpriteHandle;
    float x = 0.0f, y = 0.0f;
    float w = 0.0f, h = 0.0f;   // SolidRect only; sprites take their size from the atlas
    int32_t units_per_px = 16;
    float alpha = 1.0f;
};

// Snapshot of the ambient text state (lbTextJustifyWindow/lbTextClipWindow/
// lbFontPtr) at submission. The string itself isn't kept: the backend lays out
// glyphs from the caller's text during submission (glyph_first/glyph_count).
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

    // Western sprite fonts can use an arbitrary index remap. Snapshot it
    // alongside the rest of the text state so deferred renderers match the
    // software rasteriser.
    bool has_remap = false;
    std::array<unsigned char, 256> remap{};

    uint32_t seq = 0; // shared with UICommandBuffers so a merged replay
                       // recovers true UI+text submission order

    // Range in TextCommandBuffers::glyphs, for backends that lay out at submission.
    uint32_t glyph_first = 0;
    uint32_t glyph_count = 0;
};

struct TextCommandBuffers
{
    IRCommandBuffer<IRTextDrawCmd> draws;
    IRCommandBuffer<IRTextGlyph>   glyphs;

    uint32_t  next_seq   = 0;
    uint32_t* shared_seq = nullptr;

    uint32_t NextSeq() { return shared_seq ? (*shared_seq)++ : next_seq++; }

    void Reset()   { draws.Reset(); glyphs.Reset(); next_seq = 0; }
    void Reserve(size_t n) { draws.Reserve(n); }
    void Swap(TextCommandBuffers& other) { draws.Swap(other.draws); glyphs.Swap(other.glyphs); std::swap(next_seq, other.next_seq); }
};

#endif // RENDERER_IR_TEXTCOMMANDS_H
