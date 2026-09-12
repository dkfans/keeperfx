#ifndef RENDERER_BACKENDS_SOFTWARECURSORLAYER_H
#define RENDERER_BACKENDS_SOFTWARECURSORLAYER_H

#include "kfx/renderer/ICursorLayer.h"

class SoftwareCursorLayer final : public ICursorLayer {
public:
    void SubmitPointerSprite(const struct TbSprite* spr,
                             int32_t x, int32_t y,
                             int units_per_px) override;

    int SubmitKeeperHandSprite(short x, short y,
                               unsigned short kspr_base,
                               short angle,
                               unsigned char sprgroup,
                               int32_t scale,
                               TbDrawFlagsMask draw_flags) override;

    void Draw() override;
    void Clear() override;
    const char* GetName() const override { return "SOFTWARE_CURSOR"; }
};

#endif // RENDERER_BACKENDS_SOFTWARECURSORLAYER_H
