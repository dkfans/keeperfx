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
#include "pre_inc.h"
#include "light_data.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_planar.h"

#include "engine_render.h"
#include "player_data.h"
#include "map_data.h"

#include "thing_stats.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static void light_stat_light_map_clear_area(MapSubtlCoord x1, MapSubtlCoord y1, MapSubtlCoord x2, MapSubtlCoord y2);
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
  if ((lgt->flags2 & 0x01) != 0)
  {
    ERRORLOG("Light is already in list");
    return false;
  }
  list->count++;
  lgt->flags2 |= 0x01;
  lgt->next_in_list = list->index;
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
    lgt->flags2 = k ^ ((k ^ lgt->flags2) & 0x01);
    set_flag_byte(&lgt->flags,LgtF_Dynamic,ilght->is_dynamic);
    lgt->field_1A = ilght->field_8;
    lgt->field_18 = ilght->field_4;
    lgt->field_12 = ilght->field_12;

    struct LightAdd* lightadd = get_lightadd(lgt->index);
    LbMemorySet(lightadd, 0, sizeof(struct LightAdd)); // Clear any previously used LightAdd stuff

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
    lgt->flags |= LgtF_NeverCached;
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

void set_previous_light_position(struct Light *light) {
    struct LightAdd* lightadd = get_lightadd(light->index);
    lightadd->previous_mappos = light->mappos;
}

void light_set_light_position(long lgt_id, struct Coord3d *pos)
{
  // _DK_light_set_light_position(lgt_id, pos);
  struct Light *lgt = &game.lish.lights[lgt_id];

  set_previous_light_position(lgt);

  if ( lgt->mappos.x.val != pos->x.val
    || pos->y.val != lgt->mappos.y.val
    || pos->z.val != lgt->mappos.z.val )
  {
    if ( (lgt->flags & LgtF_Dynamic) == 0 )
    {
      stat_light_needs_updating = 1;
      unsigned char range = lgt->range;
      long end_y = lgt->mappos.y.stl.num + range;
      long end_x = lgt->mappos.x.stl.num + range;
      if ( end_y > map_subtiles_y )
      {
        end_y = map_subtiles_y;
      }
      if ( end_x > map_subtiles_x )
      {
        end_x = map_subtiles_x;
      }
      long beg_y = lgt->mappos.y.stl.num - range;
      if ( beg_y < 0 )
      {
        beg_y = 0;
      }
      long beg_x = lgt->mappos.x.stl.num - range;
      if ( beg_x < 0 )
      {
        beg_x = 0;
      }
      light_signal_stat_light_update_in_area(beg_x, beg_y, end_x, end_y);
    }
    lgt->mappos.x.val = pos->x.val;
    lgt->mappos.y.val = pos->y.val;
    lgt->mappos.z.val = pos->z.val;
    lgt->flags |= LgtF_Unkn08;
  }
}

void light_remove_light_from_list(struct Light *lgt, struct StructureList *list)
{
  // _DK_light_remove_light_from_list(lgt, list);
  if ( list->count == 0 )
  {
      ERRORLOG("List %d has no structures", list->index);
      return;
  }
  TbBool Removed = false;
  struct Light *lgt2;
  struct Light *i;
  if ( lgt->flags2 & 1 )
  {
    if ( lgt->index == list->index )
    {
      Removed = true;
      list->count--;
      list->index = lgt->next_in_list;
      lgt->next_in_list = 0;
      lgt->flags2 &= ~1;
    }
    else
    {
      lgt2 = &game.lish.lights[list->index];
      for ( i = 0; lgt2 != game.lish.lights; lgt2 = &game.lish.lights[lgt2->next_in_list] )
      {
        if ( lgt2 == lgt )
        {
          Removed = true;
          if ( i )
          {
            i->next_in_list = lgt->next_in_list;
            lgt->flags2 &= ~1;
            list->count--;
            lgt->next_in_list = 0;
          }
          else
          {
            ERRORLOG("No prev when removing light from list");
          }
        }
        i = lgt2;
      }
    }
    if ( !Removed )
    {
      ERRORLOG("Could not find light %d in list", lgt->index);
    }
  }
}

void light_signal_stat_light_update_in_area(long x1, long y1, long x2, long y2)
{
  // _DK_light_signal_stat_light_update_in_area(x1, y1, x2, y2);
  int i = 0;
  struct Light *lgt = &game.lish.lights[1];
  do
  {
    if ( lgt->flags & LgtF_Allocated )
    {
      if ( !(lgt->flags & LgtF_Dynamic) )
      {
        unsigned char range = lgt->range;
        MapSubtlCoord x = lgt->mappos.x.stl.num;
        MapSubtlCoord y = lgt->mappos.y.stl.num;
        if ( range + x >= x1 && x - range <= x2 && range + y >= y1 && y - range <= y2 )
        {
          stat_light_needs_updating = 1;
          i++;
          lgt->flags |= LgtF_Unkn08;
          lgt->flags &= ~LgtF_Unkn80;
        }
      }
    }
    lgt++;
  }
  while ( lgt < (struct Light *)game.lish.shadow_cache );
  if ( i )
    light_stat_light_map_clear_area(x1, y1, x2, y2);
}

void light_signal_update_in_area(long sx, long sy, long ex, long ey)
{
   // _DK_light_signal_update_in_area(sx, sy, ex, ey);
  struct Light *lgt = &game.lish.lights[1];
  do
  {
    if ( lgt->flags & LgtF_Allocated )
    {
      if ( lgt->flags & LgtF_Dynamic )
      {
        unsigned char range = lgt->range;;
        MapSubtlCoord x = lgt->mappos.x.stl.num;
        MapSubtlCoord y = lgt->mappos.y.stl.num;
        if ( range + x >= sx && x - range <= ex && range + y >= sy && y - range <= ey )
          lgt->flags |= LgtF_Unkn08;
      }
    }
    lgt++;
  }
  while ( lgt < (struct Light *)game.lish.shadow_cache );
  light_signal_stat_light_update_in_area(sx, sy, ex, ey);
}

