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
#include "bflib_math.h"
#include "bflib_planar.h"

#include "engine_render.h"
#include "player_data.h"
#include "map_data.h"

#include "thing_stats.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

DLLIMPORT TbBool _DK_light_render_light_sub1_sub2(int a1, SubtlCodedCoords stl_num, int a3);
DLLIMPORT char _DK_light_render_light_sub1(struct Light *lgt, int radius, int a3, unsigned int a4);
DLLIMPORT char _DK_light_render_light_sub2(struct Light *lgt, int radius, int a3, unsigned int a4);
DLLIMPORT int _DK_light_render_light_sub3(struct Light *lgt, int radius, int a3, unsigned int a4);
DLLIMPORT int _DK_light_render_light_sub1_sub1(unsigned int a1,unsigned int a2,int a3,unsigned int a4,unsigned int a5,long *a6,long *a7);

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
    lightadd->interp_initialize = true;

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
      if ( end_y > 255 )
      {
        end_y = 255;
      }
      if ( end_x > 255 )
      {
        end_x = 255;
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
    { 1, 2, 0, 255, 256 },
    { 1, 2, 1, 0, 256 },
    { 1, 2, 0, 1, 256 },
    { 1, 2, 255, 0, 256 },
    { 1, 2, 1, 255, 362 },
    { 1, 2, 1, 1, 362 },
    { 1, 2, 255, 1, 362 },
    { 1, 2, 255, 255, 362 },
    { 1, 3, 0, 254, 512 },
    { 1, 3, 2, 0, 512 },
    { 1, 3, 0, 2, 512 },
    { 1, 3, 254, 0, 512 },
    { 1, 3, 1, 254, 572 },
    { 1, 3, 2, 255, 572 },
    { 1, 3, 2, 1, 572 },
    { 1, 3, 1, 2, 572 },
    { 1, 3, 255, 2, 572 },
    { 1, 3, 254, 1, 572 },
    { 1, 3, 254, 255, 572 },
    { 1, 3, 255, 254, 572 },
    { 1, 4, 2, 254, 724 },
    { 1, 4, 2, 2, 724 },
    { 1, 4, 254, 2, 724 },
    { 1, 4, 254, 254, 724 },
    { 1, 4, 0, 253, 768 },
    { 1, 4, 3, 0, 768 },
    { 1, 4, 0, 3, 768 },
    { 1, 4, 253, 0, 768 },
    { 1, 4, 1, 253, 809 },
    { 1, 4, 3, 255, 809 },
    { 1, 4, 3, 1, 809 },
    { 1, 4, 1, 3, 809 },
    { 1, 4, 255, 3, 809 },
    { 1, 4, 253, 1, 809 },
    { 1, 4, 253, 255, 809 },
    { 1, 4, 255, 253, 809 },
    { 1, 4, 2, 253, 921 },
    { 1, 4, 3, 254, 921 },
    { 1, 4, 3, 2, 921 },
    { 1, 4, 2, 3, 921 },
    { 1, 4, 254, 3, 921 },
    { 1, 4, 253, 2, 921 },
    { 1, 4, 253, 254, 921 },
    { 1, 4, 254, 253, 921 },
    { 1, 5, 0, 252, 1024 },
    { 1, 5, 4, 0, 1024 },
    { 1, 5, 0, 4, 1024 },
    { 1, 5, 252, 0, 1024 },
    { 1, 5, 1, 252, 1055 },
    { 1, 5, 4, 255, 1055 },
    { 1, 5, 4, 1, 1055 },
    { 1, 5, 1, 4, 1055 },
    { 1, 5, 255, 4, 1055 },
    { 1, 5, 252, 1, 1055 },
    { 1, 5, 252, 255, 1055 },
    { 1, 5, 255, 252, 1055 },
    { 1, 5, 3, 253, 1086 },
    { 1, 5, 3, 3, 1086 },
    { 1, 5, 253, 3, 1086 },
    { 1, 5, 253, 253, 1086 },
    { 1, 5, 2, 252, 1144 },
    { 1, 5, 4, 254, 1144 },
    { 1, 5, 4, 2, 1144 },
    { 1, 5, 2, 4, 1144 },
    { 1, 5, 254, 4, 1144 },
    { 1, 5, 252, 2, 1144 },
    { 1, 5, 252, 254, 1144 },
    { 1, 5, 254, 252, 1144 },
    { 1, 6, 0, 251, 1280 },
    { 1, 6, 3, 252, 1280 },
    { 1, 6, 4, 253, 1280 },
    { 1, 6, 5, 0, 1280 },
    { 1, 6, 4, 3, 1280 },
    { 1, 6, 3, 4, 1280 },
    { 1, 6, 0, 5, 1280 },
    { 1, 6, 253, 4, 1280 },
    { 1, 6, 252, 3, 1280 },
    { 1, 6, 251, 0, 1280 },
    { 1, 6, 252, 253, 1280 },
    { 1, 6, 253, 252, 1280 },
    { 1, 6, 1, 251, 1305 },
    { 1, 6, 5, 255, 1305 },
    { 1, 6, 5, 1, 1305 },
    { 1, 6, 1, 5, 1305 },
    { 1, 6, 255, 5, 1305 },
    { 1, 6, 251, 1, 1305 },
    { 1, 6, 251, 255, 1305 },
    { 1, 6, 255, 251, 1305 },
    { 1, 6, 2, 251, 1377 },
    { 1, 6, 5, 254, 1377 },
    { 1, 6, 5, 2, 1377 },
    { 1, 6, 2, 5, 1377 },
    { 1, 6, 254, 5, 1377 },
    { 1, 6, 251, 2, 1377 },
    { 1, 6, 251, 254, 1377 },
    { 1, 6, 254, 251, 1377 },
    { 1, 7, 4, 252, 1448 },
    { 1, 7, 4, 4, 1448 },
    { 1, 7, 252, 4, 1448 },
    { 1, 7, 252, 252, 1448 },
    { 1, 7, 3, 251, 1491 },
    { 1, 7, 5, 253, 1491 },
    { 1, 7, 5, 3, 1491 },
    { 1, 7, 3, 5, 1491 },
    { 1, 7, 253, 5, 1491 },
    { 1, 7, 251, 3, 1491 },
    { 1, 7, 251, 253, 1491 },
    { 1, 7, 253, 251, 1491 },
    { 1, 7, 0, 250, 1536 },
    { 1, 7, 6, 0, 1536 },
    { 1, 7, 0, 6, 1536 },
    { 1, 7, 250, 0, 1536 },
    { 1, 7, 1, 250, 1556 },
    { 1, 7, 6, 255, 1556 },
    { 1, 7, 6, 1, 1556 },
    { 1, 7, 1, 6, 1556 },
    { 1, 7, 255, 6, 1556 },
    { 1, 7, 250, 1, 1556 },
    { 1, 7, 250, 255, 1556 },
    { 1, 7, 255, 250, 1556 },
    { 1, 7, 2, 250, 1618 },
    { 1, 7, 6, 254, 1618 },
    { 1, 7, 6, 2, 1618 },
    { 1, 7, 2, 6, 1618 },
    { 1, 7, 254, 6, 1618 },
    { 1, 7, 250, 2, 1618 },
    { 1, 7, 250, 254, 1618 },
    { 1, 7, 254, 250, 1618 },
    { 1, 7, 4, 251, 1636 },
    { 1, 7, 5, 252, 1636 },
    { 1, 7, 5, 4, 1636 },
    { 1, 7, 4, 5, 1636 },
    { 1, 7, 252, 5, 1636 },
    { 1, 7, 251, 4, 1636 },
    { 1, 7, 251, 252, 1636 },
    { 1, 7, 252, 251, 1636 },
    { 1, 8, 3, 250, 1717 },
    { 1, 8, 6, 253, 1717 },
    { 1, 8, 6, 3, 1717 },
    { 1, 8, 3, 6, 1717 },
    { 1, 8, 253, 6, 1717 },
    { 1, 8, 250, 3, 1717 },
    { 1, 8, 250, 253, 1717 },
    { 1, 8, 253, 250, 1717 },
    { 1, 8, 0, 249, 1792 },
    { 1, 8, 7, 0, 1792 },
    { 1, 8, 0, 7, 1792 },
    { 1, 8, 249, 0, 1792 },
    { 1, 8, 1, 249, 1809 },
    { 1, 8, 7, 255, 1809 },
    { 1, 8, 7, 1, 1809 },
    { 1, 8, 1, 7, 1809 },
    { 1, 8, 255, 7, 1809 },
    { 1, 8, 249, 1, 1809 },
    { 1, 8, 249, 255, 1809 },
    { 1, 8, 255, 249, 1809 },
    { 1, 8, 5, 251, 1810 },
    { 1, 8, 5, 5, 1810 },
    { 1, 8, 251, 5, 1810 },
    { 1, 8, 251, 251, 1810 },
    { 1, 8, 4, 250, 1843 },
    { 1, 8, 6, 252, 1843 },
    { 1, 8, 6, 4, 1843 },
    { 1, 8, 4, 6, 1843 },
    { 1, 8, 252, 6, 1843 },
    { 1, 8, 250, 4, 1843 },
    { 1, 8, 250, 252, 1843 },
    { 1, 8, 252, 250, 1843 },
    { 1, 8, 2, 249, 1863 },
    { 1, 8, 7, 254, 1863 },
    { 1, 8, 7, 2, 1863 },
    { 1, 8, 2, 7, 1863 },
    { 1, 8, 254, 7, 1863 },
    { 1, 8, 249, 2, 1863 },
    { 1, 8, 249, 254, 1863 },
    { 1, 8, 254, 249, 1863 },
    { 1, 8, 3, 249, 1947 },
    { 1, 8, 7, 253, 1947 },
    { 1, 8, 7, 3, 1947 },
    { 1, 8, 3, 7, 1947 },
    { 1, 8, 253, 7, 1947 },
    { 1, 8, 249, 3, 1947 },
    { 1, 8, 249, 253, 1947 },
    { 1, 8, 253, 249, 1947 },
    { 1, 9, 5, 250, 1998 },
    { 1, 9, 6, 251, 1998 },
    { 1, 9, 6, 5, 1998 },
    { 1, 9, 5, 6, 1998 },
    { 1, 9, 251, 6, 1998 },
    { 1, 9, 250, 5, 1998 },
    { 1, 9, 250, 251, 1998 },
    { 1, 9, 251, 250, 1998 },
    { 1, 9, 0, 248, 2048 },
    { 1, 9, 8, 0, 2048 },
    { 1, 9, 0, 8, 2048 },
    { 1, 9, 248, 0, 2048 },
    { 1, 9, 4, 249, 2063 },
    { 1, 9, 7, 252, 2063 },
    { 1, 9, 7, 4, 2063 },
    { 1, 9, 4, 7, 2063 },
    { 1, 9, 252, 7, 2063 },
    { 1, 9, 249, 4, 2063 },
    { 1, 9, 249, 252, 2063 },
    { 1, 9, 252, 249, 2063 },
    { 1, 9, 1, 248, 2064 },
    { 1, 9, 8, 255, 2064 },
    { 1, 9, 8, 1, 2064 },
    { 1, 9, 1, 8, 2064 },
    { 1, 9, 255, 8, 2064 },
    { 1, 9, 248, 1, 2064 },
    { 1, 9, 248, 255, 2064 },
    { 1, 9, 255, 248, 2064 },
    { 1, 9, 2, 248, 2111 },
    { 1, 9, 8, 254, 2111 },
    { 1, 9, 8, 2, 2111 },
    { 1, 9, 2, 8, 2111 },
    { 1, 9, 254, 8, 2111 },
    { 1, 9, 248, 2, 2111 },
    { 1, 9, 248, 254, 2111 },
    { 1, 9, 254, 248, 2111 },
    { 1, 9, 6, 250, 2172 },
    { 1, 9, 6, 6, 2172 },
    { 1, 9, 250, 6, 2172 },
    { 1, 9, 250, 250, 2172 },
    { 1, 9, 3, 248, 2187 },
    { 1, 9, 8, 253, 2187 },
    { 1, 9, 8, 3, 2187 },
    { 1, 9, 3, 8, 2187 },
    { 1, 9, 253, 8, 2187 },
    { 1, 9, 248, 3, 2187 },
    { 1, 9, 248, 253, 2187 },
    { 1, 9, 253, 248, 2187 },
    { 1, 9, 5, 249, 2198 },
    { 1, 9, 7, 251, 2198 },
    { 1, 9, 7, 5, 2198 },
    { 1, 9, 5, 7, 2198 },
    { 1, 9, 251, 7, 2198 },
    { 1, 9, 249, 5, 2198 },
    { 1, 9, 249, 251, 2198 },
    { 1, 9, 251, 249, 2198 },
    { 1, 10, 4, 248, 2289 },
    { 1, 10, 8, 252, 2289 },
    { 1, 10, 8, 4, 2289 },
    { 1, 10, 4, 8, 2289 },
    { 1, 10, 252, 8, 2289 },
    { 1, 10, 248, 4, 2289 },
    { 1, 10, 248, 252, 2289 },
    { 1, 10, 252, 248, 2289 },
    { 1, 10, 0, 247, 2304 },
    { 1, 10, 9, 0, 2304 },
    { 1, 10, 0, 9, 2304 },
    { 1, 10, 247, 0, 2304 },
    { 1, 10, 1, 247, 2317 },
    { 1, 10, 9, 255, 2317 },
    { 1, 10, 9, 1, 2317 },
    { 1, 10, 1, 9, 2317 },
    { 1, 10, 255, 9, 2317 },
    { 1, 10, 247, 1, 2317 },
    { 1, 10, 247, 255, 2317 },
    { 1, 10, 255, 247, 2317 },
    { 1, 10, 2, 247, 2358 },
    { 1, 10, 6, 249, 2358 },
    { 1, 10, 7, 250, 2358 },
    { 1, 10, 9, 254, 2358 },
    { 1, 10, 9, 2, 2358 },
    { 1, 10, 7, 6, 2358 },
    { 1, 10, 6, 7, 2358 },
    { 1, 10, 2, 9, 2358 },
    { 1, 10, 254, 9, 2358 },
    { 1, 10, 250, 7, 2358 },
    { 1, 10, 249, 6, 2358 },
    { 1, 10, 247, 2, 2358 },
    { 1, 10, 247, 254, 2358 },
    { 1, 10, 249, 250, 2358 },
    { 1, 10, 250, 249, 2358 },
    { 1, 10, 254, 247, 2358 },
    { 1, 10, 5, 248, 2415 },
    { 1, 10, 8, 251, 2415 },
    { 1, 10, 8, 5, 2415 },
    { 1, 10, 5, 8, 2415 },
    { 1, 10, 251, 8, 2415 },
    { 1, 10, 248, 5, 2415 },
    { 1, 10, 248, 251, 2415 },
    { 1, 10, 251, 248, 2415 },
    { 1, 10, 3, 247, 2427 },
    { 1, 10, 9, 253, 2427 },
    { 1, 10, 9, 3, 2427 },
    { 1, 10, 3, 9, 2427 },
    { 1, 10, 253, 9, 2427 },
    { 1, 10, 247, 3, 2427 },
    { 1, 10, 247, 253, 2427 },
    { 1, 10, 253, 247, 2427 },
    { 1, 11, 4, 247, 2518 },
    { 1, 11, 9, 252, 2518 },
    { 1, 11, 9, 4, 2518 },
    { 1, 11, 4, 9, 2518 },
    { 1, 11, 252, 9, 2518 },
    { 1, 11, 247, 4, 2518 },
    { 1, 11, 247, 252, 2518 },
    { 1, 11, 252, 247, 2518 },
    { 1, 11, 7, 249, 2534 },
    { 1, 11, 7, 7, 2534 },
    { 1, 11, 249, 7, 2534 },
    { 1, 11, 249, 249, 2534 },
    { 1, 11, 0, 246, 2560 },
    { 1, 11, 6, 248, 2560 },
    { 1, 11, 8, 250, 2560 },
    { 1, 11, 10, 0, 2560 },
    { 1, 11, 8, 6, 2560 },
    { 1, 11, 6, 8, 2560 },
    { 1, 11, 0, 10, 2560 },
    { 1, 11, 250, 8, 2560 },
    { 1, 11, 248, 6, 2560 },
    { 1, 11, 246, 0, 2560 },
    { 1, 11, 248, 250, 2560 },
    { 1, 11, 250, 248, 2560 },
    { 1, 11, 1, 246, 2572 },
    { 1, 11, 10, 255, 2572 },
    { 1, 11, 10, 1, 2572 },
    { 1, 11, 1, 10, 2572 },
    { 1, 11, 255, 10, 2572 },
    { 1, 11, 246, 1, 2572 },
    { 1, 11, 246, 255, 2572 },
    { 1, 11, 255, 246, 2572 },
    { 1, 11, 2, 246, 2610 },
    { 1, 11, 10, 254, 2610 },
    { 1, 11, 10, 2, 2610 },
    { 1, 11, 2, 10, 2610 },
    { 1, 11, 254, 10, 2610 },
    { 1, 11, 246, 2, 2610 },
    { 1, 11, 246, 254, 2610 },
    { 1, 11, 254, 246, 2610 },
    { 1, 11, 5, 247, 2634 },
    { 1, 11, 9, 251, 2634 },
    { 1, 11, 9, 5, 2634 },
    { 1, 11, 5, 9, 2634 },
    { 1, 11, 251, 9, 2634 },
    { 1, 11, 247, 5, 2634 },
    { 1, 11, 247, 251, 2634 },
    { 1, 11, 251, 247, 2634 },
    { 1, 11, 3, 246, 2670 },
    { 1, 11, 10, 253, 2670 },
    { 1, 11, 10, 3, 2670 },
    { 1, 11, 3, 10, 2670 },
    { 1, 11, 253, 10, 2670 },
    { 1, 11, 246, 3, 2670 },
    { 1, 11, 246, 253, 2670 },
    { 1, 11, 253, 246, 2670 },
    { 1, 12, 7, 248, 2721 },
    { 1, 12, 8, 249, 2721 },
    { 1, 12, 8, 7, 2721 },
    { 1, 12, 7, 8, 2721 },
    { 1, 12, 249, 8, 2721 },
    { 1, 12, 248, 7, 2721 },
    { 1, 12, 248, 249, 2721 },
    { 1, 12, 249, 248, 2721 },
    { 1, 12, 4, 246, 2755 },
    { 1, 12, 10, 252, 2755 },
    { 1, 12, 10, 4, 2755 },
    { 1, 12, 4, 10, 2755 },
    { 1, 12, 252, 10, 2755 },
    { 1, 12, 246, 4, 2755 },
    { 1, 12, 246, 252, 2755 },
    { 1, 12, 252, 246, 2755 },
    { 1, 12, 6, 247, 2765 },
    { 1, 12, 9, 250, 2765 },
    { 1, 12, 9, 6, 2765 },
    { 1, 12, 6, 9, 2765 },
    { 1, 12, 250, 9, 2765 },
    { 1, 12, 247, 6, 2765 },
    { 1, 12, 247, 250, 2765 },
    { 1, 12, 250, 247, 2765 },
    { 1, 12, 0, 245, 2816 },
    { 1, 12, 11, 0, 2816 },
    { 1, 12, 0, 11, 2816 },
    { 1, 12, 245, 0, 2816 },
    { 1, 12, 1, 245, 2827 },
    { 1, 12, 11, 255, 2827 },
    { 1, 12, 11, 1, 2827 },
    { 1, 12, 1, 11, 2827 },
    { 1, 12, 255, 11, 2827 },
    { 1, 12, 245, 1, 2827 },
    { 1, 12, 245, 255, 2827 },
    { 1, 12, 255, 245, 2827 },
    { 1, 12, 2, 245, 2861 },
    { 1, 12, 11, 254, 2861 },
    { 1, 12, 11, 2, 2861 },
    { 1, 12, 2, 11, 2861 },
    { 1, 12, 254, 11, 2861 },
    { 1, 12, 245, 2, 2861 },
    { 1, 12, 245, 254, 2861 },
    { 1, 12, 254, 245, 2861 },
    { 1, 12, 5, 246, 2862 },
    { 1, 12, 10, 251, 2862 },
    { 1, 12, 10, 5, 2862 },
    { 1, 12, 5, 10, 2862 },
    { 1, 12, 251, 10, 2862 },
    { 1, 12, 246, 5, 2862 },
    { 1, 12, 246, 251, 2862 },
    { 1, 12, 251, 246, 2862 },
    { 1, 12, 8, 248, 2896 },
    { 1, 12, 8, 8, 2896 },
    { 1, 12, 248, 8, 2896 },
    { 1, 12, 248, 248, 2896 },
    { 1, 12, 3, 245, 2916 },
    { 1, 12, 11, 253, 2916 },
    { 1, 12, 11, 3, 2916 },
    { 1, 12, 3, 11, 2916 },
    { 1, 12, 253, 11, 2916 },
    { 1, 12, 245, 3, 2916 },
    { 1, 12, 245, 253, 2916 },
    { 1, 12, 253, 245, 2916 },
    { 1, 12, 7, 247, 2918 },
    { 1, 12, 9, 249, 2918 },
    { 1, 12, 9, 7, 2918 },
    { 1, 12, 7, 9, 2918 },
    { 1, 12, 249, 9, 2918 },
    { 1, 12, 247, 7, 2918 },
    { 1, 12, 247, 249, 2918 },
    { 1, 12, 249, 247, 2918 },
    { 1, 13, 6, 246, 2982 },
    { 1, 13, 10, 250, 2982 },
    { 1, 13, 10, 6, 2982 },
    { 1, 13, 6, 10, 2982 },
    { 1, 13, 250, 10, 2982 },
    { 1, 13, 246, 6, 2982 },
    { 1, 13, 246, 250, 2982 },
    { 1, 13, 250, 246, 2982 },
    { 1, 12, 4, 245, 2996 },
    { 1, 12, 11, 252, 2996 },
    { 1, 12, 11, 4, 2996 },
    { 1, 12, 4, 11, 2996 },
    { 1, 12, 252, 11, 2996 },
    { 1, 12, 245, 4, 2996 },
    { 1, 12, 245, 252, 2996 },
    { 1, 12, 252, 245, 2996 },
    { 1, 13, 0, 244, 3072 },
    { 1, 13, 12, 0, 3072 },
    { 1, 13, 0, 12, 3072 },
    { 1, 13, 244, 0, 3072 },
    { 1, 13, 8, 247, 3079 },
    { 1, 13, 9, 248, 3079 },
    { 1, 13, 9, 8, 3079 },
    { 1, 13, 8, 9, 3079 },
    { 1, 13, 248, 9, 3079 },
    { 1, 13, 247, 8, 3079 },
    { 1, 13, 247, 248, 3079 },
    { 1, 13, 248, 247, 3079 },
    { 1, 13, 1, 244, 3082 },
    { 1, 13, 12, 255, 3082 },
    { 1, 13, 12, 1, 3082 },
    { 1, 13, 1, 12, 3082 },
    { 1, 13, 255, 12, 3082 },
    { 1, 13, 244, 1, 3082 },
    { 1, 13, 244, 255, 3082 },
    { 1, 13, 255, 244, 3082 },
    { 1, 13, 5, 245, 3091 },
    { 1, 13, 11, 251, 3091 },
    { 1, 13, 11, 5, 3091 },
    { 1, 13, 5, 11, 3091 },
    { 1, 13, 251, 11, 3091 },
    { 1, 13, 245, 5, 3091 },
    { 1, 13, 245, 251, 3091 },
    { 1, 13, 251, 245, 3091 },
    { 1, 13, 2, 244, 3113 },
    { 1, 13, 12, 254, 3113 },
    { 1, 13, 12, 2, 3113 },
    { 1, 13, 2, 12, 3113 },
    { 1, 13, 254, 12, 3113 },
    { 1, 13, 244, 2, 3113 },
    { 1, 13, 244, 254, 3113 },
    { 1, 13, 254, 244, 3113 },
    { 1, 13, 7, 246, 3123 },
    { 1, 13, 10, 249, 3123 },
    { 1, 13, 10, 7, 3123 },
    { 1, 13, 7, 10, 3123 },
    { 1, 13, 249, 10, 3123 },
    { 1, 13, 246, 7, 3123 },
    { 1, 13, 246, 249, 3123 },
    { 1, 13, 249, 246, 3123 },
    { 1, 13, 3, 244, 3166 },
    { 1, 13, 12, 253, 3166 },
    { 1, 13, 12, 3, 3166 },
    { 1, 13, 3, 12, 3166 },
    { 1, 13, 253, 12, 3166 },
    { 1, 13, 244, 3, 3166 },
    { 1, 13, 244, 253, 3166 },
    { 1, 13, 253, 244, 3166 },
    { 1, 13, 6, 245, 3204 },
    { 1, 13, 11, 250, 3204 },
    { 1, 13, 11, 6, 3204 },
    { 1, 13, 6, 11, 3204 },
    { 1, 13, 250, 11, 3204 },
    { 1, 13, 245, 6, 3204 },
    { 1, 13, 245, 250, 3204 },
    { 1, 13, 250, 245, 3204 },
    { 1, 13, 4, 244, 3237 },
    { 1, 13, 12, 252, 3237 },
    { 1, 13, 12, 4, 3237 },
    { 1, 13, 4, 12, 3237 },
    { 1, 13, 252, 12, 3237 },
    { 1, 13, 244, 4, 3237 },
    { 1, 13, 244, 252, 3237 },
    { 1, 13, 252, 244, 3237 },
    { 1, 14, 9, 247, 3258 },
    { 1, 14, 9, 9, 3258 },
    { 1, 14, 247, 9, 3258 },
    { 1, 14, 247, 247, 3258 },
    { 1, 14, 8, 246, 3273 },
    { 1, 14, 10, 248, 3273 },
    { 1, 14, 10, 8, 3273 },
    { 1, 14, 8, 10, 3273 },
    { 1, 14, 248, 10, 3273 },
    { 1, 14, 246, 8, 3273 },
    { 1, 14, 246, 248, 3273 },
    { 1, 14, 248, 246, 3273 },
    { 1, 14, 5, 244, 3324 },
    { 1, 14, 12, 251, 3324 },
    { 1, 14, 12, 5, 3324 },
    { 1, 14, 5, 12, 3324 },
    { 1, 14, 251, 12, 3324 },
    { 1, 14, 244, 5, 3324 },
    { 1, 14, 244, 251, 3324 },
    { 1, 14, 251, 244, 3324 },
    { 1, 14, 0, 243, 3328 },
    { 1, 14, 13, 0, 3328 },
    { 1, 14, 0, 13, 3328 },
    { 1, 14, 243, 0, 3328 },
    { 1, 14, 7, 245, 3332 },
    { 1, 14, 11, 249, 3332 },
    { 1, 14, 11, 7, 3332 },
    { 1, 14, 7, 11, 3332 },
    { 1, 14, 249, 11, 3332 },
    { 1, 14, 245, 7, 3332 },
    { 1, 14, 245, 249, 3332 },
    { 1, 14, 249, 245, 3332 },
    { 1, 14, 1, 243, 3337 },
    { 1, 14, 13, 255, 3337 },
    { 1, 14, 13, 1, 3337 },
    { 1, 14, 1, 13, 3337 },
    { 1, 14, 255, 13, 3337 },
    { 1, 14, 243, 1, 3337 },
    { 1, 14, 243, 255, 3337 },
    { 1, 14, 255, 243, 3337 },
    { 1, 14, 2, 243, 3366 },
    { 1, 14, 13, 254, 3366 },
    { 1, 14, 13, 2, 3366 },
    { 1, 14, 2, 13, 3366 },
    { 1, 14, 254, 13, 3366 },
    { 1, 14, 243, 2, 3366 },
    { 1, 14, 243, 254, 3366 },
    { 1, 14, 254, 243, 3366 },
    { 1, 14, 3, 243, 3415 },
    { 1, 14, 13, 253, 3415 },
    { 1, 14, 13, 3, 3415 },
    { 1, 14, 3, 13, 3415 },
    { 1, 14, 253, 13, 3415 },
    { 1, 14, 243, 3, 3415 },
    { 1, 14, 243, 253, 3415 },
    { 1, 14, 253, 243, 3415 },
    { 1, 14, 6, 244, 3434 },
    { 1, 14, 12, 250, 3434 },
    { 1, 14, 12, 6, 3434 },
    { 1, 14, 6, 12, 3434 },
    { 1, 14, 250, 12, 3434 },
    { 1, 14, 244, 6, 3434 },
    { 1, 14, 244, 250, 3434 },
    { 1, 14, 250, 244, 3434 },
    { 1, 14, 9, 246, 3441 },
    { 1, 14, 10, 247, 3441 },
    { 1, 14, 10, 9, 3441 },
    { 1, 14, 9, 10, 3441 },
    { 1, 14, 247, 10, 3441 },
    { 1, 14, 246, 9, 3441 },
    { 1, 14, 246, 247, 3441 },
    { 1, 14, 247, 246, 3441 },
    { 1, 14, 4, 243, 3479 },
    { 1, 14, 13, 252, 3479 },
    { 1, 14, 13, 4, 3479 },
    { 1, 14, 4, 13, 3479 },
    { 1, 14, 252, 13, 3479 },
    { 1, 14, 243, 4, 3479 },
    { 1, 14, 243, 252, 3479 },
    { 1, 14, 252, 243, 3479 },
    { 1, 14, 8, 245, 3480 },
    { 1, 14, 11, 248, 3480 },
    { 1, 14, 11, 8, 3480 },
    { 1, 14, 8, 11, 3480 },
    { 1, 14, 248, 11, 3480 },
    { 1, 14, 245, 8, 3480 },
    { 1, 14, 245, 248, 3480 },
    { 1, 14, 248, 245, 3480 },
    { 1, 15, 7, 244, 3554 },
    { 1, 15, 12, 249, 3554 },
    { 1, 15, 12, 7, 3554 },
    { 1, 15, 7, 12, 3554 },
    { 1, 15, 249, 12, 3554 },
    { 1, 15, 244, 7, 3554 },
    { 1, 15, 244, 249, 3554 },
    { 1, 15, 249, 244, 3554 },
    { 1, 15, 5, 243, 3563 },
    { 1, 15, 13, 251, 3563 },
    { 1, 15, 13, 5, 3563 },
    { 1, 15, 5, 13, 3563 },
    { 1, 15, 251, 13, 3563 },
    { 1, 15, 243, 5, 3563 },
    { 1, 15, 243, 251, 3563 },
    { 1, 15, 251, 243, 3563 },
    { 1, 15, 0, 242, 3584 },
    { 1, 15, 14, 0, 3584 },
    { 1, 15, 0, 14, 3584 },
    { 1, 15, 242, 0, 3584 },
    { 1, 15, 1, 242, 3592 },
    { 1, 15, 14, 255, 3592 },
    { 1, 15, 14, 1, 3592 },
    { 1, 15, 1, 14, 3592 },
    { 1, 15, 255, 14, 3592 },
    { 1, 15, 242, 1, 3592 },
    { 1, 15, 242, 255, 3592 },
    { 1, 15, 255, 242, 3592 },
    { 1, 15, 2, 242, 3619 },
    { 1, 15, 14, 254, 3619 },
    { 1, 15, 14, 2, 3619 },
    { 1, 15, 2, 14, 3619 },
    { 1, 15, 254, 14, 3619 },
    { 1, 15, 242, 2, 3619 },
    { 1, 15, 242, 254, 3619 },
    { 1, 15, 254, 242, 3619 },
    { 1, 15, 10, 246, 3620 },
    { 1, 15, 10, 10, 3620 },
    { 1, 15, 246, 10, 3620 },
    { 1, 15, 246, 246, 3620 },
    { 1, 15, 9, 245, 3635 },
    { 1, 15, 11, 247, 3635 },
    { 1, 15, 11, 9, 3635 },
    { 1, 15, 9, 11, 3635 },
    { 1, 15, 247, 11, 3635 },
    { 1, 15, 245, 9, 3635 },
    { 1, 15, 245, 247, 3635 },
    { 1, 15, 247, 245, 3635 },
    { 1, 15, 3, 242, 3662 },
    { 1, 15, 14, 253, 3662 },
    { 1, 15, 14, 3, 3662 },
    { 1, 15, 3, 14, 3662 },
    { 1, 15, 253, 14, 3662 },
    { 1, 15, 242, 3, 3662 },
    { 1, 15, 242, 253, 3662 },
    { 1, 15, 253, 242, 3662 },
    { 1, 15, 6, 243, 3664 },
    { 1, 15, 13, 250, 3664 },
    { 1, 15, 13, 6, 3664 },
    { 1, 15, 6, 13, 3664 },
    { 1, 15, 250, 13, 3664 },
    { 1, 15, 243, 6, 3664 },
    { 1, 15, 243, 250, 3664 },
    { 1, 15, 250, 243, 3664 },
    { 1, 15, 8, 244, 3687 },
    { 1, 15, 12, 248, 3687 },
    { 1, 15, 12, 8, 3687 },
    { 1, 15, 8, 12, 3687 },
    { 1, 15, 248, 12, 3687 },
    { 1, 15, 244, 8, 3687 },
    { 1, 15, 244, 248, 3687 },
    { 1, 15, 248, 244, 3687 },
    { 1, 15, 4, 242, 3727 },
    { 1, 15, 14, 252, 3727 },
    { 1, 15, 14, 4, 3727 },
    { 1, 15, 4, 14, 3727 },
    { 1, 15, 252, 14, 3727 },
    { 1, 15, 242, 4, 3727 },
    { 1, 15, 242, 252, 3727 },
    { 1, 15, 252, 242, 3727 },
    { 1, 15, 7, 243, 3774 },
    { 1, 15, 13, 249, 3774 },
    { 1, 15, 13, 7, 3774 },
    { 1, 15, 7, 13, 3774 },
    { 1, 15, 249, 13, 3774 },
    { 1, 15, 243, 7, 3774 },
    { 1, 15, 243, 249, 3774 },
    { 1, 15, 249, 243, 3774 },
    { 1, 15, 10, 245, 3800 },
    { 1, 15, 11, 246, 3800 },
    { 1, 15, 11, 10, 3800 },
    { 1, 15, 10, 11, 3800 },
    { 1, 15, 246, 11, 3800 },
    { 1, 15, 245, 10, 3800 },
    { 1, 15, 245, 246, 3800 },
    { 1, 15, 246, 245, 3800 },
    { 1, 15, 5, 242, 3803 },
    { 1, 15, 14, 251, 3803 },
    { 1, 15, 14, 5, 3803 },
    { 1, 15, 5, 14, 3803 },
    { 1, 15, 251, 14, 3803 },
    { 1, 15, 242, 5, 3803 },
    { 1, 15, 242, 251, 3803 },
    { 1, 15, 251, 242, 3803 },
    { 1, 15, 0, 241, 3840 },
    { 1, 15, 15, 0, 3840 },
    { 1, 15, 0, 15, 3840 },
    { 1, 15, 241, 0, 3840 },
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

void light_stat_light_map_clear_area(long x1, long y1, long x2, long y2)
{
  // _DK_light_stat_light_map_clear_area(x1, y1, x2, y2);
  long j,n,x,k,y;
  unsigned short *p;
  unsigned short *light_map;
  if ( y2 >= y1 )
  {
    y = y1 << 8;
    unsigned long i = x1 + (y1 << 8);
    struct Map *Mapblk1 = get_map_block_at_pos(i);
    light_map = &game.lish.stat_light_map[i];
    for (k = y1; k <= y2; k++)
    {
      if ( x2 >= x1 )
      {
        p = light_map;
        n = k - 1;
        if ( k - 1 <= 0 )
        {
          n = 0;
        }
        struct Map *Mapblk2 = get_map_block_at_pos((n << 8) + x1);
        for (j = x1; j <= x2; j++)
        {
          x = j - 1;
          if ( x < 0 )
          {
            x = 0;
          }
          struct Column *Col1 = get_map_column(Mapblk1);
          struct Column *Col2 = get_map_column(Mapblk2);
          struct Column *Col3 = get_map_column(get_map_block_at_pos(x + y));
          struct Column *Col4 = get_map_column(get_map_block_at_pos((n << 8) + x));
          if ( (!column_invalid(Col1)) && (!column_invalid(Col2)) && (!column_invalid(Col3)) && (!column_invalid(Col4)) )
          {
            *p = game.lish.field_46149 << 8;
          }
          else
          {
            *p = 0;
          }
          p++;
          Mapblk1++;
          Mapblk2++;
        }
      }
      y += 256;
      light_map += 256;
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

/*
//sub_4080B0
static __int32 light_render_light_sub1_sub1(
        unsigned int a1,
        unsigned int a2,
        int a3,
        unsigned int a4,
        unsigned int a5,
        long *a6,
        long *a7)
{
  return _DK_light_render_light_sub1_sub1(a1,a2,a3,a4,a5,a6,a7);
}

//sub_408530
static TbBool light_render_light_sub1_sub2(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_z)
{
  return _DK_light_render_light_sub1_sub2(stl_x, stl_y, stl_z);

}
*/

//sub_407770
static char light_render_light_dynamic_2(struct Light *lgt, int radius, int a3, unsigned int max_1DD41_idx)
{
  return _DK_light_render_light_sub2(lgt, radius, a3, max_1DD41_idx);
}
//sub_407C70
static int light_render_light_static(struct Light *lgt, int radius, int a3, unsigned int max_1DD41_idx)
{
  return _DK_light_render_light_sub3(lgt, radius, a3, max_1DD41_idx);
}


static char light_render_light(struct Light* lgt)
{
  struct LightAdd* lightadd = get_lightadd(lgt->index);
  int remember_original_lgt_mappos_x = lgt->mappos.x.val;
  int remember_original_lgt_mappos_y = lgt->mappos.y.val;
  if (lightadd->interp_initialize == true) {
    lightadd->interp_initialize = false;
    lightadd->interp_mappos.x.val = lgt->mappos.x.val;
    lightadd->interp_mappos.y.val = lgt->mappos.y.val;
    lightadd->previous_mappos.x.val = lgt->mappos.x.val;
    lightadd->previous_mappos.y.val = lgt->mappos.y.val;
  } else {
    lightadd->interp_mappos.x.val = interpolate(lightadd->interp_mappos.x.val, lightadd->previous_mappos.x.val, lgt->mappos.x.val);
    lightadd->interp_mappos.y.val = interpolate(lightadd->interp_mappos.y.val, lightadd->previous_mappos.y.val, lgt->mappos.y.val);
    lgt->mappos.x.val = lightadd->interp_mappos.x.val;
    lgt->mappos.y.val = lightadd->interp_mappos.y.val;
  }
  // Stop flicker by rounding off position
  lgt->mappos.x.val = ((lgt->mappos.x.val >> 8) << 8);
  lgt->mappos.y.val = ((lgt->mappos.y.val >> 8) << 8);

  int intensity;
  int rand_minimum;
  int v3;
  int v4;
  int range;
  unsigned short light_x_val;
  unsigned short light_y_val;
  int v7;
  int v8_x;
  int v8_y;
  int v11;
  unsigned int v12;
  unsigned short *v13;
  int some_x;
  int some_y;
  int v19;
  int radius;
  int v22;
  unsigned int shadow_cache_pointer;
  int v26;
  int stl_x;
  int stl_y;
  int v30;
  int v31;
  char is_dynamic;
  int v33;

  radius = lgt->radius;
  if ( (lgt->flags2 & 0xFE) != 0 )
  {
    intensity = lgt->intensity;
    rand_minimum = (intensity - 1) << 8;
    v3 = (intensity << 8) + 257;
    v22 = rand_minimum + LIGHT_RANDOM(513);
  }
  else
  {
    v3 = lgt->intensity << 8;
    v22 = v3;
  }
  v4 = radius;
  is_dynamic = lgt->flags & LgtF_Dynamic;
  if ( is_dynamic )
  {
    if ( radius < lgt->min_radius << 8 )
      v4 = lgt->min_radius << 8;
    if ( v3 < lgt->min_intensity << 8 )
      v3 = lgt->min_intensity << 8;
  }
  if ( v3 >= game.lish.field_46149 << 8 )
  {
    range = (v3 - (game.lish.field_46149 << 8)) / (v3 / (v4 / 256)) + 1;
    if ( range >= 31 )
      range = 31;
  }
  else
  {
    range = 0;
  }

  lgt->range = range;

  int lighting_tables_idx = range;
  if ( radius > 0 && v22 > 0 )
  {
    if ( is_dynamic )
    {
      if ( (lgt->flags & LgtF_Unkn40) != 0 )
      {
        ERRORLOG("flag LgtF_Unkn40 is used after all?");
        //lighting_tables_idx = light_render_light_dynamic_1(lgt, radius, v22, lighting_tables_idx);
      }
      else if ( (lgt->flags & LgtF_Unkn08) != 0 )
      {
        lighting_tables_idx = light_render_light_dynamic_2(lgt, radius, v22, lighting_tables_idx);
        lgt->flags &= ~LgtF_Unkn08;
      }
      else
      {
        v7 = lighting_tables_idx << 8;

        light_x_val = lgt->mappos.x.val;
        v8_x = light_x_val - v7;
        if ( v8_x <= 0 )
          v8_x = 0;
        stl_x = v8_x;

        light_y_val = lgt->mappos.y.val;
        v8_y = light_y_val - v7;
        if ( v8_y <= 0 )
          v8_y = 0;
        stl_y = v8_y;

        v11 = v7 + light_x_val;
        if ( v7 + light_x_val >= 0xFFFF )
          v11 = 0xFFFF;
        v12 = light_y_val + v7;
        v26 = v11;
        if ( v12 >= 0xFFFF )
          v12 = 0xFFFF;
        v30 = v12;
        v33 = stl_x / 256 - v11 / 256 + 255;
        some_y = stl_y;

        v13 = game.lish.subtile_lightness + 256 * (stl_y / 256) + stl_x / 256;

        lighting_tables_idx = *game.lish.shadow_cache[lgt->shadow_index].field_1;
        v31 = lighting_tables_idx;
        if ( v30 >= stl_y )
        {
          shadow_cache_pointer = (int)game.lish.shadow_cache[lgt->shadow_index].field_1;
          do
          {
            some_x = stl_x;

            for ( size_t i = 0; some_x <= v26; ++i )
            {
              if ( (light_bitmask[i] & v31) != 0 )
              {
                struct Coord3d pos;
                pos.x.val = some_x;
                pos.y.val = some_y;
                MapCoordDelta dist = get_2d_distance(&lgt->mappos, &pos);

                v19 = v22 * (radius - dist) / radius;
                if ( (unsigned short)*v13 < v19 )
                  *v13 = v19;
              }
              some_x += 256;
              ++v13;
            }

            v13 += v33;
            some_y += 256;
            lighting_tables_idx = *((int*)shadow_cache_pointer + 1);
            shadow_cache_pointer += 4;
            v31 = lighting_tables_idx;
          }
          while ( v30 >= some_y );
        }
      }
    }
    else
    {
      lighting_tables_idx = light_render_light_static(lgt, radius, v22, lighting_tables_idx);
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
  int v11;
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
  if ( starty <= (unsigned int)endy )
  {
    v11 = endy - starty + 1;
    do
    {
      v12 = (short *)v9;
      v13 = v10;
      v9 += 512;
      v10 += 256;
      memcpy(v12, v13, 2 * (endx - startx));
      --v11;
    }
    while ( v11 );
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
