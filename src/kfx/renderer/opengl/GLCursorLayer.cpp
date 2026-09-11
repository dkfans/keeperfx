#include "pre_inc.h"
#include "kfx/renderer/opengl/GLCursorLayer.h"
#include "kfx/renderer/opengl/GLUIRenderer.h"
#include "kfx/renderer/opengl/GLWorldViewRenderer.h"
#include "bflib_sprite.h"
#include "engine_render.h"  // resolve_keepersprite_cursor_geometry() (P5.7.5)
#include "post_inc.h"

void GLCursorLayer::SubmitPointerSprite(const struct TbSprite* spr, int32_t x, int32_t y, int units_per_px)
{

    // reject unpackable sprite.
    if (!m_ui || !spr || spr->SWidth == 0 || spr->SHeight == 0) { m_has_pointer = false; return; }
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
    int32_t draw_idx = -1;
    const unsigned char* data = nullptr;
    int src_w = 0, src_h = 0;
    int32_t dst_x = 0, dst_y = 0, dst_w = 0, dst_h = 0;
    if (!resolve_keepersprite_cursor_geometry(x, y, kspr_base, angle, sprgroup, (long)scale,
            &dst_x, &dst_y, &dst_w, &dst_h, &draw_idx, &data, &src_w, &src_h))
    {
        return 1;  // OK, Nothing drawable.
    }
    if (!m_world)
        return 1; //OK, Nothing drawable.

    m_world->BeginCursorCapture();
    m_world->SubmitKeeperSprite(dst_x, dst_y, dst_w, dst_h, data, src_w, src_h, src_h,
                                (unsigned int)draw_flags, nullptr, draw_idx);
    m_world->EndCursorCapture();

    return 1;
}

void GLCursorLayer::Draw()
{
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
}