void light_signal_stat_light_update_in_own_radius(struct Light *lgt)
{
    long radius = lgt->range;
    long end_y = (long)lgt->mappos.y.stl.num + radius;
    if (end_y >= map_subtiles_y)
        end_y = map_subtiles_y;
    long end_x = (long)lgt->mappos.x.stl.num + radius;
    if (end_x >= map_subtiles_x)
        end_x = map_subtiles_x;
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
          if ( y2 > map_subtiles_y )
            y2 = map_subtiles_y;
          x2 = lgt->mappos.x.stl.num + lgt->range;
          if ( x2 > map_subtiles_x )
            x2 = map_subtiles_x;
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
        if ( lgt->min_intensity < intensity )
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
    game.lish.light_enabled = 0;
    game.lish.light_rand_seed = 0;
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
  static const struct LightingTable values[] = {
    { 1, 2, 0, -1, 256 },
    { 1, 2, 1, 0, 256 },
    { 1, 2, 0, 1, 256 },
    { 1, 2, -1, 0, 256 },
    { 1, 2, 1, -1, 362 },
    { 1, 2, 1, 1, 362 },
    { 1, 2, -1, 1, 362 },
    { 1, 2, -1, -1, 362 },
    { 1, 3, 0, -2, 512 },
    { 1, 3, 2, 0, 512 },
    { 1, 3, 0, 2, 512 },
    { 1, 3, -2, 0, 512 },
    { 1, 3, 1, -2, 572 },
    { 1, 3, 2, -1, 572 },
    { 1, 3, 2, 1, 572 },
    { 1, 3, 1, 2, 572 },
    { 1, 3, -1, 2, 572 },
    { 1, 3, -2, 1, 572 },
    { 1, 3, -2, -1, 572 },
    { 1, 3, -1, -2, 572 },
    { 1, 4, 2, -2, 724 },
    { 1, 4, 2, 2, 724 },
    { 1, 4, -2, 2, 724 },
    { 1, 4, -2, -2, 724 },
    { 1, 4, 0, -3, 768 },
    { 1, 4, 3, 0, 768 },
    { 1, 4, 0, 3, 768 },
    { 1, 4, -3, 0, 768 },
    { 1, 4, 1, -3, 809 },
    { 1, 4, 3, -1, 809 },
    { 1, 4, 3, 1, 809 },
    { 1, 4, 1, 3, 809 },
    { 1, 4, -1, 3, 809 },
    { 1, 4, -3, 1, 809 },
    { 1, 4, -3, -1, 809 },
    { 1, 4, -1, -3, 809 },
    { 1, 4, 2, -3, 921 },
    { 1, 4, 3, -2, 921 },
    { 1, 4, 3, 2, 921 },
    { 1, 4, 2, 3, 921 },
    { 1, 4, -2, 3, 921 },
    { 1, 4, -3, 2, 921 },
    { 1, 4, -3, -2, 921 },
    { 1, 4, -2, -3, 921 },
    { 1, 5, 0, -4, 1024 },
    { 1, 5, 4, 0, 1024 },
    { 1, 5, 0, 4, 1024 },
    { 1, 5, -4, 0, 1024 },
    { 1, 5, 1, -4, 1055 },
    { 1, 5, 4, -1, 1055 },
    { 1, 5, 4, 1, 1055 },
    { 1, 5, 1, 4, 1055 },
    { 1, 5, -1, 4, 1055 },
    { 1, 5, -4, 1, 1055 },
    { 1, 5, -4, -1, 1055 },
    { 1, 5, -1, -4, 1055 },
    { 1, 5, 3, -3, 1086 },
    { 1, 5, 3, 3, 1086 },
    { 1, 5, -3, 3, 1086 },
    { 1, 5, -3, -3, 1086 },
    { 1, 5, 2, -4, 1144 },
    { 1, 5, 4, -2, 1144 },
    { 1, 5, 4, 2, 1144 },
    { 1, 5, 2, 4, 1144 },
    { 1, 5, -2, 4, 1144 },
    { 1, 5, -4, 2, 1144 },
    { 1, 5, -4, -2, 1144 },
    { 1, 5, -2, -4, 1144 },
    { 1, 6, 0, -5, 1280 },
    { 1, 6, 3, -4, 1280 },
    { 1, 6, 4, -3, 1280 },
    { 1, 6, 5, 0, 1280 },
    { 1, 6, 4, 3, 1280 },
    { 1, 6, 3, 4, 1280 },
    { 1, 6, 0, 5, 1280 },
    { 1, 6, -3, 4, 1280 },
    { 1, 6, -4, 3, 1280 },
    { 1, 6, -5, 0, 1280 },
    { 1, 6, -4, -3, 1280 },
    { 1, 6, -3, -4, 1280 },
    { 1, 6, 1, -5, 1305 },
    { 1, 6, 5, -1, 1305 },
    { 1, 6, 5, 1, 1305 },
    { 1, 6, 1, 5, 1305 },
    { 1, 6, -1, 5, 1305 },
    { 1, 6, -5, 1, 1305 },
    { 1, 6, -5, -1, 1305 },
    { 1, 6, -1, -5, 1305 },
    { 1, 6, 2, -5, 1377 },
    { 1, 6, 5, -2, 1377 },
    { 1, 6, 5, 2, 1377 },
    { 1, 6, 2, 5, 1377 },
    { 1, 6, -2, 5, 1377 },
    { 1, 6, -5, 2, 1377 },
    { 1, 6, -5, -2, 1377 },
    { 1, 6, -2, -5, 1377 },
    { 1, 7, 4, -4, 1448 },
    { 1, 7, 4, 4, 1448 },
    { 1, 7, -4, 4, 1448 },
    { 1, 7, -4, -4, 1448 },
    { 1, 7, 3, -5, 1491 },
    { 1, 7, 5, -3, 1491 },
    { 1, 7, 5, 3, 1491 },
    { 1, 7, 3, 5, 1491 },
    { 1, 7, -3, 5, 1491 },
    { 1, 7, -5, 3, 1491 },
    { 1, 7, -5, -3, 1491 },
    { 1, 7, -3, -5, 1491 },
    { 1, 7, 0, -6, 1536 },
    { 1, 7, 6, 0, 1536 },
    { 1, 7, 0, 6, 1536 },
    { 1, 7, -6, 0, 1536 },
    { 1, 7, 1, -6, 1556 },
    { 1, 7, 6, -1, 1556 },
    { 1, 7, 6, 1, 1556 },
    { 1, 7, 1, 6, 1556 },
    { 1, 7, -1, 6, 1556 },
    { 1, 7, -6, 1, 1556 },
    { 1, 7, -6, -1, 1556 },
    { 1, 7, -1, -6, 1556 },
    { 1, 7, 2, -6, 1618 },
    { 1, 7, 6, -2, 1618 },
    { 1, 7, 6, 2, 1618 },
    { 1, 7, 2, 6, 1618 },
    { 1, 7, -2, 6, 1618 },
    { 1, 7, -6, 2, 1618 },
    { 1, 7, -6, -2, 1618 },
    { 1, 7, -2, -6, 1618 },
    { 1, 7, 4, -5, 1636 },
    { 1, 7, 5, -4, 1636 },
    { 1, 7, 5, 4, 1636 },
    { 1, 7, 4, 5, 1636 },
    { 1, 7, -4, 5, 1636 },
    { 1, 7, -5, 4, 1636 },
    { 1, 7, -5, -4, 1636 },
    { 1, 7, -4, -5, 1636 },
    { 1, 8, 3, -6, 1717 },
    { 1, 8, 6, -3, 1717 },
    { 1, 8, 6, 3, 1717 },
    { 1, 8, 3, 6, 1717 },
    { 1, 8, -3, 6, 1717 },
    { 1, 8, -6, 3, 1717 },
    { 1, 8, -6, -3, 1717 },
    { 1, 8, -3, -6, 1717 },
    { 1, 8, 0, -7, 1792 },
    { 1, 8, 7, 0, 1792 },
    { 1, 8, 0, 7, 1792 },
    { 1, 8, -7, 0, 1792 },
    { 1, 8, 1, -7, 1809 },
    { 1, 8, 7, -1, 1809 },
    { 1, 8, 7, 1, 1809 },
    { 1, 8, 1, 7, 1809 },
    { 1, 8, -1, 7, 1809 },
    { 1, 8, -7, 1, 1809 },
    { 1, 8, -7, -1, 1809 },
    { 1, 8, -1, -7, 1809 },
    { 1, 8, 5, -5, 1810 },
    { 1, 8, 5, 5, 1810 },
    { 1, 8, -5, 5, 1810 },
    { 1, 8, -5, -5, 1810 },
    { 1, 8, 4, -6, 1843 },
    { 1, 8, 6, -4, 1843 },
    { 1, 8, 6, 4, 1843 },
    { 1, 8, 4, 6, 1843 },
    { 1, 8, -4, 6, 1843 },
    { 1, 8, -6, 4, 1843 },
    { 1, 8, -6, -4, 1843 },
    { 1, 8, -4, -6, 1843 },
    { 1, 8, 2, -7, 1863 },
    { 1, 8, 7, -2, 1863 },
    { 1, 8, 7, 2, 1863 },
    { 1, 8, 2, 7, 1863 },
    { 1, 8, -2, 7, 1863 },
    { 1, 8, -7, 2, 1863 },
    { 1, 8, -7, -2, 1863 },
    { 1, 8, -2, -7, 1863 },
    { 1, 8, 3, -7, 1947 },
    { 1, 8, 7, -3, 1947 },
    { 1, 8, 7, 3, 1947 },
    { 1, 8, 3, 7, 1947 },
    { 1, 8, -3, 7, 1947 },
    { 1, 8, -7, 3, 1947 },
    { 1, 8, -7, -3, 1947 },
    { 1, 8, -3, -7, 1947 },
    { 1, 9, 5, -6, 1998 },
    { 1, 9, 6, -5, 1998 },
    { 1, 9, 6, 5, 1998 },
    { 1, 9, 5, 6, 1998 },
    { 1, 9, -5, 6, 1998 },
    { 1, 9, -6, 5, 1998 },
    { 1, 9, -6, -5, 1998 },
    { 1, 9, -5, -6, 1998 },
    { 1, 9, 0, -8, 2048 },
    { 1, 9, 8, 0, 2048 },
    { 1, 9, 0, 8, 2048 },
    { 1, 9, -8, 0, 2048 },
    { 1, 9, 4, -7, 2063 },
    { 1, 9, 7, -4, 2063 },
    { 1, 9, 7, 4, 2063 },
    { 1, 9, 4, 7, 2063 },
    { 1, 9, -4, 7, 2063 },
    { 1, 9, -7, 4, 2063 },
    { 1, 9, -7, -4, 2063 },
    { 1, 9, -4, -7, 2063 },
    { 1, 9, 1, -8, 2064 },
    { 1, 9, 8, -1, 2064 },
    { 1, 9, 8, 1, 2064 },
    { 1, 9, 1, 8, 2064 },
    { 1, 9, -1, 8, 2064 },
    { 1, 9, -8, 1, 2064 },
    { 1, 9, -8, -1, 2064 },
    { 1, 9, -1, -8, 2064 },
    { 1, 9, 2, -8, 2111 },
    { 1, 9, 8, -2, 2111 },
    { 1, 9, 8, 2, 2111 },
    { 1, 9, 2, 8, 2111 },
    { 1, 9, -2, 8, 2111 },
    { 1, 9, -8, 2, 2111 },
    { 1, 9, -8, -2, 2111 },
    { 1, 9, -2, -8, 2111 },
    { 1, 9, 6, -6, 2172 },
    { 1, 9, 6, 6, 2172 },
    { 1, 9, -6, 6, 2172 },
    { 1, 9, -6, -6, 2172 },
    { 1, 9, 3, -8, 2187 },
    { 1, 9, 8, -3, 2187 },
    { 1, 9, 8, 3, 2187 },
    { 1, 9, 3, 8, 2187 },
    { 1, 9, -3, 8, 2187 },
    { 1, 9, -8, 3, 2187 },
    { 1, 9, -8, -3, 2187 },
    { 1, 9, -3, -8, 2187 },
    { 1, 9, 5, -7, 2198 },
    { 1, 9, 7, -5, 2198 },
    { 1, 9, 7, 5, 2198 },
    { 1, 9, 5, 7, 2198 },
    { 1, 9, -5, 7, 2198 },
    { 1, 9, -7, 5, 2198 },
    { 1, 9, -7, -5, 2198 },
    { 1, 9, -5, -7, 2198 },
    { 1, 10, 4, -8, 2289 },
    { 1, 10, 8, -4, 2289 },
    { 1, 10, 8, 4, 2289 },
    { 1, 10, 4, 8, 2289 },
    { 1, 10, -4, 8, 2289 },
    { 1, 10, -8, 4, 2289 },
    { 1, 10, -8, -4, 2289 },
    { 1, 10, -4, -8, 2289 },
    { 1, 10, 0, -9, 2304 },
    { 1, 10, 9, 0, 2304 },
    { 1, 10, 0, 9, 2304 },
    { 1, 10, -9, 0, 2304 },
    { 1, 10, 1, -9, 2317 },
    { 1, 10, 9, -1, 2317 },
    { 1, 10, 9, 1, 2317 },
    { 1, 10, 1, 9, 2317 },
    { 1, 10, -1, 9, 2317 },
    { 1, 10, -9, 1, 2317 },
    { 1, 10, -9, -1, 2317 },
    { 1, 10, -1, -9, 2317 },
    { 1, 10, 2, -9, 2358 },
    { 1, 10, 6, -7, 2358 },
    { 1, 10, 7, -6, 2358 },
    { 1, 10, 9, -2, 2358 },
    { 1, 10, 9, 2, 2358 },
    { 1, 10, 7, 6, 2358 },
    { 1, 10, 6, 7, 2358 },
    { 1, 10, 2, 9, 2358 },
    { 1, 10, -2, 9, 2358 },
    { 1, 10, -6, 7, 2358 },
    { 1, 10, -7, 6, 2358 },
    { 1, 10, -9, 2, 2358 },
    { 1, 10, -9, -2, 2358 },
    { 1, 10, -7, -6, 2358 },
    { 1, 10, -6, -7, 2358 },
    { 1, 10, -2, -9, 2358 },
    { 1, 10, 5, -8, 2415 },
    { 1, 10, 8, -5, 2415 },
    { 1, 10, 8, 5, 2415 },
    { 1, 10, 5, 8, 2415 },
    { 1, 10, -5, 8, 2415 },
    { 1, 10, -8, 5, 2415 },
    { 1, 10, -8, -5, 2415 },
    { 1, 10, -5, -8, 2415 },
    { 1, 10, 3, -9, 2427 },
    { 1, 10, 9, -3, 2427 },
    { 1, 10, 9, 3, 2427 },
    { 1, 10, 3, 9, 2427 },
    { 1, 10, -3, 9, 2427 },
    { 1, 10, -9, 3, 2427 },
    { 1, 10, -9, -3, 2427 },
    { 1, 10, -3, -9, 2427 },
    { 1, 11, 4, -9, 2518 },
    { 1, 11, 9, -4, 2518 },
    { 1, 11, 9, 4, 2518 },
    { 1, 11, 4, 9, 2518 },
    { 1, 11, -4, 9, 2518 },
    { 1, 11, -9, 4, 2518 },
    { 1, 11, -9, -4, 2518 },
    { 1, 11, -4, -9, 2518 },
    { 1, 11, 7, -7, 2534 },
    { 1, 11, 7, 7, 2534 },
    { 1, 11, -7, 7, 2534 },
    { 1, 11, -7, -7, 2534 },
    { 1, 11, 0, -10, 2560 },
    { 1, 11, 6, -8, 2560 },
    { 1, 11, 8, -6, 2560 },
    { 1, 11, 10, 0, 2560 },
    { 1, 11, 8, 6, 2560 },
    { 1, 11, 6, 8, 2560 },
    { 1, 11, 0, 10, 2560 },
    { 1, 11, -6, 8, 2560 },
    { 1, 11, -8, 6, 2560 },
    { 1, 11, -10, 0, 2560 },
    { 1, 11, -8, -6, 2560 },
    { 1, 11, -6, -8, 2560 },
    { 1, 11, 1, -10, 2572 },
    { 1, 11, 10, -1, 2572 },
    { 1, 11, 10, 1, 2572 },
    { 1, 11, 1, 10, 2572 },
    { 1, 11, -1, 10, 2572 },
    { 1, 11, -10, 1, 2572 },
    { 1, 11, -10, -1, 2572 },
    { 1, 11, -1, -10, 2572 },
    { 1, 11, 2, -10, 2610 },
    { 1, 11, 10, -2, 2610 },
    { 1, 11, 10, 2, 2610 },
    { 1, 11, 2, 10, 2610 },
    { 1, 11, -2, 10, 2610 },
    { 1, 11, -10, 2, 2610 },
    { 1, 11, -10, -2, 2610 },
    { 1, 11, -2, -10, 2610 },
    { 1, 11, 5, -9, 2634 },
    { 1, 11, 9, -5, 2634 },
    { 1, 11, 9, 5, 2634 },
    { 1, 11, 5, 9, 2634 },
    { 1, 11, -5, 9, 2634 },
    { 1, 11, -9, 5, 2634 },
    { 1, 11, -9, -5, 2634 },
    { 1, 11, -5, -9, 2634 },
    { 1, 11, 3, -10, 2670 },
    { 1, 11, 10, -3, 2670 },
    { 1, 11, 10, 3, 2670 },
    { 1, 11, 3, 10, 2670 },
    { 1, 11, -3, 10, 2670 },
    { 1, 11, -10, 3, 2670 },
    { 1, 11, -10, -3, 2670 },
    { 1, 11, -3, -10, 2670 },
    { 1, 12, 7, -8, 2721 },
    { 1, 12, 8, -7, 2721 },
    { 1, 12, 8, 7, 2721 },
    { 1, 12, 7, 8, 2721 },
    { 1, 12, -7, 8, 2721 },
    { 1, 12, -8, 7, 2721 },
    { 1, 12, -8, -7, 2721 },
    { 1, 12, -7, -8, 2721 },
    { 1, 12, 4, -10, 2755 },
    { 1, 12, 10, -4, 2755 },
    { 1, 12, 10, 4, 2755 },
    { 1, 12, 4, 10, 2755 },
    { 1, 12, -4, 10, 2755 },
    { 1, 12, -10, 4, 2755 },
    { 1, 12, -10, -4, 2755 },
    { 1, 12, -4, -10, 2755 },
    { 1, 12, 6, -9, 2765 },
    { 1, 12, 9, -6, 2765 },
    { 1, 12, 9, 6, 2765 },
    { 1, 12, 6, 9, 2765 },
    { 1, 12, -6, 9, 2765 },
    { 1, 12, -9, 6, 2765 },
    { 1, 12, -9, -6, 2765 },
    { 1, 12, -6, -9, 2765 },
    { 1, 12, 0, -11, 2816 },
    { 1, 12, 11, 0, 2816 },
    { 1, 12, 0, 11, 2816 },
    { 1, 12, -11, 0, 2816 },
    { 1, 12, 1, -11, 2827 },
    { 1, 12, 11, -1, 2827 },
    { 1, 12, 11, 1, 2827 },
    { 1, 12, 1, 11, 2827 },
    { 1, 12, -1, 11, 2827 },
    { 1, 12, -11, 1, 2827 },
    { 1, 12, -11, -1, 2827 },
    { 1, 12, -1, -11, 2827 },
    { 1, 12, 2, -11, 2861 },
    { 1, 12, 11, -2, 2861 },
    { 1, 12, 11, 2, 2861 },
    { 1, 12, 2, 11, 2861 },
    { 1, 12, -2, 11, 2861 },
    { 1, 12, -11, 2, 2861 },
    { 1, 12, -11, -2, 2861 },
    { 1, 12, -2, -11, 2861 },
    { 1, 12, 5, -10, 2862 },
    { 1, 12, 10, -5, 2862 },
    { 1, 12, 10, 5, 2862 },
    { 1, 12, 5, 10, 2862 },
    { 1, 12, -5, 10, 2862 },
    { 1, 12, -10, 5, 2862 },
    { 1, 12, -10, -5, 2862 },
    { 1, 12, -5, -10, 2862 },
    { 1, 12, 8, -8, 2896 },
    { 1, 12, 8, 8, 2896 },
    { 1, 12, -8, 8, 2896 },
    { 1, 12, -8, -8, 2896 },
    { 1, 12, 3, -11, 2916 },
    { 1, 12, 11, -3, 2916 },
    { 1, 12, 11, 3, 2916 },
    { 1, 12, 3, 11, 2916 },
    { 1, 12, -3, 11, 2916 },
    { 1, 12, -11, 3, 2916 },
    { 1, 12, -11, -3, 2916 },
    { 1, 12, -3, -11, 2916 },
    { 1, 12, 7, -9, 2918 },
    { 1, 12, 9, -7, 2918 },
    { 1, 12, 9, 7, 2918 },
    { 1, 12, 7, 9, 2918 },
    { 1, 12, -7, 9, 2918 },
    { 1, 12, -9, 7, 2918 },
    { 1, 12, -9, -7, 2918 },
    { 1, 12, -7, -9, 2918 },
    { 1, 13, 6, -10, 2982 },
    { 1, 13, 10, -6, 2982 },
    { 1, 13, 10, 6, 2982 },
    { 1, 13, 6, 10, 2982 },
    { 1, 13, -6, 10, 2982 },
    { 1, 13, -10, 6, 2982 },
    { 1, 13, -10, -6, 2982 },
    { 1, 13, -6, -10, 2982 },
    { 1, 12, 4, -11, 2996 },
    { 1, 12, 11, -4, 2996 },
    { 1, 12, 11, 4, 2996 },
    { 1, 12, 4, 11, 2996 },
    { 1, 12, -4, 11, 2996 },
    { 1, 12, -11, 4, 2996 },
    { 1, 12, -11, -4, 2996 },
    { 1, 12, -4, -11, 2996 },
    { 1, 13, 0, -12, 3072 },
    { 1, 13, 12, 0, 3072 },
    { 1, 13, 0, 12, 3072 },
    { 1, 13, -12, 0, 3072 },
    { 1, 13, 8, -9, 3079 },
    { 1, 13, 9, -8, 3079 },
    { 1, 13, 9, 8, 3079 },
    { 1, 13, 8, 9, 3079 },
    { 1, 13, -8, 9, 3079 },
    { 1, 13, -9, 8, 3079 },
    { 1, 13, -9, -8, 3079 },
    { 1, 13, -8, -9, 3079 },
    { 1, 13, 1, -12, 3082 },
    { 1, 13, 12, -1, 3082 },
    { 1, 13, 12, 1, 3082 },
    { 1, 13, 1, 12, 3082 },
    { 1, 13, -1, 12, 3082 },
    { 1, 13, -12, 1, 3082 },
    { 1, 13, -12, -1, 3082 },
    { 1, 13, -1, -12, 3082 },
    { 1, 13, 5, -11, 3091 },
    { 1, 13, 11, -5, 3091 },
    { 1, 13, 11, 5, 3091 },
    { 1, 13, 5, 11, 3091 },
    { 1, 13, -5, 11, 3091 },
    { 1, 13, -11, 5, 3091 },
    { 1, 13, -11, -5, 3091 },
    { 1, 13, -5, -11, 3091 },
    { 1, 13, 2, -12, 3113 },
    { 1, 13, 12, -2, 3113 },
    { 1, 13, 12, 2, 3113 },
    { 1, 13, 2, 12, 3113 },
    { 1, 13, -2, 12, 3113 },
    { 1, 13, -12, 2, 3113 },
    { 1, 13, -12, -2, 3113 },
    { 1, 13, -2, -12, 3113 },
    { 1, 13, 7, -10, 3123 },
    { 1, 13, 10, -7, 3123 },
    { 1, 13, 10, 7, 3123 },
    { 1, 13, 7, 10, 3123 },
    { 1, 13, -7, 10, 3123 },
    { 1, 13, -10, 7, 3123 },
    { 1, 13, -10, -7, 3123 },
    { 1, 13, -7, -10, 3123 },
    { 1, 13, 3, -12, 3166 },
    { 1, 13, 12, -3, 3166 },
    { 1, 13, 12, 3, 3166 },
    { 1, 13, 3, 12, 3166 },
    { 1, 13, -3, 12, 3166 },
    { 1, 13, -12, 3, 3166 },
    { 1, 13, -12, -3, 3166 },
    { 1, 13, -3, -12, 3166 },
    { 1, 13, 6, -11, 3204 },
    { 1, 13, 11, -6, 3204 },
    { 1, 13, 11, 6, 3204 },
    { 1, 13, 6, 11, 3204 },
    { 1, 13, -6, 11, 3204 },
    { 1, 13, -11, 6, 3204 },
    { 1, 13, -11, -6, 3204 },
    { 1, 13, -6, -11, 3204 },
    { 1, 13, 4, -12, 3237 },
    { 1, 13, 12, -4, 3237 },
    { 1, 13, 12, 4, 3237 },
    { 1, 13, 4, 12, 3237 },
    { 1, 13, -4, 12, 3237 },
    { 1, 13, -12, 4, 3237 },
    { 1, 13, -12, -4, 3237 },
    { 1, 13, -4, -12, 3237 },
    { 1, 14, 9, -9, 3258 },
    { 1, 14, 9, 9, 3258 },
    { 1, 14, -9, 9, 3258 },
    { 1, 14, -9, -9, 3258 },
    { 1, 14, 8, -10, 3273 },
    { 1, 14, 10, -8, 3273 },
    { 1, 14, 10, 8, 3273 },
    { 1, 14, 8, 10, 3273 },
    { 1, 14, -8, 10, 3273 },
    { 1, 14, -10, 8, 3273 },
    { 1, 14, -10, -8, 3273 },
    { 1, 14, -8, -10, 3273 },
    { 1, 14, 5, -12, 3324 },
    { 1, 14, 12, -5, 3324 },
    { 1, 14, 12, 5, 3324 },
    { 1, 14, 5, 12, 3324 },
    { 1, 14, -5, 12, 3324 },
    { 1, 14, -12, 5, 3324 },
    { 1, 14, -12, -5, 3324 },
    { 1, 14, -5, -12, 3324 },
    { 1, 14, 0, -13, 3328 },
    { 1, 14, 13, 0, 3328 },
    { 1, 14, 0, 13, 3328 },
    { 1, 14, -13, 0, 3328 },
    { 1, 14, 7, -11, 3332 },
    { 1, 14, 11, -7, 3332 },
    { 1, 14, 11, 7, 3332 },
    { 1, 14, 7, 11, 3332 },
    { 1, 14, -7, 11, 3332 },
    { 1, 14, -11, 7, 3332 },
    { 1, 14, -11, -7, 3332 },
    { 1, 14, -7, -11, 3332 },
    { 1, 14, 1, -13, 3337 },
    { 1, 14, 13, -1, 3337 },
    { 1, 14, 13, 1, 3337 },
    { 1, 14, 1, 13, 3337 },
    { 1, 14, -1, 13, 3337 },
    { 1, 14, -13, 1, 3337 },
    { 1, 14, -13, -1, 3337 },
    { 1, 14, -1, -13, 3337 },
    { 1, 14, 2, -13, 3366 },
    { 1, 14, 13, -2, 3366 },
    { 1, 14, 13, 2, 3366 },
    { 1, 14, 2, 13, 3366 },
    { 1, 14, -2, 13, 3366 },
    { 1, 14, -13, 2, 3366 },
    { 1, 14, -13, -2, 3366 },
    { 1, 14, -2, -13, 3366 },
    { 1, 14, 3, -13, 3415 },
    { 1, 14, 13, -3, 3415 },
    { 1, 14, 13, 3, 3415 },
    { 1, 14, 3, 13, 3415 },
    { 1, 14, -3, 13, 3415 },
    { 1, 14, -13, 3, 3415 },
    { 1, 14, -13, -3, 3415 },
    { 1, 14, -3, -13, 3415 },
    { 1, 14, 6, -12, 3434 },
    { 1, 14, 12, -6, 3434 },
    { 1, 14, 12, 6, 3434 },
    { 1, 14, 6, 12, 3434 },
    { 1, 14, -6, 12, 3434 },
    { 1, 14, -12, 6, 3434 },
    { 1, 14, -12, -6, 3434 },
    { 1, 14, -6, -12, 3434 },
    { 1, 14, 9, -10, 3441 },
    { 1, 14, 10, -9, 3441 },
    { 1, 14, 10, 9, 3441 },
    { 1, 14, 9, 10, 3441 },
    { 1, 14, -9, 10, 3441 },
    { 1, 14, -10, 9, 3441 },
    { 1, 14, -10, -9, 3441 },
    { 1, 14, -9, -10, 3441 },
    { 1, 14, 4, -13, 3479 },
    { 1, 14, 13, -4, 3479 },
    { 1, 14, 13, 4, 3479 },
    { 1, 14, 4, 13, 3479 },
    { 1, 14, -4, 13, 3479 },
    { 1, 14, -13, 4, 3479 },
    { 1, 14, -13, -4, 3479 },
    { 1, 14, -4, -13, 3479 },
    { 1, 14, 8, -11, 3480 },
    { 1, 14, 11, -8, 3480 },
    { 1, 14, 11, 8, 3480 },
    { 1, 14, 8, 11, 3480 },
    { 1, 14, -8, 11, 3480 },
    { 1, 14, -11, 8, 3480 },
    { 1, 14, -11, -8, 3480 },
    { 1, 14, -8, -11, 3480 },
    { 1, 15, 7, -12, 3554 },
    { 1, 15, 12, -7, 3554 },
    { 1, 15, 12, 7, 3554 },
    { 1, 15, 7, 12, 3554 },
    { 1, 15, -7, 12, 3554 },
    { 1, 15, -12, 7, 3554 },
    { 1, 15, -12, -7, 3554 },
    { 1, 15, -7, -12, 3554 },
    { 1, 15, 5, -13, 3563 },
    { 1, 15, 13, -5, 3563 },
    { 1, 15, 13, 5, 3563 },
    { 1, 15, 5, 13, 3563 },
    { 1, 15, -5, 13, 3563 },
    { 1, 15, -13, 5, 3563 },
    { 1, 15, -13, -5, 3563 },
    { 1, 15, -5, -13, 3563 },
    { 1, 15, 0, -14, 3584 },
    { 1, 15, 14, 0, 3584 },
    { 1, 15, 0, 14, 3584 },
    { 1, 15, -14, 0, 3584 },
    { 1, 15, 1, -14, 3592 },
    { 1, 15, 14, -1, 3592 },
    { 1, 15, 14, 1, 3592 },
    { 1, 15, 1, 14, 3592 },
    { 1, 15, -1, 14, 3592 },
    { 1, 15, -14, 1, 3592 },
    { 1, 15, -14, -1, 3592 },
    { 1, 15, -1, -14, 3592 },
    { 1, 15, 2, -14, 3619 },
    { 1, 15, 14, -2, 3619 },
    { 1, 15, 14, 2, 3619 },
    { 1, 15, 2, 14, 3619 },
    { 1, 15, -2, 14, 3619 },
    { 1, 15, -14, 2, 3619 },
    { 1, 15, -14, -2, 3619 },
    { 1, 15, -2, -14, 3619 },
    { 1, 15, 10, -10, 3620 },
    { 1, 15, 10, 10, 3620 },
    { 1, 15, -10, 10, 3620 },
    { 1, 15, -10, -10, 3620 },
    { 1, 15, 9, -11, 3635 },
    { 1, 15, 11, -9, 3635 },
    { 1, 15, 11, 9, 3635 },
    { 1, 15, 9, 11, 3635 },
    { 1, 15, -9, 11, 3635 },
    { 1, 15, -11, 9, 3635 },
    { 1, 15, -11, -9, 3635 },
    { 1, 15, -9, -11, 3635 },
    { 1, 15, 3, -14, 3662 },
    { 1, 15, 14, -3, 3662 },
    { 1, 15, 14, 3, 3662 },
    { 1, 15, 3, 14, 3662 },
    { 1, 15, -3, 14, 3662 },
    { 1, 15, -14, 3, 3662 },
    { 1, 15, -14, -3, 3662 },
    { 1, 15, -3, -14, 3662 },
    { 1, 15, 6, -13, 3664 },
    { 1, 15, 13, -6, 3664 },
    { 1, 15, 13, 6, 3664 },
    { 1, 15, 6, 13, 3664 },
    { 1, 15, -6, 13, 3664 },
    { 1, 15, -13, 6, 3664 },
    { 1, 15, -13, -6, 3664 },
    { 1, 15, -6, -13, 3664 },
    { 1, 15, 8, -12, 3687 },
    { 1, 15, 12, -8, 3687 },
    { 1, 15, 12, 8, 3687 },
    { 1, 15, 8, 12, 3687 },
    { 1, 15, -8, 12, 3687 },
    { 1, 15, -12, 8, 3687 },
    { 1, 15, -12, -8, 3687 },
    { 1, 15, -8, -12, 3687 },
    { 1, 15, 4, -14, 3727 },
    { 1, 15, 14, -4, 3727 },
    { 1, 15, 14, 4, 3727 },
    { 1, 15, 4, 14, 3727 },
    { 1, 15, -4, 14, 3727 },
    { 1, 15, -14, 4, 3727 },
    { 1, 15, -14, -4, 3727 },
    { 1, 15, -4, -14, 3727 },
    { 1, 15, 7, -13, 3774 },
    { 1, 15, 13, -7, 3774 },
    { 1, 15, 13, 7, 3774 },
    { 1, 15, 7, 13, 3774 },
    { 1, 15, -7, 13, 3774 },
    { 1, 15, -13, 7, 3774 },
    { 1, 15, -13, -7, 3774 },
    { 1, 15, -7, -13, 3774 },
    { 1, 15, 10, -11, 3800 },
    { 1, 15, 11, -10, 3800 },
    { 1, 15, 11, 10, 3800 },
    { 1, 15, 10, 11, 3800 },
    { 1, 15, -10, 11, 3800 },
    { 1, 15, -11, 10, 3800 },
    { 1, 15, -11, -10, 3800 },
    { 1, 15, -10, -11, 3800 },
    { 1, 15, 5, -14, 3803 },
    { 1, 15, 14, -5, 3803 },
    { 1, 15, 14, 5, 3803 },
    { 1, 15, 5, 14, 3803 },
    { 1, 15, -5, 14, 3803 },
    { 1, 15, -14, 5, 3803 },
    { 1, 15, -14, -5, 3803 },
    { 1, 15, -5, -14, 3803 },
    { 1, 15, 0, -15, 3840 },
    { 1, 15, 15, 0, 3840 },
    { 1, 15, 0, 15, 3840 },
    { 1, 15, -15, 0, 3840 },
  };

  memcpy(game.lish.lighting_tables, values, sizeof(values));
  game.lish.lighting_tables_count = sizeof(values) / sizeof(*values);
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
    if (!game.lish.lighting_tables_initialised)
    {
        light_initialise_lighting_tables();
        for (i=0; i < 32; i++) {
            light_bitmask[i] = 1 << (31-i);
        }
        game.lish.lighting_tables_initialised = true;
    }
    stat_light_needs_updating = 1;
    light_total_dynamic_lights = 0;
    light_total_stat_lights = 0;
    light_rendered_dynamic_lights = 0;
    light_rendered_optimised_dynamic_lights = 0;
    light_updated_stat_lights = 0;
    light_out_of_date_stat_lights = 0;
}

