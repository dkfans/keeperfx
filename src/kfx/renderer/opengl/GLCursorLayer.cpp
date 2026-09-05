#include "pre_inc.h"
#include "kfx/renderer/opengl/GLCursorLayer.h"
#include "kfx/renderer/opengl/GLUIRenderer.h"
#include "kfx/renderer/opengl/GLWorldViewRenderer.h"
#include "bflib_sprite.h"
#include "engine_render.h"  // resolve_keepersprite_cursor_geometry() (P5.7.5)
#include "post_inc.h"

void GLCursorLayer::SubmitPointerSprite(const struct TbSprite* spr, int32_t x, int32_t y, int units_per_px)
{
    if (!m_ui || !spr) { m_has_pointer = false; return; }
    m_pointer_handle = m_ui->ResolveSprite(spr);
    m_pointer_x = x;
    m_pointer_y = y;
    m_pointer_units_per_px = units_per_px;
    m_has_pointer = (m_pointer_handle != kInvalidSpriteHandle);
}

int GLCursorLayer::SubmitKeeperHandSprite(short x, short y, unsigned short kspr_base,
                                          short angle, unsigned char sprgroup,
                                          int32_t scale, TbDrawFlagsMask draw_flags)
{
    // Game thread only -- resolves everything (draw_idx/data/dims/dst rect)
    // up front via resolve_keepersprite_cursor_geometry() rather than calling
    // process_keeper_sprite()/its "_ex" develop equivalent directly: that
    // function's water-cutoff logic and set_thing_pointed_at() side effect
    // both read thing_being_displayed, which for a cursor sprite is stale
    // leftover state from whatever world creature was drawn last (see that
    // function's own doc comment) -- geometry-only resolution avoids both.
    int32_t draw_idx = -1;
    const unsigned char* data = nullptr;
    int src_w = 0, src_h = 0;
    int32_t dst_x = 0, dst_y = 0, dst_w = 0, dst_h = 0;
    if (!resolve_keepersprite_cursor_geometry(x, y, kspr_base, angle, sprgroup, (long)scale,
            &dst_x, &dst_y, &dst_w, &dst_h, &draw_idx, &data, &src_w, &src_h))
    {
        return 1;  // resolved to nothing drawable (e.g. invalid sprite id) --
                   // claim it anyway so the caller doesn't also draw garbage
                   // via process_keeper_sprite() with the same bad id.
    }
    if (!m_world)
        return 1;

    // Submit through GLWorldViewRenderer's cursor IR capture bracket (ported
    // from develop's BeginCursorCapture()/process_keeper_sprite_ex()/
    // EndCursorCapture() shape) instead of overwriting a single scalar slot
    // -- power_hand.c calls this up to twice per frame (held thing, then the
    // hand graphic itself) and both must survive to draw, not just the last.
    m_world->BeginCursorCapture();
    // content_h == src_h: cursor content is UI, never water/lava-clipped
    // (Beat 4's content_h param is a no-op here by construction).
    m_world->SubmitKeeperSprite(dst_x, dst_y, dst_w, dst_h, data, src_w, src_h, src_h,
                                (unsigned int)draw_flags, nullptr, draw_idx);
    m_world->EndCursorCapture();
    return 1;
}

void GLCursorLayer::Draw()
{
    // Nothing calls this directly once threaded (P5.6) -- RendererOpenGL
    // snapshots at handoff and calls DrawSnapshot() on the render thread
    // instead. Implemented in terms of the same snapshot for interface
    // consistency (ICursorLayer::Draw() is pure virtual).
    DrawSnapshot(Snapshot());
}

void GLCursorLayer::Clear()
{
    m_has_pointer = false;
    m_pointer_handle = kInvalidSpriteHandle;
}

GLCursorSnapshot GLCursorLayer::Snapshot() const
{
    GLCursorSnapshot s;
    s.has_pointer = m_has_pointer;
    s.handle = m_pointer_handle;
    s.x = m_pointer_x;
    s.y = m_pointer_y;
    s.units_per_px = m_pointer_units_per_px;
    return s;
}

void GLCursorLayer::DrawSnapshot(const GLCursorSnapshot& s)
{
    if (s.has_pointer && m_ui)
        m_ui->DrawGlyphQuad(s.handle, (float)s.x, (float)s.y,
                            s.units_per_px, 1.0f, 1.0f, 1.0f, 1.0f);

    // Power-hand keeper sprite(s): drawn separately, via
    // GLWorldViewRenderer::DrawCursorKeeperSprites(), called directly from
    // RendererOpenGL::FGExecuteCursor() right after this -- see
    // SubmitKeeperHandSprite()'s doc comment for why they no longer flow
    // through this snapshot.
}
