/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_level.c
 *     A level's DAIMON key, from its campaign [mapN] block or .lof (daimon_level.h): the fork's parser and level-start queue, as Daimon's port tools copy them (scripts/daimon_port/sync.py in the fork).
 */
/******************************************************************************/
#include "pre_inc.h"
#include "daimon_level.h"
#include <ctype.h>
#include <string.h>
#include "globals.h"
#include "bflib_basics.h"
#include "config_campaigns.h"
#include "config_daimon.h"
#include "player_data.h"
#include "player_computer.h"
#include "game_merge.h"
#include "daimon/Daimon.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

int64_t daimon_level_key_parse(const char *text, struct DaimonLevelKeyEntry *out, int64_t max)
{
    int64_t n = 0;
    const char *at = (text != NULL) ? text : "";
    while ((*at != '\0') && (n < max))
    {
        while ((*at == ' ') || (*at == '\t') || (*at == ',')) at++;
        if (*at == '\0') break;
        char tok[128];
        size_t len = 0;
        while ((*at != '\0') && (*at != ' ') && (*at != '\t') && (*at != ',') && (len + 1 < sizeof(tok))) tok[len++] = *at++;
        tok[len] = '\0';
        while ((*at != '\0') && (*at != ' ') && (*at != '\t') && (*at != ',')) at++;    // the rest of a too-long one
        // WHO[:personality[:difficulty]]
        char *who = tok, *pers = strchr(tok, ':'), *diff = NULL;
        if (pers != NULL) { *pers++ = '\0'; diff = strchr(pers, ':'); if (diff != NULL) *diff++ = '\0'; }
        struct DaimonLevelKeyEntry e;
        memset(&e, 0, sizeof(e));
        if (strcasecmp(who, "ALL") == 0) e.player = DAIMON_KEY_ALL;
        else if (strcasecmp(who, "MODELS") == 0) e.player = DAIMON_KEY_MODELS;
        else if ((strncasecmp(who, "PLAYER", 6) == 0) && (who[6] >= '0') && (who[6] <= '9') && (who[7] == '\0'))
            e.player = who[6] - '0';
        else {
            WARNMSG("DAIMON: '%s' is not ALL, MODELS or PLAYERn; skipped", tok);
            continue;
        }
        if (pers != NULL) for (size_t i = 0; (pers[i] != '\0') && (i + 1 < sizeof(e.personality)); i++) e.personality[i] = (char)tolower((unsigned char)pers[i]);
        if (diff != NULL) snprintf(e.difficulty, sizeof(e.difficulty), "%s", diff);
        out[n++] = e;
    }
    return n;
}

static void queue_level_daimon_key(void)
{
    const struct LevelInformation *lvinfo = get_level_info(get_loaded_level_number());
    if ((lvinfo == NULL) || (lvinfo->daimon[0] == '\0'))
        return;
    struct DaimonLevelKeyEntry e[DAIMON_KEY_MAX_ENTRIES];
    const int64_t n = daimon_level_key_parse(lvinfo->daimon, e, DAIMON_KEY_MAX_ENTRIES);
    for (int64_t i = 0; i < n; i++)
    {
        for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++)
        {
            if ((e[i].player >= 0) && (e[i].player != p))
                continue;
            struct PlayerInfo *player = get_player(p);
            const TbBool ai = player_exists(player) && (p != my_player_number) && flag_is_set(player->allocflags, PlaF_CompCtrl);
            if (!ai)
            {
                if (e[i].player >= 0)
                    WARNLOG("DAIMON key: player %d is not the built-in AI's at the level start; left as it is", (int)p);
                continue;
            }
            const char *personality = e[i].personality;
            if (e[i].player == DAIMON_KEY_MODELS)
            {
                const struct Computer2 *comp = get_computer_player(p);
                const char *mapped = computer_player_invalid(comp) ? NULL : daimon_config_model_personality(comp->model);
                personality = (mapped != NULL) ? mapped : "";
            }
            JUSTMSG("DAIMON key: player %d to Daimon (%s, %s)", (int)p, (personality[0] != '\0') ? personality : "default",
                (e[i].difficulty[0] != '\0') ? e[i].difficulty : "normal");
            DaimonQueueLevelStart(p, personality, e[i].difficulty);
        }
    }
}

void daimon_queue_level_key(void)
{
    queue_level_daimon_key();
}

#ifdef __cplusplus
}
#endif