static void light_stat_light_map_clear_area(MapSubtlCoord start_stl_x, MapSubtlCoord start_stl_y, MapSubtlCoord end_stl_x, MapSubtlCoord end_stl_y)
{
  MapSubtlCoord stl_x,stl_y_min_1,stl_x_min_1,stl_y;
  unsigned short *light_map;
  if ( end_stl_y >= start_stl_y )
  {
    struct Map *Mapblk1 = get_map_block_at(start_stl_x,start_stl_y);
    for (stl_y = start_stl_y; stl_y <= end_stl_y; stl_y++)
    {
      if ( end_stl_x >= start_stl_x )
      {
        stl_y_min_1 = stl_y - 1;
        if ( stl_y - 1 <= 0 )
        {
          stl_y_min_1 = 0;
        }
        struct Map *Mapblk2 = get_map_block_at(start_stl_x,stl_y_min_1);
        for (stl_x = start_stl_x; stl_x <= end_stl_x; stl_x++)
        {
          light_map = &game.lish.stat_light_map[get_subtile_number(stl_x,stl_y)];
          stl_x_min_1 = stl_x - 1;
          if ( stl_x_min_1 < 0 )
          {
            stl_x_min_1 = 0;
          }
          struct Column *Col1 = get_map_column(Mapblk1);
          struct Column *Col2 = get_map_column(Mapblk2);
          struct Column *Col3 = get_map_column(get_map_block_at(stl_x_min_1,stl_y));
          struct Column *Col4 = get_map_column(get_map_block_at(stl_x_min_1,stl_y_min_1));
          if ( (!column_invalid(Col1)) && (!column_invalid(Col2)) && (!column_invalid(Col3)) && (!column_invalid(Col4)) )
          {
            *light_map = game.lish.field_46149 << 8;
          }
          else
          {
            *light_map = 0;
          }
          Mapblk1++;
          Mapblk2++;
        }
      }
    }
  }
}

