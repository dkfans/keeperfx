/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_render_trig.c
 *     Rendering function trig() for drawing 3D view elements.
 * @par Purpose:
 *     Function for rendering 3D elements.
 * @par Comment:
 *     Go away from here, you bad optimizer! Do not compile this with optimizations.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 30 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_render.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "post_inc.h"

#include "vidmode.h"

/******************************************************************************/
#pragma pack(1)

enum RenderingStartType {
    RendStart_NO = 0,
    RendStart_LL,
    RendStart_RL,
    RendStart_FB,
    RendStart_FT,
};

enum RenderingVectorMode {
    RendVec_mode00 = 0,
    RendVec_mode01,
    RendVec_mode02,
    RendVec_mode03,
    RendVec_mode04,
    RendVec_mode05,
    RendVec_mode06,
    RendVec_mode07,
    RendVec_mode08,
    RendVec_mode09,
    RendVec_mode10,
    RendVec_mode11,
    RendVec_mode12,
    RendVec_mode13,
    RendVec_mode14,
    RendVec_mode15,
    RendVec_mode16,
    RendVec_mode17,
    RendVec_mode18,
    RendVec_mode19,
    RendVec_mode20,
    RendVec_mode21,
    RendVec_mode22,
    RendVec_mode23,
    RendVec_mode24,
    RendVec_mode25,
    RendVec_mode26,
};

struct TrigLocals {
    unsigned long zero0;// dummy, to make no offset 0
    unsigned char unused_24;// 4+
    unsigned char unused_25; // 5+
    union {
        unsigned short combined_flags; // 6+
    struct {
        unsigned char flags_low_byte;
        unsigned char flags_high_byte;
    };
    };
    unsigned long x_step_ac; // 8+
    unsigned long unusedparam; // unused
    // These are DWORDs
    unsigned long x_step_bc; // 0x10+
    unsigned long delta_f; // 0x14
    long y_start; // -0x18
    unsigned long delta_g; // 0x1C
    unsigned long x_start_b; // 0x20
    unsigned long render_height; // 0x24
    unsigned long u_step; // 0x28
    unsigned long delta_e; // 0x2C
    union {
    unsigned long texture_v_step_bc; // 0x30
    struct {
        unsigned short value_low_word;
        unsigned short value_high_word;
    };
    };
    unsigned long v_step; // 0x34
    unsigned long delta_d; // 0x38
    unsigned long texture_u_step_bc; // 0x3C
    unsigned long shade_step; // 0x40
    unsigned long delta_c; // 0x44
    unsigned long shade_step_bc; // 0x48
    unsigned long clip_offset; // 0x4C
    unsigned long delta_h; // 0x50
    unsigned long delta_i; // 0x54
    unsigned long y_top; // 0x58
    unsigned long delta_j; // 0x5C
    unsigned long delta_k; // 0x60
    unsigned long delta_b; // 0x64
    unsigned long delta_a; // 0x68
    unsigned char *clipping_below_viewport; // 0x6C
};

struct TrigLocalPrep {
    long x_step_ac;
    long x_step_ab;
    long x_step_bc;
    long trig_height_top; // counter to loop over first part of polyscans array
    long y_start;
    long trig_height_bottom; // counter to loop over second part of polyscans array
    long x_start_b;
    long u_step_ac;
    long texture_v_step_bc;
    long v_step_ac;
    long texture_u_step_bc;
    long shade_step_ac;
    long shade_step_bc;
    long clip_offset;
    long y_top;
    unsigned char clipping_above_viewport;
    TbBool hide_bottom_part; // ?Should we show low part of a triangle
    unsigned char clipping_below_viewport;
};

struct TrigLocalRend {
    unsigned char *screen_buffer_ptr;
    long render_height;
    long u_step;
    long v_step;
    long shade_step;
};

#pragma pack()
/******************************************************************************/

/**
 * whether the subtraction (x-y) of two long ints would overflow
 */
static inline unsigned char __OFSUBL__(long x, long y)
{
    return ((x < 0) ^ (y < 0)) & ((x < 0) ^ (x-y < 0));
}

unsigned char trig_reorder_input_points(struct PolyPoint **opt_a,
  struct PolyPoint **opt_b, struct PolyPoint **opt_c)
{
    struct PolyPoint *ordpt_a;
    struct PolyPoint *ordpt_b;
    struct PolyPoint *ordpt_c;
    unsigned char start_type;

    ordpt_a = *opt_a;
    ordpt_b = *opt_b;
    ordpt_c = *opt_c;
    if (ordpt_a->Y == ordpt_b->Y)
    {
        if (ordpt_a->Y == ordpt_c->Y)
            return RendStart_NO;
        if (ordpt_a->Y >= ordpt_c->Y) {
            if (ordpt_a->X <= ordpt_b->X)
                return RendStart_NO;
            ordpt_a = *opt_c;
            ordpt_b = *opt_a;
            ordpt_c = *opt_b;
            start_type = RendStart_FB;
        } else {
            if (ordpt_b->X <= ordpt_a->X)
                return RendStart_NO;
            start_type = RendStart_FT;
        }
    }
    else if (ordpt_a->Y > ordpt_b->Y)
    {
        if (ordpt_a->Y == ordpt_c->Y)
        {
            if (ordpt_c->X <= ordpt_a->X)
                return RendStart_NO;
            ordpt_a = *opt_b;
            ordpt_b = *opt_c;
            ordpt_c = *opt_a;
            start_type = RendStart_FB;
        }
        else if (ordpt_a->Y < ordpt_c->Y)
        {
            ordpt_a = *opt_b;
            ordpt_b = *opt_c;
            ordpt_c = *opt_a;
            start_type = RendStart_RL;
        }
        else if (ordpt_b->Y == ordpt_c->Y)
        {
            if (ordpt_c->X <= ordpt_b->X)
                return RendStart_NO;
            ordpt_a = *opt_b;
            ordpt_b = *opt_c;
            ordpt_c = *opt_a;
            start_type = RendStart_FT;
        }
        else if (ordpt_b->Y < ordpt_c->Y)
        {
            ordpt_a = *opt_b;
            ordpt_b = *opt_c;
            ordpt_c = *opt_a;
            start_type = RendStart_LL;
        }
        else
        {
            ordpt_a = *opt_c;
            ordpt_b = *opt_a;
            ordpt_c = *opt_b;
            start_type = RendStart_RL;
        }
    }
    else // if (ordpt_a->Y < ordpt_b->Y)
    {
        if (ordpt_a->Y == ordpt_c->Y)
        {
            if (ordpt_a->X <= ordpt_c->X)
                return RendStart_NO;
            ordpt_a = *opt_c;
            ordpt_b = *opt_a;
            ordpt_c = *opt_b;
            start_type = RendStart_FT;
        }
        else if (ordpt_a->Y >= ordpt_c->Y)
        {
            ordpt_a = *opt_c;
            ordpt_b = *opt_a;
            ordpt_c = *opt_b;
            start_type = RendStart_LL;
        }
        else if (ordpt_b->Y == ordpt_c->Y)
        {
            if (ordpt_b->X <= ordpt_c->X)
                return RendStart_NO;
            start_type = RendStart_FB;
        }
        else if (ordpt_b->Y <= ordpt_c->Y)
        {
            start_type = RendStart_LL;
        }
        else
        {
            start_type = RendStart_RL;
        }
    }

    *opt_a = ordpt_a;
    *opt_b = ordpt_b;
    *opt_c = ordpt_c;
    return start_type;
}

static inline int trig_ll_md00(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y_a, point_y_b;
    struct PolyPoint *polygon_point;

    point_x = opt_a->X << 16;
    point_y_a = opt_a->X << 16;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        // whether the addition (tlr->render_height + tlp->y_top) would overflow
        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        if (tlp->clip_offset - tlp->y_start >= 0)
        {
            tlp->trig_height_bottom -= tlp->clip_offset - tlp->y_start;
            tlp->clip_offset -= tlp->y_start;
            point_x += tlp->x_step_ac * tlp->clip_offset + tlp->y_start * tlp->x_step_ac;
            point_y_b = tlp->x_step_bc * tlp->clip_offset + tlp->x_start_b;
            if (tlp->clipping_below_viewport)
            {
                tlp->trig_height_bottom = vec_window_height;
                tlr->render_height = vec_window_height;
            }
            tlp->y_start = 0;
        }
        else
        {
            tlp->y_start -= tlp->clip_offset;
            point_x += tlp->x_step_ac * tlp->clip_offset;
            point_y_a += tlp->clip_offset * tlp->x_step_ab;
            if (tlp->clipping_below_viewport)
            {
                tlr->render_height = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->y_start = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->y_start;
                    tlp->trig_height_bottom = vec_window_height - tlp->y_start;
                }
            }
            point_y_b = tlp->x_start_b;
        }
    }
    else
    {
        if (tlp->clipping_below_viewport)
        {
            long delta_height, extent_height;
            TbBool extent_height_overflow;

            delta_height = vec_window_height - tlp->y_top;
            tlr->render_height = delta_height;
            if (tlp->hide_bottom_part) {
                tlp->y_start = delta_height;
            } else {
                // whether the subtraction (delta_height - tlp->y_start) would overflow
                extent_height_overflow = __OFSUBL__(delta_height, tlp->y_start);
                extent_height = delta_height - tlp->y_start;
                tlp->hide_bottom_part = ((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0);
                tlp->trig_height_bottom = extent_height;
            }
        }
        point_y_b = tlp->x_start_b;
    }
    polygon_point = polyscans;
    for (; tlp->y_start; tlp->y_start--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y_a;
        point_y_a += tlp->x_step_ab;
        ++polygon_point;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            polygon_point->X = point_x;
            point_x += tlp->x_step_ac;
            polygon_point->Y = point_y_b;
            point_y_b += tlp->x_step_bc;
            ++polygon_point;
        }
    }
    return 1;
}

static inline int trig_ll_md01(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    struct PolyPoint *polygon_point;
    long point_x, point_y_a, point_y_b;
    long shade_value;
    long triangle_height_ratio;

    triangle_height_ratio = (tlp->y_start << 16) / tlp->trig_height_top;
    {
        long delta_x, weighted_x;
        long extent_x;
        TbBool eX_overflow;

        delta_x = opt_a->X - opt_c->X;
        weighted_x = (triangle_height_ratio * delta_x) >> 16;
        delta_x = opt_b->X - opt_a->X;
        // whether the addition (weighted_x + delta_x) would overflow
        eX_overflow = __OFSUBL__(weighted_x, -delta_x);
        extent_x = weighted_x + delta_x;
        if ((extent_x < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)weighted_x, (long)delta_x);
            return 0;
        }
        if (extent_x != 0) {
            long long delta_shade, weighted_shade;
            delta_shade = opt_a->S - opt_c->S;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->shade_step = (opt_b->S + weighted_shade - opt_a->S) / (extent_x + 1);
        }
    }
    tlp->shade_step_ac = (opt_c->S - opt_a->S) / tlp->trig_height_top;
    point_x = opt_a->X << 16;
    point_y_a = opt_a->X << 16;
    shade_value = opt_a->S;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        if (tlp->clip_offset - tlp->y_start >= 0)
        {
            tlp->trig_height_bottom -= tlp->clip_offset - tlp->y_start;
            tlp->clip_offset -= tlp->y_start;
            point_x += tlp->x_step_ac * tlp->clip_offset + tlp->y_start * tlp->x_step_ac;
            point_y_b = tlp->x_step_bc * tlp->clip_offset + tlp->x_start_b;
            shade_value += tlp->clip_offset * tlp->shade_step_ac + tlp->y_start * tlp->shade_step_ac;
            if (tlp->clipping_below_viewport)
            {
              tlp->trig_height_bottom = vec_window_height;
              tlr->render_height = vec_window_height;
            }
            tlp->y_start = 0;
        }
        else
        {
            tlp->y_start -= tlp->clip_offset;
            point_x += tlp->x_step_ac * tlp->clip_offset;
            point_y_a += tlp->clip_offset * tlp->x_step_ab;
            shade_value += tlp->clip_offset * tlp->shade_step_ac;
            if (tlp->clipping_below_viewport)
            {
                tlr->render_height = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->y_start = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->y_start;
                    tlp->trig_height_bottom = vec_window_height - tlp->y_start;
                }
            }
            point_y_b = tlp->x_start_b;
        }
    }
    else
    {
        if (tlp->clipping_below_viewport)
        {
            long delta_height, extent_height;
            TbBool extent_height_overflow;

            delta_height = vec_window_height - tlp->y_top;
            tlr->render_height = delta_height;
            if (tlp->hide_bottom_part) {
                tlp->y_start = delta_height;
            } else {
                extent_height_overflow = __OFSUBL__(delta_height, tlp->y_start);
                extent_height = delta_height - tlp->y_start;
                tlp->hide_bottom_part = ((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0);
                tlp->trig_height_bottom = extent_height;
            }
        }
        point_y_b = tlp->x_start_b;
    }
    polygon_point = polyscans;
    for (; tlp->y_start; tlp->y_start--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y_a;
        point_y_a += tlp->x_step_ab;
        polygon_point->S = shade_value;
        shade_value += tlp->shade_step_ac;
        ++polygon_point;
    }
    if (!tlp->hide_bottom_part)
    {
      for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
      {
          polygon_point->X = point_x;
          point_x += tlp->x_step_ac;
          polygon_point->Y = point_y_b;
          point_y_b += tlp->x_step_bc;
          polygon_point->S = shade_value;
          shade_value += tlp->shade_step_ac;
          ++polygon_point;
      }
    }
    return 1;
}

