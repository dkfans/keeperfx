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
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "gui_soundmsgs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_training_room(struct Thing *thing);
DLLIMPORT short _DK_training(struct Thing *thing);
DLLIMPORT long _DK_creature_can_be_trained(struct Thing *thing);
DLLIMPORT long _DK_player_can_afford_to_train_creature(struct Thing *thing);
DLLIMPORT void _DK_process_creature_in_training_room(struct Thing *thing, struct Room *room);
DLLIMPORT void _DK_setup_move_to_new_training_position(struct Thing *thing, struct Room *room, unsigned long a3);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/** Returns if the creature meets conditions to be trained.
 *
 * @param thing The creature thing to be tested.
 * @return
 */
TbBool creature_can_be_trained(struct Thing *thing)
{
    struct CreatureStats *crstat;
    //return _DK_creature_can_be_trained(thing);
    crstat = creature_stats_get_from_thing(thing);
    // Creatures without training value can't be trained
    if (crstat->training_value <= 0)
        return false;
    // If its model can train, check if this one can gain more experience
    return creature_can_gain_experience(thing);
}

TbBool player_can_afford_to_train_creature(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct CreatureStats *crstat;
    //return _DK_player_can_afford_to_train_creature(thing);
    dungeon = get_dungeon(thing->owner);
    crstat = creature_stats_get_from_thing(thing);
    return (dungeon->total_money_owned >= crstat->training_cost);
}

void setup_training_move(struct Thing *thing, SubtlCodedCoords stl_num)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->moveto_pos.x.stl.num = stl_num_decode_x(stl_num);
    cctrl->moveto_pos.x.stl.pos = 128;
    cctrl->moveto_pos.y.stl.num = stl_num_decode_y(stl_num);
    cctrl->moveto_pos.y.stl.pos = 128;
    cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
    if (thing_in_wall_at(thing, &cctrl->moveto_pos))
    {
        ERRORLOG("Illegal setup to (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
        set_start_state(thing);
    }
}

