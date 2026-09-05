/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file MistEffect.h
 *     Mist/fog lens effect.
 * @par Purpose:
 *     Fog overlay effect implementation.
 * @par Comment:
 *     Wraps existing CMistFade functionality.
 * @author   KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef KFX_MISTEFFECT_H
#define KFX_MISTEFFECT_H

#include "LensEffect.h"
#include <cstdint>

/******************************************************************************/

class MistEffect : public LensEffect {
public:
    MistEffect();
    virtual ~MistEffect();
    
    virtual TbBool Setup(long lens_idx) override;
    virtual void Cleanup() override;
    virtual TbBool Draw(LensRenderContext* ctx) override;
    virtual TbBool BuildGPUParams(struct IRWorldLensCmd& out, long viewport_w, long viewport_h) override;

private:
    TbBool LoadMistTexture(const char* filename);

    long m_current_lens;
    uint32_t m_gpu_version;   // bumped each Setup() (P5.8a) -- lets the GL upload skip unchanged textures
};

/******************************************************************************/
#endif
