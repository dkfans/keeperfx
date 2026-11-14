/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_script_commands.h
 *     The level script's commands for Daimon, the keeper AI.
 */
/******************************************************************************/
#ifndef DK_DAIMON_SCRIPT_COMMANDS_H
#define DK_DAIMON_SCRIPT_COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

struct ScriptLine;
struct ScriptContext;

void daimon_player_check(const struct ScriptLine* scline);
void daimon_player_process(struct ScriptContext* context);
void daimon_set_check(const struct ScriptLine* scline);
void daimon_set_process(struct ScriptContext* context);
void daimon_build_check(const struct ScriptLine* scline);
void daimon_build_process(struct ScriptContext* context);
void daimon_attack_check(const struct ScriptLine* scline);
void daimon_attack_process(struct ScriptContext* context);
void daimon_dig_check(const struct ScriptLine* scline);
void daimon_dig_process(struct ScriptContext* context);

#ifdef __cplusplus
}
#endif
#endif