static inline int trig_ll_md02(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y_a, point_y_b;
    long texture_u, texture_v;
    struct PolyPoint *polygon_point;
    long triangle_height_ratio;

    triangle_height_ratio = (tlp->y_start << 16) / tlp->trig_height_top;
    {
        long delta_x, weighted_x;
        long extent_x;
        TbBool eX_overflow;

        delta_x = opt_a->X - opt_c->X;
        weighted_x = triangle_height_ratio * delta_x >> 16;
        delta_x = opt_b->X - opt_a->X;
        eX_overflow = __OFSUBL__(weighted_x, -delta_x);
        extent_x = weighted_x + delta_x;
        if ((extent_x < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)weighted_x, (long)delta_x);
            return 0;
        }
        if (extent_x != 0) {
            long long delta_shade, weighted_shade;
            delta_shade = opt_a->U - opt_c->U;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->u_step = (opt_b->U + weighted_shade - opt_a->U) / (extent_x + 1);
            delta_shade = opt_a->V - opt_c->V;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->v_step = (opt_b->V + weighted_shade - opt_a->V) / (extent_x + 1);
        }
    }
    tlp->u_step_ac = (opt_c->U - opt_a->U) / tlp->trig_height_top;
    tlp->v_step_ac = (opt_c->V - opt_a->V) / tlp->trig_height_top;
    point_x = opt_a->X << 16;
    point_y_a = opt_a->X << 16;
    texture_u = opt_a->U;
    texture_v = opt_a->V;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        if (tlp->clip_offset - tlp->y_start >= 0 )
        {
            tlp->trig_height_bottom -= tlp->clip_offset - tlp->y_start;
            tlp->clip_offset -= tlp->y_start;
            point_x += tlp->x_step_ac * tlp->clip_offset + tlp->y_start * tlp->x_step_ac;
            point_y_b = tlp->x_step_bc * tlp->clip_offset + tlp->x_start_b;
            texture_u += tlp->clip_offset * tlp->u_step_ac + tlp->y_start * tlp->u_step_ac;
            texture_v += tlp->clip_offset * tlp->v_step_ac + tlp->y_start * tlp->v_step_ac;
            if ( tlp->clipping_below_viewport )
            {
                tlp->trig_height_bottom = vec_window_height;
                tlr->render_height = vec_window_height;
            }
            tlp->y_start = 0;
        }
        else
        {
            tlp->y_start -= tlp->clip_offset;
            point_x += tlp->x_step_ac * tlp->clip_offset;
            point_y_a += tlp->clip_offset * tlp->x_step_ab;
            texture_u += tlp->clip_offset * tlp->u_step_ac;
            texture_v += tlp->clip_offset * tlp->v_step_ac;
            if ( tlp->clipping_below_viewport )
            {
                tlr->render_height = vec_window_height;
                if (tlp->hide_bottom_part) {
                  tlp->y_start = vec_window_height;
                } else {
                  tlp->hide_bottom_part = vec_window_height <= tlp->y_start;
                  tlp->trig_height_bottom = vec_window_height - tlp->y_start;
                }
            }
            point_y_b = tlp->x_start_b;
        }
    }
    else
    {
        if (tlp->clipping_below_viewport)
        {
            long delta_height, extent_height;
            TbBool extent_height_overflow;

            delta_height = vec_window_height - tlp->y_top;
            tlr->render_height = delta_height;
            if (tlp->hide_bottom_part) {
                tlp->y_start = delta_height;
            } else {
                extent_height_overflow = __OFSUBL__(delta_height, tlp->y_start);
                extent_height = delta_height - tlp->y_start;
                tlp->hide_bottom_part = ((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0);
                tlp->trig_height_bottom = extent_height;
            }
        }
        point_y_b = tlp->x_start_b;
    }
    polygon_point = polyscans;
    for (; tlp->y_start; tlp->y_start--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y_a;
        point_y_a += tlp->x_step_ab;
        polygon_point->U = texture_u;
        texture_u += tlp->u_step_ac;
        polygon_point->V = texture_v;
        texture_v += tlp->v_step_ac;
        ++polygon_point;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            polygon_point->X = point_x;
            point_x += tlp->x_step_ac;
            polygon_point->Y = point_y_b;
            point_y_b += tlp->x_step_bc;
            polygon_point->U = texture_u;
            texture_u += tlp->u_step_ac;
            polygon_point->V = texture_v;
            texture_v += tlp->v_step_ac;
            ++polygon_point;
        }
    }
    return 1;
}

static inline int trig_ll_md05(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y_a, point_y_b;
    long texture_u, texture_v, shade_value;
    struct PolyPoint *polygon_point;
    long triangle_height_ratio;

    triangle_height_ratio = (tlp->y_start << 16) / tlp->trig_height_top;
    {
        long delta_x, weighted_x;
        long extent_x;
        TbBool eX_overflow;

        delta_x = opt_a->X - opt_c->X;
        weighted_x = triangle_height_ratio * delta_x >> 16;
        delta_x = opt_b->X - opt_a->X;
        eX_overflow = __OFSUBL__(weighted_x, -delta_x);
        extent_x = weighted_x + delta_x;
        if ((extent_x < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)weighted_x, (long)delta_x);
            return 0;
        }
        if (extent_x != 0)
        {
            long long delta_shade, weighted_shade;
            delta_shade = opt_a->U - opt_c->U;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->u_step = (opt_b->U + weighted_shade - opt_a->U) / (extent_x + 1);
            delta_shade = opt_a->V - opt_c->V;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->v_step = (opt_b->V + weighted_shade - opt_a->V) / (extent_x + 1);
            delta_shade = opt_a->S - opt_c->S;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->shade_step = (opt_b->S + weighted_shade - opt_a->S) / (extent_x + 1);
        }
    }
    tlp->u_step_ac = (opt_c->U - opt_a->U) / tlp->trig_height_top;
    tlp->v_step_ac = (opt_c->V - opt_a->V) / tlp->trig_height_top;
    tlp->shade_step_ac = (opt_c->S - opt_a->S) / tlp->trig_height_top;

    point_x = opt_a->X << 16;
    point_y_a = opt_a->X << 16;
    texture_u = opt_a->U;
    texture_v = opt_a->V;
    shade_value = opt_a->S;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        if (tlp->clip_offset - tlp->y_start >= 0)
        {
            tlp->trig_height_bottom -= tlp->clip_offset - tlp->y_start;
            tlp->clip_offset -= tlp->y_start;
            point_x += tlp->x_step_ac * tlp->clip_offset + tlp->y_start * tlp->x_step_ac;
            point_y_b = tlp->x_step_bc * tlp->clip_offset + tlp->x_start_b;
            texture_u += tlp->clip_offset * tlp->u_step_ac + tlp->y_start * tlp->u_step_ac;
            texture_v += tlp->clip_offset * tlp->v_step_ac + tlp->y_start * tlp->v_step_ac;
            shade_value += tlp->clip_offset * tlp->shade_step_ac + tlp->y_start * tlp->shade_step_ac;
            if (tlp->clipping_below_viewport) {
              tlp->trig_height_bottom = vec_window_height;
              tlr->render_height = vec_window_height;
            }
            tlp->y_start = 0;
        }
        else
        {
            tlp->y_start -= tlp->clip_offset;
            point_x += tlp->x_step_ac * tlp->clip_offset;
            point_y_a += tlp->clip_offset * tlp->x_step_ab;
            texture_u += tlp->clip_offset * tlp->u_step_ac;
            texture_v += tlp->clip_offset * tlp->v_step_ac;
            shade_value += tlp->clip_offset * tlp->shade_step_ac;
            if (tlp->clipping_below_viewport)
            {
                tlr->render_height = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->y_start = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->y_start;
                    tlp->trig_height_bottom = vec_window_height - tlp->y_start;
                }
            }
            point_y_b = tlp->x_start_b;
        }
    }
    else
    {
        if (tlp->clipping_below_viewport)
        {
            long delta_height, extent_height;
            TbBool extent_height_overflow;

            delta_height = vec_window_height - tlp->y_top;
            tlr->render_height = vec_window_height - tlp->y_top;
            if (tlp->hide_bottom_part) {
                tlp->y_start = vec_window_height - tlp->y_top;
            } else {
                extent_height_overflow = __OFSUBL__(delta_height, tlp->y_start);
                extent_height = delta_height - tlp->y_start;
                tlp->hide_bottom_part = ((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0);
                tlp->trig_height_bottom = extent_height;
            }
        }
        point_y_b = tlp->x_start_b;
    }
    polygon_point = polyscans;
    for (; tlp->y_start; tlp->y_start--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y_a;
        point_y_a += tlp->x_step_ab;
        polygon_point->U = texture_u;
        texture_u += tlp->u_step_ac;
        polygon_point->V = texture_v;
        texture_v += tlp->v_step_ac;
        polygon_point->S = shade_value;
        shade_value += tlp->shade_step_ac;
        ++polygon_point;
    }
    if ( !tlp->hide_bottom_part )
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
          polygon_point->X = point_x;
          point_x += tlp->x_step_ac;
          polygon_point->Y = point_y_b;
          point_y_b += tlp->x_step_bc;
          polygon_point->U = texture_u;
          texture_u += tlp->u_step_ac;
          polygon_point->V = texture_v;
          texture_v += tlp->v_step_ac;
          polygon_point->S = shade_value;
          shade_value += tlp->shade_step_ac;
          ++polygon_point;
        }
    }
    return 1;
}

