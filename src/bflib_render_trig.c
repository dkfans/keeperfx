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
#include "bflib_memory.h"
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
    unsigned char var_24;// 4+
    unsigned char var_25; // 5+
    union {
        unsigned short flag_26; // 6+
    struct {
        unsigned char byte_26a;
        unsigned char byte_26b;
    };
    };
    unsigned long var_28; // 8+
    unsigned long var_2C; // unused
    // These are DWORDs
    unsigned long var_30; // 0x10+
    unsigned long var_34; // 0x14
    long var_38; // -0x18
    unsigned long var_3C; // 0x1C
    unsigned long var_40; // 0x20
    unsigned long var_44; // 0x24
    unsigned long var_48; // 0x28
    unsigned long delta_e; // 0x2C
    union {
    unsigned long var_50; // 0x30
    struct {
        unsigned short word_50a;
        unsigned short word_50b;
    };
    };
    unsigned long var_54; // 0x34
    unsigned long delta_d; // 0x38
    unsigned long var_5C; // 0x3C
    unsigned long var_60; // 0x40
    unsigned long delta_c; // 0x44
    unsigned long var_68; // 0x48
    unsigned long var_6C; // 0x4C
    unsigned long var_70; // 0x50
    unsigned long var_74; // 0x54
    unsigned long var_78; // 0x58
    unsigned long var_7C; // 0x5C
    unsigned long var_80; // 0x60
    unsigned long delta_b; // 0x64
    unsigned long delta_a; // 0x68
    unsigned char *var_8C; // 0x6C
};

struct TrigLocalPrep {
    long var_28;
    long var_2C;
    long var_30;
    long trig_height_top; // counter to loop over first part of polyscans array
    long var_38;
    long trig_height_bottom; // counter to loop over second part of polyscans array
    long var_40;
    long var_4C;
    long var_50;
    long var_58;
    long var_5C;
    long var_64;
    long var_68;
    long var_6C;
    long var_78;
    unsigned char var_8A;
    TbBool hide_bottom_part; // ?Should we show low part of a triangle
    unsigned char var_8C;
};

struct TrigLocalRend {
    unsigned char *var_24;
    long var_44;
    long var_48;
    long var_54;
    long var_60;
};

#pragma pack()
/******************************************************************************/
#define POLY_SCANS_COUNT 576
struct PolyPoint polyscans[8 * POLY_SCANS_COUNT]; // Allocate twice the size - this array is often exceeded, and rendering routines aren't safe

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
    long pX, pYa, pYb;
    struct PolyPoint *pp;

    pX = opt_a->X << 16;
    pYa = opt_a->X << 16;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        // whether the addition (tlr->var_44 + tlp->var_78) would overflow
        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        if (tlp->var_6C - tlp->var_38 >= 0)
        {
            tlp->trig_height_bottom -= tlp->var_6C - tlp->var_38;
            tlp->var_6C -= tlp->var_38;
            pX += tlp->var_28 * tlp->var_6C + tlp->var_38 * tlp->var_28;
            pYb = tlp->var_30 * tlp->var_6C + tlp->var_40;
            if (tlp->var_8C)
            {
                tlp->trig_height_bottom = vec_window_height;
                tlr->var_44 = vec_window_height;
            }
            tlp->var_38 = 0;
        }
        else
        {
            tlp->var_38 -= tlp->var_6C;
            pX += tlp->var_28 * tlp->var_6C;
            pYa += tlp->var_6C * tlp->var_2C;
            if (tlp->var_8C)
            {
                tlr->var_44 = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->var_38 = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->var_38;
                    tlp->trig_height_bottom = vec_window_height - tlp->var_38;
                }
            }
            pYb = tlp->var_40;
        }
    }
    else
    {
        if (tlp->var_8C)
        {
            long dH, eH;
            TbBool eH_overflow;

            dH = vec_window_height - tlp->var_78;
            tlr->var_44 = dH;
            if (tlp->hide_bottom_part) {
                tlp->var_38 = dH;
            } else {
                // whether the subtraction (dH - tlp->var_38) would overflow
                eH_overflow = __OFSUBL__(dH, tlp->var_38);
                eH = dH - tlp->var_38;
                tlp->hide_bottom_part = ((eH < 0) ^ eH_overflow) | (eH == 0);
                tlp->trig_height_bottom = eH;
            }
        }
        pYb = tlp->var_40;
    }
    pp = polyscans;
    for (; tlp->var_38; tlp->var_38--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pYa;
        pYa += tlp->var_2C;
        ++pp;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            pp->X = pX;
            pX += tlp->var_28;
            pp->Y = pYb;
            pYb += tlp->var_30;
            ++pp;
        }
    }
    return 1;
}

static inline int trig_ll_md01(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    struct PolyPoint *pp;
    long pX, pYa, pYb;
    long pS;
    long ratio_var_34;

    ratio_var_34 = (tlp->var_38 << 16) / tlp->trig_height_top;
    {
        long dX, wX;
        long eX;
        TbBool eX_overflow;

        dX = opt_a->X - opt_c->X;
        wX = (ratio_var_34 * dX) >> 16;
        dX = opt_b->X - opt_a->X;
        // whether the addition (wX + dX) would overflow
        eX_overflow = __OFSUBL__(wX, -dX);
        eX = wX + dX;
        if ((eX < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)wX, (long)dX);
            return 0;
        }
        if (eX != 0) {
            long long dS, wS;
            dS = opt_a->S - opt_c->S;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_60 = (opt_b->S + wS - opt_a->S) / (eX + 1);
        }
    }
    tlp->var_64 = (opt_c->S - opt_a->S) / tlp->trig_height_top;
    pX = opt_a->X << 16;
    pYa = opt_a->X << 16;
    pS = opt_a->S;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        if (tlp->var_6C - tlp->var_38 >= 0)
        {
            tlp->trig_height_bottom -= tlp->var_6C - tlp->var_38;
            tlp->var_6C -= tlp->var_38;
            pX += tlp->var_28 * tlp->var_6C + tlp->var_38 * tlp->var_28;
            pYb = tlp->var_30 * tlp->var_6C + tlp->var_40;
            pS += tlp->var_6C * tlp->var_64 + tlp->var_38 * tlp->var_64;
            if (tlp->var_8C)
            {
              tlp->trig_height_bottom = vec_window_height;
              tlr->var_44 = vec_window_height;
            }
            tlp->var_38 = 0;
        }
        else
        {
            tlp->var_38 -= tlp->var_6C;
            pX += tlp->var_28 * tlp->var_6C;
            pYa += tlp->var_6C * tlp->var_2C;
            pS += tlp->var_6C * tlp->var_64;
            if (tlp->var_8C)
            {
                tlr->var_44 = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->var_38 = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->var_38;
                    tlp->trig_height_bottom = vec_window_height - tlp->var_38;
                }
            }
            pYb = tlp->var_40;
        }
    }
    else
    {
        if (tlp->var_8C)
        {
            long dH, eH;
            TbBool eH_overflow;

            dH = vec_window_height - tlp->var_78;
            tlr->var_44 = dH;
            if (tlp->hide_bottom_part) {
                tlp->var_38 = dH;
            } else {
                eH_overflow = __OFSUBL__(dH, tlp->var_38);
                eH = dH - tlp->var_38;
                tlp->hide_bottom_part = ((eH < 0) ^ eH_overflow) | (eH == 0);
                tlp->trig_height_bottom = eH;
            }
        }
        pYb = tlp->var_40;
    }
    pp = polyscans;
    for (; tlp->var_38; tlp->var_38--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pYa;
        pYa += tlp->var_2C;
        pp->S = pS;
        pS += tlp->var_64;
        ++pp;
    }
    if (!tlp->hide_bottom_part)
    {
      for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
      {
          pp->X = pX;
          pX += tlp->var_28;
          pp->Y = pYb;
          pYb += tlp->var_30;
          pp->S = pS;
          pS += tlp->var_64;
          ++pp;
      }
    }
    return 1;
}

