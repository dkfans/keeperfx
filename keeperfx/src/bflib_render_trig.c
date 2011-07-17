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
#include "bflib_render.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"

/******************************************************************************/
DLLIMPORT void _DK_trig(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c);
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
};

struct TrigLocals {
    unsigned long zero0;// dummy, to make no offset 0
    unsigned char var_24;// 4+
    unsigned char var_25; // 5+
    unsigned short flag_26; // 6+
    unsigned long var_28; // 8+
    unsigned long var_2C; // unused
    // These are DWORDs
    unsigned long var_30; // -0x30
    unsigned long var_34; // -0x34
    unsigned long var_38; // -0x38
    unsigned long var_3C; // -0x3C
    unsigned long var_40; // -0x40
    unsigned long var_44; // -0x44
    unsigned long var_48; // -0x48
    unsigned long delta_e; // -0x4C
    union {
    unsigned long var_50; // -0x50
    struct {
        unsigned short var_50a;
        unsigned short var_50b;
    };
    };
    unsigned long var_54; // -0x54
    unsigned long delta_d; // -0x58
    unsigned long var_5C; // -0x5C
    unsigned long var_60; // -0x60
    unsigned long delta_c; // -0x64
    unsigned long var_68; // -0x68
    unsigned long var_6C; // -0x6C
    unsigned long var_70; // -0x70
    unsigned long var_74; // -0x74
    unsigned long var_78; // -0x78
    unsigned long var_7C; // -0x7C
    unsigned long var_80; // -0x80
    unsigned long delta_b; // -0x84
    unsigned long delta_a; // -0x88
    unsigned long var_8C; // -0x8C
};

#pragma pack()
/******************************************************************************/

int trig_reorder_input_points(struct PolyPoint **opt_a, struct PolyPoint **opt_b, struct PolyPoint **opt_c)
{
    struct PolyPoint *ordpt_a = *opt_a;
    struct PolyPoint *ordpt_b = *opt_b;
    struct PolyPoint *ordpt_c = *opt_c;
    long start_type;

    struct PolyPoint *ordpt_tmp1;
    struct PolyPoint *ordpt_tmp2;
    start_type = RendStart_NO;
    if (ordpt_a->field_4 == ordpt_b->field_4)
    {
        if (ordpt_a->field_4 == ordpt_c->field_4)
            return RendStart_NO;
        if (ordpt_a->field_4 >= ordpt_c->field_4)
        {
            if (ordpt_a->field_0 <= ordpt_b->field_0)
                return RendStart_NO;
            ordpt_tmp1 = ordpt_a;
            ordpt_a = ordpt_c;
            ordpt_tmp2 = ordpt_b;
            ordpt_b = ordpt_tmp1;
            ordpt_c = ordpt_tmp2;
            start_type = RendStart_FB;
        } else
        {
            if (ordpt_b->field_0 <= ordpt_a->field_0)
                return RendStart_NO;
            start_type = RendStart_FT;
        }
    } else
    if (ordpt_a->field_4 <= ordpt_b->field_4)
    {
        if (ordpt_a->field_4 == ordpt_c->field_4)
        {
            if (ordpt_a->field_0 <= ordpt_c->field_0)
                return RendStart_NO;
            ordpt_tmp1 = ordpt_a;
            ordpt_a = ordpt_c;
            ordpt_tmp2 = ordpt_b;
            ordpt_b = ordpt_tmp1;
            ordpt_c = ordpt_tmp2;
            start_type = RendStart_FT;
        } else
        if (ordpt_a->field_4 >= ordpt_c->field_4)
        {
            ordpt_tmp1 = ordpt_a;
            ordpt_a = ordpt_c;
            ordpt_tmp2 = ordpt_b;
            ordpt_b = ordpt_tmp1;
            ordpt_c = ordpt_tmp2;
            start_type = RendStart_LL;
        } else
        {
            if (ordpt_b->field_4 == ordpt_c->field_4)
            {
                if (ordpt_b->field_0 <= ordpt_c->field_0)
                    return RendStart_NO;
                start_type = RendStart_FB;
            } else
            if (ordpt_b->field_4 <= ordpt_c->field_4)
            {
                start_type = RendStart_LL;
            } else
            {
                start_type = RendStart_RL;
            }
        }
    } else
    {
        if (ordpt_a->field_4 == ordpt_c->field_4)
        {
            if (ordpt_a->field_0 >= ordpt_c->field_0)
                return RendStart_NO;
            ordpt_tmp1 = ordpt_a;
            ordpt_a = ordpt_b;
            ordpt_b = ordpt_c;
            ordpt_c = ordpt_tmp1;
              start_type = RendStart_FB;
        } else
        if (ordpt_a->field_4 < ordpt_c->field_4)
        {
            ordpt_tmp1 = ordpt_a;
            ordpt_a = ordpt_b;
            ordpt_b = ordpt_c;
            ordpt_c = ordpt_tmp1;
              start_type = RendStart_RL;
        } else
        {
            if (ordpt_b->field_4 == ordpt_c->field_4)
            {
                if (ordpt_b->field_0 >= ordpt_c->field_0)
                    return RendStart_NO;
                ordpt_tmp1 = ordpt_a;
                ordpt_a = ordpt_b;
                ordpt_b = ordpt_c;
                ordpt_c = ordpt_tmp1;
                start_type = RendStart_FT;
            } else
            if (ordpt_b->field_4 < ordpt_c->field_4)
            {
                ordpt_tmp1 = ordpt_a;
                ordpt_a = ordpt_b;
                ordpt_b = ordpt_c;
                ordpt_c = ordpt_tmp1;
                start_type = RendStart_LL;
            } else
            {
                ordpt_tmp1 = ordpt_a;
                ordpt_a = ordpt_c;
                ordpt_tmp2 = ordpt_b;
                ordpt_b = ordpt_tmp1;
                ordpt_c = ordpt_tmp2;
                start_type = RendStart_RL;
            }
        }
    }

/*
    asm volatile (" \
    pushl   %%ebp\n \
# ==================== PRELIMINARY TESTS - REORDER POINTS ===================\n \
    movl    4(%%esi),%%eax # PolyPoint.field_4\n \
    movl    4(%%edi),%%ebx\n \
    movl    4(%%ecx),%%edx\n \
    #C: if (ordpt_a->field_4 == ordpt_b->field_4)\n \
    cmpl    %%ebx,%%eax\n \
    jz      loc_pre04\n \
    #C: if (ordpt_a->field_4 <= ordpt_b->field_4)\n \
    jg      loc_pre01\n \
    cmpl    %%edx,%%eax\n \
    #C: if (ordpt_a->field_4 == ordpt_c->field_4)\n \
    jz      loc_pre03\n \
    #C: if (ordpt_a->field_4 >= ordpt_c->field_4)\n \
    jl      loc_pre07\n \
    xchgl   %%ecx,%%esi\n \
    xchgl   %%ecx,%%edi\n \
    movl    $1,%%eax\n \
    jmp     do_start\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_pre01:            # 17\n \
    #C: if (ordpt_a->field_4 == ordpt_c->field_4)\n \
    cmpl    %%edx,%%eax\n \
    jz      loc_pre10\n \
    #C: if (ordpt_a->field_4 < ordpt_c->field_4)\n \
    jl      loc_pre09\n \
    #C: if (ordpt_b->field_4 == ordpt_c->field_4)\n \
    cmpl    %%edx,%%ebx\n \
    jz      loc_pre08\n \
    jl      loc_pre02\n \
    xchgl   %%ecx,%%esi\n \
    xchgl   %%ecx,%%edi\n \
    movl    $2,%%eax\n \
    jmp     do_start\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_pre02:            # 3B\n \
    xchgl   %%edi,%%esi\n \
    xchgl   %%ecx,%%edi\n \
    movl    $1,%%eax\n \
    jmp     do_start\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_pre03:            # 1B\n \
    #C: if (ordpt_a->field_0 <= ordpt_c->field_0)\n \
    movl    (%%esi),%%eax\n \
    cmpl    (%%ecx),%%eax\n \
    jle     pre_finished\n \
    xchgl   %%ecx,%%esi\n \
    xchgl   %%ecx,%%edi\n \
    movl    $4,%%eax\n \
    jmp     do_start\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_pre04:            # 15\n \
    #C: if (ordpt_a->field_4 == ordpt_c->field_4)\n \
    cmpl    %%edx,%%eax\n \
    jz      pre_finished\n \
    #C: if (ordpt_a->field_4 >= ordpt_c->field_4)\n \
    jl      loc_pre05\n \
    #C: if (ordpt_a->field_0 <= ordpt_b->field_0)\n \
    movl    (%%esi),%%eax\n \
    cmpl    (%%edi),%%eax\n \
    jle     pre_finished\n \
    xchgl   %%ecx,%%esi\n \
    xchgl   %%ecx,%%edi\n \
    movl    $3,%%eax\n \
    jmp     do_start\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_pre05:            # 67\n \
    movl    (%%edi),%%eax\n \
    #C: if (ordpt_b->field_0 <= ordpt_a->field_0)\n \
    cmpl    (%%esi),%%eax\n \
    jle     pre_finished\n \
    movl    $4,%%eax\n \
    jmp     do_start\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_pre06:            # 9C\n \
    #C: if (ordpt_b->field_0 <= ordpt_a->field_0)\n \
    movl    (%%edi),%%eax\n \
    cmpl    (%%ecx),%%eax\n \
    jle     pre_finished\n \
    movl    $3,%%eax\n \
    jmp     do_start\n \
\n \
loc_pre07:            # 1D\n \
    #C: if (ordpt_b->field_4 == ordpt_c->field_4)\n \
    cmpl    %%edx,%%ebx\n \
    jz      loc_pre06\n \
    #C: if (ordpt_b->field_4 <= ordpt_c->field_4)\n \
    jg      loc_pre11\n \
    movl    $1,%%eax\n \
    jmp     do_start\n \
\n \
loc_pre08:            # 35\n \
    movl    (%%ecx),%%eax\n \
    cmpl    (%%edi),%%eax\n \
    jle     pre_finished\n \
    xchgl    %%edi,%%esi\n \
    xchgl    %%ecx,%%edi\n \
    movl    $4,%%eax\n \
    jmp     do_start\n \
\n \
loc_pre09:            # 2D\n \
    xchgl    %%edi,%%esi\n \
    xchgl    %%ecx,%%edi\n \
loc_pre11:\n \
    movl    $2,%%eax\n \
    jmp      do_start\n \
\n \
loc_pre10:            # 27\n \
    movl    (%%ecx),%%eax\n \
    cmpl    (%%esi),%%eax\n \
    jle     pre_finished\n \
    xchgl    %%edi,%%esi\n \
    xchgl    %%ecx,%%edi\n \
    movl    $3,%%eax\n \
    jmp    do_start\n \
pre_finished:\n \
    jmp      skip_start\n \
\
do_start:\n \
    jmp    phase0_finished\n \
skip_start:\n \
    movl    $0,%%eax\n \
phase0_finished:\n \
    popl    %%ebp\n \
"
         : "=S" (ordpt_a), "=D" (ordpt_b), "=c" (ordpt_c), "=a" (start_type)
         : "0" (ordpt_a), "1" (ordpt_b), "2" (ordpt_c)
         : "memory", "cc", "%ebx", "%edx");
*/
    *opt_a = ordpt_a;
    *opt_b = ordpt_b;
    *opt_c = ordpt_c;
    return start_type;
}

