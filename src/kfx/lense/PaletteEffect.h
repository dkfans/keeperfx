/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file PaletteEffect.h
 *     Palette color change lens effect.
 * @par Purpose:
 *     Color palette modification effect implementation.
 * @par Comment:
 *     Changes the rendering palette for color-shifted views.
 * @author   Peter Lockett, KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef KFX_PALETTEEFFECT_H
#define KFX_PALETTEEFFECT_H

#include "LensEffect.h"

/******************************************************************************/

class PaletteEffect : public LensEffect {
public:
    PaletteEffect();
    virtual ~PaletteEffect();
    
    virtual TbBool Setup(long lens_idx) override;
    virtual void Cleanup() override;
    virtual TbBool Draw(LensRenderContext* ctx) override;
    
private:
    long m_current_lens;
};

/******************************************************************************/
#endif 