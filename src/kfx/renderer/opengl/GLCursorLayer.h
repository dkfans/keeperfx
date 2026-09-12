#ifndef RENDERER_OPENGL_GLCURSORLAYER_H
#define RENDERER_OPENGL_GLCURSORLAYER_H

#include "kfx/renderer/ICursorLayer.h"
#include "kfx/renderer/SpriteHandle.h"

class GLUIRenderer;
class GLWorldViewRenderer;


struct GLCursorSnapshot {
    bool has_pointer = false;
    SpriteHandle handle = kInvalidSpriteHandle;
    int32_t x = 0, y = 0;
    int units_per_px = 16;
};

class GLCursorLayer : public ICursorLayer {
public:
    void SetUIRenderer(GLUIRenderer* ui) { m_ui = ui; }
    void SetWorldRenderer(GLWorldViewRenderer* world) { m_world = world; }

    void SubmitPointerSprite(const struct TbSprite* spr, int32_t x, int32_t y, int units_per_px) override;
    int SubmitKeeperHandSprite(short x, short y, unsigned short kspr_base,
                               short angle, unsigned char sprgroup,
                               int32_t scale, TbDrawFlagsMask draw_flags) override;
    void Draw() override;
    void Clear() override;
    const char* GetName() const override { return "GL-Cursor"; }

    GLCursorSnapshot Snapshot() const;
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
