#ifndef RENDERER_OPENGL_GLUIRENDERER_H
#define RENDERER_OPENGL_GLUIRENDERER_H

#include "kfx/renderer/IUIRenderer.h"
#include "kfx/renderer/ir/UICommands.h" // IRUILayer
#include "kfx/renderer/GpuResourceHandle.h"
#include <atomic>
#include <vector>
#include <unordered_map>
#include <cstdint>

class GLSpriteAtlas;
class GLTextRenderer;
class GLResourceMapper;
struct UICommandBuffers;
struct TextCommandBuffers;
struct AsianFont;

class GLUIRenderer : public IUIRenderer {
public:
    bool Init();
    void Shutdown();

    void SetAtlas(GLSpriteAtlas* atlas) { m_atlas = atlas; }
    // Palette/fade-table textures: only the handle is stored, resolved fresh
    // at each point of use rather than cached as a raw GLuint, so it stays
    // valid across a reload of the underlying texture.
    void SetResourceMapper(GLResourceMapper* mapper) { m_resource_mapper = mapper; }
    void SetPaletteTexture(GpuResourceHandle tex) { m_palette_tex_handle = tex; }
    void SetFadeTableTexture(GpuResourceHandle tex) { m_fade_table_tex_handle = tex; }
    void SetScreenSize(int w, int h) { m_screen_w = w; m_screen_h = h; }
    /** For GLTextRenderer's glScissor Y-flip (top-left clip rect -> GL's
     *  bottom-left scissor origin). */
    int GetScreenWidth() const { return m_screen_w; }
    int GetScreenHeight() const { return m_screen_h; }

    // Packs into the atlas as a side effect, so every handle IUIRenderer
    // hands out is guaranteed drawable.
    SpriteHandle ResolveSprite(const struct TbSprite* spr) override;

    void DrawFromIR(const UICommandBuffers& ui, const TextCommandBuffers& text,
                    GLTextRenderer* text_renderer);

    void BuildQuadsFromIR(const UICommandBuffers& ui);

    /** Flush the WorldOverlay layer (depth-tested against world geometry --
     *  creature status). Call after BuildQuadsFromIR(), from
     *  RendererOpenGL::FGDrawWorldSpriteLayer(). */
    void DrawWorldSpriteLayerRT();

    /** Flush the WorldOverlayFlat layer (no depth test -- room flags,
     *  floating gold/damage text). Call from
     *  RendererOpenGL::FGDrawWorldOverlayFlatLayer(). */
    void DrawWorldOverlayFlatLayerRT();

    /** Flush the Overlay layer (drawn dead-last, over GameUI -- zoom-box
     *  corner frames/thing sprites). Call from
     *  RendererOpenGL::FGDrawFrontOverlay(), after FGDrawGameUI(). */
    void DrawFrontOverlay();

    /** Flush the GameUI layer, interleaved with text draws by their shared
     *  seq (same ordering DrawFromIR() always used) -- the successor to the
     *  old all-in-one DrawFromIR() for the main per-frame buffer. Call from
     *  RendererOpenGL::FGDrawGameUI(). */
    void DrawGameUILayerRT(const TextCommandBuffers& text, GLTextRenderer* text_renderer);

    /** Game thread: latch a new slab background tile buffer (gui_slab,
     *  GUI_SLAB_DIMENSION -- see gui_draw.c/.h) for upload on the render
     *  thread. Stored via atomics (same handoff shape as other game-thread
     *  -> render-thread resource updates on this branch); consumed lazily,
     *  once, the next time a slab quad is actually flushed. Ported from
     *  develop's UpdateSlabTexture()/FlushPendingInit() split. */
    void UpdateSlabTexture(const unsigned char* data, int dim) override;

    void DrawGlyphQuad(SpriteHandle glyph, float x, float y, int units_per_px,
                       float r, float g, float b, float a, bool sample_palette = true);

    SpriteHandle ResolveDbcGlyph(const struct AsianFont* font, uint32_t codepoint);

    void DrawSolidRect(float x, float y, float w, float h, float r, float g, float b, float a)
    {
        draw_solid_quad(x, y, w, h, r, g, b, a);
    }

    const char* GetName() const override { return "GL-UI"; }

private:
    GLSpriteAtlas* m_atlas = nullptr;
    GLResourceMapper* m_resource_mapper = nullptr;
    GpuResourceHandle m_palette_tex_handle = kInvalidGpuResource;
    GpuResourceHandle m_fade_table_tex_handle = kInvalidGpuResource;
    int m_screen_w = 0;
    int m_screen_h = 0;

