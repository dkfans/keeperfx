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
#include "../../front_simple.h"
#include "../renderer/RendererManager.h"

#include "../../keeperfx.hpp"
#include "../../post_inc.h"

/******************************************************************************/

/** Makes @p pal the palette possession and pain fades work from. A running
 *  fade brings it in over its remaining steps, as the original lens code did;
 *  PaletteSetUserPalette() would reset the fade and switch at once. */
static void set_main_palette(unsigned char *pal)
{
    local_state.main_palette = pal;
    if ((local_state.palette_fade_step_possession == 0) && (local_state.palette_fade_step_pain == 0))
        RendererPaletteSet(pal);
}

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
    local_state.lens_palette = cfg->palette;
    set_main_palette(cfg->palette);
    
    m_current_lens = lens_idx;
    SYNCDBG(7, "Palette effect ready");
    return true;
}

void PaletteEffect::Cleanup()
{
    if (m_current_lens >= 0) {
        local_state.lens_palette = NULL;
        set_main_palette(engine_palette);
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