int trig_ll_start(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int ret;
    long delta_x, delta_y;

    tlp->y_top = opt_a->Y;
    if (opt_a->Y < 0) {
      tlr->screen_buffer_ptr = poly_screen;
      tlp->clipping_above_viewport = 1;
    } else if (opt_a->Y < vec_window_height) {
      tlr->screen_buffer_ptr = poly_screen + vec_screen_width * opt_a->Y;
      tlp->clipping_above_viewport = 0;
    } else {
        NOLOG("height %ld exceeded by opt_a Y %ld", (long)vec_window_height, (long)opt_a->Y);
        return 0;
    }

    tlp->clipping_below_viewport = opt_c->Y > vec_window_height;
    delta_y = opt_c->Y - opt_a->Y;
    tlp->trig_height_top = delta_y;
    tlr->render_height = delta_y;

    tlp->hide_bottom_part = opt_b->Y > vec_window_height;
    delta_y = opt_b->Y - opt_a->Y;
    tlp->y_start = delta_y;
    delta_x = opt_c->X - opt_a->X;
    tlp->x_step_ac = (delta_x << 16) / tlp->trig_height_top;
    delta_x = opt_b->X - opt_a->X;
    if ((delta_x << 16) / delta_y <= tlp->x_step_ac) {
        NOLOG("value (%ld << 16) / %ld below min %ld", (long)delta_x, (long)delta_y, (long)tlp->x_step_ac);
        return 0;
    }
    tlp->x_step_ab = (delta_x << 16) / delta_y;

    delta_y = opt_c->Y - opt_b->Y;
    delta_x = opt_c->X - opt_b->X;
    tlp->x_step_bc = (delta_x << 16) / delta_y;
    tlp->trig_height_bottom = delta_y;
    tlp->x_start_b = opt_b->X << 16;

    ret = 0;
    switch (vec_mode) /* swars-final @ 0x120F07 */
    {
    case RendVec_mode00:
    case RendVec_mode14:
    case RendVec_mode15:
        ret = trig_ll_md00(tlp, tlr, opt_a, opt_b, opt_c);
        break;

    case RendVec_mode01:
    case RendVec_mode04:
    case RendVec_mode16:
    case RendVec_mode17:
        ret = trig_ll_md01(tlp, tlr, opt_a, opt_b, opt_c);
        break;

    case RendVec_mode02:
    case RendVec_mode03:
    case RendVec_mode07:
    case RendVec_mode08:
    case RendVec_mode09:
    case RendVec_mode10:
    case RendVec_mode11:
    case RendVec_mode12:
    case RendVec_mode13:
    case RendVec_mode18:
    case RendVec_mode19:
    case RendVec_mode22:
    case RendVec_mode23:
        ret = trig_ll_md02(tlp, tlr, opt_a, opt_b, opt_c);
        break;

    case RendVec_mode05:
    case RendVec_mode06:
    case RendVec_mode20:
    case RendVec_mode21:
    case RendVec_mode24:
    case RendVec_mode25:
    case RendVec_mode26:
        ret = trig_ll_md05(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    }

    return ret;
}

static inline int trig_rl_md00(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    ulong point_x_a, point_x_b, point_y;
    struct PolyPoint *polygon_point;

    point_x_a = opt_a->X << 16;
    point_y = opt_a->X << 16;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        if (tlp->clip_offset - tlp->trig_height_top >= 0)
        {
            tlp->clip_offset -= tlp->trig_height_top;
            tlp->trig_height_bottom -= tlp->clip_offset;
            point_x_b = tlp->x_step_bc * tlp->clip_offset + tlp->x_start_b;
            point_y += tlp->clip_offset * tlp->x_step_ab + tlp->trig_height_top * tlp->x_step_ab;
            if (tlp->clipping_below_viewport) {
              tlp->trig_height_bottom = vec_window_height;
              tlr->render_height = vec_window_height;
            }
            tlp->trig_height_top = 0;
        }
        else
        {
            tlp->trig_height_top -= tlp->clip_offset;
            point_x_a += tlp->x_step_ac * tlp->clip_offset;
            point_y += tlp->clip_offset * tlp->x_step_ab;
            if (tlp->clipping_below_viewport)
            {
                tlr->render_height = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->trig_height_top = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->trig_height_top;
                    tlp->trig_height_bottom = vec_window_height - tlp->trig_height_top;
                }
            }
            point_x_b = tlp->x_start_b;
        }
    }
    else
    {
        if (tlp->clipping_below_viewport)
        {
            long delta_height, extent_height;
            TbBool extent_height_overflow;

            delta_height = vec_window_height - tlp->y_top;
            tlr->render_height = delta_height;
            if (tlp->hide_bottom_part) {
                tlp->trig_height_top = delta_height;
            } else {
                extent_height_overflow = __OFSUBL__(delta_height, tlp->trig_height_top);
                extent_height = delta_height - tlp->trig_height_top;
                tlp->hide_bottom_part = ((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0);
                tlp->trig_height_bottom = extent_height;
            }
        }
        point_x_b = tlp->x_start_b;
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x_a;
        point_x_a += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        ++polygon_point;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            polygon_point->X = point_x_b;
            point_x_b += tlp->x_step_bc;
            polygon_point->Y = point_y;
            point_y += tlp->x_step_ab;
            ++polygon_point;
        }
    }
    return 1;
}

static inline int trig_rl_md01(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x_a, point_x_b, point_y;
    long shade_value;
    struct PolyPoint *polygon_point;
    long triangle_height_ratio;

    triangle_height_ratio = (tlp->trig_height_top << 16) / tlp->y_start;
    {
        long dXa, wXb;
        long extent_x;
        TbBool eX_overflow;

        wXb = triangle_height_ratio * (opt_b->X - opt_a->X) >> 16;
        dXa = opt_a->X - opt_c->X;
        eX_overflow = __OFSUBL__(wXb, -dXa);
        extent_x = wXb + dXa;
        if ((extent_x < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)wXb, (long)dXa);
            return 0;
        }
        if (extent_x != 0) {
            long long delta_shade, weighted_shade;
            delta_shade = opt_b->S - opt_a->S;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->shade_step = (opt_a->S + weighted_shade - opt_c->S) / (extent_x + 1);
        }
    }
    tlp->shade_step_ac = (opt_c->S - opt_a->S) / tlp->trig_height_top;
    tlp->shade_step_bc = (opt_b->S - opt_c->S) / tlp->trig_height_bottom;
    point_x_a = opt_a->X << 16;
    point_y = opt_a->X << 16;
    shade_value = opt_a->S;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        if (tlp->clip_offset - tlp->trig_height_top >= 0)
        {
            tlp->clip_offset -= tlp->trig_height_top;
            tlp->trig_height_bottom -= tlp->clip_offset;
            point_x_b = tlp->x_step_bc * tlp->clip_offset + tlp->x_start_b;
            point_y += tlp->clip_offset * tlp->x_step_ab + tlp->trig_height_top * tlp->x_step_ab;
            shade_value += tlp->clip_offset * tlp->shade_step_bc + tlp->trig_height_top * tlp->shade_step_ac;
            if (tlp->clipping_below_viewport) {
                tlp->trig_height_bottom = vec_window_height;
                tlr->render_height = vec_window_height;
            }
            tlp->trig_height_top = 0;
        }
        else
        {
            tlp->trig_height_top -= tlp->clip_offset;
            point_x_a += tlp->x_step_ac * tlp->clip_offset;
            point_y += tlp->clip_offset * tlp->x_step_ab;
            shade_value += tlp->clip_offset * tlp->shade_step_ac;
            if ( tlp->clipping_below_viewport )
            {
                tlr->render_height = vec_window_height;
                if ( tlp->hide_bottom_part )
                {
                  tlp->trig_height_top = vec_window_height;
                }
                else
                {
                  tlp->hide_bottom_part = vec_window_height <= tlp->trig_height_top;
                  tlp->trig_height_bottom = vec_window_height - tlp->trig_height_top;
                }
            }
            point_x_b = tlp->x_start_b;
        }
    }
    else
    {
        if (tlp->clipping_below_viewport)
        {
            long delta_height, extent_height;
            TbBool extent_height_overflow;

            delta_height = vec_window_height - tlp->y_top;
            tlr->render_height = vec_window_height - tlp->y_top;
            if (tlp->hide_bottom_part) {
                tlp->trig_height_top = vec_window_height - tlp->y_top;
            } else {
                extent_height_overflow = __OFSUBL__(delta_height, tlp->trig_height_top);
                extent_height = delta_height - tlp->trig_height_top;
                tlp->hide_bottom_part = ((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0);
                tlp->trig_height_bottom = extent_height;
            }
        }
        point_x_b = tlp->x_start_b;
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x_a;
        point_x_a += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        polygon_point->S = shade_value;
        shade_value += tlp->shade_step_ac;
        ++polygon_point;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            polygon_point->X = point_x_b;
            point_x_b += tlp->x_step_bc;
            polygon_point->Y = point_y;
            point_y += tlp->x_step_ab;
            polygon_point->S = shade_value;
            shade_value += tlp->shade_step_bc;
            ++polygon_point;
        }
    }
    return 1;
}

static inline int trig_rl_md02(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x_a, point_x_b, point_y;
    long texture_u, texture_v;
    struct PolyPoint *polygon_point;
    long triangle_height_ratio;

    triangle_height_ratio = (tlp->trig_height_top << 16) / tlp->y_start; // Fixed point math
    {
        long dXa, wXb;
        long extent_x;
        TbBool eX_overflow;

        wXb = triangle_height_ratio * (opt_b->X - opt_a->X) >> 16;
        dXa = opt_a->X - opt_c->X;
        eX_overflow = __OFSUBL__(wXb, -dXa);
        extent_x = wXb + dXa;
        if ((extent_x < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)wXb, (long)dXa);
            return 0;
        }
        if (extent_x != 0) {
            long long delta_shade, weighted_shade;

            delta_shade = opt_b->U - opt_a->U;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->u_step = (opt_a->U + weighted_shade - opt_c->U) / (extent_x + 1);
            delta_shade = opt_b->V - opt_a->V;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->v_step = (opt_a->V + weighted_shade - opt_c->V) / (extent_x + 1);
        }
    }
    tlp->u_step_ac = (opt_c->U - opt_a->U) / tlp->trig_height_top;
    tlp->v_step_ac = (opt_c->V - opt_a->V) / tlp->trig_height_top;
    tlp->texture_v_step_bc = (opt_b->U - opt_c->U) / tlp->trig_height_bottom;
    tlp->texture_u_step_bc = (opt_b->V - opt_c->V) / tlp->trig_height_bottom;
    point_x_a = opt_a->X << 16;
    point_y = opt_a->X << 16;
    texture_u = opt_a->U;
    texture_v = opt_a->V;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        if (tlp->clip_offset - tlp->trig_height_top >= 0)
        {
            tlp->clip_offset -= tlp->trig_height_top;
            tlp->trig_height_bottom -= tlp->clip_offset;
            point_x_b = tlp->x_step_bc * tlp->clip_offset + tlp->x_start_b;
            point_y += tlp->clip_offset * tlp->x_step_ab + tlp->trig_height_top * tlp->x_step_ab;
            texture_u += tlp->clip_offset * tlp->texture_v_step_bc + tlp->trig_height_top * tlp->u_step_ac;
            texture_v += tlp->clip_offset * tlp->texture_u_step_bc + tlp->trig_height_top * tlp->v_step_ac;
            if (tlp->clipping_below_viewport) {
                tlp->trig_height_bottom = vec_window_height;
                tlr->render_height = vec_window_height;
            }
            tlp->trig_height_top = 0;
        }
        else
        {
            tlp->trig_height_top -= tlp->clip_offset;
            point_x_a += tlp->x_step_ac * tlp->clip_offset;
            point_y += tlp->clip_offset * tlp->x_step_ab;
            texture_u += tlp->clip_offset * tlp->u_step_ac;
            texture_v += tlp->clip_offset * tlp->v_step_ac;
            if ( tlp->clipping_below_viewport )
            {
                tlr->render_height = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->trig_height_top = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->trig_height_top;
                    tlp->trig_height_bottom = vec_window_height - tlp->trig_height_top;
                }
            }
            point_x_b = tlp->x_start_b;
        }
    }
    else
    {
        if (tlp->clipping_below_viewport)
        {
            long delta_height, extent_height;
            TbBool extent_height_overflow;

            delta_height = vec_window_height - tlp->y_top;
            tlr->render_height = delta_height;
            if (tlp->hide_bottom_part) {
                tlp->trig_height_top = delta_height;
            } else {
                extent_height_overflow = __OFSUBL__(delta_height, tlp->trig_height_top);
                extent_height = delta_height - tlp->trig_height_top;
                tlp->hide_bottom_part = ((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0);
                tlp->trig_height_bottom = extent_height;
            }
        }
        point_x_b = tlp->x_start_b;
    }
    polygon_point = polyscans;

    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x_a;
        point_x_a += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        polygon_point->U = texture_u;
        texture_u += tlp->u_step_ac;
        polygon_point->V = texture_v;
        texture_v += tlp->v_step_ac;
        ++polygon_point;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            polygon_point->X = point_x_b;
            point_x_b += tlp->x_step_bc;
            polygon_point->Y = point_y;
            point_y += tlp->x_step_ab;
            polygon_point->U = texture_u;
            texture_u += tlp->texture_v_step_bc;
            polygon_point->V = texture_v;
            texture_v += tlp->texture_u_step_bc;
            ++polygon_point;
        }
    }
    return 1;
}

