/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_train.c
 *     Creature state machine functions for their job in various rooms.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_train.h"
#include "globals.h"

#include "bflib_math.h"

#include "creature_states.h"
#include "creature_states_combt.h"
#include "creature_instances.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "player_utils.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

/******************************************************************************/
/** Returns if the creature meets conditions to be trained.
 *
 * @param thing The creature thing to be tested.
 * @return
 */
TbBool creature_can_be_trained(const struct Thing *thing)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    // Creatures without training value can't be trained
    if (crstat->training_value <= 0)
        return false;
    // If its model can train, check if this one can gain more experience
    return creature_can_gain_experience(thing);
}

TbBool player_can_afford_to_train_creature(const struct Thing *thing)
{
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    return (dungeon->total_money_owned >= crstat->training_cost);
}

void setup_training_move(struct Thing *creatng, SubtlCodedCoords stl_num)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->moveto_pos.x.val = subtile_coord_center(stl_num_decode_x(stl_num));
    cctrl->moveto_pos.y.val = subtile_coord_center(stl_num_decode_y(stl_num));
    cctrl->moveto_pos.z.val = get_thing_height_at(creatng, &cctrl->moveto_pos);
    if (thing_in_wall_at(creatng, &cctrl->moveto_pos))
    {
        ERRORLOG("Illegal setup to wall at (%d,%d)",
            (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
        set_start_state(creatng);
    }
    SYNCDBG(18,"The %s is moving to (%d,%d)", thing_model_name(creatng),
        (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
}

void setup_training_move_near(struct Thing *creatng, SubtlCodedCoords stl_num)
{
    MapSubtlCoord stl_x = stl_num_decode_x(stl_num);
    MapSubtlCoord stl_y = stl_num_decode_y(stl_num);
    // Select a subtile closer to current position
    MapSubtlDelta dist_x = stl_x - (MapSubtlDelta)creatng->mappos.x.stl.num;
    MapSubtlDelta dist_y = stl_y - (MapSubtlDelta)creatng->mappos.y.stl.num;
    if (abs(dist_x) > abs(dist_y))
    {
        if (dist_x > 0) {
            stl_x -= 1;
        } else {
            stl_x += 1;
        }
    } else
    {
        if (dist_y > 0) {
            stl_y -= 1;
        } else {
            stl_y += 1;
        }
    }
    SubtlCodedCoords near_stl_num = get_subtile_number(stl_x, stl_y);
    setup_training_move(creatng, near_stl_num);
}

struct Thing *get_creature_in_training_room_which_could_accept_partner(struct Room *room, struct Thing *partnertng)
{
    TRACE_THING(partnertng);
    long i = room->creatures_list;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (!creature_control_exists(cctrl))
        {
            ERRORLOG("Jump to invalid creature %d detected",(int)i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        if (thing != partnertng)
        {
            if ( (get_creature_state_besides_move(thing) == CrSt_Training) && (cctrl->training.partner_idx == 0) )
            {
                if (get_room_thing_is_on(thing) == room) {
                    return thing;
                } else {
                    WARNLOG("The %s pretends to be in room but it's not.",thing_model_name(thing));
                }
            }
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    return INVALID_THING;
}

void setup_move_to_new_training_position(struct Thing *thing, struct Room *room, unsigned long restart)
{
    struct Coord3d pos;
    SYNCDBG(8,"Starting for %s",thing_model_name(thing));
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    if ( restart )
      cctrl->training.search_timeout = 50;
    // Try partner training
    if ((crstat->partner_training > 0) && (CREATURE_RANDOM(thing, 100) < crstat->partner_training))
    {
        struct Thing* prtng = get_creature_in_training_room_which_could_accept_partner(room, thing);
        if (!thing_is_invalid(prtng))
        {
            SYNCDBG(7,"The %s found %s as training partner.",thing_model_name(thing),thing_model_name(prtng));
            struct CreatureControl* prctrl = creature_control_get_from_thing(prtng);
            prctrl->training.mode = CrTrMd_PartnerTraining;
            prctrl->training.train_timeout = 75;
            prctrl->training.partner_idx = thing->index;
            prctrl->training.partner_creation = thing->creation_turn;
            cctrl->training.mode = CrTrMd_PartnerTraining;
            cctrl->training.train_timeout = 75;
            cctrl->training.partner_idx = prtng->index;
            cctrl->training.partner_creation = prtng->creation_turn;
            return;
      }
    }
    // No partner - train at some random position
    cctrl->training.mode = CrTrMd_SearchForTrainPost;
    if (find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
        SYNCDBG(8,"Going to train at (%d,%d)",(int)pos.x.stl.num,(int)pos.y.stl.num);
        long i = get_subtile_number(pos.x.stl.num, pos.y.stl.num);
        setup_training_move(thing, i);
    } else {
        SYNCDBG(8,"No new position found, staying at (%d,%d)",(int)cctrl->moveto_pos.x.stl.num,(int)cctrl->moveto_pos.x.stl.num);
    }
    if (cctrl->instance_id == CrInst_NULL)
    {
        set_creature_instance(thing, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
    }
}

/**
 *  Finds a random training post near to the current position of given creature.
 *  Used when finding a training post seems to be taking too long; in that case, creature should start training with a nearest post.
 *  Note that this routine does not always select the nearest post - it is enough if it's 3 subtiles away.
 *
 * @param creatng The creature who wish to train with training post.
 */
void setup_training_search_for_post(struct Thing *creatng)
{
    struct Room* room = get_room_thing_is_on(creatng);
    // Let's start from a random slab
    long slb_x = -1;
    long slb_y = -1;
    long min_distance = LONG_MAX;
    struct Thing* traintng = INVALID_THING;
    long start_slab = CREATURE_RANDOM(creatng, room->slabs_count);
    long k = start_slab;
    long i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        if (k <= 0)
            break;
        k--;
    }
    // Got random starting slab, now sweep room slabs from it
    struct Thing* thing = INVALID_THING;
    k = room->slabs_count;
    i = get_slab_number(slb_x,slb_y);
    while (k > 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        if (i == 0)
          i = room->slabs_list;
        // Per room tile code - find a nearest training post
        thing = get_object_at_subtile_of_model_and_owned_by(slab_subtile_center(slb_x), slab_subtile_center(slb_y), 31, creatng->owner);
        if (!thing_is_invalid(thing))
        {
            long dist = get_2d_distance(&creatng->mappos, &thing->mappos);
            if (dist < min_distance) {
                traintng = thing;
                min_distance = dist;
                if (min_distance < (3<<8))
                    break;
            }
        }
        // Per room tile code ends
        k--;
    }
    // Got trainer (or not...), now do the correct action
    if (thing_is_invalid(traintng))
    {
        SYNCDBG(6,"Room no longer have training post, moving somewhere else.");
        setup_move_to_new_training_position(creatng, room, true);
    } else
    {
        i = get_subtile_number(traintng->mappos.x.stl.num,traintng->mappos.y.stl.num);
        setup_training_move_near(creatng, i);
    }
}

struct Thing *find_training_post_just_next_to_creature(struct Thing *creatng)
{
    struct Thing* traintng = INVALID_THING;
    for (long i = 0; i < 4; i++)
    {
        long stl_x = creatng->mappos.x.stl.num + (long)small_around[i].delta_x;
        long stl_y = creatng->mappos.y.stl.num + (long)small_around[i].delta_y;
        traintng = get_object_at_subtile_of_model_and_owned_by(stl_x, stl_y, 31, creatng->owner);
        if (!thing_is_invalid(traintng))
            break;
    }
    return traintng;
}

void process_creature_in_training_room(struct Thing *thing, struct Room *room)
{
    static const struct Around corners[] = {
        {1, 2},
        {0, 1},
        {1, 0},
        {2, 1},
    };
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Thing *traintng;
    struct Thing *crtng;
    struct CreatureControl *cctrl2;
    struct Coord3d pos;
    long speed;
    long dist;
    long i;
    cctrl = creature_control_get_from_thing(thing);
    SYNCDBG(8,"Starting %s mode %d",thing_model_name(thing),(int)cctrl->training.mode);
    cctrl->annoy_untrained_turn = 0;
    switch (cctrl->training.mode)
    {
    case CrTrMd_SearchForTrainPost:
        // While we're in an instance, just wait
        if (cctrl->instance_id != CrInst_NULL)
            break;
        // On timeout, search for nearby training posts to start training ASAP
        if (cctrl->training.search_timeout < 1)
        {
            SYNCDBG(6,"Search timeout - selecting post nearest to (%d,%d)",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
            setup_training_search_for_post(thing);
            cctrl->training.search_timeout = 100;
            break;
        }
        // Do a moving step
        cctrl->training.search_timeout--;
        speed = get_creature_speed(thing);
        i = creature_move_to(thing, &cctrl->moveto_pos, speed, 0, 0);
        if (i == 1)
        {
            // Move target is reached - find a training post which is supposed to be around here
            traintng = find_training_post_just_next_to_creature(thing);
            if (thing_is_invalid(traintng))
            {
                SYNCDBG(6,"Reached (%d,%d) but there's no training post there",(int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num);
                setup_move_to_new_training_position(thing, room, false);
                break;
            }
            // Found - go to next mode
            cctrl->training.mode = CrTrMd_SelectPositionNearTrainPost;
            cctrl->training.search_timeout = 50;
        } else
        if (i == -1)
        {
            ERRORLOG("Cannot get to (%d,%d) in the training room",(int)cctrl->moveto_pos.x.stl.num,(int)cctrl->moveto_pos.y.stl.num);
            set_start_state(thing);
        }
        break;
    case CrTrMd_SelectPositionNearTrainPost:
        for (i=0; i < 4; i++)
        {
            long slb_x;
            long slb_y;
            long stl_x;
            long stl_y;
            struct SlabMap *slb;
            slb_x = subtile_slab_fast(thing->mappos.x.stl.num) + (long)small_around[i].delta_x;
            slb_y = subtile_slab_fast(thing->mappos.y.stl.num) + (long)small_around[i].delta_y;
            slb = get_slabmap_block(slb_x,slb_y);
            if ((slb->kind != SlbT_TRAINING) || (slabmap_owner(slb) != thing->owner))
                continue;
            stl_x = slab_subtile(slb_x,corners[i].delta_x);
            stl_y = slab_subtile(slb_y,corners[i].delta_y);
            traintng = INVALID_THING;
            // Check if any other creature is using that post; allow only unused posts
            crtng = get_creature_of_model_training_at_subtile_and_owned_by(stl_x, stl_y, -1, thing->owner, thing->index);
            if (thing_is_invalid(crtng))
            {
                traintng = get_object_at_subtile_of_model_and_owned_by(slab_subtile_center(slb_x), slab_subtile_center(slb_y), 31, thing->owner);
            }
            if (!thing_is_invalid(traintng))
            {
                cctrl->training.pole_stl_x = slab_subtile_center(subtile_slab_fast(thing->mappos.x.stl.num));
                cctrl->training.pole_stl_y = slab_subtile_center(subtile_slab_fast(thing->mappos.y.stl.num));
                cctrl->moveto_pos.x.stl.num = stl_x;
                cctrl->moveto_pos.y.stl.num = stl_y;
                cctrl->moveto_pos.x.stl.pos = 128;
                cctrl->moveto_pos.y.stl.pos = 128;
                cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
                if (thing_in_wall_at(thing, &cctrl->moveto_pos))
                {
                    ERRORLOG("Illegal setup to (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
                    break;
                }
                cctrl->training.mode = CrTrMd_MoveToTrainPost;
                break;
            }
        }
        if (cctrl->training.mode == CrTrMd_SelectPositionNearTrainPost)
          setup_move_to_new_training_position(thing, room, 1);
        break;
    case CrTrMd_MoveToTrainPost:
        speed = get_creature_speed(thing);
        i = creature_move_to(thing, &cctrl->moveto_pos, speed, 0, 0);
        if (i == 1)
        {
            // If there's already someone training at that position, go somewhere else
            crtng = get_creature_of_model_training_at_subtile_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, -1, thing->owner, thing->index);
            if (!thing_is_invalid(crtng))
            {
                setup_move_to_new_training_position(thing, room, 1);
                break;
            }
            // Otherwise, train at this position
            cctrl->training.mode = CrTrMd_TurnToTrainPost;
        } else
        if (i == -1)
        {
            ERRORLOG("Cannot get where we're going in the training room.");
            set_start_state(thing);
        }
        break;
    case CrTrMd_TurnToTrainPost:
        pos.x.val = subtile_coord_center(cctrl->training.pole_stl_x);
        pos.y.val = subtile_coord_center(cctrl->training.pole_stl_y);
        if (creature_turn_to_face(thing, &pos) < LbFPMath_PI/18)
        {
          cctrl->training.mode = CrTrMd_DoTrainWithTrainPost;
          cctrl->training.train_timeout = 75;
        }
        break;
    case CrTrMd_PartnerTraining:
        if (cctrl->training.partner_idx == 0)
        {
            setup_move_to_new_training_position(thing, room, false);
            return;
        }
        crtng = thing_get(cctrl->training.partner_idx);
        TRACE_THING(crtng);
        if (!thing_exists(crtng) || (get_creature_state_besides_move(crtng) != CrSt_Training) || (crtng->creation_turn != cctrl->training.partner_creation))
        {
            SYNCDBG(8,"The %s cannot start partner training - creature to train with is gone.",thing_model_name(thing));
            setup_move_to_new_training_position(thing, room, false);
            return;
        }
        cctrl2 = creature_control_get_from_thing(crtng);
        if (cctrl2->training.partner_idx != thing->index)
        {
            SYNCDBG(6,"The %s cannot start partner training - %s changed the partner.",thing_model_name(thing),thing_model_name(crtng));
            cctrl->training.partner_idx = 0;
            setup_move_to_new_training_position(thing, room, false);
            break;
        }
        if (get_room_thing_is_on(crtng) != room)
        {
            SYNCDBG(8,"The %s cannot start partner training - partner has left the room.",thing_model_name(thing));
            cctrl->training.partner_idx = 0;
            cctrl2->training.partner_idx = 0;
            setup_move_to_new_training_position(thing, room, false);
            break;
        }
        crstat = creature_stats_get_from_thing(thing);
        dist = get_combat_distance(thing, crtng);
        if (dist > 284)
        {
            if (creature_move_to(thing, &crtng->mappos, get_creature_speed(thing), 0, 0) == -1)
            {
              WARNLOG("The %s cannot navigate to training partner",thing_model_name(thing));
              setup_move_to_new_training_position(thing, room, false);
              cctrl->training.partner_idx = 0;
            }
        } else
        if (dist >= 156)
        {
            if (creature_turn_to_face(thing, &crtng->mappos) < LbFPMath_PI/18)
            {
              cctrl->training.train_timeout--;
              if (cctrl->training.train_timeout > 0)
              {
                if ((cctrl->instance_id == CrInst_NULL) && ((cctrl->training.train_timeout % 8) == 0))
                {
                    set_creature_instance(thing, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
                }
              } else
              {
                if (cctrl->instance_id == CrInst_NULL)
                {
                    setup_move_to_new_training_position(thing, room, false);
                    cctrl->training.partner_idx = 0;
                } else
                {
                    cctrl->training.train_timeout = 1;
                }
                cctrl->exp_points += (room->efficiency * crstat->training_value);
              }
            }
        } else
        {
            creature_retreat_from_combat(thing, crtng, CrSt_Training, 0);
        }
        break;
    case CrTrMd_DoTrainWithTrainPost:
        if (cctrl->training.train_timeout > 0)
        {
            // While training timeout is positive, continue initiating the train instances
            cctrl->training.train_timeout--;
            if ((cctrl->instance_id == CrInst_NULL) && ((cctrl->training.train_timeout % 8) == 0))
            {
                set_creature_instance(thing, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
            }
        } else
        {
            // Wait for the instance to end, then select new move position
            if (cctrl->instance_id != CrInst_NULL)
            {
                cctrl->training.train_timeout = 0;
            } else
            {
                cctrl->training.train_timeout = 0;
                setup_move_to_new_training_position(thing, room, true);
            }
        }
        break;
    default:
        WARNLOG("Invalid %s training mode %d; reset",thing_model_name(thing),(int)cctrl->training.mode);
        cctrl->training.mode = CrTrMd_SearchForTrainPost;
        cctrl->training.search_timeout = 0;
        break;
    }
    SYNCDBG(18,"End");
}

short at_training_room(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->target_room_id = 0;
    if (!creature_can_be_trained(thing))
    {
        SYNCDBG(9,"Ending training of %s level %d; creature is not trainable",thing_model_name(thing),(int)cctrl->explevel);
        set_start_state(thing);
        return 0;
    }
    if (!player_can_afford_to_train_creature(thing))
    {
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoGoldToTrain, MESSAGE_DELAY_TREASURY, true);
        set_start_state(thing);
        return 0;
    }
    struct Room* room = get_room_thing_is_on(thing);
    if (!room_initially_valid_as_type_for_thing(room, get_room_for_job(Job_TRAIN), thing))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room, Job_TRAIN))
    {
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, get_continue_state_for_job(Job_TRAIN));
    setup_move_to_new_training_position(thing, room, 1);
    cctrl->turns_at_job = 0;
    return 1;
}

CrStateRet training(struct Thing *thing)
{
    TRACE_THING(thing);
    SYNCDBG(18,"Starting");
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // Check if we should finish training
    if (!creature_can_be_trained(thing))
    {
        SYNCDBG(9,"Ending training of %s level %d; creature is not trainable",thing_model_name(thing),(int)cctrl->explevel);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetOk;
    }
    if (!player_can_afford_to_train_creature(thing))
    {
        SYNCDBG(19,"Ending training %s index %d; cannot afford",thing_model_name(thing),(int)thing->index);
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoGoldToTrain, MESSAGE_DELAY_TREASURY, true);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    // Check if we're in correct room
    struct Room* room = get_room_thing_is_on(thing);
    if (creature_job_in_room_no_longer_possible(room, Job_TRAIN, thing))
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return CrStRet_ResetFail;
    }
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    // Pay for the training
    cctrl->turns_at_job++;
    if (cctrl->turns_at_job >= game.train_cost_frequency)
    {
        cctrl->turns_at_job -= game.train_cost_frequency;
        if (take_money_from_dungeon(thing->owner, crstat->training_cost, 1) < 0) {
            ERRORLOG("Cannot take %d gold from dungeon %d",(int)crstat->training_cost,(int)thing->owner);
        }
        create_price_effect(&thing->mappos, thing->owner, crstat->training_cost);
    }
    if ((cctrl->instance_id != CrInst_NULL) || !check_experience_upgrade(thing))
    {
        long work_value = compute_creature_work_value_for_room_role(thing, RoRoF_CrTrainExp, room->efficiency);
        SYNCDBG(19,"The %s index %d produced %d training points",thing_model_name(thing),(int)thing->index,(int)work_value);
        cctrl->exp_points += work_value;
        dungeon->total_experience_creatures_gained += work_value;
        process_creature_in_training_room(thing, room);
    } else
    {
        if (external_set_thing_state(thing, CrSt_CreatureBeHappy)) {
            cctrl->countdown_282 = 50;
        }
        dungeon->lvstats.creatures_trained++;
    }
    return CrStRet_Modified;
}

/******************************************************************************/
