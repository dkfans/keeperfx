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

#include "player_data.h"
#include "map_data.h"

#include "thing_stats.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_light_initialise_lighting_tables(void);

DLLIMPORT TbBool _DK_light_render_light_sub1_sub2(int a1, SubtlCodedCoords stl_num, int a3);
DLLIMPORT char _DK_light_render_light_sub1(struct Light *lgt, int radius, int a3, unsigned int a4);
DLLIMPORT char _DK_light_render_light_sub2(struct Light *lgt, int radius, int a3, unsigned int a4);
DLLIMPORT int _DK_light_render_light_sub3(struct Light *lgt, int radius, int a3, unsigned int a4);
DLLIMPORT int _DK_light_render_light_sub1_sub1(unsigned int a1,unsigned int a2,int a3,unsigned int a4,unsigned int a5,int *a6,int *a7);

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
  // _DK_light_set_light_position(lgt_id, pos);
  struct Light *lgt = &game.lish.lights[lgt_id];
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
          lgt->flags &= 0x7F;
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
        if ( lgt->field_24 < intensity )
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
//sub_4080B0
__int32 light_render_light_sub1_sub1(
        unsigned int a1,
        unsigned int a2,
        int a3,
        unsigned int a4,
        unsigned int a5,
        int *a6,
        int *a7)
{
  return _DK_light_render_light_sub1_sub1(a1,a2,a3,a4,a5,a6,a7);
}

//sub_408530
TbBool light_render_light_sub1_sub2(int a1, SubtlCodedCoords stl_num, int a3)
{
  return _DK_light_render_light_sub1_sub2(a1, stl_num, a3);
/*
    struct Map* mapblk = get_map_block_at_pos(stl_num);
    if (map_block_invalid(mapblk))
        return false;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);


  return (unsigned __int8)*(&game.columns_data[(i + 5 * a1) & 0x7FF)].bitfields) >> 4 > a3;
  */
}

//sub_4072E0
char light_render_light_sub1(struct Light *lgt, int radius, int a3, unsigned int a4)
{
  return _DK_light_render_light_sub1(lgt, radius, a3, a4);
}
//sub_407770
char light_render_light_sub2(struct Light *lgt, int radius, int a3, unsigned int a4)
{
  return _DK_light_render_light_sub2(lgt, radius, a3, a4);
}
//sub_407C70
int light_render_light_sub3(struct Light *lgt, int radius, int a3, unsigned int a4)
{
  return _DK_light_render_light_sub3(lgt, radius, a3, a4);
}