void light_set_lights_on(char state)
{
    if (state)
    {
        game.lish.field_46149 = 10;
        game.lish.light_enabled = 1;
    } else
    {
        game.lish.field_46149 = 32;
        game.lish.light_enabled = 0;
    }
    // Enable lights on all but bounding subtiles
    light_stat_light_map_clear_area(0, 0, map_subtiles_x, map_subtiles_y);
    light_signal_stat_light_update_in_area(1, 1, map_subtiles_x, map_subtiles_y);
}

static long calculate_shadow_angle(
        unsigned int pos_x,
        unsigned int pos_y,
        int quadrant,
        MapSubtlCoord stl_x,
        MapSubtlCoord stl_y,
        long *shadow_limit_idx_start,
        long *shadow_limit_idx_end)
{
    MapSubtlCoord x = coord_subtile(pos_x);
    MapSubtlCoord y = coord_subtile(pos_y);
    long shadow_end;
    long result;
    long shadow_start = 0;

  if ( x == stl_x )
  {
    if ( y <= stl_y )
    {
      shadow_start = LbArcTanAngle(subtile_coord(stl_x + 1, 0) - pos_x, subtile_coord(stl_y, 0) - pos_y) & LbFPMath_AngleMask;
      shadow_end = LbArcTanAngle(subtile_coord(stl_x, 0) - pos_x, subtile_coord(stl_y, 0) - pos_y) & LbFPMath_AngleMask;
    }
    else
    {
      shadow_start = LbArcTanAngle(subtile_coord(stl_x, 0) - pos_x - 1, subtile_coord(stl_y + 1, 0) - pos_y) & LbFPMath_AngleMask;
      shadow_end = LbArcTanAngle(subtile_coord(stl_x + 1, 0) - pos_x, subtile_coord(stl_y + 1, 0) - pos_y) & LbFPMath_AngleMask;
    }
  }
  else if ( y == stl_y )
  {
    if ( x <= stl_x )
    {
      shadow_start = LbArcTanAngle(subtile_coord(stl_x, 0) - pos_x, subtile_coord(stl_y, 0) - pos_y) & LbFPMath_AngleMask;
      shadow_end = LbArcTanAngle(subtile_coord(stl_x, 0) - pos_x, subtile_coord(stl_y + 1, 0) - pos_y) & LbFPMath_AngleMask;
    }
    else
    {
      shadow_start = LbArcTanAngle(subtile_coord(stl_x + 1, 0) - pos_x, subtile_coord(stl_y + 1, 0) - pos_y) & LbFPMath_AngleMask;
      shadow_end = LbArcTanAngle(subtile_coord(stl_x + 1, 0) - pos_x, subtile_coord(stl_y, 0) - pos_y) & LbFPMath_AngleMask;
    }
  }
  else
  {
    switch ( quadrant )
    {
      case 1:
        shadow_start = LbArcTanAngle(subtile_coord(stl_x, 0) - pos_x, subtile_coord(stl_y, 0) - pos_y) & LbFPMath_AngleMask;
        shadow_end = LbArcTanAngle(subtile_coord(stl_x + 1, 0) - pos_x, subtile_coord(stl_y + 1, 0) - pos_y) & LbFPMath_AngleMask;
        break;
      case 2:
        shadow_start = LbArcTanAngle(subtile_coord(stl_x + 1, 0) - pos_x, subtile_coord(stl_y, 0) - pos_y) & LbFPMath_AngleMask;
        shadow_end = LbArcTanAngle(subtile_coord(stl_x, 0) - pos_x, subtile_coord(stl_y + 1, 0) - pos_y) & LbFPMath_AngleMask;
        break;
      case 3:
        shadow_start = LbArcTanAngle(subtile_coord(stl_x + 1, 0) - pos_x, subtile_coord(stl_y + 1, 0) - pos_y) & LbFPMath_AngleMask;
        shadow_end = LbArcTanAngle(subtile_coord(stl_x, 0) - pos_x, subtile_coord(stl_y, 0) - pos_y) & LbFPMath_AngleMask;
        break;
      case 4:
        shadow_start = LbArcTanAngle(subtile_coord(stl_x, 0) - pos_x, subtile_coord(stl_y + 1, 0) - pos_y) & LbFPMath_AngleMask;
        shadow_end = LbArcTanAngle(subtile_coord(stl_x + 1, 0) - pos_x, subtile_coord(stl_y, 0) - pos_y) & LbFPMath_AngleMask;
        break;
      default:
        shadow_end = shadow_start;
        break;
    }
  }
  if ( (shadow_start / 512) << 9 != shadow_start )
    shadow_start = (shadow_start + 1) & LbFPMath_AngleMask;
  if ( (shadow_end / 512) << 9 != shadow_end )
    shadow_end = (shadow_end - 1) & LbFPMath_AngleMask;
  result = shadow_start;
  *shadow_limit_idx_start = shadow_start;
  *shadow_limit_idx_end = shadow_end;
  return result;
}

