/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file PaletteEffect.cpp
 *     Palette color change lens effect implementation.
 * @par Purpose:
 *     Color palette modification effect.
 * @par Comment:
 *     None.
 * @author   Peter Lockett, KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "../../pre_inc.h"
#include "PaletteEffect.h"

#include "../../config_lenses.h"
#include "../../player_data.h"
#include "../../game_legacy.h"

#include "../../keeperfx.hpp"
#include "../../post_inc.h"

/******************************************************************************/

PaletteEffect::PaletteEffect()
    : LensEffect(LensEffectType::Palette, "Palette")
    , m_current_lens(-1)
{
}

PaletteEffect::~PaletteEffect()
{
    Cleanup();
}

TbBool PaletteEffect::Setup(long lens_idx)
{
    SYNCDBG(8, "Setting up palette effect for lens %ld", lens_idx);
    
    struct LensConfig* cfg = &lenses_conf.lenses[lens_idx];
    struct PlayerInfo* player = get_my_player();
    
    // Set lens_palette - PaletteSetPlayerPalette() will update main_palette and apply
    // Do NOT set main_palette here, it breaks the condition in PaletteSetPlayerPalette()
    player->lens_palette = cfg->palette;
    
    m_current_lens = lens_idx;
    SYNCDBG(7, "Palette effect ready");
    return true;
}

void PaletteEffect::Cleanup()
{
    if (m_current_lens >= 0) {
        struct PlayerInfo* player = get_my_player();
        player->lens_palette = NULL;
        player->main_palette = engine_palette;
        m_current_lens = -1;
        SYNCDBG(9, "Palette effect cleaned up");
    }
}

TbBool PaletteEffect::Draw(LensRenderContext* ctx)
{
    // Palette effects don't directly modify buffers - they change global state
    // Return false to indicate we didn't write to dstbuf
    return false;
}

/******************************************************************************/
