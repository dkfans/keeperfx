/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file GameUI.h
 *     Owner/orchestrator of the composed in-game UI. 
 * @par Purpose:
 *     Single home for everything that is in-game 2D UI.
 */
/******************************************************************************/
#ifndef KFX_GAMEUI_H
#define KFX_GAMEUI_H

#include "bflib_basics.h"

struct PlayerInfo;

#ifdef __cplusplus

class GameUI {
public:
    static GameUI& Get();

    void DrawFrame(struct PlayerInfo* player);

    bool IsActiveForCurrentView(const struct PlayerInfo* player) const;

private:
    GameUI() = default;
    GameUI(const GameUI&) = delete;
    GameUI& operator=(const GameUI&) = delete;
};

#endif // __cplusplus

/* C-callable shim for engine_redraw.c. */
#ifdef __cplusplus
extern "C" {
#endif

void GameUI_DrawFrame(struct PlayerInfo* player);

#ifdef __cplusplus
}
#endif

/******************************************************************************/
#endif
