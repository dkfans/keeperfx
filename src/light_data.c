/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file light_data.c
 *     light_data support functions.
 * @par Purpose:
 *     Functions to light_data.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "light_data.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"

#include "player_data.h"
#include "map_data.h"

#include "thing_stats.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_light_remove_light_from_list(struct Light *lgt, struct StructureList *list);
DLLIMPORT void _DK_light_signal_stat_light_update_in_area(long x1, long y1, long x2, long y2);
DLLIMPORT void _DK_light_initialise_lighting_tables(void);
DLLIMPORT void _DK_light_set_light_minimum_size_to_cache(long a1, long a2, long a3);
DLLIMPORT void _DK_light_set_light_position(long lgt_id, struct Coord3d *pos);
DLLIMPORT long _DK_light_get_light_intensity(long idx);
DLLIMPORT long _DK_light_set_light_intensity(long a1, long a2);
DLLIMPORT void _DK_light_render_area(int startx, int starty, int endx, int endy);
DLLIMPORT void _DK_light_stat_light_map_clear_area(long x1, long y1, long x2, long y2);
DLLIMPORT void _DK_light_signal_update_in_area(long sx, long sy, long ex, long ey);

/******************************************************************************/
struct Light *light_allocate_light(void)
{
    for (long i = 1; i < LIGHTS_COUNT; i++)
    {
        struct Light* lgt = &game.lish.lights[i];
        if ((lgt->flags & LgtF_Allocated) == 0)
        {
            lgt->flags |= LgtF_Allocated;
            lgt->index = i;
            return lgt;
        }
    }
    return NULL;
}

void light_free_light(struct Light *lgt)
{
    LbMemorySet(lgt, 0, sizeof(struct Light));
}

TbBool light_is_invalid(const struct Light *lgt)
{
    if (lgt == NULL)
        return true;
    if ((lgt < &game.lish.lights[1]) || (lgt > &game.lish.lights[LIGHTS_COUNT-1]))
        return true;
    return false;
}

struct ShadowCache *light_allocate_shadow_cache(void)
{
    for (long i = 1; i < SHADOW_CACHE_COUNT; i++)
    {
        struct ShadowCache* shdc = &game.lish.shadow_cache[i];
        if ((shdc->flags & ShCF_Allocated) == 0)
        {
            shdc->flags |= ShCF_Allocated;
            return shdc;
        }
    }
    return NULL;
}

TbBool light_shadow_cache_invalid(struct ShadowCache *shdc)
{
    if (shdc == NULL)
        return true;
    if ((shdc < &game.lish.shadow_cache[1]) || (shdc > &game.lish.shadow_cache[SHADOW_CACHE_COUNT-1]))
        return true;
    return false;
}

long light_shadow_cache_index(struct ShadowCache *shdc)
{
    if (light_shadow_cache_invalid(shdc))
        return 0;
    long i = ((char*)shdc - (char*)&game.lish.shadow_cache[0]);
    return i / sizeof(struct ShadowCache);
}

void light_shadow_cache_free(struct ShadowCache *shdc)
{
    LbMemorySet(shdc, 0, sizeof(struct ShadowCache));
}

TbBool light_add_light_to_list(struct Light *lgt, struct StructureList *list)
{
  if ((lgt->field_1 & 0x01) != 0)
  {
    ERRORLOG("Light is already in list");
    return false;
  }
  list->count++;
  lgt->field_1 |= 0x01;
  lgt->field_26 = list->index;
  list->index = lgt->index;
  return true;
}

long light_create_light(struct InitLight *ilght)
{
    struct Light* lgt = light_allocate_light();
    if (light_is_invalid(lgt)) {
        return 0;
    }
    if (ilght->is_dynamic)
    {
        struct ShadowCache* shdc = light_allocate_shadow_cache();
        if (light_shadow_cache_invalid(shdc))
        {
            ERRORDBG(11,"Cannot allocate cache for dynamic light");
            light_free_light(lgt);
            return 0;
        }
        light_total_dynamic_lights++;
        lgt->shadow_index = light_shadow_cache_index(shdc);
        light_add_light_to_list(lgt, &game.thing_lists[TngList_DynamLights]);
    } else
    {
        light_total_stat_lights++;
        light_add_light_to_list(lgt, &game.thing_lists[TngList_StaticLights]);
        stat_light_needs_updating = 1;
    }
    lgt->flags |= LgtF_Unkn02;
    lgt->flags |= LgtF_Unkn08;
    lgt->mappos.x.val = ilght->mappos.x.val;
    lgt->mappos.y.val = ilght->mappos.y.val;
    lgt->mappos.z.val = ilght->mappos.z.val;
    lgt->radius = ilght->radius;
    lgt->intensity = ilght->intensity;
    unsigned long k = 2 * ilght->field_3;
    lgt->field_1 = k ^ ((k ^ lgt->field_1) & 0x01);
    set_flag_byte(&lgt->flags,LgtF_Dynamic,ilght->is_dynamic);
    lgt->field_1A = ilght->field_8;
    lgt->field_18 = ilght->field_4;
    lgt->field_12 = ilght->field_12;
    return lgt->index;
}

