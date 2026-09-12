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

// GL UI renderer: Submit* (inherited unchanged from IUIRenderer) append into
// whatever UICommandBuffers RendererOpenGL keeps bound all frame (GL always
// defers -- there is no immediate GL draw path).
//
// Beat 5: draws are now split by IRUILayer (WorldOverlay/WorldOverlayFlat/
// GameUI, see UICommands.h) into a generic UIQuad + run-length batching
// pipeline (ported from develop's UIQuad/flush_quads_from() shape, adapted
// to this branch's single-atlas reality -- no shared mode-branching
// fragment shader exists in develop either; `mode` is a CPU-side batching
// key only, each pass still binds one of the existing shaders below).
// BuildQuadsFromIR() classifies the frame's IR into 3 per-layer quad
// vectors; DrawWorldSpriteLayerRT()/DrawWorldOverlayFlatLayerRT()/
// DrawGameUILayerRT() flush one layer each, called from RendererOpenGL's
// FGDrawWorldSpriteLayer()/FGDrawWorldOverlayFlatLayer()/FGDrawGameUI() at
// the frame-graph's existing (already correctly ordered) call sites.
// DrawFromIR() stays as a self-contained "build + flush all 3 layers right
// now" convenience, unchanged in its external contract, for buffers that
// aren't the main per-frame ui_cmds (namely FGFlushSwipeOverlay()'s
// fd.swipe_cmds, which never sets ambient world-overlay state, so it always
// resolves to the GameUI layer -- functionally identical to before).
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

    // Packs into the atlas as a side effect, so every handle IUIRenderer
    // hands out is guaranteed drawable.
    SpriteHandle ResolveSprite(const struct TbSprite* spr) override;

    /** Self-contained "classify + batch-draw all layers now" entry point for
     *  a standalone IR buffer (currently only fd.swipe_cmds). Merges ui+text
     *  by their shared seq (same interleaving IUIRenderer's software replay
     *  does). Not used for the main per-frame ui_cmds -- see
     *  BuildQuadsFromIR()/DrawWorldSpriteLayerRT()/DrawGameUILayerRT() etc.
     *  for why that needs splitting across the frame instead of one call. */
    void DrawFromIR(const UICommandBuffers& ui, const TextCommandBuffers& text,
                    GLTextRenderer* text_renderer);

    /** Classify this frame's ui_cmds into the 3 per-layer UIQuad vectors
     *  (m_quads[]), in original cross-kind submission order. Pure CPU
     *  classification, no GL calls -- called once per frame (from
     *  RendererOpenGL::FGClearFrame(), before any of the 3 Draw*LayerRT()
     *  calls that consume it later in the same frame). */
    void BuildQuadsFromIR(const UICommandBuffers& ui);

    /** Flush the WorldOverlay layer (depth-tested against world geometry --
     *  creature status). Call after BuildQuadsFromIR(), from
     *  RendererOpenGL::FGDrawWorldSpriteLayer(). */
    void DrawWorldSpriteLayerRT();

    /** Flush the WorldOverlayFlat layer (no depth test -- room flags,
     *  floating gold/damage text). Call from
     *  RendererOpenGL::FGDrawWorldOverlayFlatLayer(). */
    void DrawWorldOverlayFlatLayerRT();

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

    // Used by GLTextRenderer (text glyphs) and GLCursorLayer (OS pointer
    // sprite) so both share this renderer's shader/quad path.
    // sample_palette selects which shader: true sends the sprite's own
    // indexed pixels through the palette texture and multiplies by
    // r,g,b,a. Correct for the cursor (always, real multi-colour
    // palette-indexed image, white/1,1,1,1 tint = no-op) and for the
    // *default* text case (r,g,b,a = 1,1,1,1 -- glyph shown in its own
    // baked-in palette colour, matching LbSpriteDrawResizedImmediate()).
    // false uses the sprite purely as an ink/no-ink mask and outputs
    // r,g,b,a flat, discarding the glyph's own palette colour entirely --
    // correct only for GLTextRenderer's Lb_TEXT_ONE_COLOR case
    // (LbSpriteDrawResizedOneColourImmediate() -- an explicit, opt-in solid-
    // colour text mode, not how DK's fonts normally render). See
    // GLTextRenderer::DrawGlyphs()'s own comment for the full mode
    // breakdown and GLShaders.h's UI_SPRITE_COLORED_FRAGMENT_SHADER comment
    // for the shader itself.
    void DrawGlyphQuad(SpriteHandle glyph, float x, float y, int units_per_px,
                       float r, float g, float b, float a, bool sample_palette = true);

    /** Resolve (allocating + packing on first use) a DBC/CJK glyph bitmap
     *  into the same shelf atlas text/UI sprites already share, so it can be
     *  drawn via DrawGlyphQuad(..., sample_palette=false) exactly like the
     *  Lb_TEXT_ONE_COLOR western-glyph path. Keyed by (font, codepoint), not
     *  a TbSprite* -- DBC glyphs are raw 1bpp bitmaps carved out of the DBC
     *  font's static blob (bflib_sprfnt.c's LbDbcGetGlyphBits()), so this
     *  keeps its own handle space (starting at kDbcHandleBase) independent
     *  of IUIRenderer's TbSprite*-keyed one; both spaces share the one
     *  GLSpriteAtlas underneath without colliding. Returns
     *  kInvalidSpriteHandle if the codepoint has no glyph. */
    SpriteHandle ResolveDbcGlyph(const struct AsianFont* font, uint32_t codepoint);

    /** Flat-coloured rect, immediate draw -- used by GLTextRenderer for
     *  underline geometry (AppendUnderlineRects()). Thin public wrapper
     *  around the private single-quad draw_solid_quad() path already used
     *  internally (UI boxes go through the batched UIQuad system instead;
     *  text underline draws once per segment, immediate, matching how glyph
     *  quads themselves are drawn). */
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

    // ── Beat 6: DBC/CJK glyph atlas ─────────────────────────────────────────
    // Own handle space (see ResolveDbcGlyph()'s comment) so it can't collide
    // with IUIRenderer's TbSprite*-keyed handles sharing the same atlas.
    static constexpr SpriteHandle kDbcHandleBase = 0x80000000u;
    std::unordered_map<uint64_t, SpriteHandle> m_dbc_glyph_handles;
    SpriteHandle m_next_dbc_handle = kDbcHandleBase;

    // ── Beat 5: UIQuad batching ────────────────────────────────────────────

    /** One quad, already fully resolved (screen-space corners, UV, colour,
     *  depth) -- the common currency every IR command kind converts to.
     *  `mode` is a CPU-side classification key only (see classify()); there
     *  is no shared mode-branching fragment shader, each PassType binds one
     *  of the existing shaders above. `remap_row` is only meaningful for
     *  PASS_REMAP. `seq` is carried through so DrawGameUILayerRT() can
     *  interleave with text by original submission order. */
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

    // WorldOverlay=0, WorldOverlayFlat=1, GameUI=2 (matches IRUILayer).
    static constexpr int kLayerCount = 3;
    std::vector<UIQuad> m_quads[kLayerCount]; // RT: per-frame scratch, built by BuildQuadsFromIR()

    /** Shared by BuildQuadsFromIR() (the real per-frame path) and
     *  DrawFromIR() (the standalone-buffer convenience) -- walks every IR
     *  command kind in cross-kind seq order, converts each to 1 UIQuad (2
     *  for slab backgrounds: an opaque backing quad + the tiled quad), and
     *  appends into out[cmd.layer]. Pure classification, no GL calls. */
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

    // ── Beat 5: slab background tiling ──────────────────────────────────────
    GpuResourceHandle m_slab_tex_handle = kInvalidGpuResource;
    int m_slab_dim = 0; // RT: dimension of the currently-uploaded texture, 0 = none yet
    std::atomic<const unsigned char*> m_slab_pending_data{nullptr}; // GT->RT handoff
    std::atomic<int> m_slab_pending_dim{0};

    /** Render thread: upload a pending slab buffer if UpdateSlabTexture() set
     *  one since the last check. Cheap no-op when nothing pending. Called
     *  lazily, the first time a slab quad is actually about to be flushed
     *  (mirrors develop's FlushPendingInit(), called once per frame). */
    void FlushPendingSlabUpload();
};

#endif // RENDERER_OPENGL_GLUIRENDERER_H
