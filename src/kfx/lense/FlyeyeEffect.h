/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file FlyeyeEffect.h
 *     Flyeye/compound eye lens effect.
 * @par Purpose:
 *     Hexagonal tiling effect for compound eye view.
 * @par Comment:
 *     Resolution-independent implementation using pre-computed lookup table.
 * @author   Tomasz Lis, KeeperFX Team
 * @date     11 Mar 2010 - 09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef KFX_FLYEYEEFFECT_H
#define KFX_FLYEYEEFFECT_H

#include "LensEffect.h"

/******************************************************************************/

/**
 * Pre-computed flyeye lookup table entry.
 */
struct FlyeyeLookupEntry {
    short src_x;
    short src_y;
};

class FlyeyeEffect : public LensEffect {
public:
    FlyeyeEffect();
    virtual ~FlyeyeEffect();
    
    virtual TbBool Setup(long lens_idx) override;
    virtual void Cleanup() override;
    virtual TbBool Draw(LensRenderContext* ctx) override;
    
private:
    void BuildLookupTable(long width, long height);
    void FreeLookupTable();
    
    long m_current_lens;
    
    // Pre-computed lookup table
    FlyeyeLookupEntry* m_lookup_table;
    long m_table_width;
    long m_table_height;
};

/******************************************************************************/
#endif