long light_get_total_dynamic_lights(void)
{
    return light_total_dynamic_lights;
}

long light_get_total_stat_lights(void)
{
    return light_total_stat_lights;
}

long light_get_rendered_dynamic_lights(void)
{
    return light_rendered_dynamic_lights;
}

long light_get_rendered_optimised_dynamic_lights(void)
{
    return light_rendered_optimised_dynamic_lights;
}

long light_get_updated_stat_lights(void)
{
    return light_updated_stat_lights;
}

long light_get_out_of_date_stat_lights(void)
{
    return light_out_of_date_stat_lights;
}

void light_export_system_state(struct LightSystemState *lightst)
{
    memcpy(lightst->bitmask,light_bitmask,sizeof(light_bitmask));
    lightst->static_light_needs_updating = stat_light_needs_updating;
    lightst->total_dynamic_lights = light_total_dynamic_lights;
    lightst->total_stat_lights = light_total_stat_lights;
    lightst->rendered_dynamic_lights = light_rendered_dynamic_lights;
    lightst->rendered_optimised_dynamic_lights = light_rendered_optimised_dynamic_lights;
    lightst->updated_stat_lights = light_updated_stat_lights;
    lightst->out_of_date_stat_lights = light_out_of_date_stat_lights;
}

void light_import_system_state(const struct LightSystemState *lightst)
{
    memcpy(light_bitmask,lightst->bitmask,sizeof(light_bitmask));
    stat_light_needs_updating = lightst->static_light_needs_updating;
    light_total_dynamic_lights = lightst->total_dynamic_lights;
    light_total_stat_lights = lightst->total_stat_lights;
    light_rendered_dynamic_lights = lightst->rendered_dynamic_lights;
    light_rendered_optimised_dynamic_lights = lightst->rendered_optimised_dynamic_lights;
    light_updated_stat_lights = lightst->updated_stat_lights;
    light_out_of_date_stat_lights = lightst->out_of_date_stat_lights;
}