static inline int trig_rl_md05(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x_a, point_x_b, point_y;
    long texture_u, texture_v, shade_value;
    struct PolyPoint *polygon_point;
    long triangle_height_ratio;

    triangle_height_ratio = (tlp->trig_height_top << 16) / tlp->y_start;
    {
        long dXa, wXb;
        long extent_x;
        TbBool eX_overflow;

        wXb = triangle_height_ratio * (opt_b->X - opt_a->X) >> 16;
        dXa = opt_a->X - opt_c->X;
        eX_overflow = __OFSUBL__(wXb, -dXa);
        extent_x = wXb + dXa;
        if ((extent_x < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)wXb, (long)dXa);
            return 0;
        }
        tlr->shade_step = wXb;
        if (extent_x != 0) {
            long long delta_shade, weighted_shade;

            delta_shade = opt_b->U - opt_a->U;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->u_step = (opt_a->U + weighted_shade - opt_c->U) / (extent_x + 1);
            delta_shade = opt_b->V - opt_a->V;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->v_step = (opt_a->V + weighted_shade - opt_c->V) / (extent_x + 1);
            delta_shade = opt_b->S - opt_a->S;
            weighted_shade = (triangle_height_ratio * delta_shade) >> 16;
            tlr->shade_step = (opt_a->S + weighted_shade - opt_c->S) / (extent_x + 1);
        }
    }
    tlp->u_step_ac = (opt_c->U - opt_a->U) / tlp->trig_height_top;
    tlp->v_step_ac = (opt_c->V - opt_a->V) / tlp->trig_height_top;
    tlp->shade_step_ac = (opt_c->S - opt_a->S) / tlp->trig_height_top;
    tlp->texture_v_step_bc = (opt_b->U - opt_c->U) / tlp->trig_height_bottom;
    tlp->texture_u_step_bc = (opt_b->V - opt_c->V) / tlp->trig_height_bottom;
    tlp->shade_step_bc = (opt_b->S - opt_c->S) / tlp->trig_height_bottom;
    point_x_a = opt_a->X << 16;
    point_y = opt_a->X << 16;
    texture_u = opt_a->U;
    texture_v = opt_a->V;
    shade_value = opt_a->S;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        if (tlp->clip_offset - tlp->trig_height_top >= 0)
        {
            tlp->clip_offset -= tlp->trig_height_top;
            tlp->trig_height_bottom -= tlp->clip_offset;
            point_x_b = tlp->x_step_bc * tlp->clip_offset + tlp->x_start_b;
            point_y += tlp->clip_offset * tlp->x_step_ab + tlp->trig_height_top * tlp->x_step_ab;
            texture_u += tlp->clip_offset * tlp->texture_v_step_bc + tlp->trig_height_top * tlp->u_step_ac;
            texture_v += tlp->clip_offset * tlp->texture_u_step_bc + tlp->trig_height_top * tlp->v_step_ac;
            shade_value += tlp->clip_offset * tlp->shade_step_bc + tlp->trig_height_top * tlp->shade_step_ac;
            if (tlp->clipping_below_viewport) {
                tlp->trig_height_bottom = vec_window_height;
                tlr->render_height = vec_window_height;
            }
            tlp->trig_height_top = 0;
        }
        else
        {
            tlp->trig_height_top -= tlp->clip_offset;
            point_x_a += tlp->x_step_ac * tlp->clip_offset;
            point_y += tlp->clip_offset * tlp->x_step_ab;
            texture_u += tlp->clip_offset * tlp->u_step_ac;
            texture_v += tlp->clip_offset * tlp->v_step_ac;
            shade_value += tlp->clip_offset * tlp->shade_step_ac;
            if (tlp->clipping_below_viewport) {
                tlr->render_height = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->trig_height_top = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->trig_height_top;
                    tlp->trig_height_bottom = vec_window_height - tlp->trig_height_top;
                }
            }
            point_x_b = tlp->x_start_b;
        }
    }
    else
    {
        if (tlp->clipping_below_viewport)
        {
            long delta_height, extent_height;
            TbBool extent_height_overflow;

            delta_height = vec_window_height - tlp->y_top;
            tlr->render_height = delta_height;
            if (tlp->hide_bottom_part) {
                tlp->trig_height_top = delta_height;
            } else {
                extent_height_overflow = __OFSUBL__(delta_height, tlp->trig_height_top);
                extent_height = delta_height - tlp->trig_height_top;
                tlp->hide_bottom_part = ((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0);
                tlp->trig_height_bottom = extent_height;
            }
        }
        point_x_b = tlp->x_start_b;
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x_a;
        point_x_a += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        polygon_point->U = texture_u;
        texture_u += tlp->u_step_ac;
        polygon_point->V = texture_v;
        texture_v += tlp->v_step_ac;
        polygon_point->S = shade_value;
        shade_value += tlp->shade_step_ac;
        ++polygon_point;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
          polygon_point->X = point_x_b;
          point_x_b += tlp->x_step_bc;
          polygon_point->Y = point_y;
          point_y += tlp->x_step_ab;
          polygon_point->U = texture_u;
          texture_u += tlp->texture_v_step_bc;
          polygon_point->V = texture_v;
          texture_v += tlp->texture_u_step_bc;
          polygon_point->S = shade_value;
          shade_value += tlp->shade_step_bc;
          ++polygon_point;
        }
    }
    return 1;
}

int trig_rl_start(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int ret;
    long delta_x, delta_y;

    tlp->y_top = opt_a->Y;
    if (opt_a->Y < 0) {
      tlr->screen_buffer_ptr = poly_screen;
      tlp->clipping_above_viewport = 1;
    } else if (opt_a->Y < vec_window_height) {
      tlr->screen_buffer_ptr = poly_screen + vec_screen_width * opt_a->Y;
      tlp->clipping_above_viewport = 0;
    } else  {
        NOLOG("height %ld exceeded by opt_a Y %ld", (long)vec_window_height, (long)opt_a->Y);
        return 0;
    }

    tlp->hide_bottom_part = opt_c->Y > vec_window_height;
    delta_y = opt_c->Y - opt_a->Y;
    tlp->trig_height_top = delta_y;

    tlp->clipping_below_viewport = opt_b->Y > vec_window_height;
    delta_y = opt_b->Y - opt_a->Y;
    tlp->y_start = delta_y;
    tlr->render_height = delta_y;
    delta_x = opt_c->X - opt_a->X;
    tlp->x_step_ac = (delta_x << 16) / tlp->trig_height_top;
    delta_x = opt_b->X - opt_a->X;
    if ((delta_x << 16) / delta_y <= tlp->x_step_ac) {
        NOLOG("value (%ld << 16) / %ld below min %ld", (long)delta_x, (long)delta_y, (long)tlp->x_step_ac);
        return 0;
    }
    tlp->x_step_ab = (delta_x << 16) / delta_y;

    delta_y = opt_b->Y - opt_c->Y;
    delta_x = opt_b->X - opt_c->X;
    tlp->x_step_bc = (delta_x << 16) / delta_y;
    tlp->trig_height_bottom = delta_y;
    tlp->x_start_b = opt_c->X << 16;

    ret = 0;
    switch (vec_mode) /* swars-final @ 0x121814 */
    {
    case RendVec_mode00:
    case RendVec_mode14:
    case RendVec_mode15:
        ret = trig_rl_md00(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    case RendVec_mode01:
    case RendVec_mode04:
    case RendVec_mode16:
    case RendVec_mode17:
        ret = trig_rl_md01(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    case RendVec_mode02:
    case RendVec_mode03:
    case RendVec_mode07:
    case RendVec_mode08:
    case RendVec_mode09:
    case RendVec_mode10:
    case RendVec_mode11:
    case RendVec_mode12:
    case RendVec_mode13:
    case RendVec_mode18:
    case RendVec_mode19:
    case RendVec_mode22:
    case RendVec_mode23:
        ret = trig_rl_md02(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    case RendVec_mode05:
    case RendVec_mode06:
    case RendVec_mode20:
    case RendVec_mode21:
    case RendVec_mode24:
    case RendVec_mode25:
    case RendVec_mode26:
        ret = trig_rl_md05(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    }

    return ret;
}

static inline int trig_fb_md00(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y;
    struct PolyPoint *polygon_point;

    point_x = opt_a->X << 16;
    point_y = opt_a->X << 16;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        tlp->trig_height_top += tlp->y_top;
        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        point_x += tlp->x_step_ac * (-tlp->y_top);
        point_y += (-tlp->y_top) * tlp->x_step_ab;
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height - tlp->y_top;
            tlp->trig_height_top = vec_window_height - tlp->y_top;
        }
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        ++polygon_point;
    }
    return 1;
}

static inline int trig_fb_md01(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int point_x, point_y;
    int shade_value;
    struct PolyPoint *polygon_point;

    {
        long delta_x;
        delta_x = opt_b->X - opt_c->X;
        tlr->shade_step = (opt_b->S - opt_c->S) / delta_x;
        tlp->shade_step_ac = (opt_c->S - opt_a->S) / tlr->render_height;
    }
    point_x = opt_a->X << 16;
    point_y = opt_a->X << 16;
    shade_value = opt_a->S;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        tlp->trig_height_top += tlp->y_top;
        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        point_x += tlp->x_step_ac * (-tlp->y_top);
        point_y += (-tlp->y_top) * tlp->x_step_ab;
        shade_value += (-tlp->y_top) * tlp->shade_step_ac;
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height - tlp->y_top;
            tlp->trig_height_top = vec_window_height - tlp->y_top;
        }
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        polygon_point->S = shade_value;
        shade_value += tlp->shade_step_ac;
        ++polygon_point;
    }
    return 1;
}

static inline int trig_fb_md02(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y;
    long texture_u, texture_v;
    struct PolyPoint *polygon_point;

    {
        long delta_x;
        delta_x = opt_b->X - opt_c->X;
        tlr->u_step = (opt_b->U - opt_c->U) / delta_x;
        tlr->v_step = (opt_b->V - opt_c->V) / delta_x;
        tlp->u_step_ac = (opt_c->U - opt_a->U) / tlr->render_height;
        tlp->v_step_ac = (opt_c->V - opt_a->V) / tlr->render_height;
    }
    point_x = opt_a->X << 16;
    point_y = opt_a->X << 16;
    texture_u = opt_a->U;
    texture_v = opt_a->V;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        tlp->trig_height_top += tlp->y_top;
        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        point_x += tlp->x_step_ac * (-tlp->y_top);
        point_y += (-tlp->y_top) * tlp->x_step_ab;
        texture_u += (-tlp->y_top) * tlp->u_step_ac;
        texture_v += (-tlp->y_top) * tlp->v_step_ac;
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height - tlp->y_top;
            tlp->trig_height_top = vec_window_height - tlp->y_top;
        }
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        polygon_point->U = texture_u;
        texture_u += tlp->u_step_ac;
        polygon_point->V = texture_v;
        texture_v += tlp->v_step_ac;
        ++polygon_point;
    }
    return 1;
}

static inline int trig_fb_md05(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y;
    long texture_u, texture_v, shade_value;
    struct PolyPoint *polygon_point;

    {
        long delta_x;
        delta_x = opt_b->X - opt_c->X;
        tlr->u_step = (opt_b->U - opt_c->U) / delta_x;
        tlr->v_step = (opt_b->V - opt_c->V) / delta_x;
        tlr->shade_step = (opt_b->S - opt_c->S) / delta_x;
        tlp->u_step_ac = (opt_c->U - opt_a->U) / tlr->render_height;
        tlp->v_step_ac = (opt_c->V - opt_a->V) / tlr->render_height;
        tlp->shade_step_ac = (opt_c->S - opt_a->S) / tlr->render_height;
    }
    point_x = opt_a->X << 16;
    point_y = opt_a->X << 16;
    texture_u = opt_a->U;
    texture_v = opt_a->V;
    shade_value = opt_a->S;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        tlp->trig_height_top += tlp->y_top;
        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        point_x += tlp->x_step_ac * (-tlp->y_top);
        point_y += (-tlp->y_top) * tlp->x_step_ab;
        texture_u += (-tlp->y_top) * tlp->u_step_ac;
        texture_v += (-tlp->y_top) * tlp->v_step_ac;
        shade_value += (-tlp->y_top) * tlp->shade_step_ac;
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height - tlp->y_top;
            tlp->trig_height_top = vec_window_height - tlp->y_top;
        }
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        polygon_point->U = texture_u;
        texture_u += tlp->u_step_ac;
        polygon_point->V = texture_v;
        texture_v += tlp->v_step_ac;
        polygon_point->S = shade_value;
        shade_value += tlp->shade_step_ac;
        ++polygon_point;
    }
    return 1;
}

