#ifndef RENDERER_OPENGL_GLCURSORLAYER_H
#define RENDERER_OPENGL_GLCURSORLAYER_H

#include "kfx/renderer/ICursorLayer.h"
#include "kfx/renderer/SpriteHandle.h"

class GLUIRenderer;
class GLWorldViewRenderer;

// A value copy of the cursor's stashed state, taken on the game thread at
// frame handoff (RendererOpenGL::PresentFrame) and consumed on the render
// thread (P5.6) -- Draw()'s old shape of reading m_pointer_* live would be a
// data race once submit (game thread) and draw (render thread) run
// concurrently, the same class of bug the Phase 4 cursor fixes were about,
// now across threads instead of within one frame.
struct GLCursorSnapshot {
    bool has_pointer = false;
    SpriteHandle handle = kInvalidSpriteHandle;
    int32_t x = 0, y = 0;
    int units_per_px = 16;

    // Power-hand keeper sprite(s) (P5.7.5 / Beat 3) are NOT part of this
    // snapshot -- SubmitKeeperHandSprite() now submits straight into
    // GLWorldViewRenderer's own cursor IR (m_cursor_kspr_ir, via
    // BeginCursorCapture()/SubmitKeeperSprite()/EndCursorCapture()), which
    // has its own game/render-thread double-buffer (FlipBuffers()) and is
    // drawn by GLWorldViewRenderer::DrawCursorKeeperSprites(), called
    // directly from RendererOpenGL::FGExecuteCursor(). This preserves every
    // sprite submitted in a frame instead of one scalar slot being
    // overwritten by the next submission (the held-thing-never-renders bug).
};

// Stashes the submitted OS pointer sprite, drawing it at end-of-frame.
// Unlike software's query-live design, GL genuinely defers (submit happens
// mid-frame, draw happens at present time), so a stash is the right shape
// here -- Clear() is wired through LbI_PointerHandler::Release() the same
// way it is for software, which is what actually fixed the Phase 4 cursor
// bugs (not the query-live change itself, which was software-specific).
class GLCursorLayer : public ICursorLayer {
public:
    void SetUIRenderer(GLUIRenderer* ui) { m_ui = ui; }
    // Owned externally (RendererOpenGL::Impl); must outlive this. Used to
    // bracket SubmitKeeperHandSprite()'s geometry resolution with
    // BeginCursorCapture()/EndCursorCapture() -- the power-hand sprite(s)
    // share GLWorldViewRenderer's keeper-sprite atlas/shaders/IR rather than
    // owning a second copy of that machinery (P5.7.5).
    void SetWorldRenderer(GLWorldViewRenderer* world) { m_world = world; }

    void SubmitPointerSprite(const struct TbSprite* spr, int32_t x, int32_t y, int units_per_px) override;
    int SubmitKeeperHandSprite(short x, short y, unsigned short kspr_base,
                               short angle, unsigned char sprgroup,
                               int32_t scale, TbDrawFlagsMask draw_flags) override;
    void Draw() override;
    void Clear() override;
    const char* GetName() const override { return "GL-Cursor"; }

    // Game thread: read the current stashed state by value. Safe to call any
    // time nothing else is concurrently calling Submit*/Clear() on this
    // object -- true at RendererOpenGL::PresentFrame()'s handoff point,
    // since Submit*/Clear() only ever run earlier in the same frame.
    GLCursorSnapshot Snapshot() const;

    // Render thread: draw from a snapshot taken earlier, never touching live
    // m_pointer_*/m_hand_* state.
    void DrawSnapshot(const GLCursorSnapshot& s);

private:
    GLUIRenderer* m_ui = nullptr;
    GLWorldViewRenderer* m_world = nullptr;
    bool m_has_pointer = false;
    SpriteHandle m_pointer_handle = kInvalidSpriteHandle;
    int32_t m_pointer_x = 0, m_pointer_y = 0;
    int m_pointer_units_per_px = 16;
};

#endif // RENDERER_OPENGL_GLCURSORLAYER_H