TbBool lights_stats_debug_dump(void)
{
    long lights[LIGHTS_COUNT];
    long lgh_things[THING_CLASSES_COUNT];
    long shadowcs[SHADOW_CACHE_COUNT];
    long i;
    for (i=0; i < SHADOW_CACHE_COUNT; i++)
    {
        struct ShadowCache* shdc = &game.lish.shadow_cache[i];
        if ((shdc->flags & ShCF_Allocated) != 0)
            shadowcs[i] = -1;
        else
            shadowcs[i] = 0;
    }
    long lgh_sttc = 0;
    long lgh_dynm = 0;
    for (i=0; i < LIGHTS_COUNT; i++)
    {
        struct Light* lgt = &game.lish.lights[i];
        if ((lgt->flags & LgtF_Allocated) != 0)
        {
            lights[i] = -1;
            if ((lgt->flags & LgtF_Dynamic) != 0)
                lgh_dynm++;
            else
                lgh_sttc++;
            if ( (lgt->shadow_index > 0) && (lgt->shadow_index < SHADOW_CACHE_COUNT) )
            {
                if (shadowcs[lgt->shadow_index] == -1) {
                    shadowcs[lgt->shadow_index] = i;
                } else
                if (shadowcs[lgt->shadow_index] == 0) {
                    WARNLOG("Shadow Cache %d is not allocated, but used by light %d!",(int)lgt->shadow_index,(int)i);
                } else {
                    WARNLOG("Shadow Cache %d is double-allocated, for lights %d and %d!",(int)lgt->shadow_index,(int)shadowcs[lgt->shadow_index],(int)i);
                }
            } else
            if ((lgt->flags & LgtF_Dynamic) != 0)
            {
                WARNLOG("Dynamic light %d has bad Shadow Cache %d!",(int)i,(int)lgt->shadow_index);
            }
        } else {
            lights[i] = 0;
        }
    }
    for (i=1; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing))
        {
            if ((thing->light_id > 0) && (thing->light_id < LIGHTS_COUNT))
            {
                long n = 1000 + (long)thing->class_id;
                if (lights[thing->light_id] == -1) {
                    lights[thing->light_id] = n;
                } else
                if (lights[thing->light_id] == 0) {
                    WARNLOG("Light %d is not allocated, but used by %s!",(int)thing->light_id, thing_model_name(thing));
                } else {
                    WARNLOG("Light %d is double-allocated, for %d and %d!",(int)thing->light_id, (int)lights[thing->light_id], (int)n);
                }
            }

        }
    }
    long lgh_used = 0;
    long lgh_free = 0;
    for (i=0; i < THING_CLASSES_COUNT; i++)
        lgh_things[i] = 0;
    for (i=0; i < LIGHTS_COUNT; i++)
    {
        if (lights[i] != 0)
        {
            lgh_used++;
            if ((lights[i] > 1000) && (lights[i] < 1000+THING_CLASSES_COUNT))
                lgh_things[lights[i]-1000]++;
        } else
        {
            lgh_free++;
        }
    }
    long shdc_free = 0;
    long shdc_used = 0;
    long shdc_linked = 0;
    for (i=0; i < SHADOW_CACHE_COUNT; i++)
    {
        if (shadowcs[i] != 0)
        {
            shdc_used++;
            if (shadowcs[i] > 0)
                shdc_linked++;
        } else {
            shdc_free++;
        }
    }
    SYNCLOG("Lights: %ld free, %ld used; %ld static, %ld dynamic; for things:%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld",lgh_free,lgh_used,lgh_sttc,lgh_dynm,lgh_things[1],lgh_things[2],lgh_things[3],lgh_things[4],lgh_things[5],lgh_things[6],lgh_things[7],lgh_things[8],lgh_things[9],lgh_things[10],lgh_things[11],lgh_things[12],lgh_things[13]);
    if ((shdc_used != shdc_linked) || (shdc_used != lgh_dynm))
    {
        WARNLOG("Amount of shadow cache mismatches: %ld free, %ld used, %ld linked to lights, %d dyn. lights.",shdc_free,shdc_used,shdc_linked,light_total_dynamic_lights);
    }
    if (lgh_sttc != light_total_stat_lights)
    {
        WARNLOG("Wrong global lights counter: %ld static lights and counter says %ld.",lgh_sttc,light_total_stat_lights);
    }
    if (lgh_dynm != light_total_dynamic_lights)
    {
        WARNLOG("Wrong global lights counter: %ld dynamic lights and counter says %ld.",lgh_dynm,light_total_dynamic_lights);
    }
    return false;
}

void light_set_light_never_cache(long lgt_id)
{
    if (lgt_id <= 0)
    {
        ERRORLOG("Attempt to set size of invalid light %d",(int)lgt_id);
        return;
    }
    struct Light* lgt = &game.lish.lights[lgt_id];
    if ((lgt->flags & LgtF_Allocated) == 0)
    {
        ERRORLOG("Attempt to set size of unallocated light structure %d",(int)lgt_id);
        return;
    }
    lgt->flags |= LgtF_Dynamic;
}

long light_is_light_allocated(long lgt_id)
{
    if (lgt_id <= 0)
        return false;
    struct Light* lgt = &game.lish.lights[lgt_id];
    if ((lgt->flags & LgtF_Allocated) == 0)
        return false;
    return true;
}

void light_set_light_position(long lgt_id, struct Coord3d *pos)
{
  _DK_light_set_light_position(lgt_id, pos);
}

void light_remove_light_from_list(struct Light *lgt, struct StructureList *list)
{
  _DK_light_remove_light_from_list(lgt, list);
}

void light_signal_stat_light_update_in_area(long x1, long y1, long x2, long y2)
{
  _DK_light_signal_stat_light_update_in_area(x1, y1, x2, y2);
}

void light_signal_update_in_area(long sx, long sy, long ex, long ey)
{
    _DK_light_signal_update_in_area(sx, sy, ex, ey);
}

void light_signal_stat_light_update_in_own_radius(struct Light *lgt)
{
    long radius = lgt->range;
    long end_y = (long)lgt->mappos.y.stl.num + radius;
    if (end_y >= 255)
        end_y = 255;
    long end_x = (long)lgt->mappos.x.stl.num + radius;
    if (end_x >= 255)
        end_x = 255;
    long start_y = (long)lgt->mappos.y.stl.num - radius;
    if (start_y <= 0)
        start_y = 0;
    long start_x = (long)lgt->mappos.x.stl.num - radius;
    if (start_x <= 0)
      start_x = 0;
    if ((end_x <= start_x) || (end_y <= start_y))
        return;
    light_signal_stat_light_update_in_area(start_x, start_y, end_x, end_y);
}

