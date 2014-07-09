/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_process.c
 *     Keeper powers process functions.
 * @par Purpose:
 *     Functions to check availability and use keeper powers.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "power_process.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_math.h"
#include "bflib_memory.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "player_utils.h"
#include "thing_shots.h"
#include "thing_objects.h"
#include "creature_states.h"
#include "config_terrain.h"
#include "front_simple.h"
#include "slab_data.h"
#include "game_legacy.h"
#include "power_hand.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned char _DK_backup_explored[26][26];
#define backup_explored _DK_backup_explored
/******************************************************************************/
DLLIMPORT void _DK_process_armageddon(void);
DLLIMPORT void _DK_process_disease(struct Thing *thing);
DLLIMPORT void _DK_lightning_modify_palette(struct Thing *thing);
DLLIMPORT void _DK_update_god_lightning_ball(struct Thing *thing);
DLLIMPORT void _DK_god_lightning_choose_next_creature(struct Thing *thing);
DLLIMPORT void _DK_draw_god_lightning(struct Thing *thing);
DLLIMPORT void _DK_turn_off_call_to_arms(long a);
DLLIMPORT void _DK_remove_explored_flags_for_power_sight(struct PlayerInfo *player);
DLLIMPORT void _DK_update_explored_flags_for_power_sight(struct PlayerInfo *player);
DLLIMPORT int _DK_can_thing_be_possessed(struct Thing *thing, long a2);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Sets keeper power selected by local human player.
 *
 * @param pwkind Power to select.
 * @param sptooltip Tooltip string index.
 * @note Was set_chosen_spell()
 */
void set_chosen_power(PowerKind pwkind, TextStringId sptooltip)
{
    struct SpellData *pwrdata;
    pwrdata = get_power_data(pwkind);
    if (power_data_is_invalid(pwrdata))
      pwkind = 0;
    SYNCDBG(6,"Setting to %ld",pwkind);
    game.chosen_spell_type = pwkind;
    game.chosen_spell_look = pwrdata->field_9;
    game.chosen_spell_tooltip = sptooltip;
}

void set_chosen_power_none(void)
{
    SYNCDBG(6,"Setting to %d",0);
    game.chosen_spell_type = 0;
    game.chosen_spell_look = 0;
    game.chosen_spell_tooltip = 0;
}

unsigned char general_expand_check(void)
{
    struct PlayerInfo *player;
    player = get_my_player();
    return (player->field_4D2 != 0);
}

unsigned char sight_of_evil_expand_check(void)
{
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    return (myplyr->field_4D2 != 0) && (!player_uses_power_sight(myplyr->id_number));
}

unsigned char call_to_arms_expand_check(void)
{
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    return (myplyr->field_4D2 != 0) && (!player_uses_call_to_arms(myplyr->id_number));
}

TbBool can_thing_be_possessed(const struct Thing *thing, PlayerNumber plyr_idx)
{
    //return _DK_can_thing_be_possessed(thing, plyr_idx);
    if (thing->owner != plyr_idx)
        return false;
    if (thing_is_creature(thing))
    {
        if (thing_is_picked_up(thing))  {
            return false;
        }
        if ((thing->active_state == CrSt_CreatureUnconscious)
          || creature_affected_by_spell(thing, SplK_Teleport))  {
            return false;
        }
        if (creature_is_being_sacrificed(thing) || creature_is_being_summoned(thing))  {
            return false;
        }
        if (creature_is_kept_in_custody_by_enemy(thing))  {
            return false;
        }
        return true;
    }
    if (thing_is_object(thing))
    {
        if (object_is_mature_food(thing))  {
            return true;
        }
        return false;
    }
    return false;
}

