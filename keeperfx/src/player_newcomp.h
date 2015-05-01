#ifndef PLAYER_NEWCOMP_H
#define PLAYER_NEWCOMP_H

#include "bflib_basics.h"

#include "dungeon_data.h"
#include "player_computer.h"

#ifdef __cplusplus
extern "C" {
#endif

//eval
TbBool is_digging_any_gems(struct Dungeon *dungeon);
struct Thing * find_imp_for_sacrifice(struct Dungeon* dungeon);
struct Thing * find_imp_for_claim(struct Dungeon* dungeon);
float calc_players_strength(struct Dungeon* dungeon);

//checks
long computer_check_for_claims(struct Computer2 *comp);

#ifdef __cplusplus
}
#endif

#endif //PLAYER_NEWCOMP_H