    GpuResourceHandle m_shader_sprite_handle         = kInvalidGpuResource;
    GpuResourceHandle m_shader_sprite_colored_handle = kInvalidGpuResource;
    GpuResourceHandle m_shader_remap_handle          = kInvalidGpuResource;
    GpuResourceHandle m_shader_solid_handle          = kInvalidGpuResource;
    // Single-quad immediate path (text glyphs, cursor). One GpuGeometryBuffer
    // handle bundles the VAO+VBO pair the mapper realizes together.
    GpuResourceHandle m_geom_handle = kInvalidGpuResource;

    /** Resolves a program handle to its raw GL id, or 0 if unresolved. */
    unsigned int ResolveShaderId(GpuResourceHandle handle) const;

    void draw_textured_quad(GpuResourceHandle shader_handle, float x, float y, float w, float h,
                            float u0, float v0, float u1, float v1,
                            float r, float g, float b, float a,
                            float remap_row = -1.0f);
    void draw_solid_quad(float x, float y, float w, float h, float r, float g, float b, float a);

    static constexpr SpriteHandle kDbcHandleBase = 0x80000000u;
    std::unordered_map<uint64_t, SpriteHandle> m_dbc_glyph_handles;
    SpriteHandle m_next_dbc_handle = kDbcHandleBase;

    struct UIQuad {
        float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        float u0 = 0, v0 = 0, u1 = 0, v1 = 0;
        float r = 1, g = 1, b = 1, a = 1;
        float ndc_z = 0.5f;
        float mode = 0.0f;
        int   remap_row = -1;
        uint32_t seq = 0;
    };

    enum PassType { PASS_SPRITE, PASS_SOLID, PASS_SLAB, PASS_COLORED, PASS_REMAP };
    static PassType classify(float mode);

    // WorldOverlay=0, WorldOverlayFlat=1, GameUI=2, Overlay=3 (matches IRUILayer).
    static constexpr int kLayerCount = 4;
    std::vector<UIQuad> m_quads[kLayerCount]; // RT: per-frame scratch, built by BuildQuadsFromIR()

    // Game viewport rect for this frame, captured by BuildQuadsFromIR().
    int  m_game_vp_x = 0;
    int  m_game_vp_y = 0;
    int  m_game_vp_w = 0;
    int  m_game_vp_h = 0;
    bool m_game_vp_set = false;

    /** walks every IR command kind in cross-kind seq order, converts each to 1 UIQuad (2
     *  for slab backgrounds: an opaque backing quad + the tiled quad), and
     *  appends into out[cmd.layer]. */
    void AppendQuadsFromIR(const UICommandBuffers& ui, std::vector<UIQuad> (&out)[kLayerCount]);

    /** Batched flush of one layer's quads (world layers -- no text
     *  interleaving needed, text never carries WorldOverlay/WorldOverlayFlat
     *  ambient state). Clears `quads` when done. */
    void FlushQuadLayer(std::vector<UIQuad>& quads, bool depth_test);

    /** Flush one run of same-PassType quads as a single batched draw call --
     *  the actual GL work (bind program/textures, build the shared scratch
     *  vertex buffer, glDrawArrays). Shared by FlushQuadLayer() and
     *  DrawGameUIQuadsInterleaved()'s text-interleaved walk. */
    void FlushQuadRun(const std::vector<UIQuad>& run, PassType pass, int remap_row);

    /** Merge+batch+draw one GameUI-shaped quad vector interleaved with text
     *  by seq -- shared by DrawGameUILayerRT() (the real per-frame path) and
     *  DrawFromIR() (a local one-shot vector). Clears `quads` when done. */
    void DrawGameUIQuadsInterleaved(std::vector<UIQuad>& quads, const TextCommandBuffers& text,
                                    GLTextRenderer* text_renderer);

    // Batched-layer scratch VAO/VBO -- separate from m_geom_handle (the
    // single-quad immediate path) since a batch run can be many quads.
    static constexpr int kBatchVertexCapacity = 4096;
    GpuResourceHandle m_batch_geom_handle = kInvalidGpuResource;

    GpuResourceHandle m_slab_tex_handle = kInvalidGpuResource;
    int m_slab_dim = 0; // RT: dimension of the currently-uploaded texture, 0 = none yet
    std::atomic<const unsigned char*> m_slab_pending_data{nullptr}; // GT->RT handoff
    std::atomic<int> m_slab_pending_dim{0};

    void FlushPendingSlabUpload();
};

#endif // RENDERER_OPENGL_GLUIRENDERER_H