static inline int trig_ll_md02(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pX, pYa, pYb;
    long pU, pV;
    struct PolyPoint *pp;
    long ratio_var_34;

    ratio_var_34 = (tlp->var_38 << 16) / tlp->trig_height_top;
    {
        long dX, wX;
        long eX;
        TbBool eX_overflow;

        dX = opt_a->X - opt_c->X;
        wX = ratio_var_34 * dX >> 16;
        dX = opt_b->X - opt_a->X;
        eX_overflow = __OFSUBL__(wX, -dX);
        eX = wX + dX;
        if ((eX < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)wX, (long)dX);
            return 0;
        }
        if (eX != 0) {
            long long dS, wS;
            dS = opt_a->U - opt_c->U;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_48 = (opt_b->U + wS - opt_a->U) / (eX + 1);
            dS = opt_a->V - opt_c->V;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_54 = (opt_b->V + wS - opt_a->V) / (eX + 1);
        }
    }
    tlp->var_4C = (opt_c->U - opt_a->U) / tlp->trig_height_top;
    tlp->var_58 = (opt_c->V - opt_a->V) / tlp->trig_height_top;
    pX = opt_a->X << 16;
    pYa = opt_a->X << 16;
    pU = opt_a->U;
    pV = opt_a->V;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        if (tlp->var_6C - tlp->var_38 >= 0 )
        {
            tlp->trig_height_bottom -= tlp->var_6C - tlp->var_38;
            tlp->var_6C -= tlp->var_38;
            pX += tlp->var_28 * tlp->var_6C + tlp->var_38 * tlp->var_28;
            pYb = tlp->var_30 * tlp->var_6C + tlp->var_40;
            pU += tlp->var_6C * tlp->var_4C + tlp->var_38 * tlp->var_4C;
            pV += tlp->var_6C * tlp->var_58 + tlp->var_38 * tlp->var_58;
            if ( tlp->var_8C )
            {
                tlp->trig_height_bottom = vec_window_height;
                tlr->var_44 = vec_window_height;
            }
            tlp->var_38 = 0;
        }
        else
        {
            tlp->var_38 -= tlp->var_6C;
            pX += tlp->var_28 * tlp->var_6C;
            pYa += tlp->var_6C * tlp->var_2C;
            pU += tlp->var_6C * tlp->var_4C;
            pV += tlp->var_6C * tlp->var_58;
            if ( tlp->var_8C )
            {
                tlr->var_44 = vec_window_height;
                if (tlp->hide_bottom_part) {
                  tlp->var_38 = vec_window_height;
                } else {
                  tlp->hide_bottom_part = vec_window_height <= tlp->var_38;
                  tlp->trig_height_bottom = vec_window_height - tlp->var_38;
                }
            }
            pYb = tlp->var_40;
        }
    }
    else
    {
        if (tlp->var_8C)
        {
            long dH, eH;
            TbBool eH_overflow;

            dH = vec_window_height - tlp->var_78;
            tlr->var_44 = dH;
            if (tlp->hide_bottom_part) {
                tlp->var_38 = dH;
            } else {
                eH_overflow = __OFSUBL__(dH, tlp->var_38);
                eH = dH - tlp->var_38;
                tlp->hide_bottom_part = ((eH < 0) ^ eH_overflow) | (eH == 0);
                tlp->trig_height_bottom = eH;
            }
        }
        pYb = tlp->var_40;
    }
    pp = polyscans;
    for (; tlp->var_38; tlp->var_38--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pYa;
        pYa += tlp->var_2C;
        pp->U = pU;
        pU += tlp->var_4C;
        pp->V = pV;
        pV += tlp->var_58;
        ++pp;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            pp->X = pX;
            pX += tlp->var_28;
            pp->Y = pYb;
            pYb += tlp->var_30;
            pp->U = pU;
            pU += tlp->var_4C;
            pp->V = pV;
            pV += tlp->var_58;
            ++pp;
        }
    }
    return 1;
}

static inline int trig_ll_md05(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pX, pYa, pYb;
    long pU, pV, pS;
    struct PolyPoint *pp;
    long ratio_var_34;

    ratio_var_34 = (tlp->var_38 << 16) / tlp->trig_height_top;
    {
        long dX, wX;
        long eX;
        TbBool eX_overflow;

        dX = opt_a->X - opt_c->X;
        wX = ratio_var_34 * dX >> 16;
        dX = opt_b->X - opt_a->X;
        eX_overflow = __OFSUBL__(wX, -dX);
        eX = wX + dX;
        if ((eX < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)wX, (long)dX);
            return 0;
        }
        if (eX != 0)
        {
            long long dS, wS;
            dS = opt_a->U - opt_c->U;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_48 = (opt_b->U + wS - opt_a->U) / (eX + 1);
            dS = opt_a->V - opt_c->V;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_54 = (opt_b->V + wS - opt_a->V) / (eX + 1);
            dS = opt_a->S - opt_c->S;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_60 = (opt_b->S + wS - opt_a->S) / (eX + 1);
        }
    }
    tlp->var_4C = (opt_c->U - opt_a->U) / tlp->trig_height_top;
    tlp->var_58 = (opt_c->V - opt_a->V) / tlp->trig_height_top;
    tlp->var_64 = (opt_c->S - opt_a->S) / tlp->trig_height_top;

    pX = opt_a->X << 16;
    pYa = opt_a->X << 16;
    pU = opt_a->U;
    pV = opt_a->V;
    pS = opt_a->S;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        if (tlp->var_6C - tlp->var_38 >= 0)
        {
            tlp->trig_height_bottom -= tlp->var_6C - tlp->var_38;
            tlp->var_6C -= tlp->var_38;
            pX += tlp->var_28 * tlp->var_6C + tlp->var_38 * tlp->var_28;
            pYb = tlp->var_30 * tlp->var_6C + tlp->var_40;
            pU += tlp->var_6C * tlp->var_4C + tlp->var_38 * tlp->var_4C;
            pV += tlp->var_6C * tlp->var_58 + tlp->var_38 * tlp->var_58;
            pS += tlp->var_6C * tlp->var_64 + tlp->var_38 * tlp->var_64;
            if (tlp->var_8C) {
              tlp->trig_height_bottom = vec_window_height;
              tlr->var_44 = vec_window_height;
            }
            tlp->var_38 = 0;
        }
        else
        {
            tlp->var_38 -= tlp->var_6C;
            pX += tlp->var_28 * tlp->var_6C;
            pYa += tlp->var_6C * tlp->var_2C;
            pU += tlp->var_6C * tlp->var_4C;
            pV += tlp->var_6C * tlp->var_58;
            pS += tlp->var_6C * tlp->var_64;
            if (tlp->var_8C)
            {
                tlr->var_44 = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->var_38 = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->var_38;
                    tlp->trig_height_bottom = vec_window_height - tlp->var_38;
                }
            }
            pYb = tlp->var_40;
        }
    }
    else
    {
        if (tlp->var_8C)
        {
            long dH, eH;
            TbBool eH_overflow;

            dH = vec_window_height - tlp->var_78;
            tlr->var_44 = vec_window_height - tlp->var_78;
            if (tlp->hide_bottom_part) {
                tlp->var_38 = vec_window_height - tlp->var_78;
            } else {
                eH_overflow = __OFSUBL__(dH, tlp->var_38);
                eH = dH - tlp->var_38;
                tlp->hide_bottom_part = ((eH < 0) ^ eH_overflow) | (eH == 0);
                tlp->trig_height_bottom = eH;
            }
        }
        pYb = tlp->var_40;
    }
    pp = polyscans;
    for (; tlp->var_38; tlp->var_38--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pYa;
        pYa += tlp->var_2C;
        pp->U = pU;
        pU += tlp->var_4C;
        pp->V = pV;
        pV += tlp->var_58;
        pp->S = pS;
        pS += tlp->var_64;
        ++pp;
    }
    if ( !tlp->hide_bottom_part )
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
          pp->X = pX;
          pX += tlp->var_28;
          pp->Y = pYb;
          pYb += tlp->var_30;
          pp->U = pU;
          pU += tlp->var_4C;
          pp->V = pV;
          pV += tlp->var_58;
          pp->S = pS;
          pS += tlp->var_64;
          ++pp;
        }
    }
    return 1;
}

int trig_ll_start(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int ret;
    long dX, dY;

    tlp->var_78 = opt_a->Y;
    if (opt_a->Y < 0) {
      tlr->var_24 = poly_screen;
      tlp->var_8A = 1;
    } else if (opt_a->Y < vec_window_height) {
      tlr->var_24 = poly_screen + vec_screen_width * opt_a->Y;
      tlp->var_8A = 0;
    } else {
        NOLOG("height %ld exceeded by opt_a Y %ld", (long)vec_window_height, (long)opt_a->Y);
        return 0;
    }

    tlp->var_8C = opt_c->Y > vec_window_height;
    dY = opt_c->Y - opt_a->Y;
    tlp->trig_height_top = dY;
    tlr->var_44 = dY;

    tlp->hide_bottom_part = opt_b->Y > vec_window_height;
    dY = opt_b->Y - opt_a->Y;
    tlp->var_38 = dY;
    dX = opt_c->X - opt_a->X;
    tlp->var_28 = (dX << 16) / tlp->trig_height_top;
    dX = opt_b->X - opt_a->X;
    if ((dX << 16) / dY <= tlp->var_28) {
        NOLOG("value (%ld << 16) / %ld below min %ld", (long)dX, (long)dY, (long)tlp->var_28);
        return 0;
    }
    tlp->var_2C = (dX << 16) / dY;

    dY = opt_c->Y - opt_b->Y;
    dX = opt_c->X - opt_b->X;
    tlp->var_30 = (dX << 16) / dY;
    tlp->trig_height_bottom = dY;
    tlp->var_40 = opt_b->X << 16;

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
    ulong pXa, pXb, pY;
    struct PolyPoint *pp;

    pXa = opt_a->X << 16;
    pY = opt_a->X << 16;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        if (tlp->var_6C - tlp->trig_height_top >= 0)
        {
            tlp->var_6C -= tlp->trig_height_top;
            tlp->trig_height_bottom -= tlp->var_6C;
            pXb = tlp->var_30 * tlp->var_6C + tlp->var_40;
            pY += tlp->var_6C * tlp->var_2C + tlp->trig_height_top * tlp->var_2C;
            if (tlp->var_8C) {
              tlp->trig_height_bottom = vec_window_height;
              tlr->var_44 = vec_window_height;
            }
            tlp->trig_height_top = 0;
        }
        else
        {
            tlp->trig_height_top -= tlp->var_6C;
            pXa += tlp->var_28 * tlp->var_6C;
            pY += tlp->var_6C * tlp->var_2C;
            if (tlp->var_8C)
            {
                tlr->var_44 = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->trig_height_top = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->trig_height_top;
                    tlp->trig_height_bottom = vec_window_height - tlp->trig_height_top;
                }
            }
            pXb = tlp->var_40;
        }
    }
    else
    {
        if (tlp->var_8C)
        {
            long dH, eH;
            TbBool eH_overflow;

            dH = vec_window_height - tlp->var_78;
            tlr->var_44 = dH;
            if (tlp->hide_bottom_part) {
                tlp->trig_height_top = dH;
            } else {
                eH_overflow = __OFSUBL__(dH, tlp->trig_height_top);
                eH = dH - tlp->trig_height_top;
                tlp->hide_bottom_part = ((eH < 0) ^ eH_overflow) | (eH == 0);
                tlp->trig_height_bottom = eH;
            }
        }
        pXb = tlp->var_40;
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pXa;
        pXa += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        ++pp;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            pp->X = pXb;
            pXb += tlp->var_30;
            pp->Y = pY;
            pY += tlp->var_2C;
            ++pp;
        }
    }
    return 1;
}