void process_armageddon(void)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *heartng;
    long i;
    SYNCDBG(6,"Starting");
    //_DK_process_armageddon(); return;
    if (game.armageddon_cast_turn == 0)
        return;
    if (game.armageddon.count_down+game.armageddon_cast_turn > game.play_gameturn)
    {
        if (player_cannot_win(game.armageddon_caster_idx))
        {
            // Stop the armageddon if its originator is just losing
            game.armageddon_cast_turn = 0;
        }
    } else
    if (game.armageddon.count_down+game.armageddon_cast_turn == game.play_gameturn)
    {
        for (i=0; i < PLAYERS_COUNT; i++)
        {
            player = get_player(i);
            if (player_exists(player))
            {
              if (player->field_2C == 1)
                reveal_whole_map(player);
            }
        }
    } else
    if (game.armageddon.count_down+game.armageddon_cast_turn < game.play_gameturn)
    {
        for (i=0; i < PLAYERS_COUNT; i++)
        {
            player = get_player(i);
            if ( (player_exists(player)) && (player->field_2C == 1) )
            {
                dungeon = get_dungeon(player->id_number);
                if ((player->victory_state == VicS_Undecided) && (dungeon->num_active_creatrs == 0))
                {
                    event_kill_all_players_events(i);
                    set_player_as_lost_level(player);
                    if (is_my_player_number(i))
                        LbPaletteSet(engine_palette);
                    heartng = get_player_soul_container(player->id_number);
                    if (thing_exists(heartng)) {
                        heartng->health = -1;
                    }
                }
            }
        }
    }
}

void process_disease(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    _DK_process_disease(thing);
}

void lightning_modify_palette(struct Thing *thing)
{
    struct PlayerInfo *myplyr;
    // _DK_lightning_modify_palette(thing);
    myplyr = get_my_player();

    if (thing->health == 0)
    {
      PaletteSetPlayerPalette(myplyr, engine_palette);
      myplyr->field_3 &= ~0x08;
      return;
    }
    if (myplyr->acamera == NULL)
    {
        ERRORLOG("No active camera");
        return;
    }
    if (((thing->health % 8) != 7) && (thing->health != 1) && (ACTION_RANDOM(4) != 0))
    {
        if ((myplyr->field_3 & 0x08) != 0)
        {
            if (get_2d_box_distance(&myplyr->acamera->mappos, &thing->mappos) < 11520)
            {
                PaletteSetPlayerPalette(myplyr, engine_palette);
                myplyr->field_3 &= ~0x08;
            }
        }
        return;
    }
    if ((myplyr->view_mode != PVM_ParchFadeIn) && (myplyr->view_mode != PVM_ParchFadeOut) && (myplyr->view_mode != PVM_ParchmentView))
    {
        if ((myplyr->field_3 & 0x08) == 0)
        {
            if (get_2d_box_distance(&myplyr->acamera->mappos, &thing->mappos) < 11520)
            {
              PaletteSetPlayerPalette(myplyr, lightning_palette);
              myplyr->field_3 |= 0x08;
            }
        }
    }
}

void update_god_lightning_ball(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *target;
    struct ShotConfigStats *shotst;
    long i;
//    _DK_update_god_lightning_ball(thing);
    if (thing->health <= 0)
    {
        lightning_modify_palette(thing);
        return;
    }
    i = (game.play_gameturn - thing->creation_turn) % 16;
    switch (i)
    {
    case 0:
        god_lightning_choose_next_creature(thing);
        break;
    case 1:
        target = thing_get(thing->shot.target_idx);
        if (thing_is_invalid(target))
            break;
        draw_lightning(&thing->mappos,&target->mappos, 96, 60);
        break;
    case 2:
        target = thing_get(thing->shot.target_idx);
        if (thing_is_invalid(target))
            break;
        shotst = get_shot_model_stats(24);
        apply_damage_to_thing_and_display_health(target, shotst->old->damage, shotst->damage_type, thing->owner);
        if (target->health < 0)
        {
            cctrl = creature_control_get_from_thing(target);
            cctrl->shot_model = ShM_GodLightBall;
            kill_creature(target, INVALID_THING, thing->owner, CrDed_DiedInBattle);
        }
        thing->shot.target_idx = 0;
        break;
    }
}