static TbBool point_is_above_floor(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_z)
{
    struct Column *col = get_column_at(stl_x, stl_y);
    return (get_column_floor_filled_subtiles(col) > stl_z);
}

//used for the hand and the illuminated property of creatures
static char light_render_light_dynamic_uncached(struct Light *lgt, int radius, int intensity, unsigned int max_1DD41_idx)
{
    clear_shadow_limits(&game.lish);
    unsigned int lighting_tables_idx = get_floor_filled_subtiles_at(lgt->mappos.x.stl.num, lgt->mappos.y.stl.num);
    if ( lighting_tables_idx <= lgt->mappos.z.stl.num )
    {
        int unk_4_x = lgt->mappos.x.stl.pos;
        int unk_4_y = lgt->mappos.y.stl.pos;
        int diagonal_length = LbDiagonalLength(unk_4_x, unk_4_y);
        short lightness = intensity * (radius - diagonal_length) / radius;
        SubtlCodedCoords light_stl_num = get_subtile_number(lgt->mappos.x.stl.num,lgt->mappos.y.stl.num);
        unsigned short *stl_lightness_ptr = &game.lish.subtile_lightness[light_stl_num];
        if ( *stl_lightness_ptr < lightness )
            *stl_lightness_ptr = lightness;
        struct LightingTable *lighting_table_pointer = &game.lish.lighting_tables[0];
        lighting_tables_idx = game.lish.lighting_tables_count;
        if ( &game.lish.lighting_tables[game.lish.lighting_tables_count] > &game.lish.lighting_tables[0] )
        {
            do
            {
                lighting_tables_idx = lighting_table_pointer->distance;
                if ( lighting_tables_idx > max_1DD41_idx )
                    break;
                MapSubtlCoord stl_x = lgt->mappos.x.stl.num + lighting_table_pointer->delta_x;
                MapSubtlCoord stl_y = lgt->mappos.y.stl.num + lighting_table_pointer->delta_y;
                if (!subtile_coords_invalid(stl_x, stl_y))
                {
                    int quadrant;
                    long shadow_limit_idx1 = LbArcTanAngle((stl_x << 8) - lgt->mappos.x.val, (stl_y << 8) - lgt->mappos.y.val) & LbFPMath_AngleMask;
                    if ( stl_x < lgt->mappos.x.stl.num )
                    {
                        if (stl_y < lgt->mappos.y.stl.num)
                        {
                            quadrant = 4;
                        }
                        else
                        {
                            quadrant = 3;
                        }
                    }
                    else
                    {
                        if (stl_y < lgt->mappos.y.stl.num)
                        {
                            quadrant = 1;
                        }
                        else
                        {
                            quadrant = 2;
                        }
                    }
                    long shadow_limit_idx2, shadow_limit_idx3;
                    unsigned char height = get_floor_filled_subtiles_at(stl_x, stl_y);
                    if ( game.lish.shadow_limits[shadow_limit_idx1] )
                    {
                        calculate_shadow_angle(lgt->mappos.x.val, lgt->mappos.y.val, quadrant, stl_x, stl_y, &shadow_limit_idx2, &shadow_limit_idx3);
                        if ( (!game.lish.shadow_limits[shadow_limit_idx2] || !game.lish.shadow_limits[shadow_limit_idx3])
                            && height > lgt->mappos.z.stl.num )
                        {
                            create_shadow_limits(&game.lish, shadow_limit_idx2, shadow_limit_idx3);
                        }
                    }
                    else
                    {
                        TbBool too_high = (height > lgt->mappos.z.stl.num);
                        if ( height > lgt->mappos.z.stl.num )
                        {
                            calculate_shadow_angle(lgt->mappos.x.val, lgt->mappos.y.val, quadrant, stl_x, stl_y, &shadow_limit_idx2, &shadow_limit_idx3);
                            create_shadow_limits(&game.lish, shadow_limit_idx2, shadow_limit_idx3);
                        }
                        TbBool v24;
                        if ( !too_high )
                            goto LABEL_37;
                        switch ( quadrant )
                        {
                            case 1:
                            v24 = ( get_floor_filled_subtiles_at(stl_x - 1, stl_y - 1) <= lgt->mappos.z.stl.num );
                            break;
                            case 3:
                            v24 = ( !point_is_above_floor(stl_x, stl_y - 1, lgt->mappos.z.stl.num) );
                            break;
                            case 4:
                            v24 = false;
                            break;
                            default:
                            v24 = true;
                            break;
                        }
                        if ( v24 )
                        {
                            LABEL_37:
                            {
                                unsigned int unk_1_y = stl_y << 8;
                                unsigned int unk_1_x = stl_x << 8;
                                int unk_2_x = min((lgt->mappos.x.val - unk_1_x), (unk_1_x - lgt->mappos.x.val));
                                int unk_2_y = min((lgt->mappos.y.val - unk_1_y), (unk_1_y - lgt->mappos.y.val));
                                int diagonal_length2 = LbDiagonalLength(unk_2_x, unk_2_y);
                                lighting_tables_idx = intensity * (radius - diagonal_length2) / radius;
                                if ( lighting_tables_idx <= game.lish.field_46149 )
                                    return lighting_tables_idx;
                                unsigned short *stl_lightness_ptr2 = &game.lish.subtile_lightness[get_subtile_number(stl_x,stl_y)];
                                if ( *stl_lightness_ptr2 < lighting_tables_idx )
                                    *stl_lightness_ptr2 = lighting_tables_idx;
                            }
                        }
                    }
                }
                lighting_table_pointer++;
                lighting_tables_idx = game.lish.lighting_tables_count;
            }
        while ( &game.lish.lighting_tables[game.lish.lighting_tables_count] > lighting_table_pointer );
        }
    }
    return lighting_tables_idx;
}