void light_turn_light_off(long idx)
{
    if ((idx <= 0) || (idx >= LIGHTS_COUNT)) {
        ERRORLOG("Attempt to turn off light %d",(int)idx);
        return;
    }
    struct Light* lgt = &game.lish.lights[idx];
    if ((lgt->flags & LgtF_Allocated) == 0) {
        ERRORLOG("Attempt to turn off unallocated light structure");
        return;
    }
    if ((lgt->flags & LgtF_Unkn02) == 0) {
        return;
    }
    lgt->flags &= ~LgtF_Unkn02;
    if ((lgt->flags & LgtF_Dynamic) != 0) {
        light_remove_light_from_list(lgt, &game.thing_lists[TngList_DynamLights]);
    } else {
        light_signal_stat_light_update_in_own_radius(lgt);
        light_remove_light_from_list(lgt, &game.thing_lists[TngList_StaticLights]);
        stat_light_needs_updating = 1;
    }
}

void light_turn_light_on(long idx)
{
    if ((idx <= 0) || (idx >= LIGHTS_COUNT)) {
        ERRORLOG("Attempt to turn on light %d",(int)idx);
        return;
    }
    struct Light* lgt = &game.lish.lights[idx];
    if ((lgt->flags & LgtF_Allocated) == 0) {
        ERRORLOG("Attempt to turn on unallocated light structure %d",(int)idx);
        return;
    }
    if ((lgt->flags & LgtF_Unkn02) != 0) {
        return;
    }
    lgt->flags |= LgtF_Unkn02;
    if ((lgt->flags & LgtF_Dynamic) != 0)
    {
        light_add_light_to_list(lgt, &game.thing_lists[TngList_DynamLights]);
        lgt->flags |= LgtF_Unkn08;
    } else
    {
        light_add_light_to_list(lgt, &game.thing_lists[TngList_StaticLights]);
        stat_light_needs_updating = 1;
        lgt->flags |= LgtF_Unkn08;
    }
}

unsigned char light_get_light_intensity(long idx)
{
  // return _DK_light_get_light_intensity(idx);
  if ( idx )
  {
    if ( game.lish.lights[idx].flags & LgtF_Allocated )
    {
      return game.lish.lights[idx].intensity;
    }
    else
    {
      ERRORLOG("Attempt to get intensity of unallocated light structure");
      return 0;
    }
  }
  else
  {
    ERRORLOG("Attempt to get intensity of light 0");
    return 0;
  }
}

void light_set_light_intensity(long idx, unsigned char intensity)
{
  // return _DK_light_set_light_intensity(a1, a2);
  struct Light *lgt = &game.lish.lights[idx];
  long x1,x2,y1,y2;
  if ( !light_is_invalid(lgt) )
  {
    if ((lgt->flags & LgtF_Allocated) != 0)
    {
      if ( lgt->intensity != intensity )
      {
        if ((lgt->flags & LgtF_Dynamic) == 0)
        {
          y2 = lgt->mappos.y.stl.num + lgt->range;
          if ( y2 > 255 )
            y2 = 255;
          x2 = lgt->mappos.x.stl.num + lgt->range;
          if ( x2 > 255 )
            x2 = 255;
          y1 = lgt->mappos.y.stl.num - lgt->range;
          if ( y1 < 0 )
            y1 = 0;
          x1 = lgt->mappos.x.stl.num - lgt->range;
          if ( x1 < 0 )
            x1 = 0;
          light_signal_stat_light_update_in_area(x1, y1, x2, y2);
          stat_light_needs_updating = 1;
        }
        lgt->intensity = intensity;
        if ( *(unsigned short *)&lgt->field_1C[8] < intensity )
          lgt->flags |= LgtF_Unkn08;
      }
    }
    else
    {
      ERRORLOG("Attempt to set intensity of unallocated light structure");
    }
  }
  else
  {
    ERRORLOG("Attempt to set intensity of invalid light");
  }
}

void clear_stat_light_map(void)
{
    game.lish.field_46149 = 32;
    game.lish.field_4614D = 0;
    game.lish.field_4614F = 0;
    for (unsigned long y = 0; y < (map_subtiles_y + 1); y++)
    {
        for (unsigned long x = 0; x < (map_subtiles_x + 1); x++)
        {
            unsigned long i = get_subtile_number(x, y);
            game.lish.stat_light_map[i] = 0;
        }
    }
}