static inline int trig_rl_md01(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pXa, pXb, pY;
    long pS;
    struct PolyPoint *pp;
    long ratio_var_34;

    ratio_var_34 = (tlp->trig_height_top << 16) / tlp->var_38;
    {
        long dXa, wXb;
        long eX;
        TbBool eX_overflow;

        wXb = ratio_var_34 * (opt_b->X - opt_a->X) >> 16;
        dXa = opt_a->X - opt_c->X;
        eX_overflow = __OFSUBL__(wXb, -dXa);
        eX = wXb + dXa;
        if ((eX < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)wXb, (long)dXa);
            return 0;
        }
        if (eX != 0) {
            long long dS, wS;
            dS = opt_b->S - opt_a->S;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_60 = (opt_a->S + wS - opt_c->S) / (eX + 1);
        }
    }
    tlp->var_64 = (opt_c->S - opt_a->S) / tlp->trig_height_top;
    tlp->var_68 = (opt_b->S - opt_c->S) / tlp->trig_height_bottom;
    pXa = opt_a->X << 16;
    pY = opt_a->X << 16;
    pS = opt_a->S;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        if (tlp->var_6C - tlp->trig_height_top >= 0)
        {
            tlp->var_6C -= tlp->trig_height_top;
            tlp->trig_height_bottom -= tlp->var_6C;
            pXb = tlp->var_30 * tlp->var_6C + tlp->var_40;
            pY += tlp->var_6C * tlp->var_2C + tlp->trig_height_top * tlp->var_2C;
            pS += tlp->var_6C * tlp->var_68 + tlp->trig_height_top * tlp->var_64;
            if (tlp->var_8C) {
                tlp->trig_height_bottom = vec_window_height;
                tlr->var_44 = vec_window_height;
            }
            tlp->trig_height_top = 0;
        }
        else
        {
            tlp->trig_height_top -= tlp->var_6C;
            pXa += tlp->var_28 * tlp->var_6C;
            pY += tlp->var_6C * tlp->var_2C;
            pS += tlp->var_6C * tlp->var_64;
            if ( tlp->var_8C )
            {
                tlr->var_44 = vec_window_height;
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
            pXb = tlp->var_40;
        }
    }
    else
    {
        if (tlp->var_8C)
        {
            long dH, eH;
            TbBool eH_overflow;

            dH = vec_window_height - tlp->var_78;
            tlr->var_44 = vec_window_height - tlp->var_78;
            if (tlp->hide_bottom_part) {
                tlp->trig_height_top = vec_window_height - tlp->var_78;
            } else {
                eH_overflow = __OFSUBL__(dH, tlp->trig_height_top);
                eH = dH - tlp->trig_height_top;
                tlp->hide_bottom_part = ((eH < 0) ^ eH_overflow) | (eH == 0);
                tlp->trig_height_bottom = eH;
            }
        }
        pXb = tlp->var_40;
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pXa;
        pXa += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        pp->S = pS;
        pS += tlp->var_64;
        ++pp;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            pp->X = pXb;
            pXb += tlp->var_30;
            pp->Y = pY;
            pY += tlp->var_2C;
            pp->S = pS;
            pS += tlp->var_68;
            ++pp;
        }
    }
    return 1;
}

static inline int trig_rl_md02(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pXa, pXb, pY;
    long pU, pV;
    struct PolyPoint *pp;
    long ratio_var_34;

    ratio_var_34 = (tlp->trig_height_top << 16) / tlp->var_38; // Fixed point math
    {
        long dXa, wXb;
        long eX;
        TbBool eX_overflow;

        wXb = ratio_var_34 * (opt_b->X - opt_a->X) >> 16;
        dXa = opt_a->X - opt_c->X;
        eX_overflow = __OFSUBL__(wXb, -dXa);
        eX = wXb + dXa;
        if ((eX < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)wXb, (long)dXa);
            return 0;
        }
        if (eX != 0) {
            long long dS, wS;

            dS = opt_b->U - opt_a->U;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_48 = (opt_a->U + wS - opt_c->U) / (eX + 1);
            dS = opt_b->V - opt_a->V;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_54 = (opt_a->V + wS - opt_c->V) / (eX + 1);
        }
    }
    tlp->var_4C = (opt_c->U - opt_a->U) / tlp->trig_height_top;
    tlp->var_58 = (opt_c->V - opt_a->V) / tlp->trig_height_top;
    tlp->var_50 = (opt_b->U - opt_c->U) / tlp->trig_height_bottom;
    tlp->var_5C = (opt_b->V - opt_c->V) / tlp->trig_height_bottom;
    pXa = opt_a->X << 16;
    pY = opt_a->X << 16;
    pU = opt_a->U;
    pV = opt_a->V;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        if (tlp->var_6C - tlp->trig_height_top >= 0)
        {
            tlp->var_6C -= tlp->trig_height_top;
            tlp->trig_height_bottom -= tlp->var_6C;
            pXb = tlp->var_30 * tlp->var_6C + tlp->var_40;
            pY += tlp->var_6C * tlp->var_2C + tlp->trig_height_top * tlp->var_2C;
            pU += tlp->var_6C * tlp->var_50 + tlp->trig_height_top * tlp->var_4C;
            pV += tlp->var_6C * tlp->var_5C + tlp->trig_height_top * tlp->var_58;
            if (tlp->var_8C) {
                tlp->trig_height_bottom = vec_window_height;
                tlr->var_44 = vec_window_height;
            }
            tlp->trig_height_top = 0;
        }
        else
        {
            tlp->trig_height_top -= tlp->var_6C;
            pXa += tlp->var_28 * tlp->var_6C;
            pY += tlp->var_6C * tlp->var_2C;
            pU += tlp->var_6C * tlp->var_4C;
            pV += tlp->var_6C * tlp->var_58;
            if ( tlp->var_8C )
            {
                tlr->var_44 = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->trig_height_top = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->trig_height_top;
                    tlp->trig_height_bottom = vec_window_height - tlp->trig_height_top;
                }
            }
            pXb = tlp->var_40;
        }
    }
    else
    {
        if (tlp->var_8C)
        {
            long dH, eH;
            TbBool eH_overflow;

            dH = vec_window_height - tlp->var_78;
            tlr->var_44 = dH;
            if (tlp->hide_bottom_part) {
                tlp->trig_height_top = dH;
            } else {
                eH_overflow = __OFSUBL__(dH, tlp->trig_height_top);
                eH = dH - tlp->trig_height_top;
                tlp->hide_bottom_part = ((eH < 0) ^ eH_overflow) | (eH == 0);
                tlp->trig_height_bottom = eH;
            }
        }
        pXb = tlp->var_40;
    }
    pp = polyscans;

    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pXa;
        pXa += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        pp->U = pU;
        pU += tlp->var_4C;
        pp->V = pV;
        pV += tlp->var_58;
        ++pp;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
            pp->X = pXb;
            pXb += tlp->var_30;
            pp->Y = pY;
            pY += tlp->var_2C;
            pp->U = pU;
            pU += tlp->var_50;
            pp->V = pV;
            pV += tlp->var_5C;
            ++pp;
        }
    }
    return 1;
}