void god_lightning_choose_next_creature(struct Thing *thing)
{
    _DK_god_lightning_choose_next_creature(thing);
}

void draw_god_lightning(struct Thing *thing)
{
    _DK_draw_god_lightning(thing);
}

TbBool player_uses_call_to_arms(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->cta_start_turn != 0);
}

void turn_off_call_to_arms(PlayerNumber plyr_idx)
{
  _DK_turn_off_call_to_arms(plyr_idx);
}

void store_backup_explored_flags_for_power_sight(struct PlayerInfo *player, struct Coord3d *soe_pos)
{
    struct Dungeon *dungeon;
    long stl_x,stl_y;
    long soe_x,soe_y;
    dungeon = get_players_dungeon(player);
    stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
    for (soe_y=0; soe_y < 2*MAX_SOE_RADIUS; soe_y++,stl_y++)
    {
        stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
        for (soe_x=0; soe_x < 2*MAX_SOE_RADIUS; soe_x++,stl_x++)
        {
            if (dungeon->soe_explored_flags[soe_y][soe_x])
            {
                struct Map *mapblk;
                mapblk = get_map_block_at(stl_x, stl_y);
                if (!map_block_invalid(mapblk))
                {
                    if (map_block_revealed(mapblk, player->id_number))
                        backup_explored[soe_y][soe_x] |= 0x01;
                    if ((mapblk->flags & MapFlg_Unkn04) != 0)
                        backup_explored[soe_y][soe_x] |= 0x02;
                    if ((mapblk->flags & MapFlg_Unkn80) != 0)
                        backup_explored[soe_y][soe_x] |= 0x04;
                }
            }
        }
    }
}

void update_vertical_explored_flags_for_power_sight(struct PlayerInfo *player, struct Coord3d *soe_pos)
{
    struct Dungeon *dungeon;
    long stl_x,stl_y;
    long soe_x,soe_y;
    long slb_x,slb_y;
    long boundstl_x;
    long delta;
    long i;
    dungeon = get_players_dungeon(player);
    stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
    for (soe_y=0; soe_y < 2*MAX_SOE_RADIUS; soe_y++,stl_y++)
    {
        if ( (stl_y >= 0) && (stl_y <= 255) )
        {
            stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
            for (soe_x=0; soe_x <= MAX_SOE_RADIUS; soe_x++,stl_x++)
            {
                if (dungeon->soe_explored_flags[soe_y][soe_x])
                {
                    soe_x++;
                    // Find max value for delta
                    delta = 0;
                    for (i=1; soe_x < 2*MAX_SOE_RADIUS; soe_x++,i++)
                    {
                        if (dungeon->soe_explored_flags[soe_y][soe_x])
                            delta = i;
                    }
                    boundstl_x = stl_x + delta;
                    if (stl_x < 0)
                    {
                        stl_x = 0;
                    } else
                    if (stl_x > map_subtiles_x-1)
                    {
                        stl_x = map_subtiles_x-1;
                    }
                    if (boundstl_x < 0)
                    {
                        boundstl_x = 0;
                    } else
                    if (boundstl_x > map_subtiles_x-1)
                    {
                        boundstl_x = map_subtiles_x-1;
                    }
                    if (boundstl_x >= stl_x)
                    {
                        delta = boundstl_x - stl_x + 1;
                        slb_y = subtile_slab_fast(stl_y);
                        for (i=0; i < delta; i++)
                        {
                            struct Map *mapblk;
                            struct SlabMap *slb;
                            struct SlabAttr *slbattr;
                            mapblk = get_map_block_at(stl_x+i, stl_y);
                            reveal_map_block(mapblk, player->id_number);
                            slb_x = subtile_slab_fast(stl_x+i);
                            slb = get_slabmap_block(slb_x, slb_y);
                            slbattr = get_slab_attrs(slb);
                            if ( !slbattr->is_unknflg14 )
                                mapblk->flags &= ~(MapFlg_Unkn80|MapFlg_Unkn04);
                            mapblk++;
                        }
                        stl_x += delta;
                    }
                }
            }
        }
    }
}

