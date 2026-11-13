/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_engine.c
 *     The fork's engine helpers Daimon's keeper controller and packets use that KeeperFX does not have (daimon_engine.h), each the fork's function as Daimon's port tools copy it (scripts/daimon_port/sync.py in the fork).
 */
/******************************************************************************/
#include "pre_inc.h"
#include "daimon_engine.h"
#include "roomspace.h"
#include "thing_doors.h"
#include "slab_data.h"
#include "map_data.h"
#include "creature_jobs.h"
#include "room_data.h"
#include "player_data.h"
#include "net_game.h"
#include "game_legacy.h"
#include "thing_creature.h"
#include "creature_control.h"
#include "creature_states_tortr.h"
#include "creature_states_prisn.h"
#include "power_process.h"
#include "map_blocks.h"
#include "dungeon_data.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "room_side_score.h"
#include "thing_list.h"
#include "creature_states.h"
#include "config_crtrstates.h"
#include "magic_powers.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

TbBool door_placement_allowed(PlayerNumber plyr_idx, DaiSubtlCoord stl_x, DaiSubtlCoord stl_y)
{
    DaiSlabCoord slb_x = DaiSubtileSlab(stl_x);
    DaiSlabCoord slb_y = DaiSubtileSlab(stl_y);
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    // floor, as the player knows it: revealed, not a wall or earth, not liquid
    const struct SlabConfigStats *slabst = get_slab_stats(slb);
    if (!subtile_revealed(DaiSlabSubtileCenter(slb_x), DaiSlabSubtileCenter(slb_y), plyr_idx)
        || ((slabst->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
        || slab_kind_is_liquid(slb->kind))
        return false;
    char Orientation = find_door_angle(stl_x, stl_y, plyr_idx);
    TbBool Check = false;
    switch(Orientation)
    {
        case 0:
        {
            Check = (!slab_middle_row_has_trap_on(slb_x, slb_y) );
            break;
        }
        case 1:
        {
            Check = (!slab_middle_column_has_trap_on(slb_x, slb_y) );
            break;
        }
    }
    return ( (slabmap_owner(slb) == plyr_idx) && (slb->kind == SlbT_CLAIMED) )
        && (Orientation != -1) && ( Check )
        && (!slab_has_door_thing_on(slb_x, slb_y));
}

static CreatureJob get_job_for_subtile_in_room(const struct Thing *creatng, DaiSubtlCoord stl_x, DaiSubtlCoord stl_y,
    uint64_t drop_kind_flags, const struct Room *room)
{
    // Detect the job which we will do in the area
    uint64_t required_kind_flags = drop_kind_flags;
    if (slab_is_area_inner_fill(DaiSubtileSlab(stl_x), DaiSubtileSlab(stl_y))) {
        required_kind_flags |= JoKF_AssignOnAreaCenter;
    } else {
        required_kind_flags |= JoKF_AssignOnAreaBorder;
    }
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    RoomKind rkind = room_is_invalid(room) ? RoK_NONE : room->kind;
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    if (rkind != RoK_NONE)
    {
        required_kind_flags |= JoKF_AssignAreaWithinRoom;
    }
    else
    {
        required_kind_flags |= JoKF_AssignAreaOutsideRoom;
    }
    if (creatng->owner == slabmap_owner(slb))
    {
        if (thing_is_creature_digger(creatng))
        {
            if (creature_is_for_dungeon_diggers_list(creatng))
            {
                required_kind_flags |= JoKF_OwnedDiggers;
            }
            else
            {
                CreatureJob jobpref = get_job_for_room(rkind, required_kind_flags | JoKF_OwnedDiggers, crconf->job_primary | crconf->job_secondary);
                if (jobpref == Job_NULL)
                {
                    return get_job_for_room(rkind, required_kind_flags | JoKF_OwnedCreatures, crconf->job_primary | crconf->job_secondary);
                }
                else
                {
                    return jobpref;
                }
            }
        }
        else
        {
            required_kind_flags |= JoKF_OwnedCreatures;
        }
    } else
    {
        if (creature_is_for_dungeon_diggers_list(creatng)) {
            required_kind_flags |= JoKF_EnemyDiggers;
        } else {
            required_kind_flags |= JoKF_EnemyCreatures;
        }
    }
    return get_job_for_room(rkind, required_kind_flags, crconf->job_primary | crconf->job_secondary);
}

CreatureJob predict_hand_drop_job(struct Thing *creatng, PlayerNumber plyr_idx, DaiSubtlCoord stl_x, DaiSubtlCoord stl_y)
{
    if (!thing_is_creature(creatng)) {
        return Job_NULL;
    }
    const struct Room* room = subtile_room_get(stl_x, stl_y);
    if (!room_is_invalid(room) && (room->owner != plyr_idx)) {
        return Job_NULL;
    }
    CreatureJob new_job = get_job_for_subtile_in_room(creatng, stl_x, stl_y, JoKF_AssignHumanDrop, room);
    if ((new_job != Job_NULL) && !creature_can_do_job_near_position(creatng, stl_x, stl_y, new_job, 0)) {
        return Job_NULL;
    }
    return new_job;
}

TbBool game_refuses_cheats(void)
{
    return (game.game_kind == GKind_MultiGame);
}

struct PlayerInfo *get_packet_seat(NetUserId user)
{
    return get_player(get_net_user_player_number(user));
}

int64_t creature_force_score(const struct Thing *thing)
{
    const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    const int64_t full = get_creature_thing_score(thing);
    if (creature_control_invalid(cctrl) || (cctrl->max_health <= 0)) return full;
    const int64_t hp = (thing->health < 0) ? 0 : ((thing->health > cctrl->max_health) ? cctrl->max_health : thing->health);
    return full * hp / cctrl->max_health;
}

DaiThingModel torture_death_kind(const struct Thing *thing, const struct Room *room)
{
    const struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    DaiThingModel crmodel = crconf->torture_kind;
    if ((crmodel > game.conf.crtr_conf.model_count) || (crmodel <= 0))
    {
        // If not assigned or is unknown, default to the room creature creation.
        crmodel = get_room_create_creature_model(room->kind);
    }
    return crmodel;
}

DaiThingModel prison_death_kind(const struct Thing *thing, const struct Room *room)
{
    const struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    const TbBool has_prison_kind = (crconf->prison_kind <= game.conf.crtr_conf.model_count) && (crconf->prison_kind > 0);
    // Only humanoids, or kinds configured with their own prison_kind, rise again from a prison.
    if (!crconf->humanoid_creature && !has_prison_kind) {
        return 0;
    }
    // If not assigned or is unknown, default to the room creature creation.
    return has_prison_kind ? crconf->prison_kind : get_room_create_creature_model(room->kind);
}

SlabKind slabmap_kind(const struct SlabMap *slb)
{
    return slb->kind;
}

enum RoomNeighbour room_neighbour_kind(SlabCodedCoords slab_num, SlabCodedCoords round_slab_num, int64_t synergy_slab_num)
{
    const struct SlabMap* slb = get_slabmap_direct(slab_num);
    const struct SlabMap* round_slb = get_slabmap_direct(round_slab_num);
    if (slabmap_block_invalid(round_slb)) {
        return RoNb_OffMap;
    }
    if ((slabmap_owner(round_slb) == slabmap_owner(slb)) && (round_slb->kind == slb->kind)) {
        return RoNb_SameRoom;
    }
    if (((slabmap_owner(round_slb) == slabmap_owner(slb)) || !(get_slab_kind_stats(synergy_slab_num)->is_ownable))
      && ((round_slb->kind == synergy_slab_num) && (synergy_slab_num >= 0))) {
        return RoNb_Synergy;
    }
    switch (find_core_slab_type(slb_num_decode_x(round_slab_num), slb_num_decode_y(round_slab_num)))
    {
      case SlbT_ROCK:
      case SlbT_GOLD:
      case SlbT_DENSEGOLD:
      case SlbT_EARTH:
      case SlbT_GEMS:
        return RoNb_Natural;
      case SlbT_WALLDRAPE:
        return (slabmap_owner(round_slb) == slabmap_owner(slb)) ? RoNb_OwnWall : RoNb_OtherWall;
      case SlbT_DOORWOOD1:
        return (slabmap_owner(round_slb) == slabmap_owner(slb)) ? RoNb_OwnDoor : RoNb_Open;
      default:
        return RoNb_Open;
    }
}

int64_t room_slab_side_score(SlabCodedCoords slab_num, SlabCodedCoords round_slab_num, int64_t synergy_slab_num, TbBool *continues_room)
{
    return room_side_score_of(room_neighbour_kind(slab_num, round_slab_num, synergy_slab_num), continues_room);
}

TbBool player_call_to_arms_is_charged(PlayerNumber plyr_idx)
{
    if (!player_uses_power_call_to_arms(plyr_idx)) {
        return false;
    }
    const struct Dungeon *dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon->cta_free) {
        return false;
    }
    const PlayerNumber owner = slabmap_owner(get_slabmap_for_subtile(dungeon->cta_stl_x, dungeon->cta_stl_y));
    if (owner == plyr_idx) {
        return false;
    }
    return !(game.conf.rules[plyr_idx].gameplay.allies_share_cta && players_are_mutual_allies(plyr_idx, owner));
}

static TbBool get_against(PlayerNumber agnst_plyr_idx, SlabKind agnst_slbkind, DaiSlabCoord slb_x, DaiSlabCoord slb_y)
{
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_block_invalid(slb)) {
        return 1;
    }
    struct SlabConfigStats *slabst = get_slab_stats(slb);
    struct SlabConfigStats *agnst_slabst = get_slab_kind_stats(agnst_slbkind);
    return (slabst->slb_id != agnst_slabst->slb_id)
    || ((slabmap_owner(slb) != agnst_plyr_idx) && ((slabmap_owner(slb) != game.neutral_player_num) || (slb->kind == SlbT_CLAIMED) ));
}

TbBool wall_slab_shows_face_toward(DaiSlabCoord wall_x, DaiSlabCoord wall_y, DaiSlabCoord nb_x, DaiSlabCoord nb_y)
{
    struct SlabMap *wslb = get_slabmap_block(wall_x, wall_y);
    if (slabmap_block_invalid(wslb) || slabmap_block_invalid(get_slabmap_block(nb_x, nb_y))) {
        return false;
    }
    const struct SlabConfigStats *wslabst = get_slab_stats(wslb);
    // Room walls (a non-zero slb_id) face everything but their own room, so they tell nothing about what is behind.
    if ((wslabst->category != SlbAtCtg_FortifiedWall) || (wslabst->slb_id != 0)) {
        return false;
    }
    return get_against(slabmap_owner(wslb), wslb->kind, nb_x, nb_y);
}

#ifdef __cplusplus
}
#endif