static char light_render_light_dynamic(struct Light *lgt, int radius, int render_intensity, unsigned int lighting_tables_idx)
{
    unsigned short *stl_lightness;
    int stl_num_2;
    int v10;
    int v11;
    struct LightingTable *lighting_table;
    unsigned int stl_y;
    unsigned int some_y_2;
    int angle;
    unsigned char v17;
    char v19;
    unsigned char *v20;
    int v22;
    unsigned char *shadow_limits;
    unsigned int v24;
    int bool_2;
    int v28;
    long shadow_limit_idx;
    unsigned int stl_x;
    long shadow_limit_idx2;
    MapCoord lgt_pos_x;
    MapCoord lgt_pos_y;
    int lgt_stl_z;
    unsigned int lgt_stl_x;
    unsigned int lgt_stl_y;
    int v38;
    struct ShadowCache *shadow_cache;
    TbBool bool_1;
    unsigned int some_x_2;
    char v42;
    unsigned short *subtile_lightness;

    lgt_pos_x = lgt->mappos.x.val;
    lgt_pos_y = lgt->mappos.y.val;
    lgt_stl_x = lgt_pos_x >> 8;
    lgt_stl_y = lgt_pos_y >> 8;
    lgt_stl_z = lgt->mappos.z.val / COORD_PER_STL;
    shadow_cache = &game.lish.shadow_cache[lgt->shadow_index];
    memset(game.lish.shadow_limits, 0, sizeof(game.lish.shadow_limits));
    memset(shadow_cache->field_1, 0, 0x80u);

    stl_num_2 = get_subtile_number(lgt->mappos.x.stl.num,lgt->mappos.y.stl.num);
    stl_lightness = &game.lish.subtile_lightness[stl_num_2];
    const struct Column *col = get_column_at(lgt_pos_x + 1,lgt_pos_y + 1);

    if (col->bitfields >> 4 <= lgt_stl_z)
    {
        v42 = lighting_tables_idx;
        shadow_cache->field_1[lighting_tables_idx] |= 1 << (31 - lighting_tables_idx);
        MapCoordDelta some_delta_x_2 = lgt->mappos.x.stl.pos;
        MapCoordDelta some_delta_y_2 = lgt->mappos.y.stl.pos;

        v10 = LbDiagonalLength(some_delta_x_2, some_delta_y_2);

        v11 = render_intensity * (radius - v10) / radius;
        if ((unsigned short)*stl_lightness < v11)
            *stl_lightness = v11;
        lighting_table = &game.lish.lighting_tables[0];

        stl_num_2 = get_subtile_number(game.lish.lighting_tables_count, stl_num_decode_y(stl_num_2));

        if (&game.lish.lighting_tables[game.lish.lighting_tables_count] > &game.lish.lighting_tables[0])
        {
            do
            {
                stl_num_2 = (unsigned char)lighting_table->distance;
                if (stl_num_2 > lighting_tables_idx)
                    break;
                stl_y = lighting_table->delta_y + lgt_stl_y;
                stl_x = lighting_table->delta_x + lgt_stl_x;
                if (lighting_table->delta_x + lgt_stl_x < map_subtiles_x && stl_y < map_subtiles_y)
                {
                    some_y_2 = stl_y << 8;
                    some_x_2 = stl_x << 8;
                    angle = LbArcTanAngle(some_x_2 - lgt_pos_x, some_y_2 - lgt_pos_y) & LbFPMath_AngleMask;
                    if ((unsigned char)stl_x < (unsigned char)lgt_stl_x)
                        v17 = ((unsigned char)stl_y < (unsigned char)lgt_stl_y) + 3;
                    else
                        v17 = 2 - ((unsigned char)stl_y < (unsigned char)lgt_stl_y);
                    v19 = game.lish.shadow_limits[angle];
                    v38 = v17;
                    if (v19)
                    {
                        calculate_shadow_angle(lgt_pos_x, lgt_pos_y, v38, stl_x, stl_y, &shadow_limit_idx, &shadow_limit_idx2);
                        v20 = &game.lish.shadow_limits[shadow_limit_idx];
                        const struct Column *col2 = get_column_at(stl_x + 1,stl_y + 1);
                        if ((!game.lish.shadow_limits[shadow_limit_idx] || !game.lish.shadow_limits[shadow_limit_idx2]) && col2->bitfields >> 4 > lgt_stl_z)
                        {
                            if (shadow_limit_idx2 < shadow_limit_idx)
                            {
                                memset(v20, 1u, 2047 - shadow_limit_idx);
                                memset(game.lish.shadow_limits, 1u, shadow_limit_idx2);
                            }
                            else
                            {
                                memset(v20, 1u, shadow_limit_idx2 - shadow_limit_idx);
                            }
                        }
                    }
                    else
                    {
                        const struct Column *col3 = get_column_at(stl_x,stl_y);
                        subtile_lightness = &game.lish.subtile_lightness[get_subtile_number(stl_x,stl_y)];
                        v22 = col3->bitfields >> 4;
                        bool_1 = v22 > lgt_stl_z;
                        if (v22 > lgt_stl_z)
                        {
                            calculate_shadow_angle(lgt_pos_x, lgt_pos_y, v38, stl_x, stl_y, &shadow_limit_idx, &shadow_limit_idx2);
                            if (shadow_limit_idx2 < shadow_limit_idx)
                            {
                                memset(&game.lish.shadow_limits[shadow_limit_idx], 1u, 2047 - shadow_limit_idx);
                                v24 = shadow_limit_idx2;
                                shadow_limits = &game.lish.shadow_limits[0];
                            }
                            else
                            {
                                shadow_limits = &game.lish.shadow_limits[shadow_limit_idx];
                                v24 = shadow_limit_idx2 - shadow_limit_idx;
                            }
                            memset(shadow_limits, 1u, v24);
                        }
                        bool_2 = false;
                        if (bool_1)
                        {
                            const struct Column *col4;
                            switch (v38)
                            {
                            case 1:
                                col4 = get_column_at(stl_x,stl_y + 1);
                                bool_2 = (col4->bitfields >> 4 <= lgt_stl_z);
                                break;
                            case 3:
                                bool_2 = (!point_is_above_floor(stl_x, stl_y - 1, lgt_stl_z));
                                break;
                            case 4:
                                bool_2 = 0;
                                break;
                            default:
                                bool_2 = 1;
                                break;
                            }
                        }
                        if (!bool_1 || bool_2)
                        {
                            MapCoordDelta some_delta_x = min((lgt_pos_x - some_x_2),(some_x_2 - lgt_pos_x));
                            MapCoordDelta some_delta_y = min((lgt_pos_y - some_y_2),(some_y_2 - lgt_pos_y));

                            v28 = LbDiagonalLength(some_delta_x, some_delta_y);

                            stl_num_2 = render_intensity * (radius - v28) / radius;
                            if (stl_num_2 <= game.lish.field_46149)
                                return stl_num_2;
                            shadow_cache->field_1[lighting_tables_idx + lighting_table->delta_y] |= 1 << (31 - lighting_table->delta_x - v42);
                            if ((unsigned short)*subtile_lightness < stl_num_2)
                                *subtile_lightness = stl_num_2;
                        }
                    }
                }
                ++lighting_table;
                stl_num_2 = get_subtile_number(game.lish.lighting_tables_count, stl_num_decode_y(stl_num_2));
            } while (&game.lish.lighting_tables[game.lish.lighting_tables_count] > lighting_table);
        }
    }
    return stl_num_2;
}

