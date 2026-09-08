#ifndef RENDERER_OPENGL_GLTEXTRENDERER_H
#define RENDERER_OPENGL_GLTEXTRENDERER_H

#include "kfx/renderer/ITextRenderer.h"
#include <cstdint>

class GLUIRenderer;
struct IRTextDrawCmd;
struct TbSpriteSheet;
struct AsianFont;

class GLTextRenderer : public ITextRenderer {
public:
    void SetUIRenderer(GLUIRenderer* ui) { m_ui = ui; }

    void DrawGlyphs(const IRTextDrawCmd& cmd);

    const char* GetName() const override { return "GL-TEXT"; }

private:
    GLUIRenderer* m_ui = nullptr;

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