void light_delete_light(long idx)
{
    if ((idx <= 0) || (idx >= LIGHTS_COUNT)) {
        ERRORLOG("Attempt to delete light %d",(int)idx);
        return;
    }
    struct Light* lgt = &game.lish.lights[idx];
    if ((lgt->flags & LgtF_Allocated) == 0) {
        ERRORLOG("Attempt to delete unallocated light structure %d",(int)idx);
        return;
    }
    if (lgt->shadow_index > 0)
    {
        struct ShadowCache* shdc = &game.lish.shadow_cache[lgt->shadow_index];
        light_shadow_cache_free(shdc);
    }
    if ((lgt->flags & LgtF_Dynamic) != 0)
    {
        light_total_dynamic_lights--;
        light_remove_light_from_list(lgt, &game.thing_lists[TngList_DynamLights]);
    } else
    {
        light_total_stat_lights--;
        light_signal_stat_light_update_in_own_radius(lgt);
        light_remove_light_from_list(lgt, &game.thing_lists[TngList_StaticLights]);
    }
    light_free_light(lgt);
}

void light_initialise_lighting_tables(void)
{
  _DK_light_initialise_lighting_tables();
}

void light_initialise(void)
{
    int i;
    for (i=0; i < LIGHTS_COUNT; i++)
    {
        struct Light* lgt = &game.lish.lights[i];
        if ((lgt->flags & LgtF_Allocated) != 0)
            light_delete_light(lgt->index);
    }
    if (!game.lish.field_4614E)
    {
        light_initialise_lighting_tables();
        for (i=0; i < 32; i++) {
            light_bitmask[i] = 1 << (31-i);
        }
        game.lish.field_4614E = 1;
    }
    stat_light_needs_updating = 1;
    light_total_dynamic_lights = 0;
    light_total_stat_lights = 0;
    light_rendered_dynamic_lights = 0;
    light_rendered_optimised_dynamic_lights = 0;
    light_updated_stat_lights = 0;
    light_out_of_date_stat_lights = 0;
}

void light_stat_light_map_clear_area(long x1, long y1, long x2, long y2)
{
  _DK_light_stat_light_map_clear_area(x1, y1, x2, y2);
}

void light_set_lights_on(char state)
{
    if (state)
    {
        game.lish.field_46149 = 10;
        game.lish.field_4614D = 1;
    } else
    {
        game.lish.field_46149 = 32;
        game.lish.field_4614D = 0;
    }
    // Enable lights on all but bounding subtiles
    light_stat_light_map_clear_area(0, 0, map_subtiles_x, map_subtiles_y);
    light_signal_stat_light_update_in_area(1, 1, map_subtiles_x, map_subtiles_y);
}

void light_render_area(int startx, int starty, int endx, int endy)
{
  _DK_light_render_area(startx, starty, endx, endy);
}

void update_light_render_area(void)
{
    int subtile_x;
    int subtile_y;
    int startx;
    int starty;
    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_my_player();
    if (player->view_mode >= PVM_CreatureView)
      if ((player->view_mode <= PVM_IsometricView) || (player->view_mode == PVM_FrontView))
      {
          game.field_14BB5D = LIGHT_MAX_RANGE;
          game.field_14BB59 = LIGHT_MAX_RANGE;
      }
    int delta_x = abs(game.field_14BB59);
    int delta_y = abs(game.field_14BB5D);
    // Prepare the area constraints
    if (player->acamera != NULL)
    {
      subtile_y = player->acamera->mappos.y.stl.num;
      subtile_x = player->acamera->mappos.x.stl.num;
    } else
    {
      subtile_y = 0;
      subtile_x = 0;
    }
//SYNCMSG("LghtRng %d,%d CamTil %d,%d",game.field_14BB59,game.field_14BB5D,tile_x,tile_y);
    if (subtile_y > delta_y)
    {
      starty = subtile_y - delta_y;
      if (starty > map_subtiles_y) starty = map_subtiles_y;
    } else
      starty = 0;
    if (subtile_x > delta_x)
    {
      startx = subtile_x - delta_x;
      if (startx > map_subtiles_x) startx = map_subtiles_x;
    } else
      startx = 0;
    int endy = subtile_y + delta_y;
    if (endy < starty) endy = starty;
    if (endy > map_subtiles_y) endy = map_subtiles_y;
    int endx = subtile_x + delta_x;
    if (endx < startx) endx = startx;
    if (endx > map_subtiles_x) endx = map_subtiles_x;
    // Set the area
    light_render_area(startx, starty, endx, endy);
}

void light_set_light_minimum_size_to_cache(long a1, long a2, long a3)
{
  _DK_light_set_light_minimum_size_to_cache(a1, a2, a3);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
