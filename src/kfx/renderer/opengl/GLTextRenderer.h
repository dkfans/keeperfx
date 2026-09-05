#ifndef RENDERER_OPENGL_GLTEXTRENDERER_H
#define RENDERER_OPENGL_GLTEXTRENDERER_H

#include "kfx/renderer/ITextRenderer.h"
#include <cstdint>

class GLUIRenderer;
struct IRTextDrawCmd;
struct TbSpriteSheet;
struct AsianFont;

// Beat 6: full word-wrap/justify layout (mirrors LbTextDrawResizedImmediate()/
// put_down_sprites()'s algorithm -- GL can't call those directly, they write
// pixels straight into the software framebuffer), DBC/CJK glyph rendering
// (via GLUIRenderer's glyph atlas, shared with western glyphs), underline
// geometry, and real TRANSPAR4/8 alpha. Draw-call batching is deliberately
// not done here -- the plan's own Beat 6 ordering names it lowest priority;
// this keeps one draw call per glyph/underline segment, same as before this
// port.
class GLTextRenderer : public ITextRenderer {
public:
    void SetUIRenderer(GLUIRenderer* ui) { m_ui = ui; }

    void DrawGlyphs(const IRTextDrawCmd& cmd);

    const char* GetName() const override { return "GL-TEXT"; }

private:
    GLUIRenderer* m_ui = nullptr;

    // Mutable draw state carried across word-wrapped line segments within a
    // single DrawGlyphs() call. Embedded control codes (colour markers,
    // Lb_TEXT_*/Lb_SPRITE_* toggle codepoints -- DkcodepageLetter in
    // bflib_sprfnt.h) mutate this as the text is scanned, mirroring how the
    // software path (put_down_sprites(), bflib_sprfnt.c) mutates the
    // equivalent globals via RendererSetDrawColour()/RendererToggleDrawFlags().
    // Local to one call, never persisted across commands -- unlike the
    // software path, GL must not touch those live globals here (this can run
    // out of step with the game thread once GL genuinely threads; see the
    // font_generation guard in DrawGlyphs()).
    struct DrawState {
        uint8_t  colour;
        uint32_t flags;
    };

    // Mirrors LbTextDrawResizedImmediate()'s word-wrap loop.
    void LayoutAndDraw(const IRTextDrawCmd& cmd, const struct TbSpriteSheet* font,
                       const struct AsianFont* dbc_font, DrawState& state);

    // Mirrors put_down_sprites(): draws one already-wrapped line segment,
    // handling embedded control/colour codes as it scans.
    void FlushSegment(const char* sbuf, const char* ebuf, float x, float y,
                      float space_len, int units_per_px,
                      const struct TbSpriteSheet* font, const struct AsianFont* dbc_font,
                      const IRTextDrawCmd& cmd, DrawState& state);

    // Returns the glyph's advance width in pixels (already scaled).
    float DrawWesternGlyph(const struct TbSpriteSheet* font, uint32_t chr,
                           float x, float y, int units_per_px, const DrawState& state);
    float DrawDbcGlyph(const struct AsianFont* dbc_font, uint32_t chr,
                       float x, float y, int units_per_px, const DrawState& state,
                       long face_colour, long shadow_colour);

    void EmitUnderline(float x, float y, float w, float h, int units_per_px, const DrawState& state);
};

#endif // RENDERER_OPENGL_GLTEXTRENDERER_H