static int light_render_light_static(struct Light *lgt, int radius, int intensity, SubtlCodedCoords stl_num)
{
    struct LightsShadows *lish = &game.lish;
    clear_shadow_limits(lish);
    struct Column *col = get_column_at(lgt->mappos.x.stl.num, lgt->mappos.y.stl.num);
    int floor_filled_stls = get_column_floor_filled_subtiles(col);
    if (floor_filled_stls <= lgt->mappos.z.stl.num)
    {
        signed int x = lgt->mappos.x.stl.pos;
        signed int y = lgt->mappos.y.stl.pos;
        int diagonal_length = LbDiagonalLength(x, y);
        unsigned short lightness = intensity * (radius - diagonal_length) / radius;
        unsigned int light_map_idx = get_subtile_number(lgt->mappos.x.stl.num,lgt->mappos.y.stl.num);
        if (lish->stat_light_map[light_map_idx] < lightness)
        {
            lish->stat_light_map[light_map_idx] = lightness;
        }
        unsigned int lighting_table_idx = 0;
        for (floor_filled_stls = lish->lighting_tables_count;
             lish->lighting_tables_count > lighting_table_idx;
             floor_filled_stls = lish->lighting_tables_count)
        {
            floor_filled_stls = lish->lighting_tables[lighting_table_idx].distance;
            if (floor_filled_stls > stl_num)
            {
                break;
            }
            MapSubtlCoord stl_x = lish->lighting_tables[lighting_table_idx].delta_x + lgt->mappos.x.stl.num;
            MapSubtlCoord stl_y = lish->lighting_tables[lighting_table_idx].delta_y + lgt->mappos.y.stl.num;
            if (!subtile_coords_invalid(stl_x, stl_y))
            {
                unsigned char quadrant;
                if (stl_x < lgt->mappos.x.stl.num)
                {
                    quadrant = (stl_y < lgt->mappos.y.stl.num) + 3;
                }
                else
                {
                    quadrant = 2 - (stl_y < lgt->mappos.y.stl.num);
                }
                MapCoord coord_x = subtile_coord(stl_x, 0);
                MapCoord coord_y = subtile_coord(stl_y, 0);
                long angle = LbArcTanAngle(coord_x - lgt->mappos.x.val, coord_y - lgt->mappos.y.val) & LbFPMath_AngleMask;
                unsigned char shadow_limit = lish->shadow_limits[angle];
                long shadow_start, shadow_end;
                col = get_column_at(stl_x, stl_y);
                if (shadow_limit)
                {
                    calculate_shadow_angle(lgt->mappos.x.val, lgt->mappos.y.val, quadrant, stl_x, stl_y, &shadow_start, &shadow_end);
                    if (((!lish->shadow_limits[shadow_start]) || (!lish->shadow_limits[shadow_end])) && (get_column_floor_filled_subtiles(col) > lgt->mappos.z.stl.num))
                    {
                        create_shadow_limits(lish, shadow_start, shadow_end);
                    }
                }
                else
                {
                    int height = get_column_floor_filled_subtiles(col);
                    TbBool too_high = (height > lgt->mappos.z.stl.num);
                    if (height > lgt->mappos.z.stl.num)
                    {
                        calculate_shadow_angle(lgt->mappos.x.val, lgt->mappos.y.val, quadrant, stl_x, stl_y, &shadow_start, &shadow_end);
                        create_shadow_limits(lish, shadow_start, shadow_end);
                    }
                    TbBool v24 = false;
                    
                    if (too_high)
                    {
                        switch (quadrant)
                        {
                            case 1:
                            {
                                v24 = (get_column_floor_filled_subtiles(col) <= lgt->mappos.z.stl.num);
                                break;
                            }
                            case 3:
                            {
                                v24 = (!point_is_above_floor(stl_x, stl_y - 1, lgt->mappos.z.stl.num));
                                break;
                            }
                            case 4:
                            {
                                v24 = false;
                                break;
                            }
                            default:
                            {
                                v24 = true;
                                break;
                            }
                        }
                    }

                    if ( (v24) || (!too_high) )
                    {
                        floor_filled_stls = intensity * (radius - lish->lighting_tables[lighting_table_idx].field_4) / radius;
                        if (floor_filled_stls <= lish->field_46149)
                            return floor_filled_stls;
                        SubtlCodedCoords next_stl = get_subtile_number(stl_x,stl_y);
                        if (lish->stat_light_map[next_stl] < floor_filled_stls)
                            lish->stat_light_map[next_stl] = floor_filled_stls;
                    }
                }
            }
            lighting_table_idx++;
        }
    }
    return floor_filled_stls;
}