struct Thing *get_creature_in_training_room_which_could_accept_partner(struct Room *room, struct Thing *partnertng)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i;
    i = room->creatures_list;
    k = 0;
    while (i != 0)
    {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
      {
          ERRORLOG("Jump to invalid creature %ld detected",i);
        break;
      }
      i = cctrl->next_in_room;
      // Per creature code
      if (thing != partnertng)
      {
        if (cctrl->word_9F == 0)
        {
          return thing;
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

void setup_move_to_new_training_position(struct Thing *thing, struct Room *room, unsigned long a3)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Thing *prtng;
    struct CreatureControl *prctrl;
    struct Coord3d pos;
    SYNCDBG(8,"Starting");
    //_DK_setup_move_to_new_training_position(thing, room, a3);
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    if ( a3 )
      cctrl->byte_9E = 50;
    // Try partner training
    if ((crstat->partner_training > 0) && (ACTION_RANDOM(100) < crstat->partner_training))
    {
        prtng = get_creature_in_training_room_which_could_accept_partner(room, thing);
        if (!thing_is_invalid(prtng))
        {
            prctrl = creature_control_get_from_thing(thing);
            prctrl->byte_9A = CrTrMd_PartnerTraining;
            prctrl->byte_9B = 75;
            prctrl->word_9F = thing->index;
            prctrl->long_A1 = thing->field_9;
            cctrl->byte_9A = CrTrMd_PartnerTraining;
            cctrl->byte_9B = 75;
            cctrl->word_9F = prtng->index;
            cctrl->long_A1 = prtng->field_9;
            return;
      }
    }
    // No partner - train at some random position
    cctrl->byte_9A = CrTrMd_Value1;
    if (find_random_valid_position_for_thing_in_room(thing, room, &pos))
    {
      cctrl->moveto_pos.x.stl.num = pos.x.stl.num;
      cctrl->moveto_pos.x.stl.pos = 128;
      cctrl->moveto_pos.y.stl.num = pos.y.stl.num;
      cctrl->moveto_pos.y.stl.pos = 128;
      cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
      if (thing_in_wall_at(thing, &cctrl->moveto_pos))
      {
          ERRORLOG("Illegal setup to wall at (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
          set_start_state(thing);
      }
    }
    if (cctrl->instance_id == 0)
    {
      set_creature_instance(thing, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
    }
    SYNCDBG(8,"End");
}

void setup_training_search_for_post(struct Thing *thing)
{
    struct Room *room;
    struct Thing *traintng;
    long start_slab;
    long slb_x,slb_y;
    long i,k;
    room = subtile_room_get(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    // Let's start from a random slab
    slb_x = -1;
    slb_y = -1;
    start_slab = ACTION_RANDOM(room->slabs_count);
    k = start_slab;
    i = room->slabs_list;
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
    traintng = INVALID_THING;
    k = room->slabs_count;
    i = get_slab_number(slb_x,slb_y);
    while (k > 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        if (i == 0)
          i = room->slabs_list;
        // Per room tile code - find a training post
        traintng = get_object_at_subtile_of_model_and_owned_by(3*slb_x+1, 3*slb_y+1, 31, thing->owner);
        if (!thing_is_invalid(traintng))
            break;
        // Per room tile code ends
        k--;
    }
    // Got trainer (or not...), now do the correct action
    if (thing_is_invalid(traintng))
    {
        SYNCDBG(6,"Room no longer have training post, moving somewhere else.");
        setup_move_to_new_training_position(thing, room, 1);
    } else
    {
        i = get_subtile_number(traintng->mappos.x.stl.num,traintng->mappos.y.stl.num);
        setup_training_move(thing, i);
    }
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
    long speed,dist;
    long i;
    cctrl = creature_control_get_from_thing(thing);
    SYNCDBG(18,"Starting mode %d",(int)cctrl->byte_9A);
    //_DK_process_creature_in_training_room(thing, room); return;
    cctrl->field_4A = 0;
    switch (cctrl->byte_9A)
    {
    case CrTrMd_Value1:
        if (cctrl->instance_id != 0)
            break;
        if (cctrl->byte_9E <= 1)
        {
            setup_training_search_for_post(thing);
            cctrl->byte_9E = 50;
            break;
        }
        cctrl->byte_9E--;
        speed = get_creature_speed(thing);
        i = creature_move_to(thing, &cctrl->moveto_pos, speed, 0, 0);
        if (i == 1)
        {
            // Find a training post
            traintng = get_object_at_subtile_of_model_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, 31, thing->owner);
            if (thing_is_invalid(traintng))
            {
                setup_move_to_new_training_position(thing, room, 0);
                break;
            }
            cctrl->byte_9A = CrTrMd_Value2;
            cctrl->byte_9E = 50;
        } else
        if (i == -1)
        {
            ERRORLOG("Cannot get where we're going in the training room.");
            set_start_state(thing);
        }
        break;
    case CrTrMd_Value2:
        for (i=0; i < 4; i++)
        {
            long slb_x,slb_y;
            long stl_x,stl_y;
            struct SlabMap *slb;
            slb_x = map_to_slab[thing->mappos.x.stl.num] + (long)small_around[i].delta_x;
            slb_y = map_to_slab[thing->mappos.y.stl.num] + (long)small_around[i].delta_y;
            slb = get_slabmap_block(slb_x,slb_y);
            if ((slb->kind != SlbT_TRAINING) || (slabmap_owner(slb) != thing->owner))
                continue;
            stl_x = 3*slb_x + (long)corners[i].delta_x;
            stl_y = 3*slb_y + (long)corners[i].delta_y;
            traintng = INVALID_THING;
            crtng = get_creature_of_model_training_at_subtile_and_owned_by(stl_x, stl_y, -1, thing->owner, thing->index);
            if (thing_is_invalid(crtng))
            {
                traintng = get_object_at_subtile_of_model_and_owned_by(3*slb_x+1, 3*slb_y+1, 31, thing->owner);
            }
            if (!thing_is_invalid(traintng))
            {
                cctrl->byte_9C = 3 * map_to_slab[thing->mappos.x.stl.num] + 1;
                cctrl->byte_9D = 3 * map_to_slab[thing->mappos.y.stl.num] + 1;
                cctrl->moveto_pos.x.stl.num = stl_x;
                cctrl->moveto_pos.y.stl.num = stl_y;
                cctrl->moveto_pos.x.stl.pos = 128;
                cctrl->moveto_pos.y.stl.pos = 128;
                cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
                if (thing_in_wall_at(thing, &cctrl->moveto_pos))
                {
                  ERRORLOG("Illegal setup to (%d,%d)", (int)cctrl->moveto_pos.x.stl.num, (int)cctrl->moveto_pos.y.stl.num);
                  set_start_state(thing);
                }
                cctrl->byte_9A = CrTrMd_Value3;
                break;
            }
        }
        if (cctrl->byte_9A == CrTrMd_Value2)
          setup_move_to_new_training_position(thing, room, 1);
        break;
    case CrTrMd_Value3:
        speed = get_creature_speed(thing);
        i = creature_move_to(thing, &cctrl->moveto_pos, speed, 0, 0);
        if (i == 1)
        {
            crtng = get_creature_of_model_training_at_subtile_and_owned_by(thing->mappos.x.stl.num, thing->mappos.y.stl.num, -1, thing->owner, thing->index);
            if (!thing_is_invalid(crtng))
            {
                setup_move_to_new_training_position(thing, room, 1);
                break;
            }
            cctrl->byte_9A = CrTrMd_Value4;
        } else
        if (i == -1)
        {
            ERRORLOG("Cannot get where we're going in the training room.");
            set_start_state(thing);
        }
        break;
    case CrTrMd_Value4:
        pos.x.val = ((long)cctrl->byte_9C << 8) + 128;
        pos.y.val = ((long)cctrl->byte_9D << 8) + 128;
        if (creature_turn_to_face(thing, &pos) < 56)
        {
          cctrl->byte_9A = CrTrMd_Value5;
          cctrl->byte_9B = 75;
        }
        break;
    case CrTrMd_PartnerTraining:
        if (cctrl->word_9F == 0)
        {
            setup_move_to_new_training_position(thing, room, 0);
            return;
        }
        crtng = thing_get(cctrl->word_9F);
        if ((crtng->field_9 != cctrl->long_A1) || ((crtng->field_0 & 0x01) == 0))
        {
            setup_move_to_new_training_position(thing, room, 0);
            return;
        }
        cctrl2 = creature_control_get_from_thing(crtng);
        if (cctrl2->word_9F != thing->index)
        {
            cctrl->word_9F = 0;
            setup_move_to_new_training_position(thing, room, 0);
            break;
        }
        if (subtile_room_get(crtng->mappos.x.stl.num, crtng->mappos.y.stl.num) != room)
        {
            cctrl->word_9F = 0;
            cctrl2->word_9F = 0;
            setup_move_to_new_training_position(thing, room, 0);
            break;
        }
        crstat = creature_stats_get_from_thing(thing);
        dist = get_combat_distance(thing, crtng);
        if (dist > 284)
        {
            if (creature_move_to(thing, &crtng->mappos, get_creature_speed(thing), 0, 0) == -1)
            {
              ERRORLOG("Cannot navigate to training partner");
              setup_move_to_new_training_position(thing, room, 0);
              cctrl->word_9F = 0;
            }
        } else
        if (dist >= 156)
        {
            if (creature_turn_to_face(thing, &crtng->mappos) < 56)
            {
              cctrl->byte_9B--;
              if (cctrl->byte_9B > 0)
              {
                if ((cctrl->instance_id == 0) && ((cctrl->byte_9B % 8) == 0))
                {
                    set_creature_instance(thing, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
                }
              } else
              {
                if (cctrl->instance_id == 0)
                {
                    setup_move_to_new_training_position(thing, room, 0);
                    cctrl->word_9F = 0;
                } else
                {
                    cctrl->byte_9B = 1;
                }
                cctrl->exp_points += (room->efficiency * crstat->training_value);
              }
            }
        } else
        {
            creature_retreat_from_combat(thing, crtng, 33, 0);
        }
        break;
    case CrTrMd_Value5:
    default:
        cctrl->byte_9B--;
        if (cctrl->byte_9B > 0)
        {
          if ((cctrl->instance_id == 0) && ((cctrl->byte_9B % 8) == 0))
          {
              set_creature_instance(thing, CrInst_SWING_WEAPON_SWORD, 1, 0, 0);
          }
        } else
        {
          if (cctrl->instance_id == 0)
          {
              setup_move_to_new_training_position(thing, room, 1);
          } else
          {
              cctrl->byte_9B = 1;
          }
        }
        break;
    }
    SYNCDBG(18,"End");
}

short at_training_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_training_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    if ( !creature_can_be_trained(thing) )
    {
        set_start_state(thing);
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if (dungeon->total_money_owned < crstat->training_cost)
    {
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoGoldToTrain, MESSAGE_DELAY_TREASURY, true);
        set_start_state(thing);
        return 0;
    }
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_TRAINING) || (room->owner != thing->owner))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if ( !add_creature_to_work_room(thing, room) )
    {
        set_start_state(thing);
        return 0;
    }
    internal_set_thing_state(thing, CrSt_Training);
    setup_move_to_new_training_position(thing, room, 1);
    cctrl->field_82 = 0;
    return 1;
}

short training(struct Thing *thing)
{
    struct Dungeon *dungeon;
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    TbBool finish_training;
    struct Room *room;
    long i;
    SYNCDBG(18,"Starting");
    //return _DK_training(thing);
    dungeon = get_dungeon(thing->owner);
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    // Check if we should finish training
    finish_training = false;
    if (!creature_can_be_trained(thing))
    {
        SYNCDBG(9,"Ending training %s level %d; creature isn't trainable",thing_model_name(thing),(int)cctrl->explevel);
        finish_training = true;
    }
    if (!player_can_afford_to_train_creature(thing))
    {
        SYNCDBG(19,"Ending training %s index %d; cannot afford",thing_model_name(thing),(int)thing->index);
        if (is_my_player_number(thing->owner))
            output_message(SMsg_NoGoldToTrain, MESSAGE_DELAY_TREASURY, true);
        finish_training = true;
    }
    // Check if we're in correct room
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room) || (room->kind != RoK_TRAINING)
     || (cctrl->work_room_id != room->index) || (room->owner != thing->owner))
    {
        SYNCDBG(9,"Ending training %s index %d; training room no longer valid",thing_model_name(thing),(int)thing->index);
        finish_training = true;
    }
    if (finish_training)
    {
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    // Pay for the training
    cctrl->field_82++;
    if (cctrl->field_82 >= game.train_cost_frequency)
    {
        cctrl->field_82 -= game.train_cost_frequency;
        if (take_money_from_dungeon(thing->owner, crstat->training_cost, 1) < 0)
            ERRORLOG("Cannot take %d gold from dungeon %d",(int)crstat->training_cost,(int)thing->owner);
        create_price_effect(&thing->mappos, thing->owner, crstat->training_cost);
    }
    if ((cctrl->instance_id != CrInst_NULL) || !check_experience_upgrade(thing))
    {
        i = process_work_speed_on_work_value(thing,
            (long)room->efficiency * (long)crstat->training_value);
        cctrl->exp_points += i;
        dungeon->total_experience_creatures_gained += i;
        process_creature_in_training_room(thing, room);
    } else
    {
        if (external_set_thing_state(thing, CrSt_CreatureBeHappy)) {
            cctrl->field_282 = 50;
            // Imps have special way of storing previous state
            if (thing_is_creature_special_digger(thing)) {
                cctrl->digger.last_did_job = 4; // TODO: This actually should be set when the creature is dropped to training room
            }
        }
        dungeon->lvstats.creatures_trained++;
    }
    return 1;
}

/******************************************************************************/
