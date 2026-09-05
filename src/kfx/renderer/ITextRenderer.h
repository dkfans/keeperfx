/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file ITextRenderer.h
 *     The text drawing a backend has to provide.
 */
/******************************************************************************/
#pragma once

#include "bflib_basics.h"
#include <cstdint>

struct TextCommandBuffers;
struct IRTextDrawCmd;

/******************************************************************************/

class ITextRenderer {
public:
    virtual ~ITextRenderer() = default;

    /** Draw text laid out in the current text window, scaled by units_per_px. */
    virtual TbBool DrawTextResized(int32_t x, int32_t y, int32_t units_per_px, const char* text);

    /** Open the IR write window for this frame; nullptr closes it. */
    virtual void SetTextCommandBuffers(TextCommandBuffers* cmds);

    /** Draw one already-captured command (called from IUIRenderer's merged replay). */
    virtual void ReplayTextCommand(const IRTextDrawCmd& cmd);

    virtual const char* GetName() const { return "TEXT"; }

protected:
    TextCommandBuffers* m_text_write_cmds = nullptr;
};

/******************************************************************************/