int trig_ll_start(struct TrigLocals *lv, const struct PolyPoint *opt_a, const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    struct TrigLocals llv;
    long do_render;
    long dummy;
    asm volatile (" \
        movl    4(%%esi),%%eax # PolyPoint.field_4\n \
        movl    %%eax,0x18+%4\n \
        orl     %%eax,%%eax\n \
        jns     ll_loc02\n \
        movl    _LOC_poly_screen,%%ebx\n \
        movl    %%ebx,0x6C+%4\n \
        movb    $1,6+%4\n \
        jmp     ll_loc01\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc02:            # AD\n \
        cmpl    _LOC_vec_window_height,%%eax\n \
        jge     ll_skipped\n \
        movl    %%eax,%%ebx\n \
        imull   _LOC_vec_screen_width,%%ebx\n \
        addl    _LOC_poly_screen,%%ebx\n \
        movl    %%ebx,0x6C+%4\n \
        movb    $0,6+%4\n \
    \n \
    ll_loc01:            # BD\n \
        movl    4(%%ecx),%%ebx\n \
        cmpl    _LOC_vec_window_height,%%ebx\n \
        setnle    4+%4\n \
        subl    %%eax,%%ebx\n \
        movl    %%ebx,0x5C+%4\n \
        movl    %%ebx,0x4C+%4\n \
        movl    4(%%edi),%%ebx\n \
        cmpl    _LOC_vec_window_height,%%ebx\n \
        setnle    5+%4\n \
        subl    %%eax,%%ebx\n \
        movl    %%ebx,0x58+%4\n \
        movl    (%%ecx),%%eax\n \
        subl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x68+%4\n \
        movl    (%%edi),%%eax\n \
        subl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x58+%4\n \
        cmpl    0x68+%4,%%eax\n \
        jle     ll_skipped\n \
        movl    %%eax,0x64+%4\n \
        movl    4(%%ecx),%%ebx\n \
        subl    4(%%edi),%%ebx\n \
        movl    (%%ecx),%%eax\n \
        subl    (%%edi),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x60+%4\n \
        movl    %%ebx,0x54+%4\n \
        movl    (%%edi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,0x50+%4\n \
        movzbl  _vec_mode,%%eax\n \
        jmpl    *ll_jt(,%%eax,4)\n \
    # ---------------------------------------------------------------------------\n \
    ll_jt:\n \
        .int    ll_md00\n \
        .int    ll_md01\n \
        .int    ll_md02\n \
        .int    ll_md02\n \
        .int    ll_md01\n \
        .int    ll_md05\n \
        .int    ll_md05\n \
        .int    ll_md02\n \
        .int    ll_md02\n \
        .int    ll_md02\n \
        .int    ll_md02\n \
        .int    ll_md02\n \
        .int    ll_md02\n \
        .int    ll_md02\n \
        .int    ll_md00\n \
        .int    ll_md00\n \
        .int    ll_md01\n \
        .int    ll_md01\n \
        .int    ll_md02\n \
        .int    ll_md02\n \
        .int    ll_md05\n \
        .int    ll_md05\n \
        .int    ll_md02\n \
        .int    ll_md02\n \
        .int    ll_md05\n \
        .int    ll_md05\n \
        .int    ll_md05\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_md05:            # DATA XREF: trig_+17B trig_+17F ...\n \
        movl    0x58+%4,%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x10+%4\n \
        movl    (%%esi),%%eax\n \
        subl    (%%ecx),%%eax\n \
        imull   0x10+%4\n \
        sarl    $0x10,%%eax\n \
        movl    (%%edi),%%ebx\n \
        subl    (%%esi),%%ebx\n \
        addl    %%eax,%%ebx\n \
        jl     ll_skipped\n \
        jz     ll_loc03\n \
        incl    %%ebx\n \
        movl    8(%%esi),%%eax\n \
        subl    8(%%ecx),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    8(%%edi),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x48+%4\n \
        movl    0x0C(%%esi),%%eax\n \
        subl    0x0C(%%ecx),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x3C+%4\n \
        movl    0x10(%%esi),%%eax\n \
        subl    0x10(%%ecx),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    0x10(%%edi),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x30+%4\n \
    \n \
    ll_loc03:            # 1FA\n \
        movl    8(%%ecx),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x44+%4\n \
        movl    0x0C(%%ecx),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x38+%4\n \
        movl    0x10(%%ecx),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x2C+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        movl    8(%%esi),%%ecx\n \
        movl    0x0C(%%esi),%%edx\n \
        movl    0x10(%%esi),%%esi\n \
        cmpb   $0,6+%4\n \
        jz     ll_loc23\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x4C+%4\n \
        jle     ll_skipped\n \
        movl    %%edi,0x24+%4\n \
        cmpl    0x58+%4,%%edi\n \
        js     ll_loc05\n \
        movl    0x68+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x44+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        movl    0x50+%4,%%ebx\n \
        movl    0x24+%4,%%edi\n \
        subl    0x58+%4,%%edi\n \
        subl    %%edi,0x54+%4\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x24+%4,%%edi\n \
        imull    0x60+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc04\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x54+%4\n \
        movl    %%edi,0x4C+%4\n \
    \n \
    ll_loc04:            # 32C\n \
        leal    _polyscans,%%edi\n \
        jmp     ll_loc10\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc05:            # 2AE\n \
        movl    0x24+%4,%%edi\n \
        subl    %%edi,0x58+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc08\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc06\n \
        movl    %%edi,0x58+%4\n \
        jmp     ll_loc07\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc06:            # 398\n \
        subl    0x58+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    ll_loc07:            # 39E\n \
        jmp     ll_loc08\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc23:            # 290\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc08\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc24\n \
        movl    %%edi,0x58+%4\n \
        jmp     ll_loc08\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc24:            # 3C9\n \
        subl    0x58+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    ll_loc08:\n \
        leal    _polyscans,%%edi\n \
    \n \
    ll_loc09:            # 40D\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl    0x58+%4\n \
        jnz     ll_loc09\n \
        movl    0x50+%4,%%ebx\n \
    \n \
    ll_loc10:            # 342\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc11\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc11:            # 418 trig_+451\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x60+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl    0x54+%4\n \
        jnz     ll_loc11\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_md02:\n \
        movl    0x58+%4,%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x10+%4\n \
        movl    (%%esi),%%eax\n \
        subl    (%%ecx),%%eax\n \
        imull   0x10+%4\n \
        sarl    $0x10,%%eax\n \
        movl    (%%edi),%%ebx\n \
        subl    (%%esi),%%ebx\n \
        addl    %%eax,%%ebx\n \
        jl     ll_skipped\n \
        jz     ll_loc12\n \
        incl    %%ebx\n \
        movl    8(%%esi),%%eax\n \
        subl    8(%%ecx),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    8(%%edi),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x48+%4\n \
        movl    0x0C(%%esi),%%eax\n \
        subl    0x0C(%%ecx),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x3C+%4\n \
    \n \
    ll_loc12:            # 488\n \
        movl    8(%%ecx),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x44+%4\n \
        movl    0x0C(%%ecx),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x38+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        movl    8(%%esi),%%ecx\n \
        movl    0x0C(%%esi),%%edx\n \
        cmpb   $0,6+%4\n \
        jz     ll_loc17\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x4C+%4\n \
        jle     ll_skipped\n \
        movl    %%edi,0x24+%4\n \
        cmpl    0x58+%4,%%edi\n \
        js     ll_loc14\n \
        movl    0x68+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x44+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x50+%4,%%ebx\n \
        movl    0x24+%4,%%edi\n \
        subl    0x58+%4,%%edi\n \
        subl    %%edi,0x54+%4\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x24+%4,%%edi\n \
        imull    0x60+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc13\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x54+%4\n \
        movl    %%edi,0x4C+%4\n \
    \n \
    ll_loc13:            # 573\n \
        leal    _polyscans,%%edi\n \
        jmp     ll_loc21\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc14:            # 50F\n \
        movl    0x24+%4,%%edi\n \
        subl    %%edi,0x58+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc19\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc15\n \
        movl    %%edi,0x58+%4\n \
        jmp     ll_loc16\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc15:\n \
        subl    0x58+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    ll_loc16:\n \
        jmp     ll_loc19\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc17:\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc19\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc18\n \
        movl    %%edi,0x58+%4\n \
        jmp     ll_loc19\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc18:\n \
        subl    0x58+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    ll_loc19:\n \
        leal    _polyscans,%%edi\n \
        # restrict 0x58+%4 to 576 - size of polyscans[]\n \
        cmpl   $0x240,0x58+%4\n \
        jl     ll_loc20_test2\n \
        movl   $0x240,0x58+%4\n \
    ll_loc20_test2:\n \
        # restrict 0x54+%4 to 576 minus the value of previous var\n \
        cmpl   $0x240,0x54+%4\n \
        jl     ll_loc20\n \
        movl   $0x240,0x54+%4\n \
    \n \
    ll_loc20:\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        addl    $0x14,%%edi\n \
        decl    0x58+%4\n \
        jnz     ll_loc20\n \
        movl    0x50+%4,%%ebx\n \
    \n \
    ll_loc21:\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc22\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc22:\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x60+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        addl    $0x14,%%edi\n \
        decl    0x54+%4\n \
        jnz     ll_loc22\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_md01:\n \
        movl    0x58+%4,%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x10+%4\n \
        movl    (%%esi),%%eax\n \
        subl    (%%ecx),%%eax\n \
        imull   0x10+%4\n \
        sarl    $0x10,%%eax\n \
        movl    (%%edi),%%ebx\n \
        subl    (%%esi),%%ebx\n \
        addl    %%eax,%%ebx\n \
        jl     ll_skipped\n \
        jz     ll_loc25\n \
        incl    %%ebx\n \
        movl    0x10(%%esi),%%eax\n \
        subl    0x10(%%ecx),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    0x10(%%edi),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x30+%4\n \
    \n \
    ll_loc25:\n \
        movl    0x10(%%ecx),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x2C+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        movl    0x10(%%esi),%%esi\n \
        cmpb   $0,6+%4\n \
        jz     ll_loc26\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x4C+%4\n \
        jle     ll_skipped\n \
        movl    %%edi,0x24+%4\n \
        cmpl    0x58+%4,%%edi\n \
        js     ll_loc36\n \
        movl    0x68+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        movl    0x50+%4,%%ebx\n \
        movl    0x24+%4,%%edi\n \
        subl    0x58+%4,%%edi\n \
        subl    %%edi,0x54+%4\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x24+%4,%%edi\n \
        imull    0x60+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x2C+%4,%%edi\n \
        imull   0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb    $0,4+%4\n \
        jz      ll_loc37\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x54+%4\n \
        movl    %%edi,0x4C+%4\n \
    \n \
    ll_loc37:\n \
        leal    _polyscans,%%edi\n \
        cmpb    $0,5+%4\n \
        jz      ll_loc30\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc36:            # 710\n \
        movl    0x24+%4,%%edi\n \
        subl    %%edi,0x58+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc28\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc38\n \
        movl    %%edi,0x58+%4\n \
        jmp     ll_loc39\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc38:\n \
        subl    0x58+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    ll_loc39:\n \
        jmp     ll_loc28\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc26:\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc28\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc27\n \
        movl    %%edi,0x58+%4\n \
        jmp     ll_loc28\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc27:\n \
        subl    0x58+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    ll_loc28:\n \
        leal    _polyscans,%%edi\n \
    \n \
    ll_loc29:\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl    0x58+%4\n \
        jnz     ll_loc29\n \
        movl    0x50+%4,%%ebx\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc30\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc30:            # 826 trig_+851\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x60+%4,%%ebx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl   0x54+%4\n \
        jnz     ll_loc30\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_md00:\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        cmpb   $0,6+%4\n \
        jz     ll_loc31\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x4C+%4\n \
        jle     ll_skipped\n \
        movl    %%edi,0x24+%4\n \
        cmpl    0x58+%4,%%edi\n \
        js     ll_loc40\n \
        movl    0x68+%4,%%edi\n \
        imull    0x58+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x50+%4,%%ebx\n \
        movl    0x24+%4,%%edi\n \
        subl    0x58+%4,%%edi\n \
        subl    %%edi,0x54+%4\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x24+%4,%%edi\n \
        imull    0x60+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc41\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x54+%4\n \
        movl    %%edi,0x4C+%4\n \
    \n \
    ll_loc41:\n \
        leal    _polyscans,%%edi\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc35\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc40:            # 88B\n \
        movl    0x24+%4,%%edi\n \
        subl    %%edi,0x58+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc33\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc42\n \
        movl    %%edi,0x58+%4\n \
        jmp     ll_loc43\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc42:\n \
        subl    0x58+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    ll_loc43:\n \
        jmp     ll_loc33\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc31:\n \
        cmpb   $0,4+%4\n \
        jz     ll_loc33\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc32\n \
        movl    %%edi,0x58+%4\n \
        jmp     ll_loc33\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc32:\n \
        subl    0x58+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    ll_loc33:\n \
        leal    _polyscans,%%edi\n \
    \n \
    ll_loc34:\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        addl    $0x14,%%edi\n \
        decl   0x58+%4\n \
        jnz     ll_loc34\n \
        movl    0x50+%4,%%ebx\n \
        cmpb   $0,5+%4\n \
        jz     ll_loc35\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ll_loc35:\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x60+%4,%%ebx\n \
        addl    $0x14,%%edi\n \
        decl   0x54+%4\n \
        jnz     ll_loc35\n \
        movl    $1,%%eax\n \
        jmp     ll_finished\n \
    ll_skipped:\n \
        movl    $0,%%eax\n \
    ll_finished:\n \
"
         : "=S" (dummy), "=D" (dummy), "=c" (dummy), "=a" (do_render)
         : "o" (llv), "0" (opt_a), "1" (opt_b), "2" (opt_c)
         : "memory", "cc", "%ebx", "%edx");
    memcpy(lv,&llv,sizeof(struct TrigLocals));
    return do_render;
}