TbBool player_uses_power_sight(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->sight_casted_thing_idx > 0);
}

void update_horizonal_explored_flags_for_power_sight(struct PlayerInfo *player, struct Coord3d *soe_pos)
{
    struct Dungeon *dungeon;
    long stl_x,stl_y;
    long soe_x,soe_y;
    long boundstl_y;
    long slb_x,slb_y;
    long delta;
    long i;
    dungeon = get_players_dungeon(player);
    stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
    for (soe_x=0; soe_x < 2*MAX_SOE_RADIUS; soe_x++,stl_x++)
    {
        if ( (stl_x >= 0) && (stl_x <= 255) )
        {
            stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
            for (soe_y=0; soe_y <= MAX_SOE_RADIUS; soe_y++,stl_y++)
            {
                if (dungeon->soe_explored_flags[soe_y][soe_x])
                {
                    soe_y++;
                    // Find max value for delta
                    delta = 0;
                    for (i=1; soe_y < 2*MAX_SOE_RADIUS; soe_y++,i++)
                    {
                        if (dungeon->soe_explored_flags[soe_y][soe_x])
                            delta = i;
                    }
                    boundstl_y = stl_y + delta;
                    if (boundstl_y < 0)
                    {
                        boundstl_y = 0;
                    } else
                    if (boundstl_y > map_subtiles_y-1)
                    {
                        boundstl_y = map_subtiles_y-1;
                    }
                    if (stl_y < 0)
                    {
                        stl_y = 0;
                    } else
                    if (stl_y > map_subtiles_y-1)
                    {
                        stl_y = map_subtiles_y-1;
                    }
                    if (stl_y <= boundstl_y)
                    {
                      delta = boundstl_y - stl_y + 1;
                      slb_x = subtile_slab_fast(stl_x);
                      for (i=0; i < delta; i++)
                      {
                          struct Map *mapblk;
                          slb_y = subtile_slab_fast(stl_y+i);
                          mapblk = get_map_block_at(stl_x, stl_y+i);
                          reveal_map_block(mapblk, player->id_number);
                          struct SlabMap *slb;
                          struct SlabAttr *slbattr;
                          slb = get_slabmap_block(slb_x, slb_y);
                          slbattr = get_slab_attrs(slb);
                          if ( !slbattr->is_unknflg14 )
                              mapblk->flags &= ~(MapFlg_Unkn80|MapFlg_Unkn04);
                      }
                      stl_y += delta;
                    }
                }
            }
        }
    }
}

void update_explored_flags_for_power_sight(struct PlayerInfo *player)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    SYNCDBG(9,"Starting");
    //_DK_update_explored_flags_for_power_sight(player);
    dungeon = get_players_dungeon(player);
    LbMemorySet(backup_explored, 0, sizeof(backup_explored));
    if (dungeon->sight_casted_thing_idx == 0) {
        return;
    }
    thing = thing_get(dungeon->sight_casted_thing_idx);
    if (!thing_is_object(thing)) {
        ERRORLOG("Sight thing index %d invalid", (int)dungeon->sight_casted_thing_idx);
        turn_off_sight_of_evil(player->id_number);
        dungeon->sight_casted_thing_idx = 0;
        return;
    }
    TRACE_THING(thing);
    // Fill the backup_explored array
    store_backup_explored_flags_for_power_sight(player, &thing->mappos);
    update_vertical_explored_flags_for_power_sight(player, &thing->mappos);
    update_horizonal_explored_flags_for_power_sight(player, &thing->mappos);

}

void remove_explored_flags_for_power_sight(struct PlayerInfo *player)
{
    SYNCDBG(9,"Starting");
    _DK_remove_explored_flags_for_power_sight(player);
}
/******************************************************************************/