static inline int trig_rl_md05(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pXa, pXb, pY;
    long pU, pV, pS;
    struct PolyPoint *pp;
    long ratio_var_34;

    ratio_var_34 = (tlp->trig_height_top << 16) / tlp->var_38;
    {
        long dXa, wXb;
        long eX;
        TbBool eX_overflow;

        wXb = ratio_var_34 * (opt_b->X - opt_a->X) >> 16;
        dXa = opt_a->X - opt_c->X;
        eX_overflow = __OFSUBL__(wXb, -dXa);
        eX = wXb + dXa;
        if ((eX < 0) ^ eX_overflow) {
            NOLOG("skip due to sum %ld %ld", (long)wXb, (long)dXa);
            return 0;
        }
        tlr->var_60 = wXb;
        if (eX != 0) {
            long long dS, wS;

            dS = opt_b->U - opt_a->U;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_48 = (opt_a->U + wS - opt_c->U) / (eX + 1);
            dS = opt_b->V - opt_a->V;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_54 = (opt_a->V + wS - opt_c->V) / (eX + 1);
            dS = opt_b->S - opt_a->S;
            wS = (ratio_var_34 * dS) >> 16;
            tlr->var_60 = (opt_a->S + wS - opt_c->S) / (eX + 1);
        }
    }
    tlp->var_4C = (opt_c->U - opt_a->U) / tlp->trig_height_top;
    tlp->var_58 = (opt_c->V - opt_a->V) / tlp->trig_height_top;
    tlp->var_64 = (opt_c->S - opt_a->S) / tlp->trig_height_top;
    tlp->var_50 = (opt_b->U - opt_c->U) / tlp->trig_height_bottom;
    tlp->var_5C = (opt_b->V - opt_c->V) / tlp->trig_height_bottom;
    tlp->var_68 = (opt_b->S - opt_c->S) / tlp->trig_height_bottom;
    pXa = opt_a->X << 16;
    pY = opt_a->X << 16;
    pU = opt_a->U;
    pV = opt_a->V;
    pS = opt_a->S;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        if (tlp->var_6C - tlp->trig_height_top >= 0)
        {
            tlp->var_6C -= tlp->trig_height_top;
            tlp->trig_height_bottom -= tlp->var_6C;
            pXb = tlp->var_30 * tlp->var_6C + tlp->var_40;
            pY += tlp->var_6C * tlp->var_2C + tlp->trig_height_top * tlp->var_2C;
            pU += tlp->var_6C * tlp->var_50 + tlp->trig_height_top * tlp->var_4C;
            pV += tlp->var_6C * tlp->var_5C + tlp->trig_height_top * tlp->var_58;
            pS += tlp->var_6C * tlp->var_68 + tlp->trig_height_top * tlp->var_64;
            if (tlp->var_8C) {
                tlp->trig_height_bottom = vec_window_height;
                tlr->var_44 = vec_window_height;
            }
            tlp->trig_height_top = 0;
        }
        else
        {
            tlp->trig_height_top -= tlp->var_6C;
            pXa += tlp->var_28 * tlp->var_6C;
            pY += tlp->var_6C * tlp->var_2C;
            pU += tlp->var_6C * tlp->var_4C;
            pV += tlp->var_6C * tlp->var_58;
            pS += tlp->var_6C * tlp->var_64;
            if (tlp->var_8C) {
                tlr->var_44 = vec_window_height;
                if (tlp->hide_bottom_part) {
                    tlp->trig_height_top = vec_window_height;
                } else {
                    tlp->hide_bottom_part = vec_window_height <= tlp->trig_height_top;
                    tlp->trig_height_bottom = vec_window_height - tlp->trig_height_top;
                }
            }
            pXb = tlp->var_40;
        }
    }
    else
    {
        if (tlp->var_8C)
        {
            long dH, eH;
            TbBool eH_overflow;

            dH = vec_window_height - tlp->var_78;
            tlr->var_44 = dH;
            if (tlp->hide_bottom_part) {
                tlp->trig_height_top = dH;
            } else {
                eH_overflow = __OFSUBL__(dH, tlp->trig_height_top);
                eH = dH - tlp->trig_height_top;
                tlp->hide_bottom_part = ((eH < 0) ^ eH_overflow) | (eH == 0);
                tlp->trig_height_bottom = eH;
            }
        }
        pXb = tlp->var_40;
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pXa;
        pXa += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        pp->U = pU;
        pU += tlp->var_4C;
        pp->V = pV;
        pV += tlp->var_58;
        pp->S = pS;
        pS += tlp->var_64;
        ++pp;
    }
    if (!tlp->hide_bottom_part)
    {
        for (; tlp->trig_height_bottom; tlp->trig_height_bottom--)
        {
          pp->X = pXb;
          pXb += tlp->var_30;
          pp->Y = pY;
          pY += tlp->var_2C;
          pp->U = pU;
          pU += tlp->var_50;
          pp->V = pV;
          pV += tlp->var_5C;
          pp->S = pS;
          pS += tlp->var_68;
          ++pp;
        }
    }
    return 1;
}

int trig_rl_start(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int ret;
    long dX, dY;

    tlp->var_78 = opt_a->Y;
    if (opt_a->Y < 0) {
      tlr->var_24 = poly_screen;
      tlp->var_8A = 1;
    } else if (opt_a->Y < vec_window_height) {
      tlr->var_24 = poly_screen + vec_screen_width * opt_a->Y;
      tlp->var_8A = 0;
    } else  {
        NOLOG("height %ld exceeded by opt_a Y %ld", (long)vec_window_height, (long)opt_a->Y);
        return 0;
    }

    tlp->hide_bottom_part = opt_c->Y > vec_window_height;
    dY = opt_c->Y - opt_a->Y;
    tlp->trig_height_top = dY;

    tlp->var_8C = opt_b->Y > vec_window_height;
    dY = opt_b->Y - opt_a->Y;
    tlp->var_38 = dY;
    tlr->var_44 = dY;
    dX = opt_c->X - opt_a->X;
    tlp->var_28 = (dX << 16) / tlp->trig_height_top;
    dX = opt_b->X - opt_a->X;
    if ((dX << 16) / dY <= tlp->var_28) {
        NOLOG("value (%ld << 16) / %ld below min %ld", (long)dX, (long)dY, (long)tlp->var_28);
        return 0;
    }
    tlp->var_2C = (dX << 16) / dY;

    dY = opt_b->Y - opt_c->Y;
    dX = opt_b->X - opt_c->X;
    tlp->var_30 = (dX << 16) / dY;
    tlp->trig_height_bottom = dY;
    tlp->var_40 = opt_c->X << 16;

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
    long pX, pY;
    struct PolyPoint *pp;

    pX = opt_a->X << 16;
    pY = opt_a->X << 16;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        tlp->trig_height_top += tlp->var_78;
        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        pX += tlp->var_28 * (-tlp->var_78);
        pY += (-tlp->var_78) * tlp->var_2C;
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height - tlp->var_78;
            tlp->trig_height_top = vec_window_height - tlp->var_78;
        }
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        ++pp;
    }
    return 1;
}

static inline int trig_fb_md01(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int pX, pY;
    int pS;
    struct PolyPoint *pp;

    {
        long dX;
        dX = opt_b->X - opt_c->X;
        tlr->var_60 = (opt_b->S - opt_c->S) / dX;
        tlp->var_64 = (opt_c->S - opt_a->S) / tlr->var_44;
    }
    pX = opt_a->X << 16;
    pY = opt_a->X << 16;
    pS = opt_a->S;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        tlp->trig_height_top += tlp->var_78;
        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        pX += tlp->var_28 * (-tlp->var_78);
        pY += (-tlp->var_78) * tlp->var_2C;
        pS += (-tlp->var_78) * tlp->var_64;
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height - tlp->var_78;
            tlp->trig_height_top = vec_window_height - tlp->var_78;
        }
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        pp->S = pS;
        pS += tlp->var_64;
        ++pp;
    }
    return 1;
}

static inline int trig_fb_md02(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pX, pY;
    long pU, pV;
    struct PolyPoint *pp;

    {
        long dX;
        dX = opt_b->X - opt_c->X;
        tlr->var_48 = (opt_b->U - opt_c->U) / dX;
        tlr->var_54 = (opt_b->V - opt_c->V) / dX;
        tlp->var_4C = (opt_c->U - opt_a->U) / tlr->var_44;
        tlp->var_58 = (opt_c->V - opt_a->V) / tlr->var_44;
    }
    pX = opt_a->X << 16;
    pY = opt_a->X << 16;
    pU = opt_a->U;
    pV = opt_a->V;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        tlp->trig_height_top += tlp->var_78;
        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        pX += tlp->var_28 * (-tlp->var_78);
        pY += (-tlp->var_78) * tlp->var_2C;
        pU += (-tlp->var_78) * tlp->var_4C;
        pV += (-tlp->var_78) * tlp->var_58;
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height - tlp->var_78;
            tlp->trig_height_top = vec_window_height - tlp->var_78;
        }
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        pp->U = pU;
        pU += tlp->var_4C;
        pp->V = pV;
        pV += tlp->var_58;
        ++pp;
    }
    return 1;
}

static inline int trig_fb_md05(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pX, pY;
    long pU, pV, pS;
    struct PolyPoint *pp;

    {
        long dX;
        dX = opt_b->X - opt_c->X;
        tlr->var_48 = (opt_b->U - opt_c->U) / dX;
        tlr->var_54 = (opt_b->V - opt_c->V) / dX;
        tlr->var_60 = (opt_b->S - opt_c->S) / dX;
        tlp->var_4C = (opt_c->U - opt_a->U) / tlr->var_44;
        tlp->var_58 = (opt_c->V - opt_a->V) / tlr->var_44;
        tlp->var_64 = (opt_c->S - opt_a->S) / tlr->var_44;
    }
    pX = opt_a->X << 16;
    pY = opt_a->X << 16;
    pU = opt_a->U;
    pV = opt_a->V;
    pS = opt_a->S;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        tlp->trig_height_top += tlp->var_78;
        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        pX += tlp->var_28 * (-tlp->var_78);
        pY += (-tlp->var_78) * tlp->var_2C;
        pU += (-tlp->var_78) * tlp->var_4C;
        pV += (-tlp->var_78) * tlp->var_58;
        pS += (-tlp->var_78) * tlp->var_64;
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height - tlp->var_78;
            tlp->trig_height_top = vec_window_height - tlp->var_78;
        }
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        pp->U = pU;
        pU += tlp->var_4C;
        pp->V = pV;
        pV += tlp->var_58;
        pp->S = pS;
        pS += tlp->var_64;
        ++pp;
    }
    return 1;
}

int trig_fb_start(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int ret;
    long dX, dY;

    tlp->var_78 = opt_a->Y;
    if (opt_a->Y < 0) {
        tlr->var_24 = poly_screen;
        tlp->var_8A = 1;
    } else if (opt_a->Y < vec_window_height) {
        tlr->var_24 = poly_screen + vec_screen_width * opt_a->Y;
        tlp->var_8A = 0;
    } else {
        NOLOG("height %ld exceeded by opt_a Y %ld", (long)vec_window_height, (long)opt_a->Y);
        return 0;
    }
    tlp->hide_bottom_part = opt_c->Y > vec_window_height;
    dY = opt_c->Y - opt_a->Y;
    tlp->trig_height_top = dY;
    tlr->var_44 = dY;
    dX = opt_c->X - opt_a->X;
    tlp->var_28 = (dX << 16) / dY;
    dX = opt_b->X - opt_a->X;
    tlp->var_2C = (dX << 16) / dY;

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
    long pX, pY;
    struct PolyPoint *pp;

    pX = opt_a->X << 16;
    pY = opt_b->X << 16;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        tlp->trig_height_top += tlp->var_78;
        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        pX += tlp->var_28 * (-tlp->var_78);
        pY += (-tlp->var_78) * tlp->var_2C;
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height - tlp->var_78;
            tlp->trig_height_top = vec_window_height - tlp->var_78;
        }
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        ++pp;
    }
    return 1;
}