char light_render_light(struct Light* lgt)
{
 int intensity; // ecx
  int v2; // edi
  int v3; // ecx
  int v4; // eax
  int range; // eax
  short light_x_val; // ebx
  int v7; // eax
  int v8_x; // ecx
  int light_y_val; // edx
  int v8_y; // ecx
  int v11; // ecx
  unsigned int v12; // eax
  unsigned short *v13; // ecx
  int some_x; // edi
  int v19; // eax
  int radius; // [esp+10h] [ebp-38h]
  int v22; // [esp+14h] [ebp-34h]
  int some_y; // [esp+18h] [ebp-30h]
  unsigned long shadow_cache_pointer; // [esp+20h] [ebp-28h]
  int v26; // [esp+24h] [ebp-24h]
  int stl_x; // [esp+28h] [ebp-20h]
  int stl_y; // [esp+2Ch] [ebp-1Ch]
  char flags; // [esp+30h] [ebp-18h]
  int v30; // [esp+38h] [ebp-10h]
  int v31; // [esp+3Ch] [ebp-Ch]
  char v32; // [esp+40h] [ebp-8h]
  int v33; // [esp+44h] [ebp-4h]

  radius = lgt->radius;
  if ( (lgt->flags2 & 0xFE) != 0 )
  {
    intensity = lgt->intensity;
    v2 = (intensity - 1) << 8;
    v3 = (intensity << 8) + 257;
    v22 = v2 + LIGHT_RANDOM(513);
  }
  else
  {
    v3 = lgt->intensity << 8;
    v22 = v3;
  }
  v4 = radius;
  flags = lgt->flags;
  v32 = lgt->flags & 4;
  if ( v32 )
  {
    if ( radius < lgt->field_9 << 8 )
      v4 = lgt->field_9 << 8;
    if ( v3 < lgt->field_24 << 8 )
      v3 = lgt->field_24 << 8;
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
  if ( radius > 0 && v22 > 0 )
  {
    if ( v32 )
    {
      if ( (flags & 0x40) != 0 )
      {
        range = light_render_light_sub1(lgt, radius, v22, range);
      }
      else if ( (flags & 8) != 0 )
      {
        range = light_render_light_sub2(lgt, radius, v22, range);
        lgt->flags &= ~8u;
      }
      else
      {
        v7 = range << 8;

        light_x_val = lgt->mappos.x.val;
        v8_x = (unsigned __int16)light_x_val - v7;
        if ( v8_x <= 0 )
          v8_x = 0;
        stl_x = v8_x;

        light_y_val = lgt->mappos.y.val;
        v8_y = (unsigned __int16)light_y_val - v7;
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

        range = *game.lish.shadow_cache[lgt->shadow_index].field_1;
        v31 = range;
        if ( v30 >= stl_y )
        {
          shadow_cache_pointer = game.lish.shadow_cache[lgt->shadow_index].field_1;
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
                if ( (unsigned __int16)*v13 < v19 )
                  *v13 = v19;
              }
              some_x += 256;
              ++v13;
            }

            v13 += v33;
            some_y += 256;
            range = *((int*)shadow_cache_pointer + 1);
            shadow_cache_pointer += 4;
            v31 = range;
          }
          while ( v30 >= some_y );
        }
      }
    }
    else
    {
      range = light_render_light_sub3(lgt, radius, v22, range);
    }
  }
  return range;
}

static void light_render_area(MapSubtlCoord startx, MapSubtlCoord starty, MapSubtlCoord endx, MapSubtlCoord endy)
{
  struct Light *lgt; // esi
  int range; // ebx
  char *v9; // edx
  unsigned __int16 *v10; // ebx
  int v11; // ebp
  __int16 *v12; // edi
  unsigned __int16 *v13; // esi
  unsigned __int8 v17; // al
  unsigned __int8 v18; // cl
  unsigned __int8 v19; // cl
  unsigned __int8 v20; // al
  __int16 v21; // ax
  MapSubtlDelta half_width_y; // [esp+10h] [ebp-10h]
  MapSubtlDelta half_width_x; // [esp+14h] [ebp-Ch]

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
      if ( (lgt->flags & 0x88) != 0 )
      {
        ++light_out_of_date_stat_lights;
        range = lgt->range;

        

        if ( (int)abs(half_width_x + startx - lgt->mappos.x.stl.num) < half_width_x + range 
          && (int)abs(half_width_y + starty - lgt->mappos.y.stl.num) < half_width_y + range )
        {
          ++light_updated_stat_lights;
          light_render_light(lgt);
          lgt->flags &= 0x77;
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
      v12 = (__int16 *)v9;
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
        if ( (lgt->flags & 0x10) != 0 )
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
        if ( (lgt->flags & 0x20) != 0 )
        {
          if ( lgt->field_3 == 1 )
          {
            v17 = lgt->intensity;
            v18 = lgt->field_7;
            if ( lgt->field_4 + v17 >= v18 )
            {
              lgt->intensity = v18;
              lgt->field_3 = 2;
            }
            else
            {
              lgt->intensity = lgt->field_4 + v17;
            }
          }
          else
          {
            v19 = lgt->intensity;
            v20 = lgt->field_7;
            if ( v19 - lgt->field_4 <= v20 )
            {
              lgt->intensity = v20;
              lgt->field_3 = 1;
            }
            else
            {
              lgt->intensity = v19 - lgt->field_4;
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
          lgt->field_9 = a2;
          lgt->field_24 = a3;
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