int trig_fb_start(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int ret;
    long delta_x, delta_y;

    tlp->y_top = opt_a->Y;
    if (opt_a->Y < 0) {
        tlr->screen_buffer_ptr = poly_screen;
        tlp->clipping_above_viewport = 1;
    } else if (opt_a->Y < vec_window_height) {
        tlr->screen_buffer_ptr = poly_screen + vec_screen_width * opt_a->Y;
        tlp->clipping_above_viewport = 0;
    } else {
        NOLOG("height %ld exceeded by opt_a Y %ld", (long)vec_window_height, (long)opt_a->Y);
        return 0;
    }
    tlp->hide_bottom_part = opt_c->Y > vec_window_height;
    delta_y = opt_c->Y - opt_a->Y;
    tlp->trig_height_top = delta_y;
    tlr->render_height = delta_y;
    delta_x = opt_c->X - opt_a->X;
    tlp->x_step_ac = (delta_x << 16) / delta_y;
    delta_x = opt_b->X - opt_a->X;
    tlp->x_step_ab = (delta_x << 16) / delta_y;

    ret = 0;
    switch (vec_mode) /* swars-final @ 0x122142, genewars-beta @ 0xEFE72 */
    {
    case RendVec_mode00:
    case RendVec_mode14:
    case RendVec_mode15:
        ret = trig_fb_md00(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    case RendVec_mode01:
    case RendVec_mode04:
    case RendVec_mode16:
    case RendVec_mode17:
        ret = trig_fb_md01(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    case RendVec_mode02:
    case RendVec_mode03:
    case RendVec_mode07:
    case RendVec_mode08:
    case RendVec_mode09:
    case RendVec_mode10:
    case RendVec_mode11:
    case RendVec_mode12:
    case RendVec_mode13:
    case RendVec_mode18:
    case RendVec_mode19:
    case RendVec_mode22:
    case RendVec_mode23:
        ret = trig_fb_md02(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    case RendVec_mode05:
    case RendVec_mode06:
    case RendVec_mode20:
    case RendVec_mode21:
    case RendVec_mode24:
    case RendVec_mode25:
    case RendVec_mode26:
        ret = trig_fb_md05(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    }

    return ret;
}

static inline int trig_ft_md00(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y;
    struct PolyPoint *polygon_point;

    point_x = opt_a->X << 16;
    point_y = opt_b->X << 16;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        tlp->trig_height_top += tlp->y_top;
        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        point_x += tlp->x_step_ac * (-tlp->y_top);
        point_y += (-tlp->y_top) * tlp->x_step_ab;
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height - tlp->y_top;
            tlp->trig_height_top = vec_window_height - tlp->y_top;
        }
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        ++polygon_point;
    }
    return 1;
}

static inline int trig_ft_md01(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y;
    long shade_value;
    struct PolyPoint *polygon_point;

    {
        long delta_x;
        delta_x = opt_b->X - opt_a->X;
        tlr->shade_step = (opt_b->S - opt_a->S) / delta_x;
        tlp->shade_step_ac = (opt_c->S - opt_a->S) / tlr->render_height;
    }
    point_x = opt_a->X << 16;
    point_y = opt_b->X << 16;
    shade_value = opt_a->S;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        tlp->trig_height_top += tlp->y_top;
        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        point_x += tlp->x_step_ac * (-tlp->y_top);
        point_y += (-tlp->y_top) * tlp->x_step_ab;
        shade_value += (-tlp->y_top) * tlp->shade_step_ac;
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height - tlp->y_top;
            tlp->trig_height_top = vec_window_height - tlp->y_top;
        }
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        polygon_point->S = shade_value;
        shade_value += tlp->shade_step_ac;
        ++polygon_point;
    }
    return 1;
}

static inline int trig_ft_md02(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y;
    long texture_u, texture_v;
    struct PolyPoint *polygon_point;
    {
        long delta_x;
        delta_x = opt_b->X - opt_a->X;
        tlr->u_step = (opt_b->U - opt_a->U) / delta_x;
        tlr->v_step = (opt_b->V - opt_a->V) / delta_x;
        tlp->u_step_ac = (opt_c->U - opt_a->U) / tlr->render_height;
        tlp->v_step_ac = (opt_c->V - opt_a->V) / tlr->render_height;
    }
    point_x = opt_a->X << 16;
    point_y = opt_b->X << 16;
    texture_u = opt_a->U;
    texture_v = opt_a->V;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        tlp->trig_height_top += tlp->y_top;
        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        point_x += tlp->x_step_ac * (-tlp->y_top);
        point_y += (-tlp->y_top) * tlp->x_step_ab;
        texture_u += (-tlp->y_top) * tlp->u_step_ac;
        texture_v += (-tlp->y_top) * tlp->v_step_ac;
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height - tlp->y_top;
            tlp->trig_height_top = vec_window_height - tlp->y_top;
        }
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        polygon_point->U = texture_u;
        texture_u += tlp->u_step_ac;
        polygon_point->V = texture_v;
        texture_v += tlp->v_step_ac;
        ++polygon_point;
    }
    return 1;
}

static inline int trig_ft_md05(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long point_x, point_y;
    long texture_u, texture_v, shade_value;
    struct PolyPoint *polygon_point;

    {
        long delta_x;
        delta_x = opt_b->X - opt_a->X;
        tlr->u_step = (opt_b->U - opt_a->U) / delta_x;
        tlr->v_step = (opt_b->V - opt_a->V) / delta_x;
        tlr->shade_step = (opt_b->S - opt_a->S) / delta_x;
        tlp->u_step_ac = (opt_c->U - opt_a->U) / tlr->render_height;
        tlp->v_step_ac = (opt_c->V - opt_a->V) / tlr->render_height;
        tlp->shade_step_ac = (opt_c->S - opt_a->S) / tlr->render_height;
    }
    point_x = opt_a->X << 16;
    point_y = opt_b->X << 16;
    texture_u = opt_a->U;
    texture_v = opt_a->V;
    shade_value = opt_a->S;
    if (tlp->clipping_above_viewport)
    {
        long extent_height;
        TbBool extent_height_overflow;

        tlp->trig_height_top += tlp->y_top;
        extent_height_overflow = __OFSUBL__(tlr->render_height, -tlp->y_top);
        extent_height = tlr->render_height + tlp->y_top;
        if (((extent_height < 0) ^ extent_height_overflow) | (extent_height == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->render_height, (long)tlp->y_top);
            return 0;
        }
        tlr->render_height = extent_height;
        tlp->clip_offset = -tlp->y_top;
        point_x += tlp->x_step_ac * (-tlp->y_top);
        point_y += (-tlp->y_top) * tlp->x_step_ab;
        texture_u += (-tlp->y_top) * tlp->u_step_ac;
        texture_v += (-tlp->y_top) * tlp->v_step_ac;
        shade_value += (-tlp->y_top) * tlp->shade_step_ac;
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->render_height = vec_window_height - tlp->y_top;
            tlp->trig_height_top = vec_window_height - tlp->y_top;
        }
    }
    polygon_point = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        polygon_point->X = point_x;
        point_x += tlp->x_step_ac;
        polygon_point->Y = point_y;
        point_y += tlp->x_step_ab;
        polygon_point->U = texture_u;
        texture_u += tlp->u_step_ac;
        polygon_point->V = texture_v;
        texture_v += tlp->v_step_ac;
        polygon_point->S = shade_value;
        shade_value += tlp->shade_step_ac;
        ++polygon_point;
    }
    return 1;
}