static inline int trig_ft_md01(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pX, pY;
    long pS;
    struct PolyPoint *pp;

    {
        long dX;
        dX = opt_b->X - opt_a->X;
        tlr->var_60 = (opt_b->S - opt_a->S) / dX;
        tlp->var_64 = (opt_c->S - opt_a->S) / tlr->var_44;
    }
    pX = opt_a->X << 16;
    pY = opt_b->X << 16;
    pS = opt_a->S;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        tlp->trig_height_top += tlp->var_78;
        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        pX += tlp->var_28 * (-tlp->var_78);
        pY += (-tlp->var_78) * tlp->var_2C;
        pS += (-tlp->var_78) * tlp->var_64;
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height - tlp->var_78;
            tlp->trig_height_top = vec_window_height - tlp->var_78;
        }
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        pp->S = pS;
        pS += tlp->var_64;
        ++pp;
    }
    return 1;
}

static inline int trig_ft_md02(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pX, pY;
    long pU, pV;
    struct PolyPoint *pp;
    {
        long dX;
        dX = opt_b->X - opt_a->X;
        tlr->var_48 = (opt_b->U - opt_a->U) / dX;
        tlr->var_54 = (opt_b->V - opt_a->V) / dX;
        tlp->var_4C = (opt_c->U - opt_a->U) / tlr->var_44;
        tlp->var_58 = (opt_c->V - opt_a->V) / tlr->var_44;
    }
    pX = opt_a->X << 16;
    pY = opt_b->X << 16;
    pU = opt_a->U;
    pV = opt_a->V;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        tlp->trig_height_top += tlp->var_78;
        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        pX += tlp->var_28 * (-tlp->var_78);
        pY += (-tlp->var_78) * tlp->var_2C;
        pU += (-tlp->var_78) * tlp->var_4C;
        pV += (-tlp->var_78) * tlp->var_58;
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height - tlp->var_78;
            tlp->trig_height_top = vec_window_height - tlp->var_78;
        }
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        pp->U = pU;
        pU += tlp->var_4C;
        pp->V = pV;
        pV += tlp->var_58;
        ++pp;
    }
    return 1;
}

static inline int trig_ft_md05(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    long pX, pY;
    long pU, pV, pS;
    struct PolyPoint *pp;

    {
        long dX;
        dX = opt_b->X - opt_a->X;
        tlr->var_48 = (opt_b->U - opt_a->U) / dX;
        tlr->var_54 = (opt_b->V - opt_a->V) / dX;
        tlr->var_60 = (opt_b->S - opt_a->S) / dX;
        tlp->var_4C = (opt_c->U - opt_a->U) / tlr->var_44;
        tlp->var_58 = (opt_c->V - opt_a->V) / tlr->var_44;
        tlp->var_64 = (opt_c->S - opt_a->S) / tlr->var_44;
    }
    pX = opt_a->X << 16;
    pY = opt_b->X << 16;
    pU = opt_a->U;
    pV = opt_a->V;
    pS = opt_a->S;
    if (tlp->var_8A)
    {
        long eH;
        TbBool eH_overflow;

        tlp->trig_height_top += tlp->var_78;
        eH_overflow = __OFSUBL__(tlr->var_44, -tlp->var_78);
        eH = tlr->var_44 + tlp->var_78;
        if (((eH < 0) ^ eH_overflow) | (eH == 0)) {
            NOLOG("skip due to sum %ld %ld", (long)tlr->var_44, (long)tlp->var_78);
            return 0;
        }
        tlr->var_44 = eH;
        tlp->var_6C = -tlp->var_78;
        pX += tlp->var_28 * (-tlp->var_78);
        pY += (-tlp->var_78) * tlp->var_2C;
        pU += (-tlp->var_78) * tlp->var_4C;
        pV += (-tlp->var_78) * tlp->var_58;
        pS += (-tlp->var_78) * tlp->var_64;
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height;
            tlp->trig_height_top = vec_window_height;
        }
    }
    else
    {
        if (tlp->hide_bottom_part) {
            tlr->var_44 = vec_window_height - tlp->var_78;
            tlp->trig_height_top = vec_window_height - tlp->var_78;
        }
    }
    pp = polyscans;
    for (; tlp->trig_height_top; tlp->trig_height_top--)
    {
        pp->X = pX;
        pX += tlp->var_28;
        pp->Y = pY;
        pY += tlp->var_2C;
        pp->U = pU;
        pU += tlp->var_4C;
        pp->V = pV;
        pV += tlp->var_58;
        pp->S = pS;
        pS += tlp->var_64;
        ++pp;
    }
    return 1;
}

int trig_ft_start(struct TrigLocalPrep *tlp, struct TrigLocalRend *tlr, const struct PolyPoint *opt_a,
  const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    int ret;
    long dX, dY;

    tlp->var_78 = opt_a->Y;
    if (opt_a->Y < 0) {
      tlr->var_24 = poly_screen;
      tlp->var_8A = 1;
    } else if (opt_a->Y < vec_window_height) {
      tlr->var_24 = poly_screen + vec_screen_width * opt_a->Y;
      tlp->var_8A = 0;
    } else {
        NOLOG("height %ld exceeded by opt_a Y %ld", (long)vec_window_height, (long)opt_a->Y);
        return 0;
    }
    tlp->hide_bottom_part = opt_c->Y > vec_window_height;
    dY = opt_c->Y - opt_a->Y;
    tlp->trig_height_top = dY;
    tlr->var_44 = dY;
    dX = opt_c->X - opt_a->X;
    tlp->var_28 = (dX << 16) / dY;
    dX = opt_c->X - opt_b->X;
    tlp->var_2C = (dX << 16) / dY;

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

void trig_render_md00(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *o_ln;
    unsigned char col;

    pp = polyscans;
    o_ln = tlr->var_24;
    col = vec_colour;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        long pX, pY;
        unsigned char *o;

        pX = pp->X >> 16;
        pY = pp->Y >> 16;
        o_ln += vec_screen_width;
        if (pX < 0)
        {
            if (pY <= 0)
                continue;
            if (pY > vec_window_width)
                pY = vec_window_width;
            o = &o_ln[0];
        }
        else
        {
            TbBool pY_overflow;
            if (pY > vec_window_width)
                pY = vec_window_width;
            pY_overflow = __OFSUBL__(pY, pX);
            pY = pY - pX;
            if (((pY < 0) ^ pY_overflow) | (pY == 0))
                continue;
            o = &o_ln[pX];
        }
        memset(o, col, pY);
    }
}