int trig_rl_start(struct TrigLocals *lv, const struct PolyPoint *opt_a, const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    struct TrigLocals llv;
    long do_render;
    long dummy;
    asm volatile (" \
        movl    4(%%esi),%%eax\n \
        movl    %%eax,0x18+%4\n \
        orl    %%eax,%%eax\n \
        jns     rl_loc02\n \
        movl    _LOC_poly_screen,%%ebx\n \
        movl    %%ebx,0x6C+%4\n \
        movb   $1,6+%4\n \
        jmp     rl_loc01\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc02:            # 9BA\n \
        cmpl    _LOC_vec_window_height,%%eax\n \
        jge     rl_skipped\n \
        movl    %%eax,%%ebx\n \
        imull    _LOC_vec_screen_width,%%ebx\n \
        addl    _LOC_poly_screen,%%ebx\n \
        movl    %%ebx,0x6C+%4\n \
        movb   $0,6+%4\n \
    \n \
    rl_loc01:            # 9CA\n \
        movl    4(%%ecx),%%ebx\n \
        cmpl    _LOC_vec_window_height,%%ebx\n \
        setnle    5+%4\n \
        subl    %%eax,%%ebx\n \
        movl    %%ebx,0x5C+%4\n \
        movl    4(%%edi),%%ebx\n \
        cmpl    _LOC_vec_window_height,%%ebx\n \
        setnle    4+%4\n \
        subl    %%eax,%%ebx\n \
        movl    %%ebx,0x58+%4\n \
        movl    %%ebx,0x4C+%4\n \
        movl    (%%ecx),%%eax\n \
        subl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x68+%4\n \
        movl    (%%edi),%%eax\n \
        subl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x58+%4\n \
        cmpl    0x68+%4,%%eax\n \
        jle     rl_skipped\n \
        movl    %%eax,0x64+%4\n \
        movl    4(%%edi),%%ebx\n \
        subl    4(%%ecx),%%ebx\n \
        movl    (%%edi),%%eax\n \
        subl    (%%ecx),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x60+%4\n \
        movl    %%ebx,0x54+%4\n \
        movl    (%%ecx),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,0x50+%4\n \
        movzbl  _vec_mode,%%eax\n \
        jmpl    *rl_jt(,%%eax,4)\n \
    # ---------------------------------------------------------------------------\n \
    rl_jt:            # DATA XREF: trig_+A6D\n \
        .int    rl_md00\n \
        .int    rl_md01\n \
        .int    rl_md02\n \
        .int    rl_md02\n \
        .int    rl_md01\n \
        .int    rl_md05\n \
        .int    rl_md05\n \
        .int    rl_md02\n \
        .int    rl_md02\n \
        .int    rl_md02\n \
        .int    rl_md02\n \
        .int    rl_md02\n \
        .int    rl_md02\n \
        .int    rl_md02\n \
        .int    rl_md00\n \
        .int    rl_md00\n \
        .int    rl_md01\n \
        .int    rl_md01\n \
        .int    rl_md02\n \
        .int    rl_md02\n \
        .int    rl_md05\n \
        .int    rl_md05\n \
        .int    rl_md02\n \
        .int    rl_md02\n \
        .int    rl_md05\n \
        .int    rl_md05\n \
        .int    rl_md05\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_md05:            # DATA XREF: trig_:rl_jt\n \
        movl    0x5C+%4,%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x58+%4\n \
        movl    %%eax,0x10+%4\n \
        movl    (%%edi),%%eax\n \
        subl    (%%esi),%%eax\n \
        imull   0x10+%4\n \
        sarl    $0x10,%%eax\n \
        movl    (%%esi),%%ebx\n \
        subl    (%%ecx),%%ebx\n \
        addl    %%eax,%%ebx\n \
        jl     rl_skipped\n \
        jz     rl_loc03\n \
        incl    %%ebx\n \
        movl    8(%%edi),%%eax\n \
        subl    8(%%esi),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    8(%%esi),%%eax\n \
        subl    8(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x48+%4\n \
        movl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    0x0C(%%esi),%%eax\n \
        subl    0x0C(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x3C+%4\n \
        movl    0x10(%%edi),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    0x10(%%esi),%%eax\n \
        subl    0x10(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
    \n \
    rl_loc03:            # B07\n \
        movl    %%eax,0x30+%4\n \
        movl    8(%%ecx),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x44+%4\n \
        movl    0x0C(%%ecx),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x38+%4\n \
        movl    0x10(%%ecx),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x2C+%4\n \
        movl    8(%%edi),%%eax\n \
        subl    8(%%ecx),%%eax\n \
        cltd    \n \
        idivl   0x54+%4\n \
        movl    %%eax,0x40+%4\n \
        movl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%ecx),%%eax\n \
        cltd    \n \
        idivl   0x54+%4\n \
        movl    %%eax,0x34+%4\n \
        movl    0x10(%%edi),%%eax\n \
        subl    0x10(%%ecx),%%eax\n \
        cltd    \n \
        idivl   0x54+%4\n \
        movl    %%eax,0x28+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        movl    8(%%esi),%%ecx\n \
        movl    0x0C(%%esi),%%edx\n \
        movl    0x10(%%esi),%%esi\n \
        cmpb   $0,6+%4\n \
        jz     rl_loc04\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x4C+%4\n \
        jle     rl_skipped\n \
        movl    %%edi,0x24+%4\n \
        cmpl    0x5C+%4,%%edi\n \
        js     rl_loc34\n \
        movl    0x64+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        movl    0x50+%4,%%eax\n \
        movl    0x24+%4,%%edi\n \
        subl    0x5C+%4,%%edi\n \
        movl    %%edi,0x24+%4\n \
        subl    %%edi,0x54+%4\n \
        imull    0x60+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x40+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x34+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x28+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc37\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x54+%4\n \
        movl    %%edi,0x4C+%4\n \
    \n \
    rl_loc37:            # C66\n \
        leal    _polyscans,%%edi\n \
        jmp     rl_loc38\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc34:            # BE8\n \
        movl    0x24+%4,%%edi\n \
        subl    %%edi,0x5C+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc06\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc35\n \
        movl    %%edi,0x5C+%4\n \
        jmp     rl_loc36\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc35:            # CD2\n \
        subl    0x5C+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    rl_loc36:            # CD8\n \
        jmp     rl_loc06\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc04:            # BCA\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc06\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc05\n \
        movl    %%edi,0x5C+%4\n \
        jmp     rl_loc06\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc05:            # D03\n \
        subl    0x5C+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    rl_loc06:\n \
        leal    _polyscans,%%edi\n \
    \n \
    rl_loc08:            # D47\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     rl_loc08\n \
        movl    0x50+%4,%%eax\n \
    \n \
    rl_loc38:            # C7C\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc09\n \
        movl    $1,%%eax\n \
        jmp    rl_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc09:            # D52 trig_+D8B\n \
        movl    %%eax,(%%edi)\n \
        addl    0x60+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x40+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x34+%4,%%edx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x28+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl   0x54+%4\n \
        jnz     rl_loc09\n \
        movl    $1,%%eax\n \
        jmp    rl_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_md02:\n \
        movl    0x5C+%4,%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x58+%4\n \
        movl    %%eax,0x10+%4\n \
        movl    (%%edi),%%eax\n \
        subl    (%%esi),%%eax\n \
        imull   0x10+%4\n \
        sarl    $0x10,%%eax\n \
        movl    (%%esi),%%ebx\n \
        subl    (%%ecx),%%ebx\n \
        addl    %%eax,%%ebx\n \
        jl     rl_skipped\n \
        jz     rl_loc10\n \
        incl    %%ebx\n \
        movl    8(%%edi),%%eax\n \
        subl    8(%%esi),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    8(%%esi),%%eax\n \
        subl    8(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x48+%4\n \
        movl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    0x0C(%%esi),%%eax\n \
        subl    0x0C(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x3C+%4\n \
    \n \
    rl_loc10:            # DC2\n \
        movl    8(%%ecx),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x44+%4\n \
        movl    0x0C(%%ecx),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x38+%4\n \
        movl    8(%%edi),%%eax\n \
        subl    8(%%ecx),%%eax\n \
        cltd    \n \
        idivl   0x54+%4\n \
        movl    %%eax,0x40+%4\n \
        movl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%ecx),%%eax\n \
        cltd    \n \
        idivl   0x54+%4\n \
        movl    %%eax,0x34+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        movl    8(%%esi),%%ecx\n \
        movl    0x0C(%%esi),%%edx\n \
        cmpb   $0,6+%4\n \
        jz     rl_loc39\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x4C+%4\n \
        jle     rl_skipped\n \
        movl    %%edi,0x24+%4\n \
        cmpl    0x5C+%4,%%edi\n \
        js     rl_loc12\n \
        movl    0x64+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x50+%4,%%eax\n \
        movl    0x24+%4,%%edi\n \
        subl    0x5C+%4,%%edi\n \
        movl    %%edi,0x24+%4\n \
        subl    %%edi,0x54+%4\n \
        imull    0x60+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x40+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x34+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc11\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x54+%4\n \
        movl    %%edi,0x4C+%4\n \
    \n \
    rl_loc11:            # ECB\n \
        leal    _polyscans,%%edi\n \
        jmp     rl_loc13\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc12:            # E67\n \
        movl    0x24+%4,%%edi\n \
        subl    %%edi,0x5C+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc41\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc45\n \
        movl    %%edi,0x5C+%4\n \
        jmp     rl_loc46\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc45:            # F2C\n \
        subl    0x5C+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    rl_loc46:            # F32\n \
        jmp     rl_loc41\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc39:            # E49\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc41\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc40\n \
        movl    %%edi,0x5C+%4\n \
        jmp     rl_loc41\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc40:            # F5D\n \
        subl    0x5C+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    rl_loc41:            # F1B\n \
        leal    _polyscans,%%edi\n \
        # restrict 0x5C+%4 to 576 - size of polyscans[]\n \
        cmpl   $0x240,0x5C+%4\n \
        jl     rl_loc42_test2\n \
        movl   $0x240,0x5C+%4\n \
    rl_loc42_test2:\n \
        # restrict 0x54+%4 to 576 minus the value of previous var\n \
        cmpl   $0x240,0x54+%4\n \
        jl     rl_loc42\n \
        movl   $0x240,0x54+%4\n \
        #subl   0x5C+%4,0x54+%4\n \
    \n \
    rl_loc42:            # F9A\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        addl    $0x14,%%edi\n \
        decl    0x5C+%4\n \
        jnz     rl_loc42\n \
        movl    0x50+%4,%%eax\n \
    \n \
    rl_loc13:            # EE1\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc14\n \
        movl    $1,%%eax\n \
        jmp    rl_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc14:            # FA5 trig_+FD7\n \
        movl    %%eax,(%%edi)\n \
        addl    0x60+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x40+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x34+%4,%%edx\n \
        addl    $0x14,%%edi\n \
        decl   0x54+%4\n \
        jnz     rl_loc14\n \
        movl    $1,%%eax\n \
        jmp    rl_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_md01:            # DATA XREF: trig_:rl_jt\n \
        movl    0x5C+%4,%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl   0x58+%4\n \
        movl    %%eax,0x10+%4\n \
        movl    (%%edi),%%eax\n \
        subl    (%%esi),%%eax\n \
        imull   0x10+%4\n \
        sarl    $0x10,%%eax\n \
        movl    (%%esi),%%ebx\n \
        subl    (%%ecx),%%ebx\n \
        addl    %%eax,%%ebx\n \
        jl     rl_skipped\n \
        jz     rl_loc15\n \
        incl    %%ebx\n \
        movl    0x10(%%edi),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        imull   0x10+%4\n \
        shrdl    $0x10,%%edx,%%eax\n \
        addl    0x10(%%esi),%%eax\n \
        subl    0x10(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x30+%4\n \
    \n \
    rl_loc15:            # 100E\n \
        movl    0x10(%%ecx),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x5C+%4\n \
        movl    %%eax,0x2C+%4\n \
        movl    0x10(%%edi),%%eax\n \
        subl    0x10(%%ecx),%%eax\n \
        cltd    \n \
        idivl   0x54+%4\n \
        movl    %%eax,0x28+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        movl    0x10(%%esi),%%esi\n \
        cmpb   $0,6+%4\n \
        jz     rl_loc21\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x4C+%4\n \
        jle     rl_skipped\n \
        movl    %%edi,0x24+%4\n \
        cmpl    0x5C+%4,%%edi\n \
        js     rl_loc16\n \
        movl    0x64+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        movl    0x50+%4,%%eax\n \
        movl    0x24+%4,%%edi\n \
        subl    0x5C+%4,%%edi\n \
        movl    %%edi,0x24+%4\n \
        subl    %%edi,0x54+%4\n \
        imull    0x60+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x28+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc43\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x54+%4\n \
        movl    %%edi,0x4C+%4\n \
    \n \
    rl_loc43:            # 10C5\n \
        leal    _polyscans,%%edi\n \
        jmp     rl_loc44\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc16:            # 1077\n \
        movl    0x24+%4,%%edi\n \
        subl    %%edi,0x5C+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc18\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc20\n \
        movl    %%edi,0x5C+%4\n \
        jmp     rl_loc17\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc20:            # 111B\n \
        subl    0x5C+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    rl_loc17:            # 1121\n \
        jmp     rl_loc18\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc21:            # 1059\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc18\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc19\n \
        movl    %%edi,0x5C+%4\n \
        jmp     rl_loc18\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc19:            # 114C\n \
        subl    0x5C+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    rl_loc18:            # 110A\n \
        leal    _polyscans,%%edi\n \
    \n \
    rl_loc22:            # 1182\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     rl_loc22\n \
        movl    0x50+%4,%%eax\n \
    \n \
    rl_loc44:            # 10DB\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc23\n \
        movl    $1,%%eax\n \
        jmp    rl_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc23:            # 118D trig_+11B8\n \
        movl    %%eax,(%%edi)\n \
        addl    0x60+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x28+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl   0x54+%4\n \
        jnz     rl_loc23\n \
        movl    $1,%%eax\n \
        jmp    rl_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_md00:            # DATA XREF: trig_:rl_jt\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        cmpb   $0,6+%4\n \
        jz     rl_loc31\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x4C+%4\n \
        jle     rl_skipped\n \
        movl    %%edi,0x24+%4\n \
        cmpl    0x5C+%4,%%edi\n \
        js     rl_loc25\n \
        movl    0x64+%4,%%edi\n \
        imull    0x5C+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x50+%4,%%eax\n \
        movl    0x24+%4,%%edi\n \
        subl    0x5C+%4,%%edi\n \
        movl    %%edi,0x24+%4\n \
        subl    %%edi,0x54+%4\n \
        imull    0x60+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc24\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x54+%4\n \
        movl    %%edi,0x4C+%4\n \
    \n \
    rl_loc24:            # 122A\n \
        leal    _polyscans,%%edi\n \
        jmp     rl_loc29\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc25:            # 11F2\n \
        movl    0x24+%4,%%edi\n \
        subl    %%edi,0x5C+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc33\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc26\n \
        movl    %%edi,0x5C+%4\n \
        jmp     rl_loc27\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc26:            # 1275\n \
        subl    0x5C+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    rl_loc27:            # 127B\n \
        jmp     rl_loc33\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc31:            # 11D4\n \
        cmpb   $0,4+%4\n \
        jz     rl_loc33\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc32\n \
        movl    %%edi,0x5C+%4\n \
        jmp     rl_loc33\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc32:            # 12A6\n \
        subl    0x5C+%4,%%edi\n \
        setle    5+%4\n \
        movl    %%edi,0x54+%4\n \
    \n \
    rl_loc33:            # 1264\n \
        leal    _polyscans,%%edi\n \
    \n \
    rl_loc28:            # 12D5\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     rl_loc28\n \
        movl    0x50+%4,%%eax\n \
    \n \
    rl_loc29:            # 1240\n \
        cmpb   $0,5+%4\n \
        jz     rl_loc30\n \
        movl    $1,%%eax\n \
        jmp    rl_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    rl_loc30:\n \
        movl    %%eax,(%%edi)\n \
        addl    0x60+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        addl    $0x14,%%edi\n \
        decl   0x54+%4\n \
        jnz     rl_loc30\n \
        movl    $1,%%eax\n \
        jmp    rl_finished\n \
    rl_skipped:\n \
        movl    $0,%%eax\n \
    rl_finished:\n \
"
        : "=S" (dummy), "=D" (dummy), "=c" (dummy), "=a" (do_render)
        : "o" (llv), "0" (opt_a), "1" (opt_b), "2" (opt_c)
        : "memory", "cc", "%ebx", "%edx");
    memcpy(lv,&llv,sizeof(struct TrigLocals));
    return do_render;
}

int trig_fb_start(struct TrigLocals *lv, const struct PolyPoint *opt_a, const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    struct TrigLocals llv;
    long do_render;
    long dummy;
    asm volatile (" \
        movl    4(%%esi),%%eax\n \
        movl    %%eax,0x18+%4\n \
        orl    %%eax,%%eax\n \
        jns     fb_loc02\n \
        movl    _LOC_poly_screen,%%ebx\n \
        movl    %%ebx,0x6C+%4\n \
        movb   $1,6+%4\n \
        jmp     fb_loc01\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    fb_loc02:            # 132B\n \
        cmpl    _LOC_vec_window_height,%%eax\n \
        jge     fb_skipped\n \
        movl    %%eax,%%ebx\n \
        imull    _LOC_vec_screen_width,%%ebx\n \
        addl    _LOC_poly_screen,%%ebx\n \
        movl    %%ebx,0x6C+%4\n \
        movb   $0,6+%4\n \
    \n \
    fb_loc01:            # 133B\n \
        movl    4(%%ecx),%%ebx\n \
        cmpl    _LOC_vec_window_height,%%ebx\n \
        setnle    5+%4\n \
        subl    %%eax,%%ebx\n \
        movl    %%ebx,0x5C+%4\n \
        movl    %%ebx,0x4C+%4\n \
        movl    (%%ecx),%%eax\n \
        subl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x68+%4\n \
        movl    (%%edi),%%eax\n \
        subl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x64+%4\n \
        movzbl  _vec_mode,%%eax\n \
        jmpl    *fb_jt(,%%eax,4)\n \
    # ---------------------------------------------------------------------------\n \
    fb_jt:            # DATA XREF: trig_+139B\n \
        .int    fb_md00\n \
        .int    fb_md01\n \
        .int    fb_md02\n \
        .int    fb_md02\n \
        .int    fb_md01\n \
        .int    fb_md05\n \
        .int    fb_md05\n \
        .int    fb_md02\n \
        .int    fb_md02\n \
        .int    fb_md02\n \
        .int    fb_md02\n \
        .int    fb_md02\n \
        .int    fb_md02\n \
        .int    fb_md02\n \
        .int    fb_md00\n \
        .int    fb_md00\n \
        .int    fb_md01\n \
        .int    fb_md01\n \
        .int    fb_md02\n \
        .int    fb_md02\n \
        .int    fb_md05\n \
        .int    fb_md05\n \
        .int    fb_md02\n \
        .int    fb_md02\n \
        .int    fb_md05\n \
        .int    fb_md05\n \
        .int    fb_md05\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    fb_md05:            # DATA XREF: trig_:fb_jt\n \
        movl    (%%edi),%%ebx\n \
        subl    (%%ecx),%%ebx\n \
        movl    8(%%edi),%%eax\n \
        subl    8(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x48+%4\n \
        movl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x3C+%4\n \
        movl    0x10(%%edi),%%eax\n \
        subl    0x10(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x30+%4\n \
        movl    8(%%ecx),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x44+%4\n \
        movl    0x0C(%%ecx),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x38+%4\n \
        movl    0x10(%%ecx),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x2C+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        movl    8(%%esi),%%ecx\n \
        movl    0x0C(%%esi),%%edx\n \
        movl    0x10(%%esi),%%esi\n \
        cmpb   $0,6+%4\n \
        jz     fb_loc03\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x5C+%4\n \
        subl    %%edi,0x4C+%4\n \
        jle     fb_skipped\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,5+%4\n \
        jz     fb_loc04\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
        jmp     fb_loc04\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    fb_loc03:\n \
        cmpb   $0,5+%4\n \
        jz     fb_loc04\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
    \n \
    fb_loc04:\n \
        leal    _polyscans,%%edi\n \
    \n \
    fb_loc05:\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     fb_loc05\n \
        movl    $1,%%eax\n \
        jmp    fb_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    fb_md02:\n \
        movl    (%%edi),%%ebx\n \
        subl    (%%ecx),%%ebx\n \
        movl    8(%%edi),%%eax\n \
        subl    8(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x48+%4\n \
        movl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x3C+%4\n \
        movl    8(%%ecx),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x44+%4\n \
        movl    0x0C(%%ecx),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x38+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        movl    8(%%esi),%%ecx\n \
        movl    0x0C(%%esi),%%edx\n \
        cmpb   $0,6+%4\n \
        jz     fb_loc06\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x5C+%4\n \
        subl    %%edi,0x4C+%4\n \
        jle     fb_skipped\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,5+%4\n \
        jz     fb_loc07\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
        jmp     fb_loc07\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    fb_loc06:\n \
        cmpb   $0,5+%4\n \
        jz     fb_loc07\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
    \n \
    fb_loc07:\n \
        leal    _polyscans,%%edi\n \
    \n \
    fb_loc08:            # 162A\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     fb_loc08\n \
        movl    $1,%%eax\n \
        jmp    fb_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    fb_md01:            # DATA XREF: trig_:fb_jt\n \
        movl    (%%edi),%%ebx\n \
        subl    (%%ecx),%%ebx\n \
        movl    0x10(%%edi),%%eax\n \
        subl    0x10(%%ecx),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x30+%4\n \
        movl    0x10(%%ecx),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x2C+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        movl    0x10(%%esi),%%esi\n \
        cmpb   $0,6+%4\n \
        jz     fb_loc09\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x5C+%4\n \
        subl    %%edi,0x4C+%4\n \
        jle     fb_skipped\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,5+%4\n \
        jz     fb_loc10\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
        jmp     fb_loc10\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    fb_loc09:            # 1669\n \
        cmpb   $0,5+%4\n \
        jz     fb_loc10\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
    \n \
    fb_loc10:\n \
        leal    _polyscans,%%edi\n \
    \n \
    fb_loc11:            # 16F1\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     fb_loc11\n \
        movl    $1,%%eax\n \
        jmp    fb_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    fb_md00:            # DATA XREF: trig_:fb_jt\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    %%eax,%%ebx\n \
        cmpb   $0,6+%4\n \
        jz     fb_loc13\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x5C+%4\n \
        subl    %%edi,0x4C+%4\n \
        jle     fb_skipped\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        cmpb   $0,5+%4\n \
        jz     fb_loc12\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
        jmp     fb_loc12\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    fb_loc13:            # 170D\n \
        cmpb   $0,5+%4\n \
        jz     fb_loc12\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
    \n \
    fb_loc12:            # 173E trig_+174E ...\n \
        leal    _polyscans,%%edi\n \
    \n \
    fb_loc14:            # 1783\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     fb_loc14\n \
        movl    $1,%%eax\n \
        jmp    fb_finished\n \
    fb_skipped:\n \
        movl    $0,%%eax\n \
    fb_finished:\n \
"
        : "=S" (dummy), "=D" (dummy), "=c" (dummy), "=a" (do_render)
        : "o" (llv), "0" (opt_a), "1" (opt_b), "2" (opt_c)
        : "memory", "cc", "%ebx", "%edx");
    memcpy(lv,&llv,sizeof(struct TrigLocals));
    return do_render;
}

int trig_ft_start(struct TrigLocals *lv, const struct PolyPoint *opt_a, const struct PolyPoint *opt_b, const struct PolyPoint *opt_c)
{
    struct TrigLocals llv;
    long do_render;
    long dummy;
    asm volatile (" \
    \n \
        movl    4(%%esi),%%eax\n \
        movl    %%eax,0x18+%4\n \
        orl    %%eax,%%eax\n \
        jns     ft_loc02\n \
        movl    _LOC_poly_screen,%%ebx\n \
        movl    %%ebx,0x6C+%4\n \
        movb   $1,6+%4\n \
        jmp     ft_loc01\n \
    \n \
    ft_loc02:            # 17AA\n \
        cmpl    _LOC_vec_window_height,%%eax\n \
        jge     ft_skipped\n \
        movl    %%eax,%%ebx\n \
        imull    _LOC_vec_screen_width,%%ebx\n \
        addl    _LOC_poly_screen,%%ebx\n \
        movl    %%ebx,0x6C+%4\n \
        movb   $0,6+%4\n \
    \n \
    ft_loc01:            # 17BA\n \
        movl    4(%%ecx),%%ebx\n \
        cmpl    _LOC_vec_window_height,%%ebx\n \
        setnle    5+%4\n \
        subl    %%eax,%%ebx\n \
        movl    %%ebx,0x5C+%4\n \
        movl    %%ebx,0x4C+%4\n \
        movl    (%%ecx),%%eax\n \
        subl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x68+%4\n \
        movl    (%%ecx),%%eax\n \
        subl    (%%edi),%%eax\n \
        shll    $0x10,%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x64+%4\n \
        movzbl  _vec_mode,%%eax\n \
        jmpl    *ft_jt(,%%eax,4)\n \
    # ---------------------------------------------------------------------------\n \
    ft_jt:            # DATA XREF: trig_+181A\n \
        .int    ft_md00\n \
        .int    ft_md01\n \
        .int    ft_md02\n \
        .int    ft_md02\n \
        .int    ft_md01\n \
        .int    ft_md05\n \
        .int    ft_md05\n \
        .int    ft_md02\n \
        .int    ft_md02\n \
        .int    ft_md02\n \
        .int    ft_md02\n \
        .int    ft_md02\n \
        .int    ft_md02\n \
        .int    ft_md02\n \
        .int    ft_md00\n \
        .int    ft_md00\n \
        .int    ft_md01\n \
        .int    ft_md01\n \
        .int    ft_md02\n \
        .int    ft_md02\n \
        .int    ft_md05\n \
        .int    ft_md05\n \
        .int    ft_md02\n \
        .int    ft_md02\n \
        .int    ft_md05\n \
        .int    ft_md05\n \
        .int    ft_md05\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ft_md05:            # DATA XREF: trig_+1835 trig_+1839 ...\n \
        movl    (%%edi),%%ebx\n \
        subl    (%%esi),%%ebx\n \
        movl    8(%%edi),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x48+%4\n \
        movl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x3C+%4\n \
        movl    0x10(%%edi),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x30+%4\n \
        movl    8(%%ecx),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x44+%4\n \
        movl    0x0C(%%ecx),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x38+%4\n \
        movl    0x10(%%ecx),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x2C+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    (%%edi),%%ebx\n \
        shll    $0x10,%%ebx\n \
        movl    8(%%esi),%%ecx\n \
        movl    0x0C(%%esi),%%edx\n \
        movl    0x10(%%esi),%%esi\n \
        cmpb   $0,6+%4\n \
        jz     ft_loc03\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x5C+%4\n \
        subl    %%edi,0x4C+%4\n \
        jle     ft_skipped\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,5+%4\n \
        jz     ft_loc04\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
        jmp     ft_loc04\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ft_loc03:            # 18FD\n \
        cmpb   $0,5+%4\n \
        jz     ft_loc04\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
    \n \
    ft_loc04:            # 194F trig_+195F ...\n \
        leal    _polyscans,%%edi\n \
    \n \
    ft_loc05:            # 19A9\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     ft_loc05\n \
        movl    $1,%%eax\n \
        jmp    ft_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ft_md02:\n \
        movl    (%%edi),%%ebx\n \
        subl    (%%esi),%%ebx\n \
        movl    8(%%edi),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x48+%4\n \
        movl    0x0C(%%edi),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x3C+%4\n \
        movl    8(%%ecx),%%eax\n \
        subl    8(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x44+%4\n \
        movl    0x0C(%%ecx),%%eax\n \
        subl    0x0C(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x38+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    (%%edi),%%ebx\n \
        shll    $0x10,%%ebx\n \
        movl    8(%%esi),%%ecx\n \
        movl    0x0C(%%esi),%%edx\n \
        cmpb   $0,6+%4\n \
        jz     ft_loc06\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x5C+%4\n \
        subl    %%edi,0x4C+%4\n \
        jle     ft_skipped\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x44+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ecx\n \
        movl    0x38+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%edx\n \
        cmpb   $0,5+%4\n \
        jz     ft_loc07\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
        jmp     ft_loc07\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ft_loc06:\n \
        cmpb   $0,5+%4\n \
        jz     ft_loc07\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
    \n \
    ft_loc07:\n \
        leal    _polyscans,%%edi\n \
    \n \
    ft_loc08:            # 1AA4\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%ecx,8(%%edi)\n \
        addl    0x44+%4,%%ecx\n \
        movl    %%edx,0x0C(%%edi)\n \
        addl    0x38+%4,%%edx\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     ft_loc08\n \
        movl    $1,%%eax\n \
        jmp    ft_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ft_md01:\n \
        movl    (%%edi),%%ebx\n \
        subl    (%%esi),%%ebx\n \
        movl    0x10(%%edi),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl    %%ebx\n \
        movl    %%eax,0x30+%4\n \
        movl    0x10(%%ecx),%%eax\n \
        subl    0x10(%%esi),%%eax\n \
        cltd    \n \
        idivl   0x4C+%4\n \
        movl    %%eax,0x2C+%4\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    (%%edi),%%ebx\n \
        shll    $0x10,%%ebx\n \
        movl    0x10(%%esi),%%esi\n \
        cmpb   $0,6+%4\n \
        jz     ft_loc09\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x5C+%4\n \
        subl    %%edi,0x4C+%4\n \
        jle     ft_skipped\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        movl    0x2C+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%esi\n \
        cmpb   $0,5+%4\n \
        jz     ft_loc10\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
        jmp     ft_loc10\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ft_loc09:\n \
        cmpb   $0,5+%4\n \
        jz     ft_loc10\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
    \n \
    ft_loc10:\n \
        leal    _polyscans,%%edi\n \
    \n \
    ft_loc11:            # 1B6E\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        movl    %%esi,0x10(%%edi)\n \
        addl    0x2C+%4,%%esi\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     ft_loc11\n \
        movl    $1,%%eax\n \
        jmp    ft_finished\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ft_md00:            # DATA XREF: trig_:ft_jt trig_+1859    ...\n \
        movl    (%%esi),%%eax\n \
        shll    $0x10,%%eax\n \
        movl    (%%edi),%%ebx\n \
        shll    $0x10,%%ebx\n \
        cmpb   $0,6+%4\n \
        jz     ft_loc12\n \
        movl    0x18+%4,%%edi\n \
        negl    %%edi\n \
        subl    %%edi,0x5C+%4\n \
        subl    %%edi,0x4C+%4\n \
        jle     ft_skipped\n \
        movl    %%edi,0x24+%4\n \
        imull    0x68+%4,%%edi\n \
        addl    %%edi,%%eax\n \
        movl    0x64+%4,%%edi\n \
        imull    0x24+%4,%%edi\n \
        addl    %%edi,%%ebx\n \
        cmpb   $0,5+%4\n \
        jz     ft_loc13\n \
        movl    _LOC_vec_window_height,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
        jmp     ft_loc13\n \
    # ---------------------------------------------------------------------------\n \
    \n \
    ft_loc12:            # 1B8D\n \
        cmpb   $0,5+%4\n \
        jz     ft_loc13\n \
        movl    _LOC_vec_window_height,%%edi\n \
        subl    0x18+%4,%%edi\n \
        movl    %%edi,0x4C+%4\n \
        movl    %%edi,0x5C+%4\n \
    \n \
    ft_loc13:            # 1BBE trig_+1BCE ...\n \
        leal    _polyscans,%%edi\n \
    \n \
    ft_loc14:            # 1C03\n \
        movl    %%eax,(%%edi)\n \
        addl    0x68+%4,%%eax\n \
        movl    %%ebx,4(%%edi)\n \
        addl    0x64+%4,%%ebx\n \
        addl    $0x14,%%edi\n \
        decl   0x5C+%4\n \
        jnz     ft_loc14\n \
        movl    $1,%%eax\n \
        jmp    ft_finished\n \
    ft_skipped:\n \
        movl    $0,%%eax\n \
    ft_finished:\n \
"
        : "=S" (dummy), "=D" (dummy), "=c" (dummy), "=a" (do_render)
        : "o" (llv), "0" (opt_a), "1" (opt_b), "2" (opt_c)
        : "memory", "cc", "%ebx", "%edx");
    memcpy(lv,&llv,sizeof(struct TrigLocals));
    return do_render;
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
    struct TrigLocals lv;
    long start_type;
//    JUSTLOG("Pa(%ld,%ld,%ld)",point_a->field_8,point_a->field_C,point_a->field_10);
//    JUSTLOG("Pb(%ld,%ld,%ld)",point_b->field_8,point_b->field_C,point_b->field_10);
//    JUSTLOG("Pc(%ld,%ld,%ld)",point_c->field_8,point_c->field_C,point_c->field_10);
    //_DK_trig(point_a, point_b, point_c); return;
    LOC_poly_screen = poly_screen;
    LOC_vec_map = vec_map;
    LOC_vec_screen_width = vec_screen_width;
    LOC_vec_window_width = vec_window_width;
    LOC_vec_window_height = vec_window_height;
    opt_a = point_a;
    opt_b = point_b;
    opt_c = point_c;
    start_type = trig_reorder_input_points(&opt_a, &opt_b, &opt_c);
    switch (start_type)
    {
    case RendStart_LL:
        if (!trig_ll_start(&lv, opt_a, opt_b, opt_c)) {
            return;
        }
        break;
    case RendStart_RL:
        if (!trig_rl_start(&lv, opt_a, opt_b, opt_c)) {
            return;
        }
        break;
    case RendStart_FB:
        if (!trig_fb_start(&lv, opt_a, opt_b, opt_c)) {
            return;
        }
        break;
    case RendStart_FT:
        if (!trig_ft_start(&lv, opt_a, opt_b, opt_c)) {
            return;
        }
        break;
    default:
        return;
    }

    // ================ RENDERING CODE =============================

    /*switch (vec_mode)
    {
    case RendVec_mode00:
        break;
    case RendVec_mode01:
        break;
    case RendVec_mode02:
        break;
    case RendVec_mode03:
        break;
    case RendVec_mode04:
        break;
    case RendVec_mode05:
        break;
    case RendVec_mode06:
        break;
    case RendVec_mode07:
        break;
    case RendVec_mode08:
        break;
    case RendVec_mode09:
        break;
    case RendVec_mode10:
        break;
    case RendVec_mode11:
        break;
    case RendVec_mode12:
        break;
    case RendVec_mode13:
        break;
    case RendVec_mode14:
        break;
    case RendVec_mode15:
        break;
    case RendVec_mode16:
        break;
    case RendVec_mode17:
        break;
    case RendVec_mode18:
        break;
    }*/

asm volatile (" \
pushl   %%ebp\n \
\
render_mode:\n \
    movzbl   _vec_mode,%%eax\n \
    jmpl    *render_table(,%%eax,4)\n \
# ---------------------------------------------------------------------------\n \
render_table:\n \
    .int    render_md00\n \
    .int    render_md01\n \
    .int    render_md02\n \
    .int    render_md03\n \
    .int    render_md04\n \
    .int    render_md05\n \
    .int    render_md06\n \
    .int    render_md07\n \
    .int    render_md08\n \
    .int    render_md09\n \
    .int    render_md10\n \
    .int    render_md07\n \
    .int    render_md12\n \
    .int    render_md13\n \
    .int    render_md14\n \
    .int    render_md15\n \
    .int    render_md16\n \
    .int    render_md17\n \
    .int    render_md18\n \
    .int    render_md19\n \
    .int    render_md20\n \
    .int    render_md21\n \
    .int    render_md22\n \
    .int    render_md23\n \
    .int    render_md24\n \
    .int    render_md25\n \
    .int    render_md26\n \
# ===========================================================================\n \
\n \
render_md00:\n \
    leal    _polyscans,%%esi\n \
    movl    0x6C+%0,%%edx\n \
    movb    _vec_colour,%%al\n \
    movb    %%al,%%ah\n \
    movw    %%ax,%%bx\n \
    shll    $0x10,%%eax\n \
    movw    %%bx,%%ax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
render00_loc01:            # 1CF0\n \
    movw    2(%%esi),%%bx\n \
    movzwl  6(%%esi),%%ecx\n \
    addl    _LOC_vec_screen_width,%%edx\n \
    orw     %%bx,%%bx\n \
    jns     render00_loc03\n \
    orw     %%cx,%%cx\n \
    jle     render00_loc08\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     render00_loc02\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
render00_loc02:            # 1CBA\n \
    movl    %%edx,%%edi\n \
    jmp     render00_loc05\n \
# ---------------------------------------------------------------------------\n \
\n \
render00_loc03:            # 1CAD\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     render00_loc04\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
render00_loc04:            # 1CCC\n \
    subw    %%bx,%%cx\n \
    jle     render00_loc08\n \
    leal    (%%ebx,%%edx),%%edi\n \
\n \
render00_loc05:            # 1CC4\n \
    shrl    $1,%%ecx\n \
    jnb     render00_loc06\n \
    stosb    \n \
\n \
render00_loc06:            # 1CDE\n \
    shrl    $1,%%ecx\n \
    jnb     render00_loc07\n \
    stosw    \n \
\n \
render00_loc07:            # 1CE3\n \
    rep    stosl\n \
\n \
render00_loc08:            # 1CB2 trig_+1CD7\n \
    addl    $0x14,%%esi\n \
    decl    0x4C+%0\n \
    jnz     render00_loc01\n \
    jmp     render_md01\n \
\n \
# ===========================================================================\n \
\n \
render_md01:\n \
    leal    _polyscans,%%esi\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
render01_loc01:            # 1EA8\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     render01_loc04\n \
    orw    %%cx,%%cx\n \
    jle     render01_loc02\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    imull    0x30+%0,%%eax\n \
    movw    %%ax,%%bx\n \
    shrl    $8,%%eax\n \
    addw    0x10(%%esi),%%bx\n \
    adcb    0x12(%%esi),%%ah\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     render01_loc03\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
render01_loc03:            # 1D41\n \
    movzwl    %%ax,%%eax\n \
    movb    _vec_colour,%%al\n \
    jmp     render01_loc06\n \
# ---------------------------------------------------------------------------\n \
\n \
render01_loc04:            # 1D18\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     render01_loc05\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
render01_loc05:            # 1D59\n \
    subw    %%ax,%%cx\n \
    jle     render01_loc02\n \
    addl    %%eax,%%edi\n \
    movzbl   _vec_colour,%%eax\n \
    movw    0x10(%%esi),%%bx\n \
    movb    0x12(%%esi),%%ah\n \
\n \
render01_loc06:            # 1D51 trig_+1E9C\n \
    movb    %%ah,(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,1(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,2(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,3(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,4(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,5(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,6(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,7(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,8(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,9(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,0x0A(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,0x0B(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,0x0C(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,0x0D(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,0x0E(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    movb    %%ah,0x0F(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     render01_loc02\n \
    addl    $0x10,%%edi\n \
    jmp     render01_loc06\n \
# ---------------------------------------------------------------------------\n \
\n \
render01_loc02:\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     render01_loc01\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md02:            # 2D92\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
render02_loc01:            # 2144\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     render02_loc03\n \
    orw    %%cx,%%cx\n \
    jle     loc_789C1D\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl    0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     render02_loc02\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
render02_loc02:\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_789A2C\n \
# ---------------------------------------------------------------------------\n \
\n \
render02_loc03:\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_789A12\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_789A12:            # 1F2A\n \
    subw    %%ax,%%cx\n \
    jle     loc_789C1D\n \
    addl    %%eax,%%edi\n \
    movl    0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_789A2C:            # 1F22\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_789A36:            # 2134\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,1(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,2(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,3(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,4(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,5(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,6(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,7(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,8(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,9(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,0x0A(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,0x0B(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,0x0C(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,0x0D(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,0x0E(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    movb    %%al,0x0F(%%edi)\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789C19\n \
    addl    $0x10,%%edi\n \
    jmp     loc_789A36\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_789C19:\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_789C1D:\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     render02_loc01\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md03:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_789C46:            # 2424\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_789CA0\n \
    orw    %%cx,%%cx\n \
    jle     loc_789EFD\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull    0x3C+%0,%%edx\n \
    addl    0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_789C9B\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_789C9B:            # 21B3\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_789CC8\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_789CA0:            # 217D\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_789CAE\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_789CAE:            # 21C6\n \
    subw    %%ax,%%cx\n \
    jle     loc_789EFD\n \
    addl    %%eax,%%edi\n \
    movl    0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_789CC8:            # 21BE\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_789CD2:            # 2414\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789CDB\n \
    movb    %%al,(%%edi)\n \
\n \
loc_789CDB:            # 21F7\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789CFE\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_789CFE:            # 2219\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789D21\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_789D21:            # 223C\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789D44\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_789D44:            # 225F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789D67\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_789D67:            # 2282\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789D8A\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_789D8A:            # 22A5\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789DAD\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_789DAD:            # 22C8\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789DD0\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_789DD0:            # 22EB\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789DF3\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_789DF3:            # 230E\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789E16\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_789E16:            # 2331\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789E39\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_789E39:            # 2354\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789E5C\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_789E5C:            # 2377\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789E7F\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_789E7F:            # 239A\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789E9E\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_789E9E:            # 23B9\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789EBD\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_789EBD:            # 23D8\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_789EDC\n \
    movb    %%al,0x0F(%%edi)\n \
\n \
loc_789EDC:            # 23F7\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_789EF9\n \
    addl    $0x10,%%edi\n \
    jmp     loc_789CD2\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_789EF9:\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_789EFD:\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_789C46\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md04:\n \
    leal    _polyscans,%%esi\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_789F19:            # 2648\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_789F6B\n \
    orw    %%cx,%%cx\n \
    jle     loc_78A121\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    imull    0x30+%0,%%eax\n \
    movw    %%ax,%%bx\n \
    shrl    $8,%%eax\n \
    addw    0x10(%%esi),%%bx\n \
    adcb    0x12(%%esi),%%ah\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_789F61\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_789F61:            # 2479\n \
    movzwl    %%ax,%%eax\n \
    movb    _vec_colour,%%al\n \
    jmp     loc_789F92\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_789F6B:            # 2450\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_789F79\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_789F79:            # 2491\n \
    subw    %%ax,%%cx\n \
    jle     loc_78A121\n \
    addl    %%eax,%%edi\n \
    movzbl   _vec_colour,%%eax\n \
    movw    0x10(%%esi),%%bx\n \
    movb    0x12(%%esi),%%ah\n \
\n \
loc_789F92:            # 2489 trig_+263C\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,1(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,2(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,3(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,4(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,5(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,6(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,7(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,8(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,9(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,0x0A(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,0x0B(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,0x0C(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,0x0D(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,0x0E(%%edi)\n \
    jz     loc_78A121\n \
    addw    0x30+%0,%%bx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    movb    %%dl,0x0F(%%edi)\n \
    jz     loc_78A121\n \
    addl    $0x10,%%edi\n \
    jmp     loc_789F92\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78A121:            # 2455 trig_+249C ...\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_789F19\n \
    jmp finished\n \
# ---------------------------------------------------------------------------\n \
add_to_edi:\n \
    .int    0,-15,-14,-13\n \
    .int    -12,-11,-10,-9\n \
    .int    -8,-7,-6,-5\n \
    .int    -4,-3,-2,-1\n \
# ===========================================================================\n \
\n \
render_md05:\n \
    leal    _polyscans,%%esi\n \
    movl    %%esi,0x10+%0\n \
    xorl    %%ebx,%%ebx\n \
    pushl   %%ebp\n \
    movl    0x48+%0,%%ecx\n \
    movl    0x3C+%0,%%edx\n \
    movl    0x30+%0,%%ebp\n \
    roll    $0x10,%%ecx\n \
    rol    $0x10,%%edx\n \
    shrl    $8,%%ebp\n \
    movb    %%dl,%%bl\n \
    movb    %%cl,%%dl\n \
    movw    %%bp,%%cx\n \
    xorb    %%dh,%%dh\n \
    popl    %%ebp\n \
    movl    %%ecx,0x20+%0\n \
    movl    %%edx,0x1C+%0\n \
    movb    %%bl,8+%0\n \
\n \
loc_78A1A9:            # 299F\n \
    movl    0x10+%0,%%esi\n \
    addl    $0x14,0x10+%0\n \
    movl    (%%esi),%%eax\n \
    movl    4(%%esi),%%ebp\n \
    sarl    $0x10,%%eax\n \
    sarl    $0x10,%%ebp\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orl    %%eax,%%eax\n \
    jns     loc_78A217\n \
    orl    %%ebp,%%ebp\n \
    jle     loc_78A47B\n \
    negl    %%eax\n \
    movl    0x48+%0,%%ecx\n \
    imull    %%eax,%%ecx\n \
    addl    8(%%esi),%%ecx\n \
    movl   0x3C+%0,%%edx\n \
    imull    %%eax,%%edx\n \
    addl    0x0C(%%esi),%%edx\n \
    movl    0x30+%0,%%ebx\n \
    imull    %%eax,%%ebx\n \
    addl    0x10(%%esi),%%ebx\n \
    roll    $0x10,%%ecx\n \
    rol    $0x10,%%edx\n \
    shrl    $8,%%ebx\n \
    movb    %%dl,%%al\n \
    movb    %%cl,%%dl\n \
    movw    %%bx,%%cx\n \
    movb    %%al,%%bh\n \
    cmpl    _LOC_vec_window_width,%%ebp\n \
    jle     loc_78A215\n \
    movl    _LOC_vec_window_width,%%ebp\n \
\n \
loc_78A215:            # 272D\n \
    jmp     loc_78A24A\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78A217:            # 26EB\n \
    cmpl    _LOC_vec_window_width,%%ebp\n \
    jle     loc_78A225\n \
    movl    _LOC_vec_window_width,%%ebp\n \
\n \
loc_78A225:            # 273D\n \
    subl    %%eax,%%ebp\n \
    jle     loc_78A47B\n \
    addl    %%eax,%%edi\n \
    movl    8(%%esi),%%ecx\n \
    movl   0x0C(%%esi),%%edx\n \
    movl    0x10(%%esi),%%ebx\n \
    roll    $0x10,%%ecx\n \
    rol    $0x10,%%edx\n \
    shrl    $8,%%ebx\n \
    movb    %%dl,%%al\n \
    movb    %%cl,%%dl\n \
    movw    %%bx,%%cx\n \
    movb    %%al,%%bh\n \
\n \
loc_78A24A:\n \
    xorb    %%dh,%%dh\n \
    andl    $0x0FFFF,%%ebx\n \
    movl    %%ebp,%%eax\n \
    andl    $0x0F,%%eax\n \
    addl    add_to_edi(,%%eax,4),%%edi\n \
    movl    _LOC_vec_map,%%esi\n \
    jmpl    *gt_jt(,%%eax,4)\n \
# ---------------------------------------------------------------------------\n \
gt_jt:\n \
    .int    gt_md00\n \
    .int    gt_md01\n \
    .int    gt_md02\n \
    .int    gt_md03\n \
    .int    gt_md04\n \
    .int    gt_md05\n \
    .int    gt_md06\n \
    .int    gt_md07\n \
    .int    gt_md08\n \
    .int    gt_md09\n \
    .int    gt_md10\n \
    .int    gt_md11\n \
    .int    gt_md12\n \
    .int    gt_md13\n \
    .int    gt_md14\n \
    .int    gt_md15\n \
# ---------------------------------------------------------------------------\n \
\n \
gt_md00:\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,(%%edi)\n \
\n \
gt_md15:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,1(%%edi)\n \
\n \
gt_md14:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,2(%%edi)\n \
\n \
gt_md13:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,3(%%edi)\n \
\n \
gt_md12:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,4(%%edi)\n \
\n \
gt_md11:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,5(%%edi)\n \
\n \
gt_md10:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,6(%%edi)\n \
\n \
gt_md09:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,7(%%edi)\n \
\n \
gt_md08:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,8(%%edi)\n \
\n \
gt_md07:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,9(%%edi)\n \
\n \
gt_md06:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
gt_md05:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
gt_md04:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
gt_md03:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
gt_md02:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
gt_md01:            # DATA XREF: trig_:gt_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    8+%0,%%bh\n \
    movb    %%al,0x0F(%%edi)\n \
    addl    $0x10,%%edi\n \
    subl    $0x10,%%ebp\n \
    jg      gt_md00\n \
\n \
loc_78A47B:            # 26EF trig_+2747\n \
    decl   0x4C+%0\n \
    jnz     loc_78A1A9\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md06:\n \
    leal    _polyscans,%%esi\n \
    movl    %%esi,0x10+%0\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    movl    0x30+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x1C+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78A4B0:            # 2D80\n \
    movl    0x10+%0,%%esi\n \
    addl    $0x14,0x10+%0\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78A52A\n \
    orw    %%cx,%%cx\n \
    jle     loc_78A85C\n \
    pushl   %%ebp\n \
    movw    %%cx,%%bp\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    movl    %%eax,%%ecx\n \
    imull   0x3C+%0,%%edx\n \
    addl    0x0C(%%esi),%%edx\n \
    rol     $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull   0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    imull   0x30+%0,%%ecx\n \
    addl    0x10(%%esi),%%ecx\n \
    roll    $0x10,%%ecx\n \
    movb    %%cl,%%ah\n \
    movw    %%bp,%%cx\n \
    popl    %%ebp\n \
    movzwl  %%ax,%%eax\n \
    cmpw    _LOC_vec_window_width,%%cx\n \
    jle     loc_78A528\n \
    movw    _LOC_vec_window_width,%%cx\n \
\n \
loc_78A528:            # 2A3F\n \
    jmp     loc_78A560\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78A52A:            # 29F0\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78A538\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78A538:            # 2A50\n \
    subw    %%ax,%%cx\n \
    jle     loc_78A85C\n \
    addl    %%eax,%%edi\n \
    movl    0x0C(%%esi),%%edx\n \
    movb    0x0A(%%esi),%%bl\n \
    rol    $0x10,%%edx\n \
    pushl   %%ebp\n \
    movw    %%cx,%%bp\n \
    movb    %%dl,%%bh\n \
    movl    0x10(%%esi),%%ecx\n \
    movw    8(%%esi),%%dx\n \
    roll    $0x10,%%ecx\n \
    movb    %%cl,%%ah\n \
    movw    %%bp,%%cx\n \
    popl    %%ebp\n \
\n \
loc_78A560:            # CODE XREF: trig_:loc_78A528\n \
    movw    %%cx,%%si\n \
    andl    $0x0F,%%esi\n \
    addl    add_to_edi(,%%esi,4),%%edi\n \
    movl    _LOC_vec_map,%%ebp\n \
    jmpl    *mgt_jt(,%%esi,4)\n \
# ---------------------------------------------------------------------------\n \
mgt_jt:            # DATA XREF: trig_+2A93\n \
    .int    mgt_md00\n \
    .int    mgt_md01\n \
    .int    mgt_md02\n \
    .int    mgt_md03\n \
    .int    mgt_md04\n \
    .int    mgt_md05\n \
    .int    mgt_md06\n \
    .int    mgt_md07\n \
    .int    mgt_md08\n \
    .int    mgt_md09\n \
    .int    mgt_md10\n \
    .int    mgt_md11\n \
    .int    mgt_md12\n \
    .int    mgt_md13\n \
    .int    mgt_md14\n \
    .int    mgt_md15\n \
\n \
mgt_md00:            # 2D76\n \
# DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A5CF\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,(%%edi)\n \
\n \
loc_78A5CF:            # 2AE5\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md15:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A5F8\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_78A5F8:            # 2B0D\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md14:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb     %%al,%%al\n \
    jz      loc_78A621\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_78A621:            # 2B36\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md13:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb     %%al,%%al\n \
    jz      loc_78A64A\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_78A64A:            # 2B5F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md12:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A673\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_78A673:            # 2B88\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md11:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb     %%al,%%al\n \
    jz      loc_78A69C\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_78A69C:            # 2BB1\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md10:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb     %%al,%%al\n \
    jz      loc_78A6C5\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_78A6C5:            # 2BDA\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md09:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb     %%al,%%al\n \
    jz      loc_78A6EE\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_78A6EE:            # 2C03\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md08:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb     %%al,%%al\n \
    jz      loc_78A717\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_78A717:            # 2C2C\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl    0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md07:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A740\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_78A740:            # 2C55\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md06:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A769\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_78A769:            # 2C7E\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md05:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A792\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_78A792:            # 2CA7\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md04:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A7BB\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_78A7BB:            # 2CD0\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md03:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A7E4\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_78A7E4:            # 2CF9\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md02:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A80D\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_78A80D:            # 2D22\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
\n \
mgt_md01:            # DATA XREF: trig_:mgt_jt\n \
    movb    (%%ebx,%%ebp),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78A836\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0F(%%edi)\n \
\n \
loc_78A836:            # 2D4B\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%ah\n \
    addl    $0x10,%%edi\n \
    subw    $0x10,%%cx\n \
    jg      mgt_md00\n \
\n \
loc_78A85C:\n \
    decl   0x4C+%0\n \
    jnz     loc_78A4B0\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md07:\n \
    cmpb   $0x20,_vec_colour\n \
    jz     render_md02\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78A88F:\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78A8E9\n \
    orw    %%cx,%%cx\n \
    jle     loc_78AB6C\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78A8E4\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78A8E4:\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78A911\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78A8E9:\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78A8F7\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78A8F7:            # 2E0F\n \
    subw    %%ax,%%cx\n \
    jle     loc_78AB6C\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78A911:            # 2E07\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
    movb    _vec_colour,%%ah\n \
\n \
loc_78A921:            # 3083\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,1(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,2(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,3(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,4(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,5(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,6(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,7(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,8(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,9(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0A(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0B(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0C(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0D(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0E(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0F(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AB68\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78A921\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78AB68:            # 2E5F trig_+2E84 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78AB6C:            # 2DCB trig_+2E1A\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78A88F\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md08:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78AB95:            # 33D9\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78ABEF\n \
    orw    %%cx,%%cx\n \
    jle     loc_78AEB2\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78ABEA\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78ABEA:            # 3102\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78AC17\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78ABEF:            # 30CC\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78ABFD\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78ABFD:            # 3115\n \
    subw    %%ax,%%cx\n \
    jle     loc_78AEB2\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78AC17:            # 310D\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
    movb    _vec_colour,%%ah\n \
\n \
loc_78AC27:            # 33C9\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AC3F\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,(%%edi)\n \
\n \
loc_78AC3F:            # 3155\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AC68\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_78AC68:            # 317D\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AC91\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_78AC91:            # 31A6\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78ACBA\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_78ACBA:            # 31CF\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78ACE3\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_78ACE3:            # 31F8\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AD0C\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_78AD0C:            # 3221\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AD35\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_78AD35:            # 324A\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AD5E\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_78AD5E:            # 3273\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AD87\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_78AD87:            # 329C\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78ADB0\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_78ADB0:            # 32C5\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78ADD9\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_78ADD9:            # 32EE\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AE02\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_78AE02:            # 3317\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AE2B\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_78AE2B:            # 3340\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AE50\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_78AE50:            # 3365\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AE75\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_78AE75:            # 338A\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%al,%%al\n \
    jz     loc_78AE9A\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0F(%%edi)\n \
\n \
loc_78AE9A:            # 33AF\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78AEAE\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78AC27\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78AEAE:            # 3169 trig_+3192 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78AEB2:            # 30D1 trig_+3120\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78AB95\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md09:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78AEDB:            # 374C\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78AF35\n \
    orw    %%cx,%%cx\n \
    jle     loc_78B225\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78AF30\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78AF30:            # 3448\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78AF5D\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78AF35:            # 3412\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78AF43\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78AF43:            # 345B\n \
    subw    %%ax,%%cx\n \
    jle     loc_78B225\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78AF5D:            # 3453\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78AF67:            # 373C\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78AF81\n \
    movb    (%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,(%%edi)\n \
\n \
loc_78AF81:            # 3495\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78AFAD\n \
    movb    1(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_78AFAD:            # 34BF\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78AFD9\n \
    movb    2(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_78AFD9:            # 34EB\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B005\n \
    movb    3(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_78B005:            # 3517\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B031\n \
    movb    4(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_78B031:            # 3543\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B05D\n \
    movb    5(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_78B05D:            # 356F\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B089\n \
    movb    6(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_78B089:            # 359B\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B0B5\n \
    movb    7(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_78B0B5:            # 35C7\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B0E1\n \
    movb    8(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_78B0E1:            # 35F3\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B10D\n \
    movb    9(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_78B10D:            # 361F\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B139\n \
    movb    0x0A(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_78B139:            # 364B\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B165\n \
    movb    0x0B(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_78B165:            # 3677\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B191\n \
    movb    0x0C(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_78B191:            # 36A3\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B1BD\n \
    movb    0x0D(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_78B1BD:            # 36CF\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B1E5\n \
    movb    0x0E(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_78B1E5:            # 36F7\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    orb    %%ah,%%ah\n \
    jz     loc_78B20D\n \
    movb    0x0F(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0F(%%edi)\n \
\n \
loc_78B20D:            # 371F\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B221\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78AF67\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78B221:            # 34AB trig_+34D7 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78B225:            # 3417 trig_+3466\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78AEDB\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md10:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78B24E:            # 3AC5\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78B2A8\n \
    orw    %%cx,%%cx\n \
    jle     loc_78B59E\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78B2A3\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78B2A3:            # 37BB\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78B2D0\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78B2A8:            # 3785\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78B2B6\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78B2B6:            # 37CE\n \
    subw    %%ax,%%cx\n \
    jle     loc_78B59E\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78B2D0:            # 37C6\n \
    movb    _vec_colour,%%ah\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78B2E0:            # 3AB5\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B2F1\n \
# Why EDI may be incorrect??\n \
    movb    (%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,(%%edi)\n \
\n \
loc_78B2F1:            # 3805\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B31D\n \
    movb    1(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_78B31D:            # 382F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B349\n \
    movb    2(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_78B349:            # 385B\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B375\n \
    movb    3(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_78B375:            # 3887\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B3A1\n \
    movb    4(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_78B3A1:            # 38B3\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B3CD\n \
    movb    5(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_78B3CD:            # 38DF\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B3F9\n \
    movb    6(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_78B3F9:            # 390B\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B425\n \
    movb    7(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_78B425:            # 3937\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B451\n \
    movb    8(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_78B451:            # 3963\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B47D\n \
    movb    9(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_78B47D:            # 398F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B4A9\n \
    movb    0x0A(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_78B4A9:            # 39BB\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B4D5\n \
    movb    0x0B(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_78B4D5:            # 39E7\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B501\n \
    movb    0x0C(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_78B501:            # 3A13\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B52D\n \
    movb    0x0D(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_78B52D:            # 3A3F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B555\n \
    movb    0x0E(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_78B555:            # 3A67\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78B57D\n \
    movb    0x0F(%%edi),%%al\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0F(%%edi)\n \
\n \
loc_78B57D:            # 3A8F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B59A\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78B2E0\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78B59A:            # 3824 trig_+3850 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78B59E:            # 378A trig_+37D9\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78B24E\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md12:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78B5C7:            # 3DCA\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78B621\n \
    orw    %%cx,%%cx\n \
    jle     loc_78B8A3\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78B61C\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78B61C:            # 3B34\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78B649\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78B621:            # 3AFE\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78B62F\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78B62F:            # 3B47\n \
    subw    %%ax,%%cx\n \
    jle     loc_78B8A3\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78B649:            # 3B3F\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
    movb    _vec_colour,%%al\n \
\n \
loc_78B658:            # 3DBA\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,1(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,2(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,3(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,4(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,5(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,6(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,7(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,8(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,9(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,0x0A(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,0x0B(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,0x0C(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,0x0D(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,0x0E(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%ah,0x0F(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78B89F\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78B658\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78B89F:            # 3B96 trig_+3BBB ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78B8A3:            # 3B03 trig_+3B52\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78B5C7\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md13:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78B8CC:            # 40D0\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78B926\n \
    orw    %%cx,%%cx\n \
    jle     loc_78BBA9\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78B921\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78B921:            # 3E39\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78B94E\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78B926:            # 3E03\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78B934\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78B934:            # 3E4C\n \
    subw    %%ax,%%cx\n \
    jle     loc_78BBA9\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78B94E:            # 3E44\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
    movb    _vec_colour,%%ah\n \
\n \
loc_78B95E:            # 40C0\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,1(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,2(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,3(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,4(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,5(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,6(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,7(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,8(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,9(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0A(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0B(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0C(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0D(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0E(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0F(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78BBA5\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78B95E\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78BBA5:            # 3E9C trig_+3EC1 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78BBA9:            # 3E08 trig_+3E57\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78B8CC\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md14:\n \
    leal    _polyscans,%%esi\n \
    movl   0x6C+%0,%%edx\n \
    xorl    %%eax,%%eax\n \
    movb    _vec_colour,%%ah\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78BBD0:            # 4265\n \
    movw    2(%%esi),%%bx\n \
    movzwl   6(%%esi),%%ecx\n \
    addl   _LOC_vec_screen_width,%%edx\n \
    orw    %%bx,%%bx\n \
    jns     loc_78BBFE\n \
    orw    %%cx,%%cx\n \
    jle     loc_78BD3E\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78BBFA\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78BBFA:            # 4112\n \
    movl    %%edx,%%edi\n \
    jmp     loc_78BC18\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78BBFE:            # 4101\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78BC0C\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78BC0C:            # 4124\n \
    subw    %%bx,%%cx\n \
    jle     loc_78BD3E\n \
    leal    (%%ebx,%%edx),%%edi\n \
\n \
loc_78BC18:\n \
    movb    (%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    1(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,1(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    2(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,2(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    3(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,3(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    4(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,4(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    5(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,5(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    6(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,6(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    7(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,7(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    8(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,8(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    9(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,9(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    0x0A(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0A(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    0x0B(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0B(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    0x0C(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0C(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    0x0D(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0D(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    0x0E(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0E(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    movb    0x0F(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0F(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BD3E\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78BC18\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78BD3E:            # 4106 trig_+412F ...\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78BBD0\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md15:\n \
    leal    _polyscans,%%esi\n \
    movl   0x6C+%0,%%edx\n \
    movzbl   _vec_colour,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78BD64:            # 43F9\n \
    movw    2(%%esi),%%bx\n \
    movzwl   6(%%esi),%%ecx\n \
    addl   _LOC_vec_screen_width,%%edx\n \
    orw    %%bx,%%bx\n \
    jns     loc_78BD92\n \
    orw    %%cx,%%cx\n \
    jle     loc_78BED2\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78BD8E\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78BD8E:            # 42A6\n \
    movl    %%edx,%%edi\n \
    jmp     loc_78BDAC\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78BD92:            # 4295\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78BDA0\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78BDA0:            # 42B8\n \
    subw    %%bx,%%cx\n \
    jle     loc_78BED2\n \
    leal    (%%ebx,%%edx),%%edi\n \
\n \
loc_78BDAC:            # 42B0 trig_+43ED\n \
    movb    (%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    1(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,1(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    2(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,2(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    3(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,3(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    4(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,4(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    5(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,5(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    6(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,6(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    7(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,7(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    8(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,8(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    9(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,9(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    0x0A(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,0x0A(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    0x0B(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,0x0B(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    0x0C(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,0x0C(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    0x0D(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,0x0D(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    0x0E(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,0x0E(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    movb    0x0F(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    %%ah,0x0F(%%edi)\n \
    decw    %%cx\n \
    jz     loc_78BED2\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78BDAC\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78BED2:            # 429A trig_+42C3 ...\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78BD64\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md16:\n \
    leal    _polyscans,%%esi\n \
    xor    %%edx,%%edx\n \
\n \
loc_78BEEC:            # 46B2\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78BF3E\n \
    orw    %%cx,%%cx\n \
    jle     loc_78C18B\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    imull    0x30+%0,%%eax\n \
    movw    %%ax,%%bx\n \
    shrl    $8,%%eax\n \
    addw    0x10(%%esi),%%bx\n \
    adcb    0x12(%%esi),%%ah\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78BF34\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78BF34:            # 444C\n \
    movzwl    %%ax,%%eax\n \
    movb    _vec_colour,%%al\n \
    jmp     loc_78BF65\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78BF3E:            # 4423\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78BF4C\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78BF4C:            # 4464\n \
    subw    %%ax,%%cx\n \
    jle     loc_78C18B\n \
    addl    %%eax,%%edi\n \
    movzbl   _vec_colour,%%eax\n \
    movw    0x10(%%esi),%%bx\n \
    movb    0x12(%%esi),%%ah\n \
\n \
loc_78BF65:            # 445C trig_+46A6\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    (%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    1(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,1(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    2(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,2(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    3(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,3(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    4(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,4(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    5(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,5(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    6(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,6(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    7(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,7(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    8(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,8(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    9(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,9(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    0x0A(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0A(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    0x0B(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0B(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    0x0C(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0C(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    0x0D(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0D(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    0x0E(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0E(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dh\n \
    popl    %%ebx\n \
    movb    0x0F(%%edi),%%dl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0F(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C18B\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78BF65\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78C18B:            # 4428 trig_+446F ...\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78BEEC\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md17:\n \
    leal    _polyscans,%%esi\n \
    xor    %%edx,%%edx\n \
\n \
loc_78C1A5:            # 496B\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78C1F7\n \
    orw    %%cx,%%cx\n \
    jle     loc_78C444\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    imull    0x30+%0,%%eax\n \
    movw    %%ax,%%bx\n \
    shrl    $8,%%eax\n \
    addw    0x10(%%esi),%%bx\n \
    adcb    0x12(%%esi),%%ah\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78C1ED\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78C1ED:            # 4705\n \
    movzwl    %%ax,%%eax\n \
    movb    _vec_colour,%%al\n \
    jmp     loc_78C21E\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78C1F7:            # 46DC\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78C205\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78C205:            # 471D\n \
    subw    %%ax,%%cx\n \
    jle     loc_78C444\n \
    addl    %%eax,%%edi\n \
    movzbl  _vec_colour,%%eax\n \
    movw    0x10(%%esi),%%bx\n \
    movb    0x12(%%esi),%%ah\n \
\n \
loc_78C21E:            # 4715 trig_+495F\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    (%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    1(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,1(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    2(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,2(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    3(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,3(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    4(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,4(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    5(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,5(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    6(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,6(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    7(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,7(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    8(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,8(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    9(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,9(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    0x0A(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0A(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    0x0B(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0B(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    0x0C(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0C(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    0x0D(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0D(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    0x0E(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0E(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%dl\n \
    popl    %%ebx\n \
    movb    0x0F(%%edi),%%dh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%edx,%%ebx),%%dl\n \
    popl    %%ebx\n \
    movb    %%dl,0x0F(%%edi)\n \
    addw    0x30+%0,%%bx\n \
    adcb    0x32+%0,%%ah\n \
    decw    %%cx\n \
    jz     loc_78C444\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78C21E\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78C444:            # 46E1 trig_+4728 ...\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78C1A5\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md18:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78C46D:            # 4C9A\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78C4C7\n \
    orw    %%cx,%%cx\n \
    jle     loc_78C773\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78C4C2\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78C4C2:            # 49DA\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78C4EF\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78C4C7:            # 49A4\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78C4D5\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78C4D5:            # 49ED\n \
    subw    %%ax,%%cx\n \
    jle     loc_78C773\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78C4EF:            # 49E5\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78C4F9:            # 4C8A\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    (%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    1(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,1(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    2(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,2(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    3(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,3(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    4(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,4(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    5(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,5(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    6(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,6(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    7(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,7(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    8(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,8(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    9(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,9(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0A(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0A(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0B(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0B(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0C(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0C(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0D(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0D(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0E(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0E(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    movb    (%%ebx,%%esi),%%ah\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0F(%%edi),%%al\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0F(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78C76F\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78C4F9\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78C76F:            # 4A39 trig_+4A61 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78C773:            # 49A9 trig_+49F8\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78C46D\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md19:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78C79C:            # 4FC9\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78C7F6\n \
    orw    %%cx,%%cx\n \
    jle     loc_78CAA2\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78C7F1\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78C7F1:            # 4D09\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78C81E\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78C7F6:            # 4CD3\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78C804\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78C804:            # 4D1C\n \
    subw    %%ax,%%cx\n \
    jle     loc_78CAA2\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78C81E:            # 4D14\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78C828:            # 4FB9\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    (%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    1(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,1(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    2(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,2(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    3(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,3(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    4(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,4(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    5(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,5(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    6(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,6(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    7(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,7(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    8(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,8(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    9(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,9(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0A(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0A(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0B(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0B(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0C(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0C(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0D(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0D(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0E(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0E(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    0x0F(%%edi),%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    %%al,0x0F(%%edi)\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78CA9E\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78C828\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78CA9E:\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78CAA2:\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78C79C\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md20:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    movl    0x30+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x1C+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78CAD6:            # 5442\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78CB41\n \
    orw    %%cx,%%cx\n \
    jle     loc_78CF1B\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78CB06\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78CB06:            # 501E\n \
    movl    %%ecx,0x14+%0\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    movl    %%eax,%%ecx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    imull    0x30+%0,%%ecx\n \
    addl    0x10(%%esi),%%ecx\n \
    roll    $0x10,%%ecx\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78CB73\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78CB41:            # 500D\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78CB4F\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78CB4F:            # 5067\n \
    subw    %%ax,%%cx\n \
    jle     loc_78CF1B\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
    movl    %%ecx,0x14+%0\n \
    movl    0x10(%%esi),%%ecx\n \
    roll    $0x10,%%ecx\n \
\n \
loc_78CB73:            # 505F\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78CB7D:            # 5432\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    (%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    1(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,1(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    2(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,2(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    3(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,3(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    4(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,4(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    5(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,5(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    6(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,6(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    7(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,7(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    8(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,8(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    9(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,9(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0A(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0A(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0B(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0B(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0C(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0C(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0D(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0D(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0E(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0E(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0F(%%edi),%%al\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0F(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78CF17\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78CB7D\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78CF17:            # 50CF trig_+5109 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78CF1B:            # 5012 trig_+5072\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78CAD6\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md21:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    movl    0x30+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x1C+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78CF4F:            # 58BB\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78CFBA\n \
    orw    %%cx,%%cx\n \
    jle     loc_78D394\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78CF7F\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78CF7F:            # 5497\n \
    movl    %%ecx,0x14+%0\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    movl    %%eax,%%ecx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    imull    0x30+%0,%%ecx\n \
    addl    0x10(%%esi),%%ecx\n \
    roll    $0x10,%%ecx\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78CFEC\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78CFBA:            # 5486\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78CFC8\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78CFC8:            # 54E0\n \
    subw    %%ax,%%cx\n \
    jle     loc_78D394\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
    movl    %%ecx,0x14+%0\n \
    movl    0x10(%%esi),%%ecx\n \
    roll    $0x10,%%ecx\n \
\n \
loc_78CFEC:            # 54D8\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78CFF6:            # 58AB\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    (%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    1(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,1(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    2(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,2(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    3(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,3(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    4(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,4(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    5(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,5(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    6(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,6(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    7(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,7(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    8(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,8(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    9(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,9(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0A(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0A(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0B(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0B(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0C(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0C(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0D(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0D(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0E(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0E(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    movb    (%%ebx,%%esi),%%al\n \
    addw    0x48+%0,%%dx\n \
    movb    %%cl,%%ah\n \
    adcb    0x4a+%0,%%bl\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    addl   0x20+%0,%%edx\n \
    movb    0x0F(%%edi),%%ah\n \
    adcb    0x3e+%0,%%bh\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    addl    0x1C+%0,%%ecx\n \
    movb    %%al,0x0F(%%edi)\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78D390\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78CFF6\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78D390:            # 5548 trig_+5582 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78D394:            # 548B trig_+54EB\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78CF4F\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md22:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78D3BD:            # 5C2E\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78D417\n \
    orw    %%cx,%%cx\n \
    jle     loc_78D707\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78D412\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78D412:            # 592A\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78D43F\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78D417:            # 58F4\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78D425\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78D425:            # 593D\n \
    subw    %%ax,%%cx\n \
    jle     loc_78D707\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78D43F:            # 5935\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78D449:            # 5C1E\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D45A\n \
    movb    (%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,(%%edi)\n \
\n \
loc_78D45A:            # 596E\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D486\n \
    movb    1(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_78D486:            # 5998\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D4B2\n \
    movb    2(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_78D4B2:            # 59C4\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D4DE\n \
    movb    3(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_78D4DE:            # 59F0\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D50A\n \
    movb    4(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_78D50A:            # 5A1C\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D536\n \
    movb    5(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_78D536:            # 5A48\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D562\n \
    movb    6(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_78D562:            # 5A74\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D58E\n \
    movb    7(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_78D58E:            # 5AA0\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D5BA\n \
    movb    8(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_78D5BA:            # 5ACC\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D5E6\n \
    movb    9(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_78D5E6:            # 5AF8\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D612\n \
    movb    0x0A(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_78D612:            # 5B24\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D63E\n \
    movb    0x0B(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_78D63E:            # 5B50\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D66A\n \
    movb    0x0C(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_78D66A:            # 5B7C\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D696\n \
    movb    0x0D(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_78D696:            # 5BA8\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D6BE\n \
    movb    0x0E(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_78D6BE:            # 5BD0\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    movb    (%%ebx,%%esi),%%ah\n \
    orb    %%ah,%%ah\n \
    jz     loc_78D6E6\n \
    movb    0x0F(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0F(%%edi)\n \
\n \
loc_78D6E6:            # 5BF8\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78D703\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78D449\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78D703:            # 598D trig_+59B9 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78D707:            # 58F9 trig_+5948\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78D3BD\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md23:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78D730:            # 5FA1\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78D78A\n \
    orw    %%cx,%%cx\n \
    jle     loc_78DA7A\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78D785\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78D785:            # 5C9D\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78D7B2\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78D78A:            # 5C67\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78D798\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78D798:            # 5CB0\n \
    subw    %%ax,%%cx\n \
    jle     loc_78DA7A\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
\n \
loc_78D7B2:            # 5CA8\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78D7BC:            # 5F91\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D7CD\n \
    movb    (%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,(%%edi)\n \
\n \
loc_78D7CD:            # 5CE1\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D7F9\n \
    movb    1(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_78D7F9:            # 5D0B\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D825\n \
    movb    2(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_78D825:            # 5D37\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D851\n \
    movb    3(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_78D851:            # 5D63\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D87D\n \
    movb    4(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_78D87D:            # 5D8F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D8A9\n \
    movb    5(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_78D8A9:            # 5DBB\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D8D5\n \
    movb    6(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_78D8D5:            # 5DE7\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D901\n \
    movb    7(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_78D901:            # 5E13\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D92D\n \
    movb    8(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_78D92D:            # 5E3F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D959\n \
    movb    9(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_78D959:            # 5E6B\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D985\n \
    movb    0x0A(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_78D985:            # 5E97\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D9B1\n \
    movb    0x0B(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_78D9B1:            # 5EC3\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78D9DD\n \
    movb    0x0C(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_78D9DD:            # 5EEF\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DA09\n \
    movb    0x0D(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_78DA09:            # 5F1B\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DA31\n \
    movb    0x0E(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_78DA31:            # 5F43\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DA59\n \
    movb    0x0F(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0F(%%edi)\n \
\n \
loc_78DA59:            # 5F6B\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    decw    %%cx\n \
    jz     loc_78DA76\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78D7BC\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78DA76:            # 5D00 trig_+5D2C ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78DA7A:            # 5C6C trig_+5CBB\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78D730\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md24:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    movl    0x30+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x1C+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78DAAE:            # 645A\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78DB19\n \
    orw    %%cx,%%cx\n \
    jle     loc_78DF33\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78DADE\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78DADE:            # 5FF6\n \
    movl    %%ecx,0x14+%0\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    movl    %%eax,%%ecx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    imull    0x30+%0,%%ecx\n \
    addl    0x10(%%esi),%%ecx\n \
    roll    $0x10,%%ecx\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78DB4B\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78DB19:            # 5FE5\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78DB27\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78DB27:            # 603F\n \
    subw    %%ax,%%cx\n \
    jle     loc_78DF33\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
    movl    %%ecx,0x14+%0\n \
    movl    0x10(%%esi),%%ecx\n \
    roll    $0x10,%%ecx\n \
\n \
loc_78DB4B:            # 6037\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78DB55:            # 644A\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DB6E\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    (%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,(%%edi)\n \
\n \
loc_78DB6E:            # 607A\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DBAC\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    1(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_78DBAC:            # 60B6\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DBEA\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    2(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_78DBEA:            # 60F4\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DC28\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    3(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_78DC28:            # 6132\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DC66\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    4(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_78DC66:            # 6170\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DCA4\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    5(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_78DCA4:            # 61AE\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DCE2\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    6(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_78DCE2:            # 61EC\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DD20\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    7(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_78DD20:            # 622A\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DD5E\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    8(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_78DD5E:            # 6268\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DD9C\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    9(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_78DD9C:            # 62A6\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DDDA\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    0x0A(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_78DDDA:            # 62E4\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DE18\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    0x0B(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_78DE18:            # 6322\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DE56\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    0x0C(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_78DE56:            # 6360\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DE94\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    0x0D(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_78DE94:            # 639E\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DECE\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    0x0E(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_78DECE:            # 63D8\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78DF08\n \
    movb    %%cl,%%ah\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%ah\n \
    popl    %%ebx\n \
    movb    0x0F(%%edi),%%al\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0F(%%edi)\n \
\n \
loc_78DF08:            # 6412\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78DF2F\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78DB55\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78DF2F:            # 60AB trig_+60E9 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78DF33:            # 5FEA trig_+604A\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78DAAE\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md25:\n \
    leal    _polyscans,%%esi\n \
    movl    0x3C+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x20+%0\n \
    movl    0x30+%0,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,0x1C+%0\n \
    xorl    %%eax,%%eax\n \
    xorl    %%ebx,%%ebx\n \
    xorl    %%ecx,%%ecx\n \
\n \
loc_78DF67:            # 6913\n \
    movw    2(%%esi),%%ax\n \
    movzwl   6(%%esi),%%ecx\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orw    %%ax,%%ax\n \
    jns     loc_78DFD2\n \
    orw    %%cx,%%cx\n \
    jle     loc_78E3EC\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78DF97\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78DF97:            # 64AF\n \
    movl    %%ecx,0x14+%0\n \
    negw    %%ax\n \
    movzwl    %%ax,%%eax\n \
    movl    %%eax,%%edx\n \
    movl    %%eax,%%ecx\n \
    imull   0x3C+%0,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    imull    0x48+%0,%%eax\n \
    addl    8(%%esi),%%eax\n \
    movw    %%ax,%%dx\n \
    shrl    $8,%%eax\n \
    movb    %%ah,%%bl\n \
    imull    0x30+%0,%%ecx\n \
    addl    0x10(%%esi),%%ecx\n \
    roll    $0x10,%%ecx\n \
    movzwl    %%ax,%%eax\n \
    jmp     loc_78E004\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78DFD2:            # 649E\n \
    cmpl    _LOC_vec_window_width,%%ecx\n \
    jle     loc_78DFE0\n \
    movl    _LOC_vec_window_width,%%ecx\n \
\n \
loc_78DFE0:            # 64F8\n \
    subw    %%ax,%%cx\n \
    jle     loc_78E3EC\n \
    addl    %%eax,%%edi\n \
    movl   0x0C(%%esi),%%edx\n \
    rol    $0x10,%%edx\n \
    movb    %%dl,%%bh\n \
    movw    8(%%esi),%%dx\n \
    movb    0x0A(%%esi),%%bl\n \
    movl    %%ecx,0x14+%0\n \
    movl    0x10(%%esi),%%ecx\n \
    roll    $0x10,%%ecx\n \
\n \
loc_78E004:            # 64F0\n \
    movl    %%esi,0x10+%0\n \
    movl    _LOC_vec_map,%%esi\n \
\n \
loc_78E00E:            # 6903\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E027\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    (%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,(%%edi)\n \
\n \
loc_78E027:            # 6533\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E065\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    1(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_78E065:            # 656F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E0A3\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    2(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_78E0A3:            # 65AD\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E0E1\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    3(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_78E0E1:            # 65EB\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E11F\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    4(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_78E11F:            # 6629\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E15D\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    5(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_78E15D:            # 6667\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E19B\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    6(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_78E19B:            # 66A5\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E1D9\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    7(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_78E1D9:            # 66E3\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E217\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    8(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_78E217:            # 6721\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E255\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    9(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_78E255:            # 675F\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E293\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0A(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_78E293:            # 679D\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E2D1\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0B(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_78E2D1:            # 67DB\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E30F\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0C(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_78E30F:            # 6819\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E34D\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0D(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_78E34D:            # 6857\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E387\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0E(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_78E387:            # 6891\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    movb    (%%ebx,%%esi),%%al\n \
    orb    %%al,%%al\n \
    jz     loc_78E3C1\n \
    movb    %%cl,%%ah\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0F(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0F(%%edi)\n \
\n \
loc_78E3C1:            # 68CB\n \
    addw    0x48+%0,%%dx\n \
    adcb    0x4a+%0,%%bl\n \
    addl   0x20+%0,%%edx\n \
    adcb    0x3e+%0,%%bh\n \
    addl    0x1C+%0,%%ecx\n \
    adcb    0x32+%0,%%cl\n \
    decl   0x14+%0\n \
    jz     loc_78E3E8\n \
    addl    $0x10,%%edi\n \
    jmp     loc_78E00E\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78E3E8:            # 6564 trig_+65A2 ...\n \
    movl    0x10+%0,%%esi\n \
\n \
loc_78E3EC:            # 64A3 trig_+6503\n \
    addl    $0x14,%%esi\n \
    decl   0x4C+%0\n \
    jnz     loc_78DF67\n \
    jmp finished\n \
# ===========================================================================\n \
\n \
render_md26:\n \
    leal    _polyscans,%%esi\n \
    movl    %%esi,0x10+%0\n \
    xorl    %%ebx,%%ebx\n \
    pushl   %%ebp\n \
    movl    0x48+%0,%%ecx\n \
    movl    0x3C+%0,%%edx\n \
    movl    0x30+%0,%%ebp\n \
    roll    $0x10,%%ecx\n \
    rol    $0x10,%%edx\n \
    shrl    $8,%%ebp\n \
    movb    %%dl,%%bl\n \
    movb    %%cl,%%dl\n \
    movw    %%bp,%%cx\n \
    popl    %%ebp\n \
    xorb    %%dh,%%dh\n \
    movl    %%ecx,0x20+%0\n \
    movl   %%edx,0x1C+%0\n \
    movb    %%bl,8+%0\n \
\n \
loc_78E434:            # 6CAF trig_+6F98\n \
    movl    0x10+%0,%%esi\n \
    addl   $0x14,0x10+%0\n \
    movl    (%%esi),%%eax\n \
    movl    4(%%esi),%%ebp\n \
    sarl    $0x10,%%eax\n \
    sarl    $0x10,%%ebp\n \
    movl    0x6C+%0,%%edi\n \
    addl    _LOC_vec_screen_width,%%edi\n \
    movl    %%edi,0x6C+%0\n \
    orl    %%eax,%%eax\n \
    jns     loc_78E4A2\n \
    orl    %%ebp,%%ebp\n \
    jle     loc_78E78B\n \
    negl    %%eax\n \
    movl    0x48+%0,%%ecx\n \
    imull    %%eax,%%ecx\n \
    addl    8(%%esi),%%ecx\n \
    movl   0x3C+%0,%%edx\n \
    imull    %%eax,%%edx\n \
    addl   0x0C(%%esi),%%edx\n \
    movl    0x30+%0,%%ebx\n \
    imull    %%eax,%%ebx\n \
    addl    0x10(%%esi),%%ebx\n \
    roll    $0x10,%%ecx\n \
    rol    $0x10,%%edx\n \
    shrl    $8,%%ebx\n \
    movb    %%dl,%%al\n \
    movb    %%cl,%%dl\n \
    movw    %%bx,%%cx\n \
    movb    %%al,%%bh\n \
    cmpl    _LOC_vec_window_width,%%ebp\n \
    jle     loc_78E4A0\n \
    movl    _LOC_vec_window_width,%%ebp\n \
\n \
loc_78E4A0:            # 69B8\n \
    jmp     loc_78E4D5\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78E4A2:            # 6976\n \
    cmpl    _LOC_vec_window_width,%%ebp\n \
    jle     loc_78E4B0\n \
    movl    _LOC_vec_window_width,%%ebp\n \
\n \
loc_78E4B0:            # 69C8\n \
    subl    %%eax,%%ebp\n \
    jle     loc_78E78B\n \
    addl    %%eax,%%edi\n \
    movl    8(%%esi),%%ecx\n \
    movl   0x0C(%%esi),%%edx\n \
    movl    0x10(%%esi),%%ebx\n \
    roll    $0x10,%%ecx\n \
    rol    $0x10,%%edx\n \
    shrl    $8,%%ebx\n \
    movb    %%dl,%%al\n \
    movb    %%cl,%%dl\n \
    movw    %%bx,%%cx\n \
    movb    %%al,%%bh\n \
\n \
loc_78E4D5:            # CODE XREF: trig_:loc_78E4A0\n \
    xorb    %%dh,%%dh\n \
    andl    $0x0FFFF,%%ebx\n \
    movl    %%ebp,%%eax\n \
    andl    $0x0F,%%eax\n \
    addl    add_to_edi(,%%eax,4),%%edi\n \
    movl    _LOC_vec_map,%%esi\n \
    jmpl    *t12_jt(,%%eax,4)\n \
# ---------------------------------------------------------------------------\n \
t12_jt:            # DATA XREF: trig_+6A0F\n \
    .int    t12_md00\n \
    .int    t12_md01\n \
    .int    t12_md02\n \
    .int    t12_md03\n \
    .int    t12_md04\n \
    .int    t12_md05\n \
    .int    t12_md06\n \
    .int    t12_md07\n \
    .int    t12_md08\n \
    .int    t12_md09\n \
    .int    t12_md10\n \
    .int    t12_md11\n \
    .int    t12_md12\n \
    .int    t12_md13\n \
    .int    t12_md14\n \
    .int    t12_md15\n \
# ---------------------------------------------------------------------------\n \
\n \
t12_md00:\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E7B5\n \
\n \
loc_78E55B:            # 6CCF\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,(%%edi)\n \
\n \
t12_md15:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E7E0\n \
\n \
loc_78E57E:            # 6CFA\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,1(%%edi)\n \
\n \
t12_md14:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E80D\n \
\n \
loc_78E5A2:            # 6D27\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,2(%%edi)\n \
\n \
t12_md13:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E83A\n \
\n \
loc_78E5C6:            # 6D54\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,3(%%edi)\n \
\n \
t12_md12:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E867\n \
\n \
loc_78E5EA:            # 6D81\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,4(%%edi)\n \
\n \
t12_md11:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E894\n \
\n \
loc_78E60E:            # 6DAE\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,5(%%edi)\n \
\n \
t12_md10:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E8C1\n \
\n \
loc_78E632:            # 6DDB\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,6(%%edi)\n \
\n \
t12_md09:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E8EE\n \
\n \
loc_78E656:            # 6E08\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,7(%%edi)\n \
\n \
t12_md08:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E91B\n \
\n \
loc_78E67A:            # 6E35\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,8(%%edi)\n \
\n \
t12_md07:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E948\n \
\n \
loc_78E69E:            # 6E62\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,9(%%edi)\n \
\n \
t12_md06:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E975\n \
\n \
loc_78E6C2:            # 6E8F\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
t12_md05:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E9A2\n \
\n \
loc_78E6E6:            # 6EBC\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
t12_md04:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E9CF\n \
\n \
loc_78E70A:            # 6EE9\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
t12_md03:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78E9FC\n \
\n \
loc_78E72E:            # 6F16\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
t12_md02:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78EA29\n \
\n \
loc_78E752:            # 6F43\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
t12_md01:            # DATA XREF: trig_:t12_jt\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    jbe     loc_78EA56\n \
\n \
loc_78E776:            # 6F70\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    %%al,0x0F(%%edi)\n \
    addl    $0x10,%%edi\n \
    subl    $0x10,%%ebp\n \
    jg      t12_md00\n \
\n \
loc_78E78B:            # 697A trig_+69D2\n \
    decl   0x4C+%0\n \
    jnz     loc_78E434\n \
    jmp finished\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_78E79A:            # 6F8E\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E55B\n \
\n \
loc_78E7B5:            # 6A75\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    (%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E57E\n \
\n \
loc_78E7E0:            # 6A98\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    1(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,1(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E5A2\n \
\n \
loc_78E80D:            # 6ABC\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    2(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,2(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E5C6\n \
\n \
loc_78E83A:            # 6AE0\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    3(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,3(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E5EA\n \
\n \
loc_78E867:            # 6B04\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    4(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,4(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E60E\n \
\n \
loc_78E894:            # 6B28\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    5(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,5(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E632\n \
\n \
loc_78E8C1:            # 6B4C\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    6(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,6(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E656\n \
\n \
loc_78E8EE:            # 6B70\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    7(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,7(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E67A\n \
\n \
loc_78E91B:            # 6B94\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    8(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,8(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E69E\n \
\n \
loc_78E948:            # 6BB8\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    9(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,9(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E6C2\n \
\n \
loc_78E975:            # 6BDC\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0A(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0A(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E6E6\n \
\n \
loc_78E9A2:            # 6C00\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0B(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0B(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E70A\n \
\n \
loc_78E9CF:            # 6C24\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0C(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0C(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E72E\n \
\n \
loc_78E9FC:            # 6C48\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0D(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0D(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E752\n \
\n \
loc_78EA29:            # 6C6C\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0E(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0E(%%edi)\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    0x20+%0,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl   0x1C+%0,%%edx\n \
    adcb    8+%0,%%bh\n \
    cmpb    $0x0C,%%al\n \
    ja     loc_78E776\n \
\n \
loc_78EA56:            # 6C90\n \
    pushl   %%edx\n \
    movl    _render_fade_tables,%%edx\n \
    movb    (%%eax,%%edx),%%al\n \
    popl    %%edx\n \
    movb    0x0F(%%edi),%%ah\n \
    pushl   %%ebx\n \
    movl    _render_ghost,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    movb    %%al,0x0F(%%edi)\n \
    addl    $0x10,%%edi\n \
    subl    $0x10,%%ebp\n \
    jg      loc_78E79A\n \
    decl    0x4C+%0\n \
    jnz     loc_78E434\n \
finished:\n \
    popl    %%ebp\n \
"
         : "=o" (lv)
         :
         : "memory", "cc", "%eax", "%ebx", "%edx", "%ecx", "%edi", "%esi");
    //JUSTLOG("end");
}
/******************************************************************************/