int trig_ft_start(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int ret;
    long delta_x, delta_y;

    tlp->y_top = opt_a->Y;
    if (opt_a->Y < 0) {
      tlr->screen_buffer_ptr = poly_screen;
      tlp->clipping_above_viewport = 1;
    } else if (opt_a->Y < vec_window_height) {
      tlr->screen_buffer_ptr = poly_screen + vec_screen_width * opt_a->Y;
      tlp->clipping_above_viewport = 0;
    } else {
        NOLOG("height %ld exceeded by opt_a Y %ld", (long)vec_window_height, (long)opt_a->Y);
        return 0;
    }
    tlp->hide_bottom_part = opt_c->Y > vec_window_height;
    delta_y = opt_c->Y - opt_a->Y;
    tlp->trig_height_top = delta_y;
    tlr->render_height = delta_y;
    delta_x = opt_c->X - opt_a->X;
    tlp->x_step_ac = (delta_x << 16) / delta_y;
    delta_x = opt_c->X - opt_b->X;
    tlp->x_step_ab = (delta_x << 16) / delta_y;

    ret = 0;
    switch (vec_mode) /* swars-final @ 0x1225c1, genewars-beta @ 0xF02F1 */
    {
    case RendVec_mode00:
    case RendVec_mode14:
    case RendVec_mode15:
        ret = trig_ft_md00(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    case RendVec_mode01:
    case RendVec_mode04:
    case RendVec_mode16:
    case RendVec_mode17:
        ret = trig_ft_md01(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    case RendVec_mode02:
    case RendVec_mode03:
    case RendVec_mode07:
    case RendVec_mode08:
    case RendVec_mode09:
    case RendVec_mode10:
    case RendVec_mode11:
    case RendVec_mode12:
    case RendVec_mode13:
    case RendVec_mode18:
    case RendVec_mode19:
    case RendVec_mode22:
    case RendVec_mode23:
        ret = trig_ft_md02(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    case RendVec_mode05:
    case RendVec_mode06:
    case RendVec_mode20:
    case RendVec_mode21:
    case RendVec_mode24:
    case RendVec_mode25:
    case RendVec_mode26:
        ret = trig_ft_md05(tlp, tlr, opt_a, opt_b, opt_c);
        break;
    }

    return ret;
}

/**
 * whether the subtraction (x-y) of two short ints would overflow
 */
static inline unsigned char __OFSUBS__(short x, short y)
{
    return ((x < 0) ^ (y < 0)) & ((x < 0) ^ (x-y < 0));
}

/**
 * whether the addition (x+y) of two short ints would use carry
 */
static inline unsigned char __CFADDS__(short x, short y)
{
    return (ushort)(x) > (ushort)(x+y);
}

/**
 * whether the addition (x+y) of two long ints would use carry
 */
static inline unsigned char __CFADDL__(long x, long y)
{
    return (ulong)(x) > (ulong)(x+y);
}

/**
 * rotate left unsigned long
 */
static inline ulong __ROL4__(ulong value, int count)
{
    const uint nbits = 4 * 8;

    if (count > 0) {
        count %= nbits;
        ulong high = value >> (nbits - count);
        value <<= count;
        value |= high;
    } else {
        count = -count % nbits;
        ulong low = value << (nbits - count);
        value >>= count;
        value |= low;
    }
    return value;
}

/**
 * Flat color fill - renders solid colored triangles with no shading or texturing.
 * Used for simple UI elements and solid geometry.
 */
void trig_render_md00(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *o_ln;
    unsigned char col;

    polygon_point = polyscans;
    if (polygon_point == NULL) {
        ERRORLOG("global array not set: 0x%p", polygon_point);
        return;
    }
    o_ln = tlr->screen_buffer_ptr;
    col = vec_colour;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        long point_x, point_y;
        unsigned char *o;

        point_x = polygon_point->X >> 16;
        point_y = polygon_point->Y >> 16;
        o_ln += vec_screen_width;
        if (point_x < 0)
        {
            if (point_y <= 0)
                continue;
            if (point_y > vec_window_width)
                point_y = vec_window_width;
            o = &o_ln[0];
        }
        else
        {
            TbBool pY_overflow;
            if (point_y > vec_window_width)
                point_y = vec_window_width;
            pY_overflow = __OFSUBL__(point_y, point_x);
            point_y = point_y - point_x;
            if (((point_y < 0) ^ pY_overflow) | (point_y == 0))
                continue;
            o = &o_ln[point_x];
        }
        memset(o, col, point_y);
    }
}

/**
 * Gouraud shading - renders smooth color gradients across triangle vertices.
 * No texture mapping, just interpolated vertex colors.
 */
void trig_render_md01(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    TbBool shade_value_carry;
    polygon_point = polyscans;
    if (polygon_point == NULL) {
        ERRORLOG("global array not set: 0x%p", polygon_point);
        return;
    }

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x, point_y;
        short shade_value;
        ushort colS;
        unsigned char *o;

        point_x = polygon_point->X >> 16;
        point_y = polygon_point->Y >> 16;
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;

        if (point_x  < 0)
        {
            long multiplier_x;
            short colH;

            if (point_y <= 0)
                continue;
            multiplier_x = tlr->shade_step * (ushort)(-point_x);
            shade_value_carry = __CFADDS__(polygon_point->S, multiplier_x);
            shade_value = polygon_point->S + multiplier_x;
            // Delcate code - if we add before shifting, the result is different
            colH = (multiplier_x >> 16) + (polygon_point->S >> 16) + shade_value_carry;
            if (point_y > vec_window_width)
                point_y = vec_window_width;

            colS = ((colH & 0xFF) << 8) + vec_colour;
        }
        else
        {
            TbBool pY_overflow;
            short colH;

            if (point_y > vec_window_width)
              point_y = vec_window_width;
            pY_overflow = __OFSUBS__(point_y, point_x);
            point_y = point_y - point_x;
            if (((point_y < 0) ^ pY_overflow) | (point_y == 0))
                continue;
            o += point_x;
            colH = polygon_point->S >> 16;
            shade_value = polygon_point->S;

            colS = ((colH & 0xFF) << 8) + vec_colour;
        }

        for (;point_y > 0; point_y--, o++)
        {
            short colH, colL;
            *o = colS >> 8;

            colL = colS;
            shade_value_carry = __CFADDS__(tlr->shade_step, shade_value);
            shade_value = tlr->shade_step + shade_value;
            colH = (tlr->shade_step >> 16) + shade_value_carry + (colS >> 8);

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

/**
 * 256x256 texture mapping - renders sprite textures without shading.
 * Uses full 8-bit texture coordinates (0-255) for both U and V axes.
 */
void trig_render_md02(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    long texture_v_step_fixed;

    m = vec_map;
    polygon_point = polyscans;
    if ((m == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p", m, polygon_point);
        return;
    }
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x, point_y;
        long texture_u;
        ushort colS;
        unsigned char *o;

        point_x = polygon_point->X >> 16;
        point_y = polygon_point->Y >> 16;
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;

        if (point_x < 0)
        {
            ushort colL, colH;
            ulong factorA;
            long multiplier_x;

            if (point_y <= 0)
                continue;
            multiplier_x = tlr->v_step * (-point_x);
            factorA = __ROL4__(polygon_point->V + multiplier_x, 16);
            colH = factorA;
            multiplier_x = tlr->u_step * (-point_x);
            texture_u = (factorA & 0xFFFF0000) | ((polygon_point->U + multiplier_x) & 0xFFFF);
            colL = (polygon_point->U + multiplier_x) >> 16;
            if (point_y > vec_window_width)
                point_y = vec_window_width;
            point_x = (polygon_point->U + multiplier_x) >> 8;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            short colL, colH;
            TbBool pY_overflow;

            if (point_y > vec_window_width)
                point_y = vec_window_width;
            pY_overflow = __OFSUBS__(point_y, point_x);
            point_y = point_y - point_x;
            if (((point_y < 0) ^ pY_overflow) | (point_y == 0))
                continue;
            o += point_x;
            texture_u = __ROL4__(polygon_point->V, 16);
            colH = texture_u;
            texture_u = (texture_u & 0xFFFF0000) | (polygon_point->U & 0xFFFF);
            colL = polygon_point->U >> 16;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; point_y > 0; point_y--, o++)
        {
            short colL, colH;
            TbBool texture_u_carry;

            *o = m[colS];

            texture_u_carry = __CFADDS__(tlr->u_step, texture_u);
            texture_u = (texture_u & 0xFFFF0000) | ((tlr->u_step + texture_u) & 0xFFFF);
            colL = (tlr->u_step >> 16) + texture_u_carry + colS;

            texture_u_carry = __CFADDL__(texture_v_step_fixed, texture_u);
            texture_u = texture_v_step_fixed + texture_u;
            colH = (tlr->v_step >> 16) + texture_u_carry + (colS >> 8);

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md03(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    long texture_v_step_fixed;

    m = vec_map;
    polygon_point = polyscans;
    if ((m == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p", m, polygon_point);
        return;
    }
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x, point_y;
        long texture_u;
        ulong factorA;
        ushort colS;
        unsigned char *o;

        point_x = polygon_point->X >> 16;
        point_y = polygon_point->Y >> 16;
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;

        if (point_x < 0)
        {
            short colL, colH;
            long multiplier_x;

            if (point_y <= 0)
                continue;
            multiplier_x = tlr->v_step * (-point_x);
            factorA = __ROL4__(polygon_point->V + multiplier_x, 16);
            colH = factorA;
            multiplier_x = tlr->u_step * (-point_x);
            texture_u = (factorA & 0xFFFF0000) | ((polygon_point->U + multiplier_x) & 0xFFFF);
            colL = (polygon_point->U + multiplier_x) >> 16;
            if (point_y > vec_window_width)
                point_y = vec_window_width;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            short colL, colH;
            TbBool pY_overflow;

            if (point_y > vec_window_width)
                point_y = vec_window_width;
            pY_overflow = __OFSUBS__(point_y, point_x);
            point_y = point_y - point_x;
            if (((point_y < 0) ^ pY_overflow) | (point_y == 0))
                continue;
            o += point_x;
            texture_u = __ROL4__(polygon_point->V, 16);
            colH = texture_u;
            texture_u = (texture_u & 0xFFFF0000) | ((polygon_point->U) & 0xFFFF);
            colL = (polygon_point->U) >> 16;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; point_y > 0; point_y--, o++)
        {
            short colL, colH;
            TbBool texture_u_carry;

            if (m[colS] != 0)
                *o = m[colS];

            texture_u_carry = __CFADDS__(tlr->u_step, texture_u);
            texture_u = (texture_u & 0xFFFF0000) | ((tlr->u_step + texture_u) & 0xFFFF);
            colL = (tlr->u_step >> 16) + texture_u_carry + colS;
            texture_u_carry = __CFADDL__(texture_v_step_fixed, texture_u);
            texture_u = texture_v_step_fixed + texture_u;
            colH = (tlr->v_step >> 16) + texture_u_carry + (colS >> 8);

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

/**
 * Verified in swars - uses:
 * - zealot car antennas
 * - tank lower chassis
 * - Large red and white rocket building - red stipes
 */
void trig_render_md04(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *f;

    f = pixmap.fade_tables;
    polygon_point = polyscans;
    if ((f == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p", f, polygon_point);
        return;
    }

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x, point_y;
        short texture_u;
        ushort colS;
        unsigned char *o;

        point_x = polygon_point->X >> 16;
        point_y = polygon_point->Y >> 16;
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x < 0)
        {
            ushort colL, colH;
            TbBool texture_u_carry;
            long multiplier_x;

            if (point_y <= 0)
                continue;
            multiplier_x = tlr->shade_step * (-point_x);
            texture_u_carry = __CFADDS__(polygon_point->S, multiplier_x);
            texture_u = polygon_point->S + multiplier_x;
            colH = (polygon_point->S >> 16) + texture_u_carry + (multiplier_x >> 16);
            if (point_y > vec_window_width)
                point_y = vec_window_width;
            colL = vec_colour;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            TbBool pY_overflow;

            if (point_y > vec_window_width)
                point_y = vec_window_width;
            pY_overflow = __OFSUBS__(point_y, point_x);
            point_y = point_y - point_x;
            if (((point_y < 0) ^ pY_overflow) | (point_y == 0))
                continue;
            o += point_x;
            colL = vec_colour;
            texture_u = polygon_point->S;
            colH = polygon_point->S >> 16;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (;point_y > 0; point_y--, o++)
        {
            ushort colL, colH;
            TbBool texture_u_carry;

            texture_u_carry = __CFADDS__(tlr->shade_step, texture_u);
            texture_u = tlr->shade_step + texture_u;
            colL = colS;
            colH = (tlr->shade_step >> 16) + texture_u_carry + (colS >> 8);
            *o = f[colS];

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

/**
 * 32x256 terrain texture with shading - renders terrain using 5-bit V (32 rows)
 * and 8-bit U (256 columns) texture coordinates, with fade table shading.
 */
void trig_render_md05(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *f;
    long texture_v_step_fixed;
    long shade_step_fixed;
    long texture_v_lower_byte;

    m = vec_map;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    if ((m == NULL) || (f == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p 0x%p", m, f, polygon_point);
        return;
    }

    {
        ulong factorA, factorB, factorC;
        factorC = tlr->u_step;
        // original code used unsigned compare here, making the condition always false
        //if (tlr->shade_step < 0) factorC--;
        factorC = __ROL4__(factorC, 16);
        factorA = __ROL4__(tlr->v_step, 16);
        factorB = ((ulong)tlr->shade_step) >> 8;
        texture_v_step_fixed = (factorC & 0xFFFF0000) | (factorB & 0xFFFF);
        shade_step_fixed = (factorA & 0xFFFFFF00) | (factorC & 0xFF);
        texture_v_lower_byte = (factorA & 0xFF);
    }

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        long point_x, point_y;
        long rfactA, rfactB;
        ushort colM;
        unsigned char *o;
        unsigned char *o_ln;

        point_x = polygon_point->X >> 16;
        point_y = polygon_point->Y >> 16;
        o_ln = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;

        if (point_x < 0)
        {
            ulong factorA, factorB;
            ushort colL, colH;
            long multiplier_x;

            if (point_y <= 0)
                continue;
            multiplier_x = tlr->u_step * (-point_x);
            factorA = __ROL4__(polygon_point->U + multiplier_x, 16);
            multiplier_x = tlr->v_step * (-point_x);
            factorB = __ROL4__(polygon_point->V + multiplier_x, 16);
            multiplier_x = tlr->shade_step * (-point_x);
            colL = (polygon_point->S + multiplier_x) >> 8;
            colH = factorB;
            rfactB = (factorB & 0xFFFF0000) | (factorA & 0xFF);
            rfactA = (factorA & 0xFFFF0000) | (colL & 0xFFFF);
            if (point_y > vec_window_width)
                point_y = vec_window_width;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ulong factorA, factorB;
            ushort colL, colH;
            TbBool pY_overflow;

            if (point_y > vec_window_width)
                point_y = vec_window_width;
            pY_overflow = __OFSUBS__(point_y, point_x);
            point_y = point_y - point_x;
            if (((point_y < 0) ^ pY_overflow) | (point_y == 0))
                continue;
            o_ln += point_x;
            factorA = __ROL4__(polygon_point->U, 16);
            factorB = __ROL4__(polygon_point->V, 16);
            colL = polygon_point->S >> 8;
            colH = factorB;
            // Should the high part really be preserved?
            rfactB = (factorB & 0xFFFF0000) | (factorA & 0xFF);
            rfactA = (factorA & 0xFFFF0000) | (colL & 0xFFFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        o = o_ln;

        for (; point_y > 0; point_y--, o++)
        {
            ushort colL, colH;
            ushort colS;
            TbBool rfactA_carry;
            TbBool rfactB_carry;

            colM = (colM & 0xFF00) + (rfactB & 0xFF);
            colS = (((rfactA >> 8) & 0xFF) << 8) + m[colM];

            rfactA_carry = __CFADDL__(rfactA, texture_v_step_fixed);
            rfactA = rfactA + texture_v_step_fixed;

            rfactB_carry = __CFADDL__(rfactB + rfactA_carry, shade_step_fixed);
            rfactB = rfactB + shade_step_fixed + rfactA_carry;

            colH = texture_v_lower_byte + rfactB_carry + (colM >> 8);
            colL = colM;
            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);

            *o = f[colS];
        }
    }
}

/**
 * Verified in swars - uses:
 * - wire fences
 * - cross bars
 */
void trig_render_md06(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *f;
    long texture_v_step_fixed;
    long shade_step_fixed;

    m = vec_map;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    if ((m == NULL) || (f == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p 0x%p", m, f, polygon_point);
        return;
    }
    texture_v_step_fixed = tlr->v_step << 16;
    shade_step_fixed = tlr->shade_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        unsigned char *o;
        short point_x_a, point_y_a;
        long factorA;
        long point_y;
        ulong factorB;
        ushort colM;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;

        if (point_x_a < 0)
        {
            ushort colL, colH;
            ushort pXMa;
            long pXMb;
            ulong multiplier_x;

            if (point_y_a <= 0)
                continue;
            pXMa = (ushort)-point_x_a;
            pXMb = pXMa;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXMa, 16);
            colH = factorA;
            multiplier_x = polygon_point->U + tlr->u_step * pXMa;
            factorA = (factorA & 0xFFFF0000) | (multiplier_x & 0xFFFF);
            point_x_a = multiplier_x >> 8;
            colL = (point_x_a >> 8);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);

            factorB = __ROL4__(polygon_point->S + tlr->shade_step * pXMb, 16);
            point_x_a = (point_x_a & 0xFFFF00FF) | ((factorB & 0xFF) << 8);
            factorB = (factorB & 0xFFFF0000) | (point_y_a & 0xFFFF);
            point_x_a = (point_x_a & 0xFFFF);
            point_y = factorB & 0xFFFF;
            if (point_y > vec_window_width)
                point_y = vec_window_width;
        }
        else
        {
            ushort colL, colH;
            unsigned char pLa_overflow;
            short pLa;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pLa_overflow = __OFSUBS__(point_y_a, point_x_a);
            pLa = point_y_a - point_x_a;
            if (((pLa < 0) ^ pLa_overflow) | (pLa == 0))
                continue;

            o += point_x_a;
            colL = (polygon_point->U >> 16);
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorB = __ROL4__(polygon_point->S, 16);
            factorA = (factorA & 0xFFFF0000) | (polygon_point->U & 0xFFFF);
            point_x_a = (point_x_a & 0xFFFF00FF) | ((factorB & 0xFF) << 8);
            factorB = (factorB & 0xFFFF0000) | (pLa & 0xFFFF);
            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
            point_y = factorB & 0xFFFF;
        }

        for (; (point_y & 0xFFFF) > 0; point_y--, o++)
        {
            ushort colL, colH;
            unsigned char fct_carry;

            point_x_a = (point_x_a & 0xFF00) | (m[colM] & 0xFF);
            if (point_x_a & 0xFF)
                *o = f[point_x_a];

            fct_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) | ((tlr->u_step + factorA) & 0xFFFF);
            colL = (tlr->u_step >> 16) + fct_carry + colM;
            fct_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            colH = (tlr->v_step >> 16) + fct_carry + (colM >> 8);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);

            factorB = (factorB & 0xFFFF0000) | (point_y & 0xFFFF);
            fct_carry = __CFADDL__(shade_step_fixed, factorB);
            factorB += shade_step_fixed;
            point_x_a = (((point_x_a >> 8) + (tlr->shade_step >> 16) + fct_carry) << 8) | (point_x_a & 0xFF);
            point_y += shade_step_fixed; // Very alarming. Bug, maybe?
        }
    }
}

void trig_render_md07(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *f;
    long texture_v_step_fixed;

    m = vec_map;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    if ((m == NULL) || (f == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p 0x%p", m, f, polygon_point);
        return;
    }
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a;
        long point_y_a;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if ( (point_x_a & 0x8000u) != 0 )
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if ( (short)point_y_a <= 0 )
                continue;
            pXm = (ushort)-(short)point_x_a;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;
            factorC = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) | (factorC & 0xFFFF);
            factorB = factorC >> 8;
            colL = ((factorB >> 8) & 0xFF);
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            point_x_a = (ushort)factorB;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( (unsigned char)(((point_y_a & 0x8000u) != 0) ^ pY_overflow) | ((ushort)point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) | (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = (vec_colour << 8) + m[colM];
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) | ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            *o = f[colS];
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md08(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *f;
    long texture_v_step_fixed;

    m = vec_map;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    if ((m == NULL) || (f == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p 0x%p", m, f, polygon_point);
        return;
    }
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a;
        long point_y_a;
        ushort colM;
        unsigned char *o;
        long factorA;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if ( (point_x_a & 0x8000u) != 0 )
        {
            ushort colL, colH;
            ulong factorB, factorC;
            long pXm;

            if ( (short)point_y_a <= 0 )
                continue;
            pXm = (ushort)-(short)point_x_a;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            point_x_a = (ushort)factorC;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( (unsigned char)(((point_y_a & 0x8000u) != 0) ^ pY_overflow) | ((ushort)point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = (vec_colour << 8) + m[colM];
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            if (colS & 0xFF)
                *o = f[colS];
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md09(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *f;
    long texture_v_step_fixed;

    m = vec_map;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    if ((m == NULL) || (f == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p 0x%p", m, f, polygon_point);
        return;
    }
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if (point_y_a <= 0)
                continue;
            pXm = (ushort)-point_x_a;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            point_x_a = (ushort)factorC;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if (((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0))
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = m[colM] << 8;
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            if ((colS >> 8) & 0xFF) {
                colS = (colS & 0xFF00) | (*o);
                *o = f[colS];
            }
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

/**
 * Translucent sprite rendering - used for creature shadows.
 * Samples from 256x256 sprite texture (big_scratch buffer) and blends with
 * screen using fade table for shadow transparency effect.
 * 
 * Texture coordinate packing:
 *   colM = (V_row << 8) | U_column
 *   - High byte (colH & 0xFF): V coordinate (0-255 rows) - uses 8 bits for sprites
 *   - Low byte (colL & 0xFF): U coordinate (0-255 columns)
 *   - Total: 16-bit index into 256x256 = 65536 byte texture
 */
void trig_render_md10(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;  // texture map (256x256 sprite in big_scratch)
    unsigned char *f;  // fade/transparency table
    long texture_v_step_fixed;

    m = vec_map;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    if ((m == NULL) || (f == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p 0x%p", m, f, polygon_point);
        return;
    }
    if (tlr->screen_buffer_ptr == NULL) {
        ERRORLOG("screen buffer not set");
        return;
    }
    // Convert v_step to 16.16 fixed-point for sub-pixel precision
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a;
        short point_y_a;
        ulong factorB;
        long factorA;
        ulong factorC;
        ushort colM;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            long pXm;

            if (point_y_a <= 0)
                continue;
            // Clipping: point starts off-screen left, advance UV to screen edge
            pXm = (ushort)-(short)point_x_a;
            // __ROL4__ rotates 32-bit value left by 16, swapping high/low 16-bit halves
            // This extracts the integer part of the fixed-point V coordinate
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;  // V texture row (integer part)
            factorB = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);  // U texture column (integer part)
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            point_x_a = (ushort)factorC;

            // Pack V row and U column into 16-bit texture index
            // 0xFF mask allows full 256 rows (sprite textures), not 0x1F (32 rows for terrain)
            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( (unsigned char)(((point_y_a & 0x8000u) != 0) ^ pY_overflow) | ((ushort)point_y_a == 0) )
                continue;
            o += point_x_a;
            // Extract integer part of V coordinate via rotate-left-16
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;  // V row
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);  // U column

            // Pack into texture index: (V_row << 8) | U_column
            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        // Per-pixel scanline loop
        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            // Sample texture; if non-zero (inside shadow), apply fade
            if (m[colM]) {
                // Fade table lookup: high byte = shadow intensity, low byte = screen pixel
                // Result is blended shadow color
                colS = (vec_colour << 8) | (*o);
                *o = f[colS];
            }

            // Step U coordinate with carry propagation for fixed-point precision
            // __CFADDS__ returns 1 if 16-bit addition overflows (carry flag)
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            // Add integer part of u_step plus any carry overflow to U column
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;

            // Step V coordinate with carry propagation
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            // Add integer part of v_step plus any carry overflow to V row
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;

            // Repack texture index for next pixel
            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md12(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    long texture_v_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    polygon_point = polyscans;
    if ((m == NULL) || (g == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p 0x%p", m, g, polygon_point);
        return;
    }
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        long point_x_a;
        short point_y_a;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if ( (point_x_a & 0x8000u) != 0 )
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if ( (short)point_y_a <= 0 )
                continue;
            pXm = (ushort)-(short)point_x_a;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;
            factorC = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorC & 0xFFFF);
            factorB = factorC >> 8;
            colL = ((factorB >> 8) & 0xFF);
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            point_x_a = (ushort)factorB;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( (unsigned char)(((point_y_a & 0x8000u) != 0) ^ pY_overflow) | ((ushort)point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = (m[colM] << 8) | vec_colour;
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            *o = g[colS];
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md13(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    long texture_v_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    polygon_point = polyscans;
    if ((m == NULL) || (g == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p 0x%p", m, g, polygon_point);
        return;
    }
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if ( (short)point_y_a <= 0 )
                continue;
            pXm = (ushort)-(short)point_x_a;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            point_x_a = (ushort)factorC;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( (unsigned char)(((point_y_a & 0x8000u) != 0) ^ pY_overflow) | ((ushort)point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = m[colM] | (vec_colour << 8);
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            *o = g[colS];
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md14(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *g;
    ushort colM;
    unsigned char *o_ln;

    g = pixmap.ghost;
    polygon_point = polyscans;
    if ((g == NULL) || (polygon_point == NULL)) {
        ERRORLOG("global arrays not set: 0x%p 0x%p", g, polygon_point);
        return;
    }
    o_ln = tlr->screen_buffer_ptr;
    colM = (vec_colour << 8);

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o_ln += vec_screen_width;

        if (point_x_a < 0)
        {
            if (point_y_a <= 0)
                continue;
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            o = o_ln;
        }
        else
        {
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( ((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o = &o_ln[point_x_a];
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
              colM = (colM & 0xFF00) | *o;
              *o = g[colM];
        }
    }
}

void trig_render_md15(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *g;
    ushort colM;
    unsigned char *o_ln;

    g = pixmap.ghost;
    polygon_point = polyscans;
    o_ln = tlr->screen_buffer_ptr;
    colM = vec_colour;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o_ln += vec_screen_width;
        if (point_x_a < 0)
        {
            if (point_y_a <= 0)
                continue;
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            o = o_ln;
        }
        else
        {
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( ((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o = &o_ln[point_x_a];
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
              colM = (*o << 8) | (colM & 0xFF);
              *o = g[colM];
        }
    }
}

void trig_render_md16(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *g;
    unsigned char *f;

    g = pixmap.ghost;
    f = pixmap.fade_tables;
    polygon_point = polyscans;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        short factorA;
        ushort colM;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;

        if (point_x_a < 0)
        {
            ushort colL, colH;
            unsigned char factorA_carry;
            ulong pXMa;
            short pXMb;

            if (point_y_a <= 0)
                continue;
            pXMa = tlr->shade_step * (ushort)-point_x_a;
            pXMb = pXMa;
            point_x_a = pXMa >> 8;
            factorA_carry = __CFADDS__(polygon_point->S, pXMb);
            factorA = (polygon_point->S) + pXMb;
            colH = (point_x_a >> 8) + (polygon_point->S >> 16) + factorA_carry;
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            colL = vec_colour;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( ((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o += point_x_a;
            colL = vec_colour;
            factorA = polygon_point->S;
            colH = (polygon_point->S >> 16);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = (f[colM] << 8) | *o;
            *o = g[colS];
            factorA_carry = __CFADDS__(tlr->shade_step, factorA);
            factorA += (tlr->shade_step & 0xFFFF);
            colH = (colM >> 8) + (tlr->shade_step >> 16) + factorA_carry;
            colL = colM;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md17(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *g;
    unsigned char *f;

    g = pixmap.ghost;
    f = pixmap.fade_tables;
    polygon_point = polyscans;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        unsigned char factorA_carry;
        short factorA;
        ushort colS;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;

        if (point_x_a < 0)
        {
            ushort colL, colH;
            ulong pXMa;
            short pXMb;

            if (point_y_a <= 0)
                continue;
            pXMa = tlr->shade_step * (ushort)-point_x_a;
            pXMb = pXMa;
            point_x_a = pXMa >> 8;
            factorA_carry = __CFADDS__(polygon_point->S, pXMb);
            factorA = polygon_point->S + pXMb;
            colH = (point_x_a >> 8) + (polygon_point->S >> 16) + factorA_carry;
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            colL = vec_colour;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if (((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0))
                continue;

            o += point_x_a;
            colL = vec_colour;
            factorA = polygon_point->S;
            colH = (polygon_point->S >> 16);

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colM;

            colM = ((*o) << 8) + f[colS];
            *o = g[colM];

            factorA_carry = __CFADDS__(tlr->shade_step, factorA);
            factorA += (tlr->shade_step & 0xFFFF);
            colH = (colS >> 8) + ((tlr->shade_step >> 16) & 0xFF) + factorA_carry;
            colL = colS;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md18(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    long texture_v_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    polygon_point = polyscans;
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if (point_y_a <= 0)
                continue;
            pXm = (ushort)-point_x_a;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = (factorC >> 8);
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            point_x_a = (ushort)factorC;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_carry;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_carry = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( ((point_y_a < 0) ^ pY_carry) | (point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = (polygon_point->U >> 16);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colH = m[colM];
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) | ((tlr->u_step + factorA) & 0xFFFF);
            colL = *o;
            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            *o = g[colS];
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md19(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    long texture_v_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    polygon_point = polyscans;
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        long factorA;
        ushort colM;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            long pXm;
            ulong factorB, factorC;

            if (point_y_a <= 0)
                continue;
            pXm = (ushort)-point_x_a;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( ((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colS = ((*o) << 8) + m[colM];
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            *o = g[colS];
            colH = (colM >> 8) + (tlr->v_step >> 16) + factorA_carry;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md20(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long texture_v_step_fixed;
    long shade_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    texture_v_step_fixed = tlr->v_step << 16;
    shade_step_fixed = tlr->shade_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        long pXMa;
        long pXMb;
        long factorA;
        long factorC;
        ushort colM;
        unsigned char *o;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            ulong factorB;

            if (point_y_a <= 0)
                continue;
            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pXMa = (ushort)-point_x_a;
            pXMb = pXMa;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXMa, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXMa;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            point_x_a = factorB >> 8;
            colL = ((point_x_a >> 8) & 0xFF);
            factorC = __ROL4__(polygon_point->S + tlr->shade_step * pXMb, 16);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( ((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);
            factorC = __ROL4__(polygon_point->S, 16);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colS = ((factorC & 0xFF) << 8) + m[colM];
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            colS = ((f[colS] & 0xFF) << 8) + *o;
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;
            factorA_carry = __CFADDL__(shade_step_fixed, factorC);
            factorC += shade_step_fixed;
            *o = g[colS];
            factorC = (factorC & 0xFFFFFF00) | (((tlr->shade_step >> 16) + factorA_carry + factorC) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md21(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long texture_v_step_fixed;
    long shade_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    texture_v_step_fixed = tlr->v_step << 16;
    shade_step_fixed = tlr->shade_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a, point_y_a;
        ushort colM;
        unsigned char *o;
        long factorA, factorC;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            long pXMa;
            long pXMb;
            ulong factorB;

            if (point_y_a <= 0)
                continue;
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            pXMa = (ushort)-point_x_a;
            pXMb = pXMa;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXMa, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXMa;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            point_x_a = factorB >> 8;
            colL = ((point_x_a >> 8) & 0xFF);
            factorC = __ROL4__(polygon_point->S + tlr->shade_step * pXMb, 16);
            point_x_a = (point_x_a & 0xFFFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( ((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);
            factorC = __ROL4__(polygon_point->S, 16);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            colS = ((factorC & 0xFF) << 8) + (m[colM] & 0xFF);
            colS = (((*o) & 0xFF) << 8) + (f[colS] & 0xFF);
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;
            factorA_carry = __CFADDL__(shade_step_fixed, factorC);
            factorC += shade_step_fixed;
            *o = g[colS];
            factorC = (factorC & 0xFFFFFF00) | (((tlr->shade_step >> 16) + factorA_carry + factorC) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md22(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    long texture_v_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    polygon_point = polyscans;
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a;
        ushort colM;
        short point_y_a;
        unsigned char *o;
        long pXm;
        long factorA;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if (point_y_a <= 0)
                continue;
            pXm = (ushort)-point_x_a;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            point_x_a = factorC & 0xFFFF;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if ( ((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            if (m[colM]) {
                colS = ((m[colM] & 0xFF) << 8) + *o;
                *o = g[colS];
            }
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md23(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    long texture_v_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    polygon_point = polyscans;
    texture_v_step_fixed = tlr->v_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a;
        ushort colM;
        short point_y_a;
        unsigned char *o;
        long pXm;
        long factorA;
        unsigned char factorA_carry;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if ( (point_x_a & 0x8000u) != 0 )
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if (point_y_a <= 0)
                continue;
            pXm = (ushort)-point_x_a;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXm, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (point_y_a > vec_window_width)
              point_y_a = vec_window_width;
            point_x_a = (ushort)factorC;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if (((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            ushort colS;

            if (m[colM]) {
                colS = (((*o) & 0xFF) << 8) + m[colM];
                *o = g[colS];
            }
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md24(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long texture_v_step_fixed;
    long shade_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    texture_v_step_fixed = tlr->v_step << 16;
    shade_step_fixed = tlr->shade_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a;
        ushort colM;
        short point_y_a;
        unsigned char *o;
        long pXMa;
        long pXMb;
        long factorA;
        long factorC;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            ulong factorB;

            if (point_y_a <= 0)
                continue;
            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pXMa = (ushort)-point_x_a;
            pXMb = pXMa;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXMa, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXMa;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            point_x_a = factorB >> 8;
            colL = ((point_x_a >> 8) & 0xFF);
            factorC = __ROL4__(polygon_point->S + tlr->shade_step * pXMb, 16);
            point_x_a = (ushort)point_x_a;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if (((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);
            factorC = __ROL4__(polygon_point->S, 16);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            unsigned char factorA_carry;

            if (m[colM]) {
                ushort colS;

                colS = ((factorC & 0xFF) << 8) + m[colM];
                colS = (f[colS] << 8) + *o;
                *o = g[colS];
            }
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;
            factorA_carry = __CFADDL__(shade_step_fixed, factorC);
            factorC += shade_step_fixed;
            factorC = (factorC & 0xFFFFFF00) + (((tlr->shade_step >> 16) + factorA_carry + factorC) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md25(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long texture_v_step_fixed;
    long shade_step_fixed;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    polygon_point = polyscans;
    texture_v_step_fixed = tlr->v_step << 16;
    shade_step_fixed = tlr->shade_step << 16;

    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        short point_x_a;
        ushort colM;
        short point_y_a;
        unsigned char *o;
        long pXMa;
        long pXMb;
        long factorA;
        long factorC;

        point_x_a = (polygon_point->X >> 16);
        point_y_a = (polygon_point->Y >> 16);
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;
        if (point_x_a < 0)
        {
            ushort colL, colH;
            ulong factorB;

            if (point_y_a <= 0)
                continue;
            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pXMa = (ushort)-point_x_a;
            pXMb = pXMa;
            factorA = __ROL4__(polygon_point->V + tlr->v_step * pXMa, 16);
            colH = factorA;
            factorB = polygon_point->U + tlr->u_step * pXMa;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            point_x_a = factorB >> 8;
            colL = ((point_x_a >> 8) & 0xFF);
            factorC = __ROL4__(polygon_point->S + tlr->shade_step * pXMb, 16);
            point_x_a = (ushort)point_x_a;

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a = point_y_a - point_x_a;
            if (((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0) )
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (polygon_point->U & 0xFFFF);
            colL = ((polygon_point->U >> 16) & 0xFF);
            factorC = __ROL4__(polygon_point->S, 16);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colL, colH;
            unsigned char factorA_carry;

            if (m[colM]) {
                ushort colS;

                colS = ((factorC & 0xFF) << 8) + m[colM];
                colS = (((*o) & 0xFF) << 8) + f[colS];
                *o = g[colS];
            }
            factorA_carry = __CFADDS__(tlr->u_step, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->u_step + factorA) & 0xFFFF);
            colL = ((tlr->u_step >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA += texture_v_step_fixed;
            colH = (colM >> 8) + ((tlr->v_step >> 16) & 0xFF) + factorA_carry;
            factorA_carry = __CFADDL__(shade_step_fixed, factorC);
            factorC += shade_step_fixed;
            factorC = (factorC & 0xFFFFFF00) | (((tlr->shade_step >> 16) + factorA_carry + factorC) & 0xFF);

            colM = ((colH & 0x1F) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md26(struct TrigLocalRend *tlr)
{
    struct PolyPoint *polygon_point;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long texture_v_step_fixed;
    long shade_step_fixed;
    long texture_v_lower_byte;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    polygon_point = polyscans;

    {
        ulong texture_u_rotated;
        ulong texture_v_rotated;
        unsigned char local_texture_v_lower_byte;

        texture_u_rotated = __ROL4__(tlr->u_step, 16);
        texture_v_rotated = __ROL4__(tlr->v_step, 16);
        local_texture_v_lower_byte = texture_v_rotated;
        texture_v_rotated = (texture_v_rotated & 0xFFFFFF00) + (texture_u_rotated & 0xFF);
        texture_u_rotated = (texture_u_rotated & 0xFFFF0000) + (((ulong)tlr->shade_step >> 8) & 0xFFFF);
        texture_v_rotated = (texture_v_rotated & 0xFFFF0000) + (texture_v_rotated & 0xFF);
        texture_v_step_fixed = texture_u_rotated;
        shade_step_fixed = texture_v_rotated;
        texture_v_lower_byte = local_texture_v_lower_byte;
    }
    for (; tlr->render_height; tlr->render_height--, polygon_point++)
    {
        long point_x_a;
        long point_y_a;
        unsigned char *o;
        ulong factorB, factorD;
        long factorA;
        ulong factorC;
        unsigned char pY_overflow;
        ushort colM;

        point_x_a = polygon_point->X >> 16;
        point_y_a = polygon_point->Y >> 16;
        o = &tlr->screen_buffer_ptr[vec_screen_width];
        tlr->screen_buffer_ptr += vec_screen_width;

        if (point_x_a < 0)
        {
            if (point_y_a <= 0)
                continue;
            point_x_a = -point_x_a;
            factorA = __ROL4__(polygon_point->U + point_x_a * tlr->u_step, 16);
            factorB = __ROL4__(polygon_point->V + point_x_a * tlr->v_step, 16);
            factorC = (ulong)(polygon_point->S + point_x_a * tlr->shade_step) >> 8;
            factorB = (factorB & 0xFFFFFF00) | (factorA & 0xFF);
            factorA = (factorA & 0xFFFF0000) | (factorC & 0xFFFF);
            factorD = __ROL4__(polygon_point->V + point_x_a * tlr->v_step, 16);
            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;

            colM = (factorC & 0xFF) + ((factorD & 0xFF) << 8);
        }
        else
        {
            if (point_y_a > vec_window_width)
                point_y_a = vec_window_width;
            pY_overflow = __OFSUBS__(point_y_a, point_x_a);
            point_y_a -= point_x_a;
            if (((point_y_a < 0) ^ pY_overflow) | (point_y_a == 0))
                continue;
            o += point_x_a;
            factorA = __ROL4__(polygon_point->U, 16);
            factorB = __ROL4__(polygon_point->V, 16);
            factorC = (ulong)polygon_point->S >> 8;
            factorB = (factorB & 0xFFFFFF00) | (factorA & 0xFF);
            factorA = (factorA & 0xFFFF0000) | (factorC & 0xFFFF);
            factorD = __ROL4__(polygon_point->V, 16);

            colM = (factorC & 0xFF) + ((factorD & 0xFF) << 8);
        }

        factorB = (factorB & 0xFFFF00FF);

        for (; point_y_a > 0; point_y_a--, o++)
        {
            ushort colS;
            unsigned char factorA_carry, factorB_carry;

            colM = (colM & 0xFF00) | (factorB & 0xFF);
            colS = (factorA & 0xFF00) | m[colM];
            factorA_carry = __CFADDL__(texture_v_step_fixed, factorA);
            factorA = texture_v_step_fixed + factorA;
            factorB_carry = __CFADDL__(shade_step_fixed, factorB + factorA_carry);
            factorB = shade_step_fixed + factorB + factorA_carry;
            colM = (colM & 0xFF) + ((((colM >> 8) + texture_v_lower_byte + factorB_carry) & 0xFF) << 8);

            if ((colS & 0xFF) <= 0xCu) {
                colS = ((*o) << 8) | f[colS];
                *o = g[colS];
            } else {
                *o = f[colS];
            }
        }
    }
}

/** Triangle rendering function.
 *
 * @param point_a
 * @param point_b
 * @param point_c
 */
void trig(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c)
{
    struct PolyPoint *opt_a;
    struct PolyPoint *opt_b;
    struct PolyPoint *opt_c;
    unsigned char start_type;
    struct TrigLocalPrep tlp;
    struct TrigLocalRend tlr;

    NOLOG("Pa(%ld,%ld,%ld)", point_a->X, point_a->Y, point_a->S);
    NOLOG("Pb(%ld,%ld,%ld)", point_b->X, point_b->Y, point_b->S);
    NOLOG("Pc(%ld,%ld,%ld)", point_c->X, point_c->Y, point_c->S);

    opt_a = point_a;
    opt_b = point_b;
    opt_c = point_c;
    start_type = trig_reorder_input_points(&opt_a, &opt_b, &opt_c);

    NOLOG("start type %d",(int)start_type);

    switch (start_type)
    {
    case RendStart_LL:
        if (!trig_ll_start(&tlp, &tlr, opt_a, opt_b, opt_c)) {
            return;
        }
        break;
    case RendStart_RL:
        if (!trig_rl_start(&tlp, &tlr, opt_a, opt_b, opt_c)) {
            return;
        }
        break;
    case RendStart_FB:
        if (!trig_fb_start(&tlp, &tlr, opt_a, opt_b, opt_c)) {
            return;
        }
        break;
    case RendStart_FT:
        if (!trig_ft_start(&tlp, &tlr, opt_a, opt_b, opt_c)) {
            return;
        }
        break;
    case RendStart_NO:
        return;
    }

    NOLOG("render mode %d",(int)vec_mode);

    switch (vec_mode)
    {
    case RendVec_mode00:
        trig_render_md00(&tlr);
        break;

    case RendVec_mode01:
        trig_render_md01(&tlr);
        break;

    case RendVec_mode02:
        trig_render_md02(&tlr);
        break;

    case RendVec_mode03:
        trig_render_md03(&tlr);
        break;

    case RendVec_mode04:
        trig_render_md04(&tlr);
        break;

    case RendVec_mode05:
        trig_render_md05(&tlr);
        break;

    case RendVec_mode06:
        trig_render_md06(&tlr);
        break;

    case RendVec_mode07:
    case RendVec_mode11:
        if (vec_colour == 0x20)
            trig_render_md02(&tlr);
        else
            trig_render_md07(&tlr);
        break;

    case RendVec_mode08:
        trig_render_md08(&tlr);
        break;

    case RendVec_mode09:
        trig_render_md09(&tlr);
        break;

    case RendVec_mode10:
        trig_render_md10(&tlr);
        break;

    case RendVec_mode12:
        trig_render_md12(&tlr);
        break;

    case RendVec_mode13:
        trig_render_md13(&tlr);
        break;

    case RendVec_mode14:
        trig_render_md14(&tlr);
        break;

    case RendVec_mode15:
        trig_render_md15(&tlr);
        break;

    case RendVec_mode16:
        trig_render_md16(&tlr);
        break;

    case RendVec_mode17:
        trig_render_md17(&tlr);
        break;

    case RendVec_mode18:
        trig_render_md18(&tlr);
        break;

    case RendVec_mode19:
        trig_render_md19(&tlr);
        break;

    case RendVec_mode20:
        trig_render_md20(&tlr);
        break;

    case RendVec_mode21:
        trig_render_md21(&tlr);
        break;

    case RendVec_mode22:
        trig_render_md22(&tlr);
        break;

    case RendVec_mode23:
        trig_render_md23(&tlr);
        break;

    case RendVec_mode24:
        trig_render_md24(&tlr);
        break;

    case RendVec_mode25:
        trig_render_md25(&tlr);
        break;

    case RendVec_mode26:
        trig_render_md26(&tlr);
        break;
    }

    NOLOG("end");
}
/******************************************************************************/