void trig_render_md01(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    TbBool pS_carry;
    pp = polyscans;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pX, pY;
        short pS;
        ushort colS;
        unsigned char *o;

        pX = pp->X >> 16;
        pY = pp->Y >> 16;
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;

        if (pX  < 0)
        {
            long mX;
            short colH;

            if (pY <= 0)
                continue;
            mX = tlr->var_60 * (ushort)(-pX);
            pS_carry = __CFADDS__(pp->S, mX);
            pS = pp->S + mX;
            // Delcate code - if we add before shifting, the result is different
            colH = (mX >> 16) + (pp->S >> 16) + pS_carry;
            if (pY > vec_window_width)
                pY = vec_window_width;

            colS = ((colH & 0xFF) << 8) + vec_colour;
        }
        else
        {
            TbBool pY_overflow;
            short colH;

            if (pY > vec_window_width)
              pY = vec_window_width;
            pY_overflow = __OFSUBS__(pY, pX);
            pY = pY - pX;
            if (((pY < 0) ^ pY_overflow) | (pY == 0))
                continue;
            o += pX;
            colH = pp->S >> 16;
            pS = pp->S;

            colS = ((colH & 0xFF) << 8) + vec_colour;
        }

        for (;pY > 0; pY--, o++)
        {
            short colH, colL;
            *o = colS >> 8;

            colL = colS;
            pS_carry = __CFADDS__(tlr->var_60, pS);
            pS = tlr->var_60 + pS;
            colH = (tlr->var_60 >> 16) + pS_carry + (colS >> 8);

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md02(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    long lsh_var_54;

    m = vec_map;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pX, pY;
        long pU;
        ushort colS;
        unsigned char *o;

        pX = pp->X >> 16;
        pY = pp->Y >> 16;
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;

        if (pX < 0)
        {
            ushort colL, colH;
            ulong factorA;
            long mX;

            if (pY <= 0)
                continue;
            mX = tlr->var_54 * (-pX);
            factorA = __ROL4__(pp->V + mX, 16);
            colH = factorA;
            mX = tlr->var_48 * (-pX);
            pU = (factorA & 0xFFFF0000) | ((pp->U + mX) & 0xFFFF);
            colL = (pp->U + mX) >> 16;
            if (pY > vec_window_width)
                pY = vec_window_width;
            pX = (pp->U + mX) >> 8;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            short colL, colH;
            TbBool pY_overflow;

            if (pY > vec_window_width)
                pY = vec_window_width;
            pY_overflow = __OFSUBS__(pY, pX);
            pY = pY - pX;
            if (((pY < 0) ^ pY_overflow) | (pY == 0))
                continue;
            o += pX;
            pU = __ROL4__(pp->V, 16);
            colH = pU;
            pU = (pU & 0xFFFF0000) | (pp->U & 0xFFFF);
            colL = pp->U >> 16;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pY > 0; pY--, o++)
        {
            short colL, colH;
            TbBool pU_carry;

            *o = m[colS];

            pU_carry = __CFADDS__(tlr->var_48, pU);
            pU = (pU & 0xFFFF0000) | ((tlr->var_48 + pU) & 0xFFFF);
            colL = (tlr->var_48 >> 16) + pU_carry + colS;

            pU_carry = __CFADDL__(lsh_var_54, pU);
            pU = lsh_var_54 + pU;
            colH = (tlr->var_54 >> 16) + pU_carry + (colS >> 8);

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md03(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    long lsh_var_54;

    m = vec_map;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pX, pY;
        long pU;
        ulong factorA;
        ushort colS;
        unsigned char *o;

        pX = pp->X >> 16;
        pY = pp->Y >> 16;
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;

        if (pX < 0)
        {
            short colL, colH;
            long mX;

            if (pY <= 0)
                continue;
            mX = tlr->var_54 * (-pX);
            factorA = __ROL4__(pp->V + mX, 16);
            colH = factorA;
            mX = tlr->var_48 * (-pX);
            pU = (factorA & 0xFFFF0000) | ((pp->U + mX) & 0xFFFF);
            colL = (pp->U + mX) >> 16;
            if (pY > vec_window_width)
                pY = vec_window_width;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            short colL, colH;
            TbBool pY_overflow;

            if (pY > vec_window_width)
                pY = vec_window_width;
            pY_overflow = __OFSUBS__(pY, pX);
            pY = pY - pX;
            if (((pY < 0) ^ pY_overflow) | (pY == 0))
                continue;
            o += pX;
            pU = __ROL4__(pp->V, 16);
            colH = pU;
            pU = (pU & 0xFFFF0000) | ((pp->U) & 0xFFFF);
            colL = (pp->U) >> 16;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pY > 0; pY--, o++)
        {
            short colL, colH;
            TbBool pU_carry;

            if (m[colS] != 0)
                *o = m[colS];

            pU_carry = __CFADDS__(tlr->var_48, pU);
            pU = (pU & 0xFFFF0000) | ((tlr->var_48 + pU) & 0xFFFF);
            colL = (tlr->var_48 >> 16) + pU_carry + colS;
            pU_carry = __CFADDL__(lsh_var_54, pU);
            pU = lsh_var_54 + pU;
            colH = (tlr->var_54 >> 16) + pU_carry + (colS >> 8);

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
    struct PolyPoint *pp;
    unsigned char *f;

    f = pixmap.fade_tables;
    pp = polyscans;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pX, pY;
        short pU;
        ushort colS;
        unsigned char *o;

        pX = pp->X >> 16;
        pY = pp->Y >> 16;
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pX < 0)
        {
            ushort colL, colH;
            TbBool pU_carry;
            long mX;

            if (pY <= 0)
                continue;
            mX = tlr->var_60 * (-pX);
            pU_carry = __CFADDS__(pp->S, mX);
            pU = pp->S + mX;
            colH = (pp->S >> 16) + pU_carry + (mX >> 16);
            if (pY > vec_window_width)
                pY = vec_window_width;
            colL = vec_colour;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            TbBool pY_overflow;

            if (pY > vec_window_width)
                pY = vec_window_width;
            pY_overflow = __OFSUBS__(pY, pX);
            pY = pY - pX;
            if (((pY < 0) ^ pY_overflow) | (pY == 0))
                continue;
            o += pX;
            colL = vec_colour;
            pU = pp->S;
            colH = pp->S >> 16;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (;pY > 0; pY--, o++)
        {
            ushort colL, colH;
            TbBool pU_carry;

            pU_carry = __CFADDS__(tlr->var_60, pU);
            pU = tlr->var_60 + pU;
            colL = colS;
            colH = (tlr->var_60 >> 16) + pU_carry + (colS >> 8);
            *o = f[colS];

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md05(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *f;
    long lsh_var_54;
    long lsh_var_60;
    long lvr_var_54;

    m = vec_map;
    f = pixmap.fade_tables;
    pp = polyscans;

    {
        ulong factorA, factorB, factorC;
        factorC = tlr->var_48;
        // original code used unsigned compare here, making the condition always false
        //if (tlr->var_60 < 0) factorC--;
        factorC = __ROL4__(factorC, 16);
        factorA = __ROL4__(tlr->var_54, 16);
        factorB = ((ulong)tlr->var_60) >> 8;
        lsh_var_54 = (factorC & 0xFFFF0000) | (factorB & 0xFFFF);
        lsh_var_60 = (factorA & 0xFFFFFF00) | (factorC & 0xFF);
        lvr_var_54 = (factorA & 0xFF);
    }

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        long pX, pY;
        long rfactA, rfactB;
        ushort colM;
        unsigned char *o;
        unsigned char *o_ln;

        pX = pp->X >> 16;
        pY = pp->Y >> 16;
        o_ln = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;

        if (pX < 0)
        {
            ulong factorA, factorB;
            ushort colL, colH;
            long mX;

            if (pY <= 0)
                continue;
            mX = tlr->var_48 * (-pX);
            factorA = __ROL4__(pp->U + mX, 16);
            mX = tlr->var_54 * (-pX);
            factorB = __ROL4__(pp->V + mX, 16);
            mX = tlr->var_60 * (-pX);
            colL = (pp->S + mX) >> 8;
            colH = factorB;
            rfactB = (factorB & 0xFFFF0000) | (factorA & 0xFF);
            rfactA = (factorA & 0xFFFF0000) | (colL & 0xFFFF);
            if (pY > vec_window_width)
                pY = vec_window_width;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ulong factorA, factorB;
            ushort colL, colH;
            TbBool pY_overflow;

            if (pY > vec_window_width)
                pY = vec_window_width;
            pY_overflow = __OFSUBS__(pY, pX);
            pY = pY - pX;
            if (((pY < 0) ^ pY_overflow) | (pY == 0))
                continue;
            o_ln += pX;
            factorA = __ROL4__(pp->U, 16);
            factorB = __ROL4__(pp->V, 16);
            colL = pp->S >> 8;
            colH = factorB;
            // Should the high part really be preserved?
            rfactB = (factorB & 0xFFFF0000) | (factorA & 0xFF);
            rfactA = (factorA & 0xFFFF0000) | (colL & 0xFFFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        o = o_ln;

        for (; pY > 0; pY--, o++)
        {
            ushort colL, colH;
            ushort colS;
            TbBool rfactA_carry;
            TbBool rfactB_carry;

            colM = (colM & 0xFF00) + (rfactB & 0xFF);
            colS = (((rfactA >> 8) & 0xFF) << 8) + m[colM];

            rfactA_carry = __CFADDL__(rfactA, lsh_var_54);
            rfactA = rfactA + lsh_var_54;

            rfactB_carry = __CFADDL__(rfactB + rfactA_carry, lsh_var_60);
            rfactB = rfactB + lsh_var_60 + rfactA_carry;

            colH = lvr_var_54 + rfactB_carry + (colM >> 8);
            colL = colM;
            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);

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
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *f;
    long lsh_var_54;
    long lsh_var_60;

    m = vec_map;
    f = pixmap.fade_tables;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;
    lsh_var_60 = tlr->var_60 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        unsigned char *o;
        short pXa, pYa;
        long factorA;
        long pY;
        ulong factorB;
        ushort colM;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;

        if (pXa < 0)
        {
            ushort colL, colH;
            ushort pXMa;
            long pXMb;
            ulong mX;

            if (pYa <= 0)
                continue;
            pXMa = (ushort)-pXa;
            pXMb = pXMa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXMa, 16);
            colH = factorA;
            mX = pp->U + tlr->var_48 * pXMa;
            factorA = (factorA & 0xFFFF0000) | (mX & 0xFFFF);
            pXa = mX >> 8;
            colL = (pXa >> 8);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);

            factorB = __ROL4__(pp->S + tlr->var_60 * pXMb, 16);
            pXa = (pXa & 0xFFFF00FF) | ((factorB & 0xFF) << 8);
            factorB = (factorB & 0xFFFF0000) | (pYa & 0xFFFF);
            pXa = (pXa & 0xFFFF);
            pY = factorB & 0xFFFF;
            if (pY > vec_window_width)
                pY = vec_window_width;
        }
        else
        {
            ushort colL, colH;
            unsigned char pLa_overflow;
            short pLa;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pLa_overflow = __OFSUBS__(pYa, pXa);
            pLa = pYa - pXa;
            if (((pLa < 0) ^ pLa_overflow) | (pLa == 0))
                continue;

            o += pXa;
            colL = (pp->U >> 16);
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorB = __ROL4__(pp->S, 16);
            factorA = (factorA & 0xFFFF0000) | (pp->U & 0xFFFF);
            pXa = (pXa & 0xFFFF00FF) | ((factorB & 0xFF) << 8);
            factorB = (factorB & 0xFFFF0000) | (pLa & 0xFFFF);
            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
            pY = factorB & 0xFFFF;
        }

        for (; (pY & 0xFFFF) > 0; pY--, o++)
        {
            ushort colL, colH;
            unsigned char fct_carry;

            pXa = (pXa & 0xFF00) | (m[colM] & 0xFF);
            if (pXa & 0xFF)
                *o = f[pXa];

            fct_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) | ((tlr->var_48 + factorA) & 0xFFFF);
            colL = (tlr->var_48 >> 16) + fct_carry + colM;
            fct_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colH = (tlr->var_54 >> 16) + fct_carry + (colM >> 8);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);

            factorB = (factorB & 0xFFFF0000) | (pY & 0xFFFF);
            fct_carry = __CFADDL__(lsh_var_60, factorB);
            factorB += lsh_var_60;
            pXa = (((pXa >> 8) + (tlr->var_60 >> 16) + fct_carry) << 8) | (pXa & 0xFF);
            pY += lsh_var_60; // Very alarming. Bug, maybe?
        }
    }
}

void trig_render_md07(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *f;
    long lsh_var_54;

    m = vec_map;
    f = pixmap.fade_tables;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa;
        long pYa;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if ( (pXa & 0x8000u) != 0 )
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if ( (short)pYa <= 0 )
                continue;
            pXm = (ushort)-(short)pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorC = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) | (factorC & 0xFFFF);
            factorB = factorC >> 8;
            colL = ((factorB >> 8) & 0xFF);
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXa = (ushort)factorB;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( (unsigned char)(((pYa & 0x8000u) != 0) ^ pY_overflow) | ((ushort)pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) | (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = (vec_colour << 8) + m[colM];
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) | ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            *o = f[colS];
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md08(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *f;
    long lsh_var_54;

    m = vec_map;
    f = pixmap.fade_tables;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa;
        long pYa;
        ushort colM;
        unsigned char *o;
        long factorA;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if ( (pXa & 0x8000u) != 0 )
        {
            ushort colL, colH;
            ulong factorB, factorC;
            long pXm;

            if ( (short)pYa <= 0 )
                continue;
            pXm = (ushort)-(short)pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXa = (ushort)factorC;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( (unsigned char)(((pYa & 0x8000u) != 0) ^ pY_overflow) | ((ushort)pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = (vec_colour << 8) + m[colM];
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            if (colS & 0xFF)
                *o = f[colS];
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md09(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *f;
    long lsh_var_54;

    m = vec_map;
    f = pixmap.fade_tables;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if (pYa <= 0)
                continue;
            pXm = (ushort)-pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXa = (ushort)factorC;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if (((pYa < 0) ^ pY_overflow) | (pYa == 0))
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = m[colM] << 8;
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            if ((colS >> 8) & 0xFF) {
                colS = (colS & 0xFF00) | (*o);
                *o = f[colS];
            }
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md10(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *f;
    long lsh_var_54;

    m = vec_map;
    f = pixmap.fade_tables;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa;
        short pYa;
        ulong factorB;
        long factorA;
        ulong factorC;
        ushort colM;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            long pXm;

            if (pYa <= 0)
                continue;
            pXm = (ushort)-(short)pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXa = (ushort)factorC;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( (unsigned char)(((pYa & 0x8000u) != 0) ^ pY_overflow) | ((ushort)pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            if (m[colM]) {
                colS = (vec_colour << 8) | (*o);
                *o = f[colS];
            }
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md12(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    long lsh_var_54;

    m = vec_map;
    g = pixmap.ghost;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        long pXa;
        short pYa;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if ( (pXa & 0x8000u) != 0 )
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if ( (short)pYa <= 0 )
                continue;
            pXm = (ushort)-(short)pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorC = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorC & 0xFFFF);
            factorB = factorC >> 8;
            colL = ((factorB >> 8) & 0xFF);
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXa = (ushort)factorB;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( (unsigned char)(((pYa & 0x8000u) != 0) ^ pY_overflow) | ((ushort)pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = (m[colM] << 8) | vec_colour;
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            *o = g[colS];
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md13(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    long lsh_var_54;

    m = vec_map;
    g = pixmap.ghost;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if ( (short)pYa <= 0 )
                continue;
            pXm = (ushort)-(short)pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXa = (ushort)factorC;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( (unsigned char)(((pYa & 0x8000u) != 0) ^ pY_overflow) | ((ushort)pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = m[colM] | (vec_colour << 8);
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            *o = g[colS];
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md14(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *g;
    ushort colM;
    unsigned char *o_ln;

    g = pixmap.ghost;
    pp = polyscans;
    o_ln = tlr->var_24;
    colM = (vec_colour << 8);

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o_ln += vec_screen_width;

        if (pXa < 0)
        {
            if (pYa <= 0)
                continue;
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            o = o_ln;
        }
        else
        {
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( ((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o = &o_ln[pXa];
        }

        for (; pYa > 0; pYa--, o++)
        {
              colM = (colM & 0xFF00) | *o;
              *o = g[colM];
        }
    }
}

void trig_render_md15(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *g;
    ushort colM;
    unsigned char *o_ln;

    g = pixmap.ghost;
    pp = polyscans;
    o_ln = tlr->var_24;
    colM = vec_colour;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o_ln += vec_screen_width;
        if (pXa < 0)
        {
            if (pYa <= 0)
                continue;
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            o = o_ln;
        }
        else
        {
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( ((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o = &o_ln[pXa];
        }

        for (; pYa > 0; pYa--, o++)
        {
              colM = (*o << 8) | (colM & 0xFF);
              *o = g[colM];
        }
    }
}

void trig_render_md16(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *g;
    unsigned char *f;

    g = pixmap.ghost;
    f = pixmap.fade_tables;
    pp = polyscans;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        short factorA;
        ushort colM;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;

        if (pXa < 0)
        {
            ushort colL, colH;
            unsigned char factorA_carry;
            ulong pXMa;
            short pXMb;

            if (pYa <= 0)
                continue;
            pXMa = tlr->var_60 * (ushort)-pXa;
            pXMb = pXMa;
            pXa = pXMa >> 8;
            factorA_carry = __CFADDS__(pp->S, pXMb);
            factorA = (pp->S) + pXMb;
            colH = (pXa >> 8) + (pp->S >> 16) + factorA_carry;
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            colL = vec_colour;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( ((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o += pXa;
            colL = vec_colour;
            factorA = pp->S;
            colH = (pp->S >> 16);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colS = (f[colM] << 8) | *o;
            *o = g[colS];
            factorA_carry = __CFADDS__(tlr->var_60, factorA);
            factorA += (tlr->var_60 & 0xFFFF);
            colH = (colM >> 8) + (tlr->var_60 >> 16) + factorA_carry;
            colL = colM;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md17(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *g;
    unsigned char *f;

    g = pixmap.ghost;
    f = pixmap.fade_tables;
    pp = polyscans;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        unsigned char factorA_carry;
        short factorA;
        ushort colS;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;

        if (pXa < 0)
        {
            ushort colL, colH;
            ulong pXMa;
            short pXMb;

            if (pYa <= 0)
                continue;
            pXMa = tlr->var_60 * (ushort)-pXa;
            pXMb = pXMa;
            pXa = pXMa >> 8;
            factorA_carry = __CFADDS__(pp->S, pXMb);
            factorA = pp->S + pXMb;
            colH = (pXa >> 8) + (pp->S >> 16) + factorA_carry;
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            colL = vec_colour;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if (((pYa < 0) ^ pY_overflow) | (pYa == 0))
                continue;

            o += pXa;
            colL = vec_colour;
            factorA = pp->S;
            colH = (pp->S >> 16);

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colM;

            colM = ((*o) << 8) + f[colS];
            *o = g[colM];

            factorA_carry = __CFADDS__(tlr->var_60, factorA);
            factorA += (tlr->var_60 & 0xFFFF);
            colH = (colS >> 8) + ((tlr->var_60 >> 16) & 0xFF) + factorA_carry;
            colL = colS;

            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md18(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    long lsh_var_54;

    m = vec_map;
    g = pixmap.ghost;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        long pXm;
        long factorA;
        ushort colM;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if (pYa <= 0)
                continue;
            pXm = (ushort)-pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = (factorC >> 8);
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXa = (ushort)factorC;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_carry;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_carry = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( ((pYa < 0) ^ pY_carry) | (pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = (pp->U >> 16);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            colH = m[colM];
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) | ((tlr->var_48 + factorA) & 0xFFFF);
            colL = *o;
            colS = ((colH & 0xFF) << 8) + (colL & 0xFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            *o = g[colS];
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md19(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    long lsh_var_54;

    m = vec_map;
    g = pixmap.ghost;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        long factorA;
        ushort colM;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            long pXm;
            ulong factorB, factorC;

            if (pYa <= 0)
                continue;
            pXm = (ushort)-pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (pYa > vec_window_width)
              pYa = vec_window_width;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( ((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colS = ((*o) << 8) + m[colM];
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            *o = g[colS];
            colH = (colM >> 8) + (tlr->var_54 >> 16) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md20(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long lsh_var_54;
    long lsh_var_60;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;
    lsh_var_60 = tlr->var_60 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        long pXMa;
        long pXMb;
        long factorA;
        long factorC;
        ushort colM;
        unsigned char *o;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            ulong factorB;

            if (pYa <= 0)
                continue;
            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pXMa = (ushort)-pXa;
            pXMb = pXMa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXMa, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXMa;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            pXa = factorB >> 8;
            colL = ((pXa >> 8) & 0xFF);
            factorC = __ROL4__(pp->S + tlr->var_60 * pXMb, 16);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( ((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);
            factorC = __ROL4__(pp->S, 16);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colS = ((factorC & 0xFF) << 8) + m[colM];
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colS = ((f[colS] & 0xFF) << 8) + *o;
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;
            factorA_carry = __CFADDL__(lsh_var_60, factorC);
            factorC += lsh_var_60;
            *o = g[colS];
            factorC = (factorC & 0xFFFFFF00) | (((tlr->var_60 >> 16) + factorA_carry + factorC) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md21(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long lsh_var_54;
    long lsh_var_60;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;
    lsh_var_60 = tlr->var_60 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa, pYa;
        ushort colM;
        unsigned char *o;
        long factorA, factorC;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            long pXMa;
            long pXMb;
            ulong factorB;

            if (pYa <= 0)
                continue;
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXMa = (ushort)-pXa;
            pXMb = pXMa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXMa, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXMa;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            pXa = factorB >> 8;
            colL = ((pXa >> 8) & 0xFF);
            factorC = __ROL4__(pp->S + tlr->var_60 * pXMb, 16);
            pXa = (pXa & 0xFFFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( ((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);
            factorC = __ROL4__(pp->S, 16);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            colS = ((factorC & 0xFF) << 8) + (m[colM] & 0xFF);
            colS = (((*o) & 0xFF) << 8) + (f[colS] & 0xFF);
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;
            factorA_carry = __CFADDL__(lsh_var_60, factorC);
            factorC += lsh_var_60;
            *o = g[colS];
            factorC = (factorC & 0xFFFFFF00) | (((tlr->var_60 >> 16) + factorA_carry + factorC) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md22(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    long lsh_var_54;

    m = vec_map;
    g = pixmap.ghost;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa;
        ushort colM;
        short pYa;
        unsigned char *o;
        long pXm;
        long factorA;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if (pYa <= 0)
                continue;
            pXm = (ushort)-pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXa = factorC & 0xFFFF;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if ( ((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;
            unsigned char factorA_carry;

            if (m[colM]) {
                colS = ((m[colM] & 0xFF) << 8) + *o;
                *o = g[colS];
            }
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md23(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    long lsh_var_54;

    m = vec_map;
    g = pixmap.ghost;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa;
        ushort colM;
        short pYa;
        unsigned char *o;
        long pXm;
        long factorA;
        unsigned char factorA_carry;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if ( (pXa & 0x8000u) != 0 )
        {
            ushort colL, colH;
            ulong factorB, factorC;

            if (pYa <= 0)
                continue;
            pXm = (ushort)-pXa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXm, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXm;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            factorC = factorB >> 8;
            colL = ((factorC >> 8) & 0xFF);
            if (pYa > vec_window_width)
              pYa = vec_window_width;
            pXa = (ushort)factorC;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if (((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            ushort colS;

            if (m[colM]) {
                colS = (((*o) & 0xFF) << 8) + m[colM];
                *o = g[colS];
            }
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md24(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long lsh_var_54;
    long lsh_var_60;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;
    lsh_var_60 = tlr->var_60 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa;
        ushort colM;
        short pYa;
        unsigned char *o;
        long pXMa;
        long pXMb;
        long factorA;
        long factorC;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            ulong factorB;

            if (pYa <= 0)
                continue;
            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pXMa = (ushort)-pXa;
            pXMb = pXMa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXMa, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXMa;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            pXa = factorB >> 8;
            colL = ((pXa >> 8) & 0xFF);
            factorC = __ROL4__(pp->S + tlr->var_60 * pXMb, 16);
            pXa = (ushort)pXa;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if (((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);
            factorC = __ROL4__(pp->S, 16);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            unsigned char factorA_carry;

            if (m[colM]) {
                ushort colS;

                colS = ((factorC & 0xFF) << 8) + m[colM];
                colS = (f[colS] << 8) + *o;
                *o = g[colS];
            }
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;
            factorA_carry = __CFADDL__(lsh_var_60, factorC);
            factorC += lsh_var_60;
            factorC = (factorC & 0xFFFFFF00) + (((tlr->var_60 >> 16) + factorA_carry + factorC) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md25(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long lsh_var_54;
    long lsh_var_60;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    pp = polyscans;
    lsh_var_54 = tlr->var_54 << 16;
    lsh_var_60 = tlr->var_60 << 16;

    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        short pXa;
        ushort colM;
        short pYa;
        unsigned char *o;
        long pXMa;
        long pXMb;
        long factorA;
        long factorC;

        pXa = (pp->X >> 16);
        pYa = (pp->Y >> 16);
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;
        if (pXa < 0)
        {
            ushort colL, colH;
            ulong factorB;

            if (pYa <= 0)
                continue;
            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pXMa = (ushort)-pXa;
            pXMb = pXMa;
            factorA = __ROL4__(pp->V + tlr->var_54 * pXMa, 16);
            colH = factorA;
            factorB = pp->U + tlr->var_48 * pXMa;
            factorA = (factorA & 0xFFFF0000) + (factorB & 0xFFFF);
            pXa = factorB >> 8;
            colL = ((pXa >> 8) & 0xFF);
            factorC = __ROL4__(pp->S + tlr->var_60 * pXMb, 16);
            pXa = (ushort)pXa;

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
        else
        {
            ushort colL, colH;
            unsigned char pY_overflow;

            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa = pYa - pXa;
            if (((pYa < 0) ^ pY_overflow) | (pYa == 0) )
                continue;
            o += pXa;
            factorA = __ROL4__(pp->V, 16);
            colH = factorA;
            factorA = (factorA & 0xFFFF0000) + (pp->U & 0xFFFF);
            colL = ((pp->U >> 16) & 0xFF);
            factorC = __ROL4__(pp->S, 16);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }

        for (; pYa > 0; pYa--, o++)
        {
            ushort colL, colH;
            unsigned char factorA_carry;

            if (m[colM]) {
                ushort colS;

                colS = ((factorC & 0xFF) << 8) + m[colM];
                colS = (((*o) & 0xFF) << 8) + f[colS];
                *o = g[colS];
            }
            factorA_carry = __CFADDS__(tlr->var_48, factorA);
            factorA = (factorA & 0xFFFF0000) + ((tlr->var_48 + factorA) & 0xFFFF);
            colL = ((tlr->var_48 >> 16) & 0xFF) + factorA_carry + colM;
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA += lsh_var_54;
            colH = (colM >> 8) + ((tlr->var_54 >> 16) & 0xFF) + factorA_carry;
            factorA_carry = __CFADDL__(lsh_var_60, factorC);
            factorC += lsh_var_60;
            factorC = (factorC & 0xFFFFFF00) | (((tlr->var_60 >> 16) + factorA_carry + factorC) & 0xFF);

            colM = ((colH & 0xFF) << 8) + (colL & 0xFF);
        }
    }
}

void trig_render_md26(struct TrigLocalRend *tlr)
{
    struct PolyPoint *pp;
    unsigned char *m;
    unsigned char *g;
    unsigned char *f;
    long lsh_var_54;
    long lsh_var_60;
    long lvr_var_54;

    m = vec_map;
    g = pixmap.ghost;
    f = pixmap.fade_tables;
    pp = polyscans;

    {
        ulong v1;
        ulong v2;
        unsigned char v3;

        v1 = __ROL4__(tlr->var_48, 16);
        v2 = __ROL4__(tlr->var_54, 16);
        v3 = v2;
        v2 = (v2 & 0xFFFFFF00) + (v1 & 0xFF);
        v1 = (v1 & 0xFFFF0000) + (((ulong)tlr->var_60 >> 8) & 0xFFFF);
        v2 = (v2 & 0xFFFF0000) + (v2 & 0xFF);
        lsh_var_54 = v1;
        lsh_var_60 = v2;
        lvr_var_54 = v3;
    }
    for (; tlr->var_44; tlr->var_44--, pp++)
    {
        long pXa;
        long pYa;
        unsigned char *o;
        ulong factorB, factorD;
        long factorA;
        ulong factorC;
        unsigned char pY_overflow;
        ushort colM;

        pXa = pp->X >> 16;
        pYa = pp->Y >> 16;
        o = &tlr->var_24[vec_screen_width];
        tlr->var_24 += vec_screen_width;

        if (pXa < 0)
        {
            if (pYa <= 0)
                continue;
            pXa = -pXa;
            factorA = __ROL4__(pp->U + pXa * tlr->var_48, 16);
            factorB = __ROL4__(pp->V + pXa * tlr->var_54, 16);
            factorC = (ulong)(pp->S + pXa * tlr->var_60) >> 8;
            factorB = (factorB & 0xFFFFFF00) | (factorA & 0xFF);
            factorA = (factorA & 0xFFFF0000) | (factorC & 0xFFFF);
            factorD = __ROL4__(pp->V + pXa * tlr->var_54, 16);
            if (pYa > vec_window_width)
                pYa = vec_window_width;

            colM = (factorC & 0xFF) + ((factorD & 0xFF) << 8);
        }
        else
        {
            if (pYa > vec_window_width)
                pYa = vec_window_width;
            pY_overflow = __OFSUBS__(pYa, pXa);
            pYa -= pXa;
            if (((pYa < 0) ^ pY_overflow) | (pYa == 0))
                continue;
            o += pXa;
            factorA = __ROL4__(pp->U, 16);
            factorB = __ROL4__(pp->V, 16);
            factorC = (ulong)pp->S >> 8;
            factorB = (factorB & 0xFFFFFF00) | (factorA & 0xFF);
            factorA = (factorA & 0xFFFF0000) | (factorC & 0xFFFF);
            factorD = __ROL4__(pp->V, 16);

            colM = (factorC & 0xFF) + ((factorD & 0xFF) << 8);
        }

        factorB = (factorB & 0xFFFF00FF);

        for (; pYa > 0; pYa--, o++)
        {
            ushort colS;
            unsigned char factorA_carry, factorB_carry;

            colM = (colM & 0xFF00) | (factorB & 0xFF);
            colS = (factorA & 0xFF00) | m[colM];
            factorA_carry = __CFADDL__(lsh_var_54, factorA);
            factorA = lsh_var_54 + factorA;
            factorB_carry = __CFADDL__(lsh_var_60, factorB + factorA_carry);
            factorB = lsh_var_60 + factorB + factorA_carry;
            colM = (colM & 0xFF) + ((((colM >> 8) + lvr_var_54 + factorB_carry) & 0xFF) << 8);

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
