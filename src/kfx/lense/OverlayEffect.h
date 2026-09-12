/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file OverlayEffect.h
 *     Overlay compositing lens effect.
 * @par Purpose:
 *     Overlay graphics compositing effect implementation.
 * @author   Peter Lockett, KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef KFX_OVERLAYEFFECT_H
#define KFX_OVERLAYEFFECT_H

#include "LensEffect.h"
#include <cstdint>

/******************************************************************************/

class OverlayEffect : public LensEffect {
public:
    OverlayEffect();
    virtual ~OverlayEffect();

    virtual TbBool Setup(long lens_idx) override;
    virtual void Cleanup() override;
    virtual TbBool Draw(LensRenderContext* ctx) override;
    virtual TbBool BuildGPUParams(struct IRWorldLensCmd& out, long viewport_w, long viewport_h) override;

private:
    long m_current_lens;
    uint32_t m_gpu_version;   // bumped each Setup() -- lets the GL upload skip unchanged textures
};

/******************************************************************************/
#endif