static char light_render_light(struct Light* lgt)
{
  struct LightAdd* lightadd = get_lightadd(lgt->index);
  int remember_original_lgt_mappos_x = lgt->mappos.x.val;
  int remember_original_lgt_mappos_y = lgt->mappos.y.val;
  if ((lightadd->interp_has_been_initialized == false) || (game.play_gameturn - lightadd->last_turn_drawn > 1)) {
    lightadd->interp_has_been_initialized = true;
    lightadd->interp_mappos.x.val = lgt->mappos.x.val;
    lightadd->interp_mappos.y.val = lgt->mappos.y.val;
    lightadd->previous_mappos.x.val = lgt->mappos.x.val;
    lightadd->previous_mappos.y.val = lgt->mappos.y.val;
  } else {
    lightadd->interp_mappos.x.val = interpolate(lightadd->interp_mappos.x.val, lightadd->previous_mappos.x.val, lgt->mappos.x.val);
    lightadd->interp_mappos.y.val = interpolate(lightadd->interp_mappos.y.val, lightadd->previous_mappos.y.val, lgt->mappos.y.val);
  }
  lightadd->last_turn_drawn = game.play_gameturn;
  lgt->mappos.x.val = lightadd->interp_mappos.x.val;
  lgt->mappos.y.val = lightadd->interp_mappos.y.val;
  // Stop flicker by rounding off position
  TbBool is_dynamic = lgt->flags & LgtF_Dynamic;
  if (is_dynamic)
  {
      lgt->mappos.x.val = ((lgt->mappos.x.val >> 8) << 8);
      lgt->mappos.y.val = ((lgt->mappos.y.val >> 8) << 8);
  }

  int intensity;
  int radius = lgt->radius;
  int render_radius = radius;
  int render_intensity;

  if ( (lgt->flags2 & 0xFE) != 0 )
  {
    int rand_minimum = (lgt->intensity - 1) << 8;
    intensity = (lgt->intensity << 8) + 257;
    render_intensity = rand_minimum + LIGHT_RANDOM(513);
  }
  else
  {
    intensity = lgt->intensity << 8;
    render_intensity = intensity;
  }
  if ( is_dynamic )
  {
    if ( radius < lgt->min_radius << 8 )
      render_radius = lgt->min_radius << 8;
    if ( intensity < lgt->min_intensity << 8 )
      intensity = lgt->min_intensity << 8;
  }
  unsigned int lighting_tables_idx;
  if ( intensity >= game.lish.field_46149 << 8 )
  {
    lighting_tables_idx = (intensity - (game.lish.field_46149 << 8)) / (intensity / (render_radius / 256)) + 1;
    if ( lighting_tables_idx > 31 )
      lighting_tables_idx = 31;
  }
  else
  {
    lighting_tables_idx = 0;
  }

  lgt->range = lighting_tables_idx;

  if ( (radius > 0) && (render_intensity > 0) )
  {
    if ( is_dynamic )
    {
      if ( (lgt->flags & LgtF_NeverCached) != 0 )
      {
        lighting_tables_idx = light_render_light_dynamic_uncached(lgt, radius, render_intensity, lighting_tables_idx);
      }
      else if ( (lgt->flags & LgtF_Unkn08) != 0 )
      {
        lighting_tables_idx = light_render_light_dynamic(lgt, radius, render_intensity, lighting_tables_idx);
        lgt->flags &= ~LgtF_Unkn08;
      }
      else
      {
        int lighting_radius = lighting_tables_idx << 8;

        MapCoord x_start = lgt->mappos.x.val - lighting_radius;
        if ( x_start < 0 )
          x_start = 0;
        MapCoord y_start = lgt->mappos.y.val - lighting_radius;
        if ( y_start < 0 )
          y_start = 0;

        MapCoord x_end = lgt->mappos.x.val + lighting_radius;
        if ( x_end > ((map_subtiles_x + 1) * COORD_PER_STL) - 1)
          x_end = ((map_subtiles_x + 1) * COORD_PER_STL - 1);
        MapCoord y_end = lgt->mappos.y.val + lighting_radius;
        if ( y_end > ((map_subtiles_y + 1) * COORD_PER_STL - 1) )
          y_end = ((map_subtiles_y + 1) * COORD_PER_STL - 1);
        MapSubtlCoord stl_x = coord_subtile(x_start);
        MapSubtlCoord stl_y = coord_subtile(y_start);
        int v33 = stl_x - coord_subtile(x_end) + map_subtiles_x;
        unsigned short* lightness = &game.lish.subtile_lightness[get_subtile_number(stl_x, stl_y)];
        struct ShadowCache *shdc = &game.lish.shadow_cache[lgt->shadow_index];
        lighting_tables_idx = *shdc->field_1;
        if ( y_end >= y_start )
        {
          unsigned int shadow_cache_pointer = (unsigned int)shdc->field_1;
          MapCoord y = y_start;
          do
          {
            MapCoord x = x_start;
            for ( size_t i = 0; x <= x_end; i++ )
            {
              if ( (light_bitmask[i] & lighting_tables_idx) != 0 )
              {
                struct Coord3d pos;
                pos.x.val = x;
                pos.y.val = y;
                MapCoordDelta dist = get_2d_distance(&lgt->mappos, &pos);
                short new_lightness = render_intensity * (radius - dist) / radius;
                if ( *lightness < new_lightness )
                  *lightness = new_lightness;
              }
              x += COORD_PER_STL;
              lightness++;
            }

            lightness += v33;
            y += COORD_PER_STL;
            lighting_tables_idx = *((unsigned int*)shadow_cache_pointer + 1);
            shadow_cache_pointer += 4;
          }
          while ( y_end >= y );
        }
      }
    }
    else
    {
      lighting_tables_idx = light_render_light_static(lgt, radius, render_intensity, lighting_tables_idx);
    }
  }
  lgt->mappos.x.val = remember_original_lgt_mappos_x;
  lgt->mappos.y.val = remember_original_lgt_mappos_y;
  return lighting_tables_idx;
}

static void light_render_area(MapSubtlCoord startx, MapSubtlCoord starty, MapSubtlCoord endx, MapSubtlCoord endy)
{
  struct Light *lgt;
  int range;
  char *v9;
  unsigned short *v10;
  short *v12;
  unsigned short *v13;
  short v21;
  MapSubtlDelta half_width_y;
  MapSubtlDelta half_width_x;

  light_rendered_dynamic_lights = 0;
  light_rendered_optimised_dynamic_lights = 0;
  light_updated_stat_lights = 0;
  light_out_of_date_stat_lights = 0;
  half_width_x = (endx - startx) / 2 + 1;
  half_width_y = (endy - starty) / 2 + 1;


  // this block applies to static lights
  if ( game.lish.light_enabled )
  {
    for ( lgt = &game.lish.lights[game.thing_lists[TngList_StaticLights].index];
          lgt > game.lish.lights;
          lgt = &game.lish.lights[lgt->next_in_list] )
    {
      if ( (lgt->flags & (LgtF_Unkn80 | LgtF_Unkn08)) != 0 )
      {
        ++light_out_of_date_stat_lights;
        range = lgt->range;



        if ( (int)abs(half_width_x + startx - lgt->mappos.x.stl.num) < half_width_x + range
          && (int)abs(half_width_y + starty - lgt->mappos.y.stl.num) < half_width_y + range )
        {
          ++light_updated_stat_lights;
          light_render_light(lgt);
          lgt->flags &= ~(LgtF_Unkn80 | LgtF_Unkn08);
        }
      }
    }
  }


  SubtlCodedCoords start_num = get_subtile_number(startx, starty);
  v9 = (char *)&game.lish.subtile_lightness + start_num * 2;
  v10 = &game.lish.stat_light_map[start_num];

  
  if ( starty <= endy )
  {
    MapSubtlCoord y = endy - starty + 1;
    do
    {
      v12 = (short *)v9;
      v13 = v10;
      v9 += (map_subtiles_x + 1) * 2;
      v10 += (map_subtiles_x + 1);
      memcpy(v12, v13, 2 * (endx - startx));
      --y;
    }
    while ( y );
  }

  if ( game.lish.light_enabled )
  {
    for ( lgt = &game.lish.lights[game.thing_lists[TngList_DynamLights].index]; lgt > game.lish.lights; lgt = &game.lish.lights[lgt->next_in_list] )
    {
      range = lgt->range;
      if ( (int)abs(half_width_x + startx - lgt->mappos.x.stl.num) < half_width_x + range
        && (int)abs(half_width_y + starty - lgt->mappos.y.stl.num) < half_width_y + range )
      {
        ++light_rendered_dynamic_lights;
        if ( (lgt->flags & LgtF_Unkn08) == 0 )
          ++light_rendered_optimised_dynamic_lights;
        if ( (lgt->flags & LgtF_Unkn10) != 0 )
        {
          if ( lgt->field_6 == 1 )
          {
            if ( lgt->field_1E + lgt->radius >= lgt->field_20 )
            {
              lgt->radius = lgt->field_20;
              lgt->field_6 = 2;
            }
            else
            {
              lgt->radius += lgt->field_1E;
            }
          }
          else if ( lgt->radius - lgt->field_1E <= lgt->field_22 )
          {
            lgt->radius = lgt->field_22;
            lgt->field_6 = 1;
          }
          else
          {
            lgt->radius -= lgt->field_1E;
          }
          lgt->flags |= LgtF_Unkn08;
        }
        if ( (lgt->flags & LgtF_Unkn20) != 0 )
        {
          if ( lgt->field_3 == 1 )
          {
            if ( lgt->field_4 + lgt->intensity >= lgt->field_7 )
            {
              lgt->intensity = lgt->field_7;
              lgt->field_3 = 2;
            }
            else
            {
              lgt->intensity = lgt->field_4 + lgt->intensity;
            }
          }
          else
          {
            if ( lgt->intensity - lgt->field_4 <= lgt->field_7 )
            {
              lgt->intensity = lgt->field_7;
              lgt->field_3 = 1;
            }
            else
            {
              lgt->intensity = lgt->intensity - lgt->field_4;
            }
          }
          lgt->flags |= LgtF_Unkn08;
        }
        v21 = lgt->field_1C;
        if ( v21 )
        {
          lgt->field_18 += v21;
          lgt->flags |= LgtF_Unkn08;
        }
        light_render_light(lgt);
      }
    }
  }
}

void update_light_render_area(void)
{
    int subtile_x;
    int subtile_y;
    int startx;
    int starty;
    SYNCDBG(6,"Starting");
    struct PlayerInfo* player = get_my_player();
    if (
        player->view_mode == PVM_CreatureView ||
        player->view_mode == PVM_IsoWibbleView ||
        player->view_mode == PVM_FrontView ||
        player->view_mode == PVM_IsoStraightView
    ) {
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

void light_set_light_minimum_size_to_cache(long lgt_id, long a2, long a3)
{
  struct Light *lgt;
  if ( lgt_id )
  {
    lgt = &game.lish.lights[lgt_id];
    if ( lgt->flags & LgtF_Allocated )
    {
      if ( lgt->flags & LgtF_Unkn02 )
      {
        lgt->flags &= ~LgtF_Unkn02;
        if ( lgt->flags & LgtF_Dynamic )
        {
          lgt->min_radius = a2;
          lgt->min_intensity = a3;
        }
        else
        {
          ERRORLOG("Attempt to set_minimum light size to cache on non dynamic light");
        }
      }
    }
    else
    {
      ERRORLOG("Attempt to set minimum light size for unallocated light structure");
    }
  }
  else
  {
    ERRORLOG("Attempt to set minimum light size for light 0");
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
