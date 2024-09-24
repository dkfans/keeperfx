/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_render_gpoly.c
 *     Rendering function draw_gpoly() for drawing 3D view elements.
 * @par Purpose:
 *     Function for rendering 3D elements.
 * @par Comment:
 *     Go away from here, you bad optimizer! Do not compile this with optimizations.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 14 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
// clang-format off
#include "pre_inc.h"
#include "bflib_render.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "post_inc.h"
// clang-format on

/******************************************************************************/
/******************************************************************************/
long gpoly_countdown[] = {0, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1};

long gpoly_reptable[] = {
    0x0,        0x7FFFFFFF, 0x3FFFFFFF, 0x2AAAAAAA, 0x1FFFFFFF, 0x19999999, 0x15555555, 0x12492492,
    0x0FFFFFFF, 0x0E38E38E, 0x0CCCCCCC, 0x0BA2E8BA, 0x0AAAAAAA, 0x9D89D89,  0x9249249,  0x8888888,
    0x7FFFFFF,  0x7878787,  0x71C71C7,  0x6BCA1AF,  0x6666666,  0x6186186,  0x5D1745D,  0x590B216,
    0x5555555,  0x51EB851,  0x4EC4EC4,  0x4BDA12F,  0x4924924,  0x469EE58,  0x4444444,  0x4210842,
    0x3FFFFFF,  0x3E0F83E,  0x3C3C3C3,  0x3A83A83,  0x38E38E3,  0x3759F22,  0x35E50D7,  0x3483483,
    0x3333333,  0x31F3831,  0x30C30C3,  0x2FA0BE8,  0x2E8BA2E,  0x2D82D82,  0x2C8590B,  0x2B93105,
    0x2AAAAAA,  0x29CBC14,  0x28F5C28,  0x2828282,  0x2762762,  0x26A439F,  0x25ED097,  0x253C825,
    0x2492492,  0x23EE08F,  0x234F72C,  0x22B63CB,  0x2222222,  0x2192E29,  0x2108421,  0x2082082,
    0x1FFFFFF,  0x1F81F81,  0x1F07C1F,  0x1E9131A,  0x1E1E1E1,  0x1DAE607,  0x1D41D41,  0x1CD8568,
    0x1C71C71,  0x1C0E070,  0x1BACF91,  0x1B4E81B,  0x1AF286B,  0x1A98EF6,  0x1A41A41,  0x19EC8E9,
    0x1999999,  0x1948B0F,  0x18F9C18,  0x18ACB90,  0x1861861,  0x1818181,  0x17D05F4,  0x178A4C8,
    0x1745D17,  0x1702E05,  0x16C16C1,  0x1681681,  0x1642C85,  0x1605816,  0x15C9882,  0x158ED23,
    0x1555555,  0x151D07E,  0x14E5E0A,  0x14AFD6A,  0x147AE14,  0x1446F86,  0x1414141,  0x13E22CB,
    0x13B13B1,  0x1381381,  0x13521CF,  0x1323E34,  0x12F684B,  0x12C9FB4,  0x129E412,  0x127350B,
    0x1249249,  0x121FB78,  0x11F7047,  0x11CF06A,  0x11A7B96,  0x1181181,  0x115B1E5,  0x1135C81,
    0x1111111,  0x10ECF56,  0x10C9714,  0x10A6810,  0x1084210,  0x10624DD,  0x1041041,  0x1020408,
    0x0FFFFFF,  0x0FE03F8,  0x0FC0FC0,  0x0FA232C,  0x0F83E0F,  0x0F6603D,  0x0F4898D,  0x0F2B9D6,
    0x0F0F0F0,  0x0EF2EB7,  0x0ED7303,  0x0EBBDB2,  0x0EA0EA0,  0x0E865AC,  0x0E6C2B4,  0x0E52598,
    0x0E38E38,  0x0E1FC78,  0x0E07038,  0x0DEE95C,  0x0DD67C8,  0x0DBEB61,  0x0DA740D,  0x0D901B2,
    0x0D79435,  0x0D62B80,  0x0D4C77B,  0x0D3680D,  0x0D20D20,  0x0D0B69F,  0x0CF6474,  0x0CE168A,
    0x0CCCCCC,  0x0CB8727,  0x0CA4587,  0x0C907DA,  0x0C7CE0C,  0x0C6980C,  0x0C565C8,  0x0C4372F,
    0x0C30C30,  0x0C1E4BB,  0x0C0C0C0,  0x0BFA02F,  0x0BE82FA,  0x0BD6910,  0x0BC5264,  0x0BB3EE7,
    0x0BA2E8B,  0x0B92143,  0x0B81702,  0x0B70FBB,  0x0B60B60,  0x0B509E6,  0x0B40B40,  0x0B30F63,
    0x0B21642,  0x0B11FD3,  0x0B02C0B,  0x0AF3ADD,  0x0AE4C41,  0x0AD602B,  0x0AC7691,  0x0AB8F69,
    0x0AAAAAA,  0x0A9C84A,  0x0A8E83F,  0x0A80A80,  0x0A72F05,  0x0A655C4,  0x0A57EB5,  0x0A4A9CF,
    0x0A3D70A,  0x0A3065E,  0x0A237C3,  0x0A16B31,  0x0A0A0A0,  0x09FD809,  0x09F1165,  0x09E4CAD,
    0x09D89D8,  0x09CC8E1,  0x09C09C0,  0x09B4C6F,  0x09A90E7,  0x099D722,  0x0991F1A,  0x09868C8,
    0x097B425,  0x097012E,  0x0964FDA,  0x095A025,  0x094F209,  0x0944580,  0x0939A85,  0x092F113,
    0x0924924,  0x091A2B3,  0x090FDBC,  0x0905A38,  0x08FB823,  0x08F1779,  0x08E7835,  0x08DDA52,
    0x08D3DCB,  0x08CA29C,  0x08C08C0,  0x08B7034,  0x08AD8F2,  0x08A42F8,  0x089AE40,  0x0891AC7,
    0x0888888,  0x087F780,  0x08767AB,  0x086D905,  0x0864B8A,  0x085BF37,  0x0853408,  0x084A9F9,
    0x0842108,  0x0839930,  0x083126E,  0x0828CBF,  0x0820820,  0x081848D,  0x0810204,  0x0808080,
    0x0,        0x0};

long gpoly_divtable[][64] = {
    {
        -8388607, -8388607, -8388607, -8388607, -8388607, -8388607, -8388607, -8388607,
        -8388607, -8388607, -8388607, -8388607, -8388607, -8388607, -8388607, -8388607,
        -8388607, -8388607, -8388607, -8388607, -8388607, -8388607, -8388607, -8388607,
        -8388607, -8388607, -8388607, -8388607, -8388607, -8388607, -8388607, -8388607,
        0,        8388607,  8388607,  8388607,  8388607,  8388607,  8388607,  8388607,
        8388607,  8388607,  8388607,  8388607,  8388607,  8388607,  8388607,  8388607,
        8388607,  8388607,  8388607,  8388607,  8388607,  8388607,  8388607,  8388607,
        8388607,  8388607,  8388607,  8388607,  8388607,  8388607,  8388607,  8388607,
    },
    {
        -2097152, -2031616, -1966080, -1900544, -1835008, -1769472, -1703936, -1638400,
        -1572864, -1507328, -1441792, -1376256, -1310720, -1245184, -1179648, -1114112,
        -1048576, -983040,  -917504,  -851968,  -786432,  -720896,  -655360,  -589824,
        -524288,  -458752,  -393216,  -327680,  -262144,  -196608,  -131072,  -65536,
        0,        65536,    131072,   196608,   262144,   327680,   393216,   458752,
        524288,   589824,   655360,   720896,   786432,   851968,   917504,   983040,
        1048576,  1114112,  1179648,  1245184,  1310720,  1376256,  1441792,  1507328,
        1572864,  1638400,  1703936,  1769472,  1835008,  1900544,  1966080,  2031616,
    },
    {
        -1048576, -1015808, -983040, -950272, -917504, -884736, -851968, -819200, -786432, -753664,
        -720896,  -688128,  -655360, -622592, -589824, -557056, -524288, -491520, -458752, -425984,
        -393216,  -360448,  -327680, -294912, -262144, -229376, -196608, -163840, -131072, -98304,
        -65536,   -32768,   0,       32768,   65536,   98304,   131072,  163840,  196608,  229376,
        262144,   294912,   327680,  360448,  393216,  425984,  458752,  491520,  524288,  557056,
        589824,   622592,   655360,  688128,  720896,  753664,  786432,  819200,  851968,  884736,
        917504,   950272,   983040,  1015808,
    },
    {
        -699050, -677205, -655360, -633514, -611669, -589824, -567978, -546133, -524288, -502442,
        -480597, -458752, -436906, -415061, -393216, -371370, -349525, -327680, -305834, -283989,
        -262144, -240298, -218453, -196608, -174762, -152917, -131072, -109226, -87381,  -65536,
        -43690,  -21845,  0,       21845,   43690,   65536,   87381,   109226,  131072,  152917,
        174762,  196608,  218453,  240298,  262144,  283989,  305834,  327680,  349525,  371370,
        393216,  415061,  436906,  458752,  480597,  502442,  524288,  546133,  567978,  589824,
        611669,  633514,  655360,  677205,
    },
    {
        -524288, -507904, -491520, -475136, -458752, -442368, -425984, -409600, -393216, -376832,
        -360448, -344064, -327680, -311296, -294912, -278528, -262144, -245760, -229376, -212992,
        -196608, -180224, -163840, -147456, -131072, -114688, -98304,  -81920,  -65536,  -49152,
        -32768,  -16384,  0,       16384,   32768,   49152,   65536,   81920,   98304,   114688,
        131072,  147456,  163840,  180224,  196608,  212992,  229376,  245760,  262144,  278528,
        294912,  311296,  327680,  344064,  360448,  376832,  393216,  409600,  425984,  442368,
        458752,  475136,  491520,  507904,
    },
    {
        -419430, -406323, -393216, -380108, -367001, -353894, -340787, -327680, -314572, -301465,
        -288358, -275251, -262144, -249036, -235929, -222822, -209715, -196608, -183500, -170393,
        -157286, -144179, -131072, -117964, -104857, -91750,  -78643,  -65536,  -52428,  -39321,
        -26214,  -13107,  0,       13107,   26214,   39321,   52428,   65536,   78643,   91750,
        104857,  117964,  131072,  144179,  157286,  170393,  183500,  196608,  209715,  222822,
        235929,  249036,  262144,  275251,  288358,  301465,  314572,  327680,  340787,  353894,
        367001,  380108,  393216,  406323,
    },
    {
        -349525, -338602, -327680, -316757, -305834, -294912, -283989, -273066, -262144, -251221,
        -240298, -229376, -218453, -207530, -196608, -185685, -174762, -163840, -152917, -141994,
        -131072, -120149, -109226, -98304,  -87381,  -76458,  -65536,  -54613,  -43690,  -32768,
        -21845,  -10922,  0,       10922,   21845,   32768,   43690,   54613,   65536,   76458,
        87381,   98304,   109226,  120149,  131072,  141994,  152917,  163840,  174762,  185685,
        196608,  207530,  218453,  229376,  240298,  251221,  262144,  273066,  283989,  294912,
        305834,  316757,  327680,  338602,
    },
    {
        -299593, -290230, -280868, -271506, -262144, -252781, -243419, -234057, -224694, -215332,
        -205970, -196608, -187245, -177883, -168521, -159158, -149796, -140434, -131072, -121709,
        -112347, -102985, -93622,  -84260,  -74898,  -65536,  -56173,  -46811,  -37449,  -28086,
        -18724,  -9362,   0,       9362,    18724,   28086,   37449,   46811,   56173,   65536,
        74898,   84260,   93622,   102985,  112347,  121709,  131072,  140434,  149796,  159158,
        168521,  177883,  187245,  196608,  205970,  215332,  224694,  234057,  243419,  252781,
        262144,  271506,  280868,  290230,
    },
    {
        -262144, -253952, -245760, -237568, -229376, -221184, -212992, -204800, -196608, -188416,
        -180224, -172032, -163840, -155648, -147456, -139264, -131072, -122880, -114688, -106496,
        -98304,  -90112,  -81920,  -73728,  -65536,  -57344,  -49152,  -40960,  -32768,  -24576,
        -16384,  -8192,   0,       8192,    16384,   24576,   32768,   40960,   49152,   57344,
        65536,   73728,   81920,   90112,   98304,   106496,  114688,  122880,  131072,  139264,
        147456,  155648,  163840,  172032,  180224,  188416,  196608,  204800,  212992,  221184,
        229376,  237568,  245760,  253952,
    },
    {
        -233016, -225735, -218453, -211171, -203889, -196608, -189326, -182044, -174762, -167480,
        -160199, -152917, -145635, -138353, -131072, -123790, -116508, -109226, -101944, -94663,
        -87381,  -80099,  -72817,  -65536,  -58254,  -50972,  -43690,  -36408,  -29127,  -21845,
        -14563,  -7281,   0,       7281,    14563,   21845,   29127,   36408,   43690,   50972,
        58254,   65536,   72817,   80099,   87381,   94663,   101944,  109226,  116508,  123790,
        131072,  138353,  145635,  152917,  160199,  167480,  174762,  182044,  189326,  196608,
        203889,  211171,  218453,  225735,
    },
    {
        -209715, -203161, -196608, -190054, -183500, -176947, -170393, -163840, -157286, -150732,
        -144179, -137625, -131072, -124518, -117964, -111411, -104857, -98304,  -91750,  -85196,
        -78643,  -72089,  -65536,  -58982,  -52428,  -45875,  -39321,  -32768,  -26214,  -19660,
        -13107,  -6553,   0,       6553,    13107,   19660,   26214,   32768,   39321,   45875,
        52428,   58982,   65536,   72089,   78643,   85196,   91750,   98304,   104857,  111411,
        117964,  124518,  131072,  137625,  144179,  150732,  157286,  163840,  170393,  176947,
        183500,  190054,  196608,  203161,
    },
    {
        -190650, -184692, -178734, -172776, -166818, -160861, -154903, -148945, -142987, -137029,
        -131072, -125114, -119156, -113198, -107240, -101282, -95325,  -89367,  -83409,  -77451,
        -71493,  -65536,  -59578,  -53620,  -47662,  -41704,  -35746,  -29789,  -23831,  -17873,
        -11915,  -5957,   0,       5957,    11915,   17873,   23831,   29789,   35746,   41704,
        47662,   53620,   59578,   65536,   71493,   77451,   83409,   89367,   95325,   101282,
        107240,  113198,  119156,  125114,  131072,  137029,  142987,  148945,  154903,  160861,
        166818,  172776,  178734,  184692,
    },
    {
        -174762, -169301, -163840, -158378, -152917, -147456, -141994, -136533, -131072, -125610,
        -120149, -114688, -109226, -103765, -98304,  -92842,  -87381,  -81920,  -76458,  -70997,
        -65536,  -60074,  -54613,  -49152,  -43690,  -38229,  -32768,  -27306,  -21845,  -16384,
        -10922,  -5461,   0,       5461,    10922,   16384,   21845,   27306,   32768,   38229,
        43690,   49152,   54613,   60074,   65536,   70997,   76458,   81920,   87381,   92842,
        98304,   103765,  109226,  114688,  120149,  125610,  131072,  136533,  141994,  147456,
        152917,  158378,  163840,  169301,
    },
    {
        -161319, -156278, -151236, -146195, -141154, -136113, -131072, -126030, -120989, -115948,
        -110907, -105865, -100824, -95783,  -90742,  -85700,  -80659,  -75618,  -70577,  -65536,
        -60494,  -55453,  -50412,  -45371,  -40329,  -35288,  -30247,  -25206,  -20164,  -15123,
        -10082,  -5041,   0,       5041,    10082,   15123,   20164,   25206,   30247,   35288,
        40329,   45371,   50412,   55453,   60494,   65536,   70577,   75618,   80659,   85700,
        90742,   95783,   100824,  105865,  110907,  115948,  120989,  126030,  131072,  136113,
        141154,  146195,  151236,  156278,
    },
    {
        -149796, -145115, -140434, -135753, -131072, -126390, -121709, -117028, -112347, -107666,
        -102985, -98304,  -93622,  -88941,  -84260,  -79579,  -74898,  -70217,  -65536,  -60854,
        -56173,  -51492,  -46811,  -42130,  -37449,  -32768,  -28086,  -23405,  -18724,  -14043,
        -9362,   -4681,   0,       4681,    9362,    14043,   18724,   23405,   28086,   32768,
        37449,   42130,   46811,   51492,   56173,   60854,   65536,   70217,   74898,   79579,
        84260,   88941,   93622,   98304,   102985,  107666,  112347,  117028,  121709,  126390,
        131072,  135753,  140434,  145115,
    },
    {
        -139810, -135441, -131072, -126702, -122333, -117964, -113595, -109226, -104857, -100488,
        -96119,  -91750,  -87381,  -83012,  -78643,  -74274,  -69905,  -65536,  -61166,  -56797,
        -52428,  -48059,  -43690,  -39321,  -34952,  -30583,  -26214,  -21845,  -17476,  -13107,
        -8738,   -4369,   0,       4369,    8738,    13107,   17476,   21845,   26214,   30583,
        34952,   39321,   43690,   48059,   52428,   56797,   61166,   65536,   69905,   74274,
        78643,   83012,   87381,   91750,   96119,   100488,  104857,  109226,  113595,  117964,
        122333,  126702,  131072,  135441,
    },
    {
        -131072, -126976, -122880, -118784, -114688, -110592, -106496, -102400, -98304, -94208,
        -90112,  -86016,  -81920,  -77824,  -73728,  -69632,  -65536,  -61440,  -57344, -53248,
        -49152,  -45056,  -40960,  -36864,  -32768,  -28672,  -24576,  -20480,  -16384, -12288,
        -8192,   -4096,   0,       4096,    8192,    12288,   16384,   20480,   24576,  28672,
        32768,   36864,   40960,   45056,   49152,   53248,   57344,   61440,   65536,  69632,
        73728,   77824,   81920,   86016,   90112,   94208,   98304,   102400,  106496, 110592,
        114688,  118784,  122880,  126976,
    },
    {
        -123361, -119506, -115651, -111796, -107941, -104086, -100231, -96376, -92521, -88666,
        -84811,  -80956,  -77101,  -73246,  -69391,  -65536,  -61680,  -57825, -53970, -50115,
        -46260,  -42405,  -38550,  -34695,  -30840,  -26985,  -23130,  -19275, -15420, -11565,
        -7710,   -3855,   0,       3855,    7710,    11565,   15420,   19275,  23130,  26985,
        30840,   34695,   38550,   42405,   46260,   50115,   53970,   57825,  61680,  65536,
        69391,   73246,   77101,   80956,   84811,   88666,   92521,   96376,  100231, 104086,
        107941,  111796,  115651,  119506,
    },
    {
        -116508, -112867, -109226, -105585, -101944, -98304, -94663, -91022, -87381, -83740, -80099,
        -76458,  -72817,  -69176,  -65536,  -61895,  -58254, -54613, -50972, -47331, -43690, -40049,
        -36408,  -32768,  -29127,  -25486,  -21845,  -18204, -14563, -10922, -7281,  -3640,  0,
        3640,    7281,    10922,   14563,   18204,   21845,  25486,  29127,  32768,  36408,  40049,
        43690,   47331,   50972,   54613,   58254,   61895,  65536,  69176,  72817,  76458,  80099,
        83740,   87381,   91022,   94663,   98304,   101944, 105585, 109226, 112867,
    },
    {
        -110376, -106927, -103477, -100028, -96579, -93130, -89680, -86231, -82782, -79333, -75883,
        -72434,  -68985,  -65536,  -62086,  -58637, -55188, -51738, -48289, -44840, -41391, -37941,
        -34492,  -31043,  -27594,  -24144,  -20695, -17246, -13797, -10347, -6898,  -3449,  0,
        3449,    6898,    10347,   13797,   17246,  20695,  24144,  27594,  31043,  34492,  37941,
        41391,   44840,   48289,   51738,   55188,  58637,  62086,  65536,  68985,  72434,  75883,
        79333,   82782,   86231,   89680,   93130,  96579,  100028, 103477, 106927,
    },
    {
        -104857, -101580, -98304, -95027, -91750, -88473, -85196, -81920, -78643, -75366, -72089,
        -68812,  -65536,  -62259, -58982, -55705, -52428, -49152, -45875, -42598, -39321, -36044,
        -32768,  -29491,  -26214, -22937, -19660, -16384, -13107, -9830,  -6553,  -3276,  0,
        3276,    6553,    9830,   13107,  16384,  19660,  22937,  26214,  29491,  32768,  36044,
        39321,   42598,   45875,  49152,  52428,  55705,  58982,  62259,  65536,  68812,  72089,
        75366,   78643,   81920,  85196,  88473,  91750,  95027,  98304,  101580,
    },
    {
        -99864, -96743, -93622, -90502, -87381, -84260, -81139, -78019, -74898, -71777, -68656,
        -65536, -62415, -59294, -56173, -53052, -49932, -46811, -43690, -40569, -37449, -34328,
        -31207, -28086, -24966, -21845, -18724, -15603, -12483, -9362,  -6241,  -3120,  0,
        3120,   6241,   9362,   12483,  15603,  18724,  21845,  24966,  28086,  31207,  34328,
        37449,  40569,  43690,  46811,  49932,  53052,  56173,  59294,  62415,  65536,  68656,
        71777,  74898,  78019,  81139,  84260,  87381,  90502,  93622,  96743,
    },
    {
        -95325, -92346, -89367, -86388, -83409, -80430, -77451, -74472, -71493, -68514, -65536,
        -62557, -59578, -56599, -53620, -50641, -47662, -44683, -41704, -38725, -35746, -32768,
        -29789, -26810, -23831, -20852, -17873, -14894, -11915, -8936,  -5957,  -2978,  0,
        2978,   5957,   8936,   11915,  14894,  17873,  20852,  23831,  26810,  29789,  32768,
        35746,  38725,  41704,  44683,  47662,  50641,  53620,  56599,  59578,  62557,  65536,
        68514,  71493,  74472,  77451,  80430,  83409,  86388,  89367,  92346,
    },
    {
        -91180, -88331, -85481, -82632, -79782, -76933, -74084, -71234, -68385, -65536, -62686,
        -59837, -56987, -54138, -51289, -48439, -45590, -42740, -39891, -37042, -34192, -31343,
        -28493, -25644, -22795, -19945, -17096, -14246, -11397, -8548,  -5698,  -2849,  0,
        2849,   5698,   8548,   11397,  14246,  17096,  19945,  22795,  25644,  28493,  31343,
        34192,  37042,  39891,  42740,  45590,  48439,  51289,  54138,  56987,  59837,  62686,
        65536,  68385,  71234,  74084,  76933,  79782,  82632,  85481,  88331,
    },
    {
        -87381, -84650, -81920, -79189, -76458, -73728, -70997, -68266, -65536, -62805, -60074,
        -57344, -54613, -51882, -49152, -46421, -43690, -40960, -38229, -35498, -32768, -30037,
        -27306, -24576, -21845, -19114, -16384, -13653, -10922, -8192,  -5461,  -2730,  0,
        2730,   5461,   8192,   10922,  13653,  16384,  19114,  21845,  24576,  27306,  30037,
        32768,  35498,  38229,  40960,  43690,  46421,  49152,  51882,  54613,  57344,  60074,
        62805,  65536,  68266,  70997,  73728,  76458,  79189,  81920,  84650,
    },
    {
        -83886, -81264, -78643, -76021, -73400, -70778, -68157, -65536, -62914, -60293, -57671,
        -55050, -52428, -49807, -47185, -44564, -41943, -39321, -36700, -34078, -31457, -28835,
        -26214, -23592, -20971, -18350, -15728, -13107, -10485, -7864,  -5242,  -2621,  0,
        2621,   5242,   7864,   10485,  13107,  15728,  18350,  20971,  23592,  26214,  28835,
        31457,  34078,  36700,  39321,  41943,  44564,  47185,  49807,  52428,  55050,  57671,
        60293,  62914,  65536,  68157,  70778,  73400,  76021,  78643,  81264,
    },
    {
        -80659, -78139, -75618, -73097, -70577, -68056, -65536, -63015, -60494, -57974, -55453,
        -52932, -50412, -47891, -45371, -42850, -40329, -37809, -35288, -32768, -30247, -27726,
        -25206, -22685, -20164, -17644, -15123, -12603, -10082, -7561,  -5041,  -2520,  0,
        2520,   5041,   7561,   10082,  12603,  15123,  17644,  20164,  22685,  25206,  27726,
        30247,  32768,  35288,  37809,  40329,  42850,  45371,  47891,  50412,  52932,  55453,
        57974,  60494,  63015,  65536,  68056,  70577,  73097,  75618,  78139,
    },
    {
        -77672, -75245, -72817, -70390, -67963, -65536, -63108, -60681, -58254, -55826, -53399,
        -50972, -48545, -46117, -43690, -41263, -38836, -36408, -33981, -31554, -29127, -26699,
        -24272, -21845, -19418, -16990, -14563, -12136, -9709,  -7281,  -4854,  -2427,  0,
        2427,   4854,   7281,   9709,   12136,  14563,  16990,  19418,  21845,  24272,  26699,
        29127,  31554,  33981,  36408,  38836,  41263,  43690,  46117,  48545,  50972,  53399,
        55826,  58254,  60681,  63108,  65536,  67963,  70390,  72817,  75245,
    },
    {
        -74898, -72557, -70217, -67876, -65536, -63195, -60854, -58514, -56173, -53833, -51492,
        -49152, -46811, -44470, -42130, -39789, -37449, -35108, -32768, -30427, -28086, -25746,
        -23405, -21065, -18724, -16384, -14043, -11702, -9362,  -7021,  -4681,  -2340,  0,
        2340,   4681,   7021,   9362,   11702,  14043,  16384,  18724,  21065,  23405,  25746,
        28086,  30427,  32768,  35108,  37449,  39789,  42130,  44470,  46811,  49152,  51492,
        53833,  56173,  58514,  60854,  63195,  65536,  67876,  70217,  72557,
    },
    {
        -72315, -70055, -67795, -65536, -63276, -61016, -58756, -56496, -54236, -51976, -49716,
        -47457, -45197, -42937, -40677, -38417, -36157, -33897, -31638, -29378, -27118, -24858,
        -22598, -20338, -18078, -15819, -13559, -11299, -9039,  -6779,  -4519,  -2259,  0,
        2259,   4519,   6779,   9039,   11299,  13559,  15819,  18078,  20338,  22598,  24858,
        27118,  29378,  31638,  33897,  36157,  38417,  40677,  42937,  45197,  47457,  49716,
        51976,  54236,  56496,  58756,  61016,  63276,  65536,  67795,  70055,
    },
    {
        -69905, -67720, -65536, -63351, -61166, -58982, -56797, -54613, -52428, -50244, -48059,
        -45875, -43690, -41506, -39321, -37137, -34952, -32768, -30583, -28398, -26214, -24029,
        -21845, -19660, -17476, -15291, -13107, -10922, -8738,  -6553,  -4369,  -2184,  0,
        2184,   4369,   6553,   8738,   10922,  13107,  15291,  17476,  19660,  21845,  24029,
        26214,  28398,  30583,  32768,  34952,  37137,  39321,  41506,  43690,  45875,  48059,
        50244,  52428,  54613,  56797,  58982,  61166,  63351,  65536,  67720,
    },
    {
        -67650, -65536, -63421, -61307, -59193, -57079, -54965, -52851, -50737, -48623, -46509,
        -44395, -42281, -40167, -38053, -35939, -33825, -31710, -29596, -27482, -25368, -23254,
        -21140, -19026, -16912, -14798, -12684, -10570, -8456,  -6342,  -4228,  -2114,  0,
        2114,   4228,   6342,   8456,   10570,  12684,  14798,  16912,  19026,  21140,  23254,
        25368,  27482,  29596,  31710,  33825,  35939,  38053,  40167,  42281,  44395,  46509,
        48623,  50737,  52851,  54965,  57079,  59193,  61307,  63421,  65536,
    },
};

static long gpoly_pro_enable_mode_ofs;
// Static variables used only inside draw_gpoly().
// These don't really have to be global; but this helps
// in using these inside assembly code
long gpoly_mode;
long factor_ca, factor_ba, factor_cb, crease_len;
// More variables - made global temporarly to ease assembly rewriting
struct PolyPoint *gploc_point_a, *gploc_point_b, *shadeveltop;
long gploc_1A4, shadevelbottom;
short gploc_word01, gploc_word02, gploc_word03;
long mapxvelbottom, mapxveltop, mapyvelbottom, mapyveltop, gploc_180;
long gploc_pt_ay, gploc_pt_ax, gploc_pt_shax, point1shade, point1mapx, point1mapy;
long gploc_pt_by, gploc_pt_bx, gploc_pt_shbx, point2shade, point2mapx, point2mapy;
long gploc_pt_cy, gploc_pt_cx, gploc_pt_shcx, point3shade, point3mapx, point3mapy;
long gploc_12C, gploc_128, gploc_104, gploc_FC, gploc_F8, gploc_F4, gploc_E4, gploc_E0;
long gploc_D8, gploc_D4, gploc_CC, gploc_C8, gploc_C4, gploc_C0, gploc_BC, gploc_B8, gploc_B4,
    mapxhstep, mapyhstep, gploc_A7, shadehstep, gploc_A4, gploc_A0, gploc_9C;
long gploc_98, gploc_94, gploc_90, gploc_8C, gploc_88, gploc_84, gploc_80, gploc_7C, gploc_78,
    gploc_74, gploc_68, gploc_64, gploc_60;
long gploc_5C, startposshadetop, startposmapxtop, startposmapytop, startposshadebottom,
    startposmapxbottom, startposmapybottom, gploc_34, gploc_30, gploc_2C, gploc_28;
/******************************************************************************/
void gpoly_enable_pentium_pro(TbBool state) {
  SYNCMSG("Pentium Pro polygon rendering %s", state ? "on" : "off");
  if (state)
    gpoly_pro_enable_mode_ofs = (1 << 6);
  else
    gpoly_pro_enable_mode_ofs = 0;
}

void draw_gpoly_sub1a();
void draw_gpoly_sub1b();
void draw_gpoly_sub1c();
void draw_gpoly_sub2a();
void draw_gpoly_sub2b();
void draw_gpoly_sub2c();
void draw_gpoly_sub3a();
void draw_gpoly_sub3b();
void draw_gpoly_sub4();
void draw_gpoly_sub5();
void draw_gpoly_sub6();
void draw_gpoly_sub7();
void draw_gpoly_sub11();
void draw_gpoly_sub12();
void draw_gpoly_sub13();
void draw_gpoly_sub14();

void draw_gpoly(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c) {
  LOC_poly_screen = poly_screen;
  LOC_vec_map = vec_map;
  LOC_vec_screen = vec_screen;
  LOC_vec_screen_width = vec_screen_width;
  LOC_vec_window_width = vec_window_width;
  LOC_vec_window_height = vec_window_height;
  gpoly_mode = gpoly_pro_enable_mode_ofs + vec_mode;
  {  // Check for outranged poly size
    // test lengths
    int len_bc_x = point_b->X - point_c->X;
    if ((len_bc_x < -16383) || (len_bc_x > 16383)) return;
    int len_bc_y = point_b->Y - point_c->Y;
    if ((len_bc_y < -16383) || (len_bc_y > 16383)) return;
    int len_ba_x = point_b->X - point_a->X;
    if ((len_ba_x < -16383) || (len_ba_x > 16383)) return;
    int len_ca_y = point_c->Y - point_a->Y;
    if ((len_ca_y < -16383) || (len_ca_y > 16383)) return;
    int len_ca_x = point_c->X - point_a->X;
    if ((len_ca_x < -16383) || (len_ca_x > 16383)) return;
    int len_ba_y = point_b->Y - point_a->Y;
    if ((len_ba_y < -16383) || (len_ba_y > 16383)) return;
    // test area
    if ((len_ca_x * len_ba_y) - (len_ba_x * len_ca_y) >= 0) return;
  }
  long exceeds_window = ((point_a->X | point_b->X | point_c->X) < 0)
                        || (point_a->X > vec_window_width) || (point_b->X > vec_window_width)
                        || (point_c->X > vec_window_width);
  {  // Reorder points
    int min_y = point_a->Y;
    struct PolyPoint *point_tmp;
    if (min_y > point_b->Y) {
      min_y = point_b->Y;
      point_tmp = point_a;
      point_a = point_b;
      point_b = point_tmp;
    }
    if (min_y > point_c->Y) {
      point_tmp = point_a;
      point_a = point_c;
      point_c = point_tmp;
    }
    if (point_b->Y > point_c->Y) {
      point_tmp = point_b;
      point_b = point_c;
      point_c = point_tmp;
    }
  }
  // Check if y coord is same for all of them
  if (point_a->Y == point_c->Y) return;
  {
    long len_y = point_c->Y - point_a->Y;
    long len_x = point_c->X - point_a->X;
    if (len_y != 0) {
      if ((len_y < 0) || (len_y > 31) || (len_x < -32) || (len_x > 31))
        factor_ca = (len_x << 16) / len_y;
      else
        factor_ca = gpoly_divtable[len_y][len_x + 32];
    } else {
      if (len_x < -32)
        factor_ca = gpoly_divtable[len_y][-32 + 32];
      else if (len_x > 31)
        factor_ca = gpoly_divtable[len_y][31 + 32];
      else
        factor_ca = gpoly_divtable[len_y][len_x + 32];
    }
    len_y = point_b->Y - point_a->Y;
    len_x = point_b->X - point_a->X;
    if (len_y != 0) {
      if ((len_y < 0) || (len_y > 31) || (len_x < -32) || (len_x > 31))
        factor_ba = (len_x << 16) / len_y;
      else
        factor_ba = gpoly_divtable[len_y][len_x + 32];
    } else {
      if (len_x < -32)
        factor_ba = gpoly_divtable[len_y][-32 + 32];
      else if (len_x > 31)
        factor_ba = gpoly_divtable[len_y][31 + 32];
      else
        factor_ba = gpoly_divtable[len_y][len_x + 32];
    }
    len_y = point_c->Y - point_b->Y;
    len_x = point_c->X - point_b->X;
    if (len_y != 0) {
      if ((len_y < 0) || (len_y > 31) || (len_x < -32) || (len_x > 31))
        factor_cb = (len_x << 16) / len_y;
      else
        factor_cb = gpoly_divtable[len_y][len_x + 32];
    } else {
      if (len_x < -32)
        factor_cb = gpoly_divtable[len_y][-32 + 32];
      else if (len_x > 31)
        factor_cb = gpoly_divtable[len_y][31 + 32];
      else
        factor_cb = gpoly_divtable[len_y][len_x + 32];
    }
    len_x = (point_a->X << 16) - (point_b->X << 16);
    len_y = (point_b->Y - point_a->Y);
    crease_len = len_y * factor_ca + len_x;
  }

  gploc_pt_ax = point_a->X;
  gploc_pt_ay = point_a->Y;
  gploc_pt_shax = point_a->X << 16;
  gploc_pt_bx = point_b->X;
  gploc_pt_by = point_b->Y;
  gploc_pt_shbx = point_b->X << 16;
  gploc_pt_cx = point_c->X;
  gploc_pt_cy = point_c->Y;
  gploc_pt_shcx = point_c->X << 16;
  point1shade = point_a->S >> 16;
  point2shade = point_b->S >> 16;
  point3shade = point_c->S >> 16;
  point1mapx = point_a->U >> 16;
  point1mapy = point_a->V >> 16;
  point2mapx = point_b->U >> 16;
  point2mapy = point_b->V >> 16;
  point3mapx = point_c->U >> 16;
  point3mapy = point_c->V >> 16;

  gploc_point_a = point_a;
  gploc_point_b = point_b;
  shadeveltop = point_c;

  switch (gpoly_mode) {
    case 27:
    case 29:
    case 31:
    case 32:
    case 33:
    case 34:
    case 37:
    case 38:
    case 91:
    case 93:
    case 95:
    case 96:
    case 97:
    case 98:
    case 101:
    case 102:
      draw_gpoly_sub1a();
      draw_gpoly_sub1b();
      draw_gpoly_sub1c();
      break;
    case 28:
    case 30:
    case 35:
    case 36:
    case 39:
    case 40:
    case 92:
    case 94:
    case 99:
    case 100:
    case 103:
    case 104:
      draw_gpoly_sub2a();
      draw_gpoly_sub2b();
      draw_gpoly_sub2c();
      break;
    case 4:
    case 16:
    case 17:
    case 68:
    case 80:
    case 81:
      draw_gpoly_sub3a();
      draw_gpoly_sub3b();
      break;
    case 3:
    case 12:
    case 13:
    case 18:
    case 19:
    case 22:
    case 23:
    case 67:
    case 76:
    case 77:
    case 82:
    case 83:
    case 86:
    case 87:
      draw_gpoly_sub4();
      break;
    case 2:
    case 66:
      draw_gpoly_sub5();
      break;
    case 5:
    case 6:
    case 20:
    case 21:
    case 24:
    case 25:
    case 70:
    case 84:
    case 85:
    case 88:
    case 89:
      draw_gpoly_sub6();
      break;
    case 69:
      draw_gpoly_sub7();
      break;
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
      return;
  }

  // End of first switch - now the second
  switch (gpoly_mode) {
    case 5:
      if (exceeds_window) {
        gploc_104 = LOC_vec_screen_width;
        gploc_180 = 2;
        gploc_60 = gploc_1A4;
        gploc_CC = gploc_A4;
        gploc_C4 = gploc_A0;
        gploc_C8 = gploc_9C;
        if (crease_len < 0) {
          gploc_12C = factor_ca;
          gploc_128 = factor_ba;
        } else {
          gploc_12C = factor_ba;
          gploc_128 = factor_ca;
        }
        draw_gpoly_sub11();
      } else  // not exceeds_window
      {
        draw_gpoly_sub12();
      }
      break;

    case 64 + 5:  // Pentium pro index + 5
      if (exceeds_window) {
        gploc_104 = LOC_vec_screen_width;
        gploc_180 = 2;
        gploc_60 = gploc_68;
        gploc_CC = gploc_A4;
        gploc_C4 = gploc_A0;
        if (crease_len < 0) {
          gploc_12C = factor_ca;
          gploc_128 = factor_ba;

        } else {
          gploc_12C = factor_ba;
          gploc_128 = factor_ca;
        }
        draw_gpoly_sub13();
      } else  // not exceeds_window
      {
        gploc_104 = LOC_vec_screen_width;
        gploc_180 = 2;
        gploc_60 = gploc_68;
        gploc_CC = gploc_A4;
        gploc_C4 = gploc_A0;
        if (crease_len < 0) {
          gploc_12C = factor_ca;
          gploc_128 = factor_ba;
        } else {
          gploc_12C = factor_ba;
          gploc_128 = factor_ca;
        }
        draw_gpoly_sub14();
      }
      break;
  }
}

static void unk_update_gpoly1_tri8a(long *vout0, long *vout1, long vinp2, long vin0, long delta) {
  long tmp1 = (vin0 << 16);
  long tmp2 = (vin0 >> 16);
  tmp1 += (unsigned char)(vinp2);
  if ((char)(vinp2) < 0) {
    long tmp3 = (unsigned int)tmp1 < delta;
    tmp1 -= delta;
    tmp2 = (tmp2 & ~0xff) | ((tmp2 - tmp3) & 0xff);
  }
  *vout1 = tmp1;
  *vout0 = tmp2;
}

static void unk_update_gpoly1_tri16a(long *vout0, long *vout1, long vinp2, long vin0, long delta) {
  long tmp1 = (vin0 << 16);
  long tmp2 = (vin0 >> 16);
  tmp1 += (unsigned short)(vinp2);
  if ((short)(vinp2) < 0) {
    long tmp3 = (unsigned int)tmp1 < delta;
    tmp1 -= delta;
    tmp2 = (tmp2 & ~0xff) | ((tmp2 - tmp3) & 0xff);
  }
  *vout1 = tmp1;
  *vout0 = tmp2;
}

static void unk_update_gpoly1_tri8b(long *vout0, long *vout1, long *vout2, long vin0, long vin1) {
  *vout2 = (vin1 << 16);
  long tmp1 = (vin0 << 16);
  tmp1 += ((unsigned int)vin1 >> 16) & 0xff;
  *vout1 = tmp1;
  *vout0 = (unsigned int)(vin0 << 8) >> 24 << 8;
}

static void unk_update_gpoly1_tri16b(long *vout0, long *vout1, long *vout2, long vin0, long vin1,
                                     long vin2) {
  long tmp1 = (vin1 << 16);
  tmp1 += ((unsigned int)vin2 >> 8) & 0xffff;
  *vout2 = tmp1;
  tmp1 = (vin0 << 16);
  tmp1 += ((unsigned int)vin1 >> 16) & 0xff;
  *vout1 = tmp1;
  tmp1 = (unsigned int)(vin0 << 8) >> 24 << 8;
  tmp1 += (vin2) & 0xff;
  *vout0 = tmp1;
}

void draw_gpoly_sub1a() {
  JUSTLOG(
      "inputs: gploc_pt_bx=%d, crease_len=%d, gploc_pt_ax=%d, gploc_pt_cx=%d, "
      "gploc_pt_ay=%d, gploc_pt_by=%d, gploc_pt_cy=%d, point1mapx=%d, point3mapx=%d, point2mapx=%d",
      gploc_pt_bx, crease_len, gploc_pt_ax, gploc_pt_cx, gploc_pt_ay, gploc_pt_by, gploc_pt_cy,
      point1mapx, point3mapx, point2mapx);
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _gploc_pt_bx,%%esi\n \
    movl    _crease_len,%%edi\n \
    orl %%edi,%%edi\n \
    subl    $0,%%esi\n \
    addl    $0,%%esi\n \
    movl    _gploc_pt_ax,%%eax\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_cx,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_by,%%ebx\n \
    subl    %%eax,%%ebx\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    %%eax,%%ecx\n \
    movl    %%ecx,%%eax\n \
    imull   %%esi,%%ecx\n \
    movl    _crease_len,%%ebp\n \
    orl %%ebp,%%ebp\n \
    js  gpo_loc_0532\n \
    subl    %%eax,%%ecx\n \
    subl    %%eax,%%ecx\n \
\n \
gpo_loc_0532:         # 33C\n \
    addl    %%eax,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    jz  gpo_loc_05B8\n \
    xorl    %%edx,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ebx\n \
    movl    %%eax,%%ebp\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_cy,%%esi\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_by,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapx,%%edx\n \
    movl    _point3mapx,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapx,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_057F\n \
    incl    %%eax\n \
\n \
gpo_loc_057F:         # 38C\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapy,%%edx\n \
    movl    _point3mapy,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapy,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_05AF\n \
    incl    %%eax\n \
\n \
gpo_loc_05AF:         # 3BC\n \
    movl    %%eax,_mapyhstep\n \
    jmp gpo_loc_05C8\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_05B8:         # 34\n \
    xorl    %%eax,%%eax\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%eax,_mapyhstep\n \
\n \
gpo_loc_05C8:         # 3C6\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
  JUSTLOG("outputs: mapxhstep=%d, mapyhstep=%d", mapxhstep, mapyhstep);
}

#if 0
// Primitive IDA decompilation
void start()
{
  int v0; // eax
  int v1; // ecx
  int v2; // ebx
  bool v4; // sf
  int v5; // eax
  int v7; // eax

  v0 = gploc_pt_cy - gploc_pt_ay;
  v1 = (gploc_pt_bx - gploc_pt_ax) * (gploc_pt_cy - gploc_pt_ay);
  if ( crease_len >= 0 )
    v1 = v1 - v0 - v0;
  v2 = (gploc_pt_cx - gploc_pt_ax) * (gploc_pt_by - gploc_pt_ay) - (v0 + v1);
  if ( v2 )
  {
    _RAX = ((gploc_pt_by - gploc_pt_ay) * (point3mapx - point1mapx) - (gploc_pt_cy - gploc_pt_ay)
                                                                  * (point2mapx - point1mapx))
         * (__int64)(0x7FFFFFFF / v2);
    LODWORD(_RAX) = 2 * _RAX;
    v4 = (int)_RAX < 0;
    __asm { rcl     edx, 1 }
    LOWORD(_RAX) = WORD2(_RAX);
    v5 = __ROL4__(_RAX, 16);
    if ( v4 )
      ++v5;
    mapxhstep = v5;
    _RAX = ((gploc_pt_by - gploc_pt_ay) * (point3mapy - point1mapy) - (gploc_pt_cy - gploc_pt_ay)
                                                                  * (point2mapy - point1mapy))
         * (__int64)(0x7FFFFFFF / v2);
    LODWORD(_RAX) = 2 * _RAX;
    v4 = (int)_RAX < 0;
    __asm { rcl     edx, 1 }
    LOWORD(_RAX) = WORD2(_RAX);
    v7 = __ROL4__(_RAX, 16);
    if ( v4 )
      ++v7;
    mapyhstep = v7;
  }
  else
  {
    mapxhstep = 0;
    mapyhstep = 0;
  }
}

// GPT-4 rewrite
void start()
{
    int dy_ac = gploc_pt_cy - gploc_pt_ay; // Delta Y from A to C
    int dx_ab = gploc_pt_bx - gploc_pt_ax; // Delta X from A to B
    int dx_ac = gploc_pt_cx - gploc_pt_ax; // Delta X from A to C
    int dy_ab = gploc_pt_by - gploc_pt_ay; // Delta Y from A to B

    int delta1 = dx_ab * dy_ac;
    int adjustment = crease_len >= 0 ? -(dy_ac + dy_ac) : dy_ac; // Conditionally adjust based on crease_len
    int v2 = dx_ac * dy_ab - (delta1 + adjustment);

    if (v2 != 0)
    {
        int64_t scale_factor = 0x7FFFFFFF / v2;

        int calc1 = ((dy_ab * (point3mapx - point1mapx)) - (dy_ac * (point2mapx - point1mapx))) * scale_factor;
        calc1 *= 2;
        if(calc1 < 0) {
            calc1 += 1 << 16; // Adjusting similar to RCL and ROL instructions
        }
        mapxhstep = (calc1 >> 16) & 0xFFFF; // Accessing the higher word (ignoring potential overflow issues)

        int calc2 = ((dy_ab * (point3mapy - point1mapy)) - (dy_ac * (point2mapy - point1mapy))) * scale_factor;
        calc2 *= 2;
        if(calc2 < 0) {
            calc2 += 1 << 16;
        }
        mapyhstep = (calc2 >> 16) & 0xFFFF;
    }
    else
    {
        mapxhstep = 0;
        mapyhstep = 0;
    }
}

// Better variable naming
void calculateVertexScaleFactors()
{
    int deltaY_AC = vertex_C_Y - vertex_A_Y; // Delta Y from vertex A to C
    int deltaX_AB = vertex_B_X - vertex_A_X; // Delta X from vertex A to B
    int deltaX_AC = vertex_C_X - vertex_A_X; // Delta X from vertex A to C
    int deltaY_AB = vertex_B_Y - vertex_A_Y; // Delta Y from vertex A to B

    int edgeDifference = deltaX_AB * deltaY_AC;
    int adjustment = edgeAdjustmentFactor >= 0 ? -(deltaY_AC + deltaY_AC) : deltaY_AC;
    int determinant = deltaX_AC * deltaY_AB - (edgeDifference + adjustment);

    if (determinant != 0)
    {
        int64_t scaleFactor = 0x7FFFFFFF / determinant;

        int scaleFactorA = ((deltaY_AB * (globalValue_13C - globalValue_16C)) -
                            (deltaY_AC * (globalValue_154 - globalValue_16C))) * scaleFactor;
        scaleFactorA *= 2;
        if (scaleFactorA < 0) {
            scaleFactorA += 1 << 16; // Adjustment for scaling
        }
        globalResult_A = (scaleFactorA >> 16) & 0xFFFF; // Extracting scaled factor for vertex A

        int scaleFactorB = ((deltaY_AB * (globalValue_138 - globalValue_168)) -
                            (deltaY_AC * (globalValue_150 - globalValue_168))) * scaleFactor;
        scaleFactorB *= 2;
        if (scaleFactorB < 0) {
            scaleFactorB += 1 << 16; // Adjustment for scaling
        }
        globalResult_B = (scaleFactorB >> 16) & 0xFFFF; // Extracting scaled factor for vertex B
    }
    else
    {
        globalResult_A = 0;
        globalResult_B = 0;
    }
}
#endif

void draw_gpoly_sub1b() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_loc_0715\n \
    movl    _gploc_pt_by,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_05ED\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_05FB\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_05ED:         # 3F2\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_05FB:         # 3FB\n \
    movl    _point2mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0610\n \
    incl    %%eax\n \
\n \
gpo_loc_0610:         # 41D\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point2mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0629\n \
    incl    %%eax\n \
\n \
gpo_loc_0629:         # 436\n \
    movl    %%eax,_mapyveltop\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_by,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_0646\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_0654\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_0646:         # 44B\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_0654:         # 45\n \
    movl    _point3mapx,%%eax\n \
    subl    _point2mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0669\n \
    incl    %%eax\n \
\n \
gpo_loc_0669:         # 476\n \
    movl    %%eax,_mapxvelbottom\n \
    movl    _point3mapy,%%eax\n \
    subl    _point2mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0682\n \
    incl    %%eax\n \
\n \
gpo_loc_0682:         # 48\n \
    movl    %%eax,_mapyvelbottom\n \
    movl    _gploc_point_a,%%eax\n \
    movl    _mapxhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapxveltop\n \
    movl    _gploc_point_a,%%eax\n \
    movl    _mapyhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapyveltop\n \
    movl    _gploc_point_a,%%eax\n \
    movl    _shadehstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_shadeveltop\n \
    movl    _gploc_point_b,%%eax\n \
    movl    _mapxhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapxvelbottom\n \
    movl    _gploc_point_b,%%eax\n \
    movl    _mapyhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapyvelbottom\n \
    movl    _gploc_point_b,%%eax\n \
    movl    _shadehstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_shadevelbottom\n \
    jmp gpo_loc_07B0\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_0715:         # 3DE\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_072E\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_073C\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_072E:         # 533\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_073C:         # 53C\n \
    movl    _point3mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0751\n \
    incl    %%eax\n \
\n \
gpo_loc_0751:         # 55E\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point3mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_076A\n \
    incl    %%eax\n \
\n \
gpo_loc_076A:         # 577\n \
    movl    %%eax,_mapyveltop\n \
    movl    _factor_ca,%%eax\n \
    movl    _mapxhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapxveltop\n \
    movl    _factor_ca,%%eax\n \
    movl    _mapyhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapyveltop\n \
    movl    _factor_ca,%%eax\n \
    movl    _shadehstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_shadeveltop\n \
\n \
gpo_loc_07B0:         # 520\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

#if 0
// IDA decompilation
void start()
{
  int v0; // ecx
  int v1; // ebx
  char v2; // sf
  int v3; // eax
  int v4; // eax
  int v5; // eax
  int v6; // eax
  int v7; // ecx
  int v8; // ebx
  int v9; // eax
  int v10; // eax
  int v11; // eax
  int v12; // eax
  int v13; // eax
  int v14; // ecx
  int v15; // ebx
  int v16; // eax
  int v17; // eax
  int v18; // eax
  int v19; // eax
  int v20; // eax

  if ( crease_len < 0 )
  {
    v14 = gploc_pt_cy - gploc_pt_ay;
    if ( gploc_pt_cy - gploc_pt_ay > 255 )
      v15 = 0x7FFFFFFF / v14;
    else
      v15 = gpoly_reptable[v14];
    HIWORD(v16) = (unsigned int)(v15 * 2 * (point3mapx - point1mapx)) >> 16;
    LOWORD(v16) = (unsigned __int64)(v15 * (__int64)(2 * (point3mapx - point1mapx))) >> 32;
    v17 = __ROL4__(v16, 16);
    if ( v2 )
      ++v17;
    mapxveltop = v17;
    HIWORD(v18) = (unsigned int)(v15 * 2 * (point3mapy - point1mapy)) >> 16;
    LOWORD(v18) = (unsigned __int64)(v15 * (__int64)(2 * (point3mapy - point1mapy))) >> 32;
    v19 = __ROL4__(v18, 16);
    if ( v2 )
      ++v19;
    mapyveltop = v19;
    HIWORD(v20) = (unsigned int)(mapxhstep * factor_ca) >> 16;
    LOWORD(v20) = (unsigned __int64)(mapxhstep * (__int64)factor_ca) >> 32;
    mapxveltop -= __ROL4__(v20, 16);
    HIWORD(v20) = (unsigned int)(mapyhstep * factor_ca) >> 16;
    LOWORD(v20) = (unsigned __int64)(mapyhstep * (__int64)factor_ca) >> 32;
    mapyveltop -= __ROL4__(v20, 16);
    HIWORD(v20) = (unsigned int)(shadehstep * factor_ca) >> 16;
    LOWORD(v20) = (unsigned __int64)(shadehstep * (__int64)factor_ca) >> 32;
    shadeveltop -= __ROL4__(v20, 16);
  }
  else
  {
    v0 = gploc_pt_by - gploc_pt_ay;
    if ( gploc_pt_by - gploc_pt_ay > 255 )
      v1 = 0x7FFFFFFF / v0;
    else
      v1 = gpoly_reptable[v0];
    HIWORD(v3) = (unsigned int)(v1 * 2 * (point2mapx - point1mapx)) >> 16;
    LOWORD(v3) = (unsigned __int64)(v1 * (__int64)(2 * (point2mapx - point1mapx))) >> 32;
    v4 = __ROL4__(v3, 16);
    if ( v2 )
      ++v4;
    mapxveltop = v4;
    HIWORD(v5) = (unsigned int)(v1 * 2 * (point2mapy - point1mapy)) >> 16;
    LOWORD(v5) = (unsigned __int64)(v1 * (__int64)(2 * (point2mapy - point1mapy))) >> 32;
    v6 = __ROL4__(v5, 16);
    if ( v2 )
      ++v6;
    mapyveltop = v6;
    v7 = gploc_pt_cy - gploc_pt_by;
    if ( gploc_pt_cy - gploc_pt_by > 255 )
      v8 = 0x7FFFFFFF / v7;
    else
      v8 = gpoly_reptable[v7];
    HIWORD(v9) = (unsigned int)(v8 * 2 * (point3mapx - point2mapx)) >> 16;
    LOWORD(v9) = (unsigned __int64)(v8 * (__int64)(2 * (point3mapx - point2mapx))) >> 32;
    v10 = __ROL4__(v9, 16);
    if ( v2 )
      ++v10;
    mapxvelbottom = v10;
    HIWORD(v11) = (unsigned int)(v8 * 2 * (point3mapy - point2mapy)) >> 16;
    LOWORD(v11) = (unsigned __int64)(v8 * (__int64)(2 * (point3mapy - point2mapy))) >> 32;
    v12 = __ROL4__(v11, 16);
    if ( v2 )
      ++v12;
    mapyvelbottom = v12;
    HIWORD(v13) = (unsigned int)(mapxhstep * gploc_point_a) >> 16;
    LOWORD(v13) = (unsigned __int64)(mapxhstep * (__int64)gploc_point_a) >> 32;
    mapxveltop -= __ROL4__(v13, 16);
    HIWORD(v13) = (unsigned int)(mapyhstep * gploc_point_a) >> 16;
    LOWORD(v13) = (unsigned __int64)(mapyhstep * (__int64)gploc_point_a) >> 32;
    mapyveltop -= __ROL4__(v13, 16);
    HIWORD(v13) = (unsigned int)(shadehstep * gploc_point_a) >> 16;
    LOWORD(v13) = (unsigned __int64)(shadehstep * (__int64)gploc_point_a) >> 32;
    shadeveltop -= __ROL4__(v13, 16);
    HIWORD(v13) = (unsigned int)(mapxhstep * gploc_point_b) >> 16;
    LOWORD(v13) = (unsigned __int64)(mapxhstep * (__int64)gploc_point_b) >> 32;
    mapxvelbottom -= __ROL4__(v13, 16);
    HIWORD(v13) = (unsigned int)(mapyhstep * gploc_point_b) >> 16;
    LOWORD(v13) = (unsigned __int64)(mapyhstep * (__int64)gploc_point_b) >> 32;
    mapyvelbottom -= __ROL4__(v13, 16);
    HIWORD(v13) = (unsigned int)(shadehstep * gploc_point_b) >> 16;
    LOWORD(v13) = (unsigned __int64)(shadehstep * (__int64)gploc_point_b) >> 32;
    shadevelbottom -= __ROL4__(v13, 16);
  }
}

// GPT-4 refactor
void draw_gpoly_sub1b()
{
    int delta;
    int scale_factor;
    int v16, v18, v20;

    if (crease_len < 0)
    {
        delta = gploc_pt_cy - gploc_pt_ay;
        scale_factor = (delta > 255) ? (0x7FFFFFFF / delta) : gpoly_reptable[delta];

        v16 = ((scale_factor * (point3mapx - point1mapx)) << 1) >> 16;
        mapxveltop = v16;

        v18 = ((scale_factor * (point3mapy - point1mapy)) << 1) >> 16;
        mapyveltop = v18;

        v20 = (mapxhstep * factor_ca) >> 16;
        mapxveltop -= v20;

        v20 = (mapyhstep * factor_ca) >> 16;
        mapyveltop -= v20;

        v20 = (shadehstep * factor_ca) >> 16;
        shadeveltop -= v20;
    }
    else
    {
        delta = gploc_pt_by - gploc_pt_ay;
        scale_factor = (delta > 255) ? (0x7FFFFFFF / delta) : gpoly_reptable[delta];

        v16 = ((scale_factor * (point2mapx - point1mapx)) << 1) >> 16;
        mapxveltop = v16;

        v18 = ((scale_factor * (point2mapy - point1mapy)) << 1) >> 16;
        mapyveltop = v18;

        delta = gploc_pt_cy - gploc_pt_by;
        scale_factor = (delta > 255) ? (0x7FFFFFFF / delta) : gpoly_reptable[delta];

        v16 = ((scale_factor * (point3mapx - point2mapx)) << 1) >> 16;
        mapxvelbottom = v16;

        v18 = ((scale_factor * (point3mapy - point2mapy)) << 1) >> 16;
        mapyvelbottom = v18;

        v20 = (mapxhstep * gploc_point_a) >> 16;
        mapxveltop -= v20;

        v20 = (mapyhstep * gploc_point_a) >> 16;
        mapyveltop -= v20;

        v20 = (shadehstep * gploc_point_a) >> 16;
        shadeveltop -= v20;

        v20 = (mapxhstep * gploc_point_b) >> 16;
        mapxvelbottom -= v20;

        v20 = (mapyhstep * gploc_point_b) >> 16;
        mapyvelbottom -= v20;

        v20 = (shadehstep * gploc_point_b) >> 16;
        shadevelbottom -= v20;
    }
}

// Further cleanups
void draw_gpoly_adjustedScaling()
{
    int scaleDifference;
    int scaleFactor;
    int adjustedValue1, adjustedValue2, adjustedValueForC;

    if (conditionFactor < 0) // conditionFactor seems related to 'crease_len'
    {
        scaleDifference = vertexC_Y - vertexA_Y;
        scaleFactor = (scaleDifference > 255) ? (INT_MAX / scaleDifference) : repetitionScale[scaleDifference];

        adjustedValue1 = ((scaleFactor * (textureCoordC_U - textureCoordA_U)) << 1) >> 16;
        globalAdjusted_U = adjustedValue1;

        adjustedValue2 = ((scaleFactor * (textureCoordC_V - textureCoordA_V)) << 1) >> 16;
        globalAdjusted_V = adjustedValue2;

        adjustedValueForC = (precomputedValue1 * scaleConditionFactor) >> 16;
        globalAdjusted_U -= adjustedValueForC;

        adjustedValueForC = (precomputedValue2 * scaleConditionFactor) >> 16;
        globalAdjusted_V -= adjustedValueForC;

        adjustedValueForC = (precomputedValue3 * scaleConditionFactor) >> 16;
        adjustedVertexC -= adjustedValueForC;
    }
    else
    {
        scaleDifference = vertexB_Y - vertexA_Y;
        scaleFactor = (scaleDifference > 255) ? (INT_MAX / scaleDifference) : repetitionScale[scaleDifference];

        adjustedValue1 = ((scaleFactor * (textureCoordB_U - textureCoordA_U)) << 1) >> 16;
        globalAdjusted_U = adjustedValue1;

        adjustedValue2 = ((scaleFactor * (textureCoordB_V - textureCoordA_V)) << 1) >> 16;
        globalAdjusted_V = adjustedValue2;

        scaleDifference = vertexC_Y - vertexB_Y;
        scaleFactor = (scaleDifference > 255) ? (INT_MAX / scaleDifference) : repetitionScale[scaleDifference];

        adjustedValue1 = ((scaleFactor * (textureCoordC_U - textureCoordB_U)) << 1) >> 16;
        globalAdjusted_U_Next = adjustedValue1;

        adjustedValue2 = ((scaleFactor * (textureCoordC_V - textureCoordB_V)) << 1) >> 16;
        globalAdjusted_V_Next = adjustedValue2;

        // Adjust based on precomputed values and specific scaling factors
        // Pattern repeats with variants for B -> A, C -> A, including adjustedVertexC

        adjustedValueForC = (precomputedValue1 * initialPointFactor) >> 16;
        globalAdjusted_U -= adjustedValueForC;

        // Repeat adjustments for V coordinates and possibly other variables
    }
}
#endif

void draw_gpoly_sub1c() {
  startposmapxtop = (point1mapx << 16) + mapxhstep;
  startposmapytop = (point1mapy << 16) + mapyhstep;
  startposmapxbottom = (point2mapx << 16) + mapxhstep;
  startposmapybottom = (point2mapy << 16) + mapyhstep;

  gploc_BC = (mapxhstep << 16);
  gploc_B8 = (mapxhstep >> 16);
  unk_update_gpoly1_tri8a(&gploc_B4, &gploc_B8, gploc_B8, mapyhstep, 256);
  gploc_A4 = (mapxveltop << 16);
  gploc_A0 = (mapxveltop >> 16);
  unk_update_gpoly1_tri8a(&gploc_9C, &gploc_A0, gploc_A0, mapyveltop, 256);
  unk_update_gpoly1_tri8b(&gploc_84, &gploc_88, &gploc_8C, startposmapytop, startposmapxtop);
  if (crease_len >= 0) {
    gploc_98 = (mapxvelbottom << 16);
    gploc_94 = (mapxvelbottom >> 16);
    unk_update_gpoly1_tri8a(&gploc_90, &gploc_94, gploc_94, mapyvelbottom, 256);
    unk_update_gpoly1_tri8b(&gploc_78, &gploc_7C, &gploc_80, startposmapybottom,
                            startposmapxbottom);
  }
}

void draw_gpoly_sub2a() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _gploc_pt_bx,%%esi\n \
    movl    _crease_len,%%edi\n \
    orl %%edi,%%edi\n \
    subl    $0,%%esi\n \
    addl    $0,%%esi\n \
    movl    _gploc_pt_ax,%%eax\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_cx,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_by,%%ebx\n \
    subl    %%eax,%%ebx\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    %%eax,%%ecx\n \
    movl    %%ecx,%%eax\n \
    imull   %%esi,%%ecx\n \
    movl    _crease_len,%%ebp\n \
    orl %%ebp,%%ebp\n \
    js  gpo_loc_09AC\n \
    subl    %%eax,%%ecx\n \
    subl    %%eax,%%ecx\n \
\n \
gpo_loc_09AC:         # 7B6\n \
    addl    %%eax,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    jz  gpo_loc_0A66\n \
    xorl    %%edx,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ebx\n \
    movl    %%eax,%%ebp\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_cy,%%esi\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_by,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    %%ebp,%%eax\n \
    movl    _point1shade,%%edx\n \
    movl    _point3shade,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2shade,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_09FD\n \
    incl    %%eax\n \
\n \
gpo_loc_09FD:         # 80A\n \
    movl    %%eax,_shadehstep\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapx,%%edx\n \
    movl    _point3mapx,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapx,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0A2D\n \
    incl    %%eax\n \
\n \
gpo_loc_0A2D:         # 83A\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapy,%%edx\n \
    movl    _point3mapy,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapy,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0A5D\n \
    incl    %%eax\n \
\n \
gpo_loc_0A5D:         # 86A\n \
    movl    %%eax,_mapyhstep\n \
    jmp gpo_loc_0A7D\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_0A66:         # 7C3\n \
    xorl    %%eax,%%eax\n \
    movl    %%eax,_shadehstep\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%eax,_mapyhstep\n \
\n \
gpo_loc_0A7D:         # 87\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

#if 0
// IDA decompilation
void draw_gpoly_sub2a()
{
  int v0; // eax
  int v1; // ecx
  int v2; // ebx
  int v3; // ebp
  int v4; // esi
  int v5; // edi
  bool v7; // sf
  int v8; // eax
  int v10; // eax
  int v12; // eax

  v0 = gploc_pt_cy - gploc_pt_ay;
  v1 = (gploc_pt_bx - gploc_pt_ax) * (gploc_pt_cy - gploc_pt_ay);
  if ( crease_len >= 0 )
    v1 = v1 - v0 - v0;
  v2 = (gploc_pt_cx - gploc_pt_ax) * (gploc_pt_by - gploc_pt_ay) - (v0 + v1);
  if ( v2 )
  {
    v3 = 0x7FFFFFFF / v2;
    v4 = gploc_pt_cy - gploc_pt_ay;
    v5 = gploc_pt_by - gploc_pt_ay;
    _RAX = ((gploc_pt_by - gploc_pt_ay) * (point3shade - point1shade) - (gploc_pt_cy - gploc_pt_ay)
                                                                  * (point2shade - point1shade))
         * (__int64)(0x7FFFFFFF / v2);
    LODWORD(_RAX) = 2 * _RAX;
    v7 = (int)_RAX < 0;
    __asm { rcl     edx, 1 }
    LOWORD(_RAX) = WORD2(_RAX);
    v8 = __ROL4__(_RAX, 16);
    if ( v7 )
      ++v8;
    shadehstep = v8;
    _RAX = (v5 * (point3mapx - point1mapx) - v4 * (point2mapx - point1mapx)) * (__int64)v3;
    LODWORD(_RAX) = 2 * _RAX;
    v7 = (int)_RAX < 0;
    __asm { rcl     edx, 1 }
    LOWORD(_RAX) = WORD2(_RAX);
    v10 = __ROL4__(_RAX, 16);
    if ( v7 )
      ++v10;
    mapxhstep = v10;
    _RAX = (v5 * (point3mapy - point1mapy) - v4 * (point2mapy - point1mapy)) * (__int64)v3;
    LODWORD(_RAX) = 2 * _RAX;
    v7 = (int)_RAX < 0;
    __asm { rcl     edx, 1 }
    LOWORD(_RAX) = WORD2(_RAX);
    v12 = __ROL4__(_RAX, 16);
    if ( v7 )
      ++v12;
    mapyhstep = v12;
  }
  else
  {
    shadehstep = 0;
    mapxhstep = 0;
    mapyhstep = 0;
  }
}

// GPT-4 refactor
void draw_gpoly_sub2a()
{
    int delta_ay_cy = gploc_pt_cy - gploc_pt_ay; // Difference in Y between points A and C
    int composite1 = (gploc_pt_bx - gploc_pt_ax) * delta_ay_cy; // Composite calculation 1
    if (crease_len >= 0)
    {
        composite1 = composite1 - delta_ay_cy - delta_ay_cy;
    }
    int composite2 = (gploc_pt_cx - gploc_pt_ax) * (gploc_pt_by - gploc_pt_ay) - (delta_ay_cy + composite1); // Composite calculation 2
    if (composite2 != 0)
    {
        int v3 = 0x7FFFFFFF / composite2;
        int delta_by_ay = gploc_pt_by - gploc_pt_ay; // Difference in Y between points B and A

        // For shadehstep
        int64_t calculation_A8 = ((int64_t)delta_by_ay * (point3shade - point1shade) - (int64_t)delta_ay_cy * (point2shade - point1shade)) * v3;
        calculation_A8 *= 2; // Doubling the result
        shadehstep = (calculation_A8 >> 16) & 0xFFFFFFFF; // Taking upper 32 bits

        // For mapxhstep
        int64_t calculation_B0 = ((int64_t)delta_by_ay * (point3mapx - point1mapx) - (int64_t)delta_ay_cy * (point2mapx - point1mapx)) * v3;
        calculation_B0 *= 2; // Doubling the result
        mapxhstep = (calculation_B0 >> 16) & 0xFFFFFFFF; // Taking upper 32 bits

        // For mapyhstep
        int64_t calculation_AC = ((int64_t)delta_by_ay * (point3mapy - point1mapy) - (int64_t)delta_ay_cy * (point2mapy - point1mapy)) * v3;
        calculation_AC *= 2; // Doubling the result
        mapyhstep = (calculation_AC >> 16) & 0xFFFFFFFF; // Taking upper 32 bits
    }
    else
    {
        shadehstep = 0;
        mapxhstep = 0;
        mapyhstep = 0;
    }
}

// More refactoring
void calculateAdjustedTextureFactors()
{
    int deltaY_AtoC = vertexC_Y - vertexA_Y; // Difference in Y between vertices A and C
    int combinedFactor1 = (vertexB_X - vertexA_X) * deltaY_AtoC; // Preliminary composite calculation 1

    // Adjust combinedFactor1 based on a condition factor determined earlier
    if (conditionFactor >= 0)
    {
        combinedFactor1 -= (deltaY_AtoC * 2);
    }

    // Further combined calculation, taking into account differences along both axes
    int combinedFactor2 = (vertexC_X - vertexA_X) * (vertexB_Y - vertexA_Y) - (deltaY_AtoC + combinedFactor1);

    if (combinedFactor2 != 0) // Ensure that the denominator in the subsequent division isn't zero
    {
        int scalingFactor = INT_MAX / combinedFactor2;  // Use a large constant to scale the division result
        int deltaY_BtoA = vertexB_Y - vertexA_Y; // Difference in Y between points B and A

        // Calculate adjusted factor for shadehstep considering the differences in Y coordinates and scaled
        int64_t calculation_A8 = ((int64_t)deltaY_BtoA * (adjustmentFactor_CtoA - adjustmentFactor_A) -
                                  (int64_t)deltaY_AtoC * (adjustmentFactor_BtoA - adjustmentFactor_A)) * scalingFactor;
        calculation_A8 *= 2; // Doubling the result
        shadehstep = (calculation_A8 >> 16) & 0xFFFFFFFF; // Retain upper 32 bits

        // Similar calculation for mapxhstep
        int64_t calculation_B0 = ((int64_t)deltaY_BtoA * (textureCoord_C_U - textureCoord_A_U) -
                                  (int64_t)deltaY_AtoC * (textureCoord_B_U - textureCoord_A_U)) * scalingFactor;
        calculation_B0 *= 2;  // Doubling the result
        mapxhstep = (calculation_B0 >> 16) & 0xFFFFFFFF; // Retain upper 32 bits

        // Similar calculation for mapyhstep
        int64_t calculation_AC = ((int64_t)deltaY_BtoA * (textureCoord_C_V - textureCoord_A_V) -
                                  (int64_t)deltaY_AtoC * (textureCoord_B_V - textureCoord_A_V)) * scalingFactor;
        calculation_AC *= 2;  // Doubling the result
        mapyhstep = (calculation_AC >> 16) & 0xFFFFFFFF; // Retain upper 32 bits
    }
    else
    {
        // In case combinedFactor2 equals to zero (to prevent division by zero), reset values
        shadehstep = 0;
        mapxhstep = 0;
        mapyhstep = 0;
    }
}
#endif

void draw_gpoly_sub2b() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_loc_0BFC\n \
    movl    _gploc_pt_by,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_0AA2\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_0AB0\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_0AA2:         # 8A7\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_0AB0:         # 8B0\n \
    movl    _point2shade,%%eax\n \
    subl    _point1shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0AC5\n \
    incl    %%eax\n \
\n \
gpo_loc_0AC5:         # 8D2\n \
    movl    %%eax,_shadeveltop\n \
    movl    _point2mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0ADE\n \
    incl    %%eax\n \
\n \
gpo_loc_0ADE:         # 8EB\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point2mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0AF7\n \
    incl    %%eax\n \
\n \
gpo_loc_0AF7:         # 90\n \
    movl    %%eax,_mapyveltop\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_by,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_0B14\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_0B22\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_0B14:         # 91\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_0B22:         # 922\n \
    movl    _point3shade,%%eax\n \
    subl    _point2shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0B37\n \
    incl    %%eax\n \
\n \
gpo_loc_0B37:         # 94\n \
    movl    %%eax,_shadevelbottom\n \
    movl    _point3mapx,%%eax\n \
    subl    _point2mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0B50\n \
    incl    %%eax\n \
\n \
gpo_loc_0B50:         # 95D\n \
    movl    %%eax,_mapxvelbottom\n \
    movl    _point3mapy,%%eax\n \
    subl    _point2mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0B69\n \
    incl    %%eax\n \
\n \
gpo_loc_0B69:         # 976\n \
    movl    %%eax,_mapyvelbottom\n \
    movl    _gploc_point_a,%%eax\n \
    movl    _mapxhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapxveltop\n \
    movl    _gploc_point_a,%%eax\n \
    movl    _mapyhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapyveltop\n \
    movl    _gploc_point_a,%%eax\n \
    movl    _shadehstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_shadeveltop\n \
    movl    _gploc_point_b,%%eax\n \
    movl    _mapxhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapxvelbottom\n \
    movl    _gploc_point_b,%%eax\n \
    movl    _mapyhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapyvelbottom\n \
    movl    _gploc_point_b,%%eax\n \
    movl    _shadehstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_shadevelbottom\n \
    jmp gpo_loc_0CB0\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_0BFC:         # 893\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_0C15\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_0C23\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_0C15:         # A1A\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_0C23:         # A23\n \
    movl    _point3shade,%%eax\n \
    subl    _point1shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0C38\n \
    incl    %%eax\n \
\n \
gpo_loc_0C38:         # A45\n \
    movl    %%eax,_shadeveltop\n \
    movl    _point3mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0C51\n \
    incl    %%eax\n \
\n \
gpo_loc_0C51:         # A5E\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point3mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0C6A\n \
    incl    %%eax\n \
\n \
gpo_loc_0C6A:         # A77\n \
    movl    %%eax,_mapyveltop\n \
    movl    _factor_ca,%%eax\n \
    movl    _mapxhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapxveltop\n \
    movl    _factor_ca,%%eax\n \
    movl    _mapyhstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_mapyveltop\n \
    movl    _factor_ca,%%eax\n \
    movl    _shadehstep,%%ebx\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    subl    %%eax,_shadeveltop\n \
\n \
gpo_loc_0CB0:         # A07\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

#if 0
// IDA decompilation
void draw_gpoly_sub2b()
{
  int v0; // ecx
  int v1; // ebx
  char v2; // sf
  int v3; // eax
  int v4; // eax
  int v5; // eax
  int v6; // eax
  int v7; // eax
  int v8; // eax
  int v9; // ecx
  int v10; // ebx
  int v11; // eax
  int v12; // eax
  int v13; // eax
  int v14; // eax
  int v15; // eax
  int v16; // eax
  int v17; // eax
  int v18; // ecx
  int v19; // ebx
  int v20; // eax
  int v21; // eax
  int v22; // eax
  int v23; // eax
  int v24; // eax
  int v25; // eax
  int v26; // eax

  if ( crease_len < 0 )
  {
    v18 = gploc_pt_cy - gploc_pt_ay;
    if ( gploc_pt_cy - gploc_pt_ay > 255 )
      v19 = 0x7FFFFFFF / v18;
    else
      v19 = gpoly_reptable[v18];
    HIWORD(v20) = (unsigned int)(v19 * 2 * (point3shade - point1shade)) >> 16;
    LOWORD(v20) = (unsigned __int64)(v19 * (__int64)(2 * (point3shade - point1shade))) >> 32;
    v21 = __ROL4__(v20, 16);
    if ( v2 )
      ++v21;
    shadeveltop = v21;
    HIWORD(v22) = (unsigned int)(v19 * 2 * (point3mapx - point1mapx)) >> 16;
    LOWORD(v22) = (unsigned __int64)(v19 * (__int64)(2 * (point3mapx - point1mapx))) >> 32;
    v23 = __ROL4__(v22, 16);
    if ( v2 )
      ++v23;
    mapxveltop = v23;
    HIWORD(v24) = (unsigned int)(v19 * 2 * (point3mapy - point1mapy)) >> 16;
    LOWORD(v24) = (unsigned __int64)(v19 * (__int64)(2 * (point3mapy - point1mapy))) >> 32;
    v25 = __ROL4__(v24, 16);
    if ( v2 )
      ++v25;
    mapyveltop = v25;
    HIWORD(v26) = (unsigned int)(mapxhstep * factor_ca) >> 16;
    LOWORD(v26) = (unsigned __int64)(mapxhstep * (__int64)factor_ca) >> 32;
    mapxveltop -= __ROL4__(v26, 16);
    HIWORD(v26) = (unsigned int)(mapyhstep * factor_ca) >> 16;
    LOWORD(v26) = (unsigned __int64)(mapyhstep * (__int64)factor_ca) >> 32;
    mapyveltop -= __ROL4__(v26, 16);
    HIWORD(v26) = (unsigned int)(shadehstep * factor_ca) >> 16;
    LOWORD(v26) = (unsigned __int64)(shadehstep * (__int64)factor_ca) >> 32;
    shadeveltop -= __ROL4__(v26, 16);
  }
  else
  {
    v0 = gploc_pt_by - gploc_pt_ay;
    if ( gploc_pt_by - gploc_pt_ay > 255 )
      v1 = 0x7FFFFFFF / v0;
    else
      v1 = gpoly_reptable[v0];
    HIWORD(v3) = (unsigned int)(v1 * 2 * (point2shade - point1shade)) >> 16;
    LOWORD(v3) = (unsigned __int64)(v1 * (__int64)(2 * (point2shade - point1shade))) >> 32;
    v4 = __ROL4__(v3, 16);
    if ( v2 )
      ++v4;
    shadeveltop = v4;
    HIWORD(v5) = (unsigned int)(v1 * 2 * (point2mapx - point1mapx)) >> 16;
    LOWORD(v5) = (unsigned __int64)(v1 * (__int64)(2 * (point2mapx - point1mapx))) >> 32;
    v6 = __ROL4__(v5, 16);
    if ( v2 )
      ++v6;
    mapxveltop = v6;
    HIWORD(v7) = (unsigned int)(v1 * 2 * (point2mapy - point1mapy)) >> 16;
    LOWORD(v7) = (unsigned __int64)(v1 * (__int64)(2 * (point2mapy - point1mapy))) >> 32;
    v8 = __ROL4__(v7, 16);
    if ( v2 )
      ++v8;
    mapyveltop = v8;
    v9 = gploc_pt_cy - gploc_pt_by;
    if ( gploc_pt_cy - gploc_pt_by > 255 )
      v10 = 0x7FFFFFFF / v9;
    else
      v10 = gpoly_reptable[v9];
    HIWORD(v11) = (unsigned int)(v10 * 2 * (point3shade - point2shade)) >> 16;
    LOWORD(v11) = (unsigned __int64)(v10 * (__int64)(2 * (point3shade - point2shade))) >> 32;
    v12 = __ROL4__(v11, 16);
    if ( v2 )
      ++v12;
    shadevelbottom = v12;
    HIWORD(v13) = (unsigned int)(v10 * 2 * (point3mapx - point2mapx)) >> 16;
    LOWORD(v13) = (unsigned __int64)(v10 * (__int64)(2 * (point3mapx - point2mapx))) >> 32;
    v14 = __ROL4__(v13, 16);
    if ( v2 )
      ++v14;
    mapxvelbottom = v14;
    HIWORD(v15) = (unsigned int)(v10 * 2 * (point3mapy - point2mapy)) >> 16;
    LOWORD(v15) = (unsigned __int64)(v10 * (__int64)(2 * (point3mapy - point2mapy))) >> 32;
    v16 = __ROL4__(v15, 16);
    if ( v2 )
      ++v16;
    mapyvelbottom = v16;
    HIWORD(v17) = (unsigned int)(v10 * mapxhstep) >> 16;
    LOWORD(v17) = (unsigned __int64)(v10 * (__int64)mapxhstep) >> 32;
    mapxveltop -= __ROL4__(v17, 16);
    HIWORD(v17) = (unsigned int)(mapyhstep * gploc_point_a) >> 16;
    LOWORD(v17) = (unsigned __int64)(mapyhstep * (__int64)gploc_point_a) >> 32;
    mapyveltop -= __ROL4__(v17, 16);
    HIWORD(v17) = (unsigned int)(shadehstep * gploc_point_a) >> 16;
    LOWORD(v17) = (unsigned __int64)(shadehstep * (__int64)gploc_point_a) >> 32;
    shadeveltop -= __ROL4__(v17, 16);
    HIWORD(v17) = (unsigned int)(mapxhstep * gploc_point_b) >> 16;
    LOWORD(v17) = (unsigned __int64)(mapxhstep * (__int64)gploc_point_b) >> 32;
    mapxvelbottom -= __ROL4__(v17, 16);
    HIWORD(v17) = (unsigned int)(mapyhstep * gploc_point_b) >> 16;
    LOWORD(v17) = (unsigned __int64)(mapyhstep * (__int64)gploc_point_b) >> 32;
    mapyvelbottom -= __ROL4__(v17, 16);
    HIWORD(v17) = (unsigned int)(shadehstep * gploc_point_b) >> 16;
    LOWORD(v17) = (unsigned __int64)(shadehstep * (__int64)gploc_point_b) >> 32;
    shadevelbottom -= __ROL4__(v17, 16);
  }
}

// GPT-4 cleaned up version
void draw_gpoly_sub2b()
{
    int delta; // Variable to store differences computed
    int scale_factor;
    int result;

    if (crease_len < 0)
    {
        delta = gploc_pt_cy - gploc_pt_ay;
        scale_factor = (delta > 255) ? (0x7FFFFFFF / delta) : gpoly_reptable[delta];

        // For shadeveltop
        result = ((scale_factor * 2 * (point3shade - point1shade)) >> 16);
        shadeveltop = result;

        // For mapxveltop
        result = ((scale_factor * 2 * (point3mapx - point1mapx)) >> 16);
        mapxveltop = result;

        // For mapyveltop
        result = ((scale_factor * 2 * (point3mapy - point1mapy)) >> 16);
        mapyveltop = result;

        mapxveltop -= ((mapxhstep * factor_ca) >> 16);
        mapyveltop -= ((mapyhstep * factor_ca) >> 16);
        shadeveltop -= ((shadehstep * factor_ca) >> 16);
    }
    else
    {
        delta = gploc_pt_by - gploc_pt_ay;
        scale_factor = (delta > 255) ? (0x7FFFFFFF / delta) : gpoly_reptable[delta];

        // For shadeveltop
        result = ((scale_factor * 2 * (point2shade - point1shade)) >> 16);
        shadeveltop = result;

        // For mapxveltop
        result = ((scale_factor * 2 * (point2mapx - point1mapx)) >> 16);
        mapxveltop = result;

        // For mapyveltop
        result = ((scale_factor * 2 * (point2mapy - point1mapy)) >> 16);
        mapyveltop = result;

        delta = gploc_pt_cy - gploc_pt_by;
        scale_factor = (delta > 255) ? (0x7FFFFFFF / delta) : gpoly_reptable[delta];

        shadevelbottom = ((scale_factor * 2 * (point3shade - point2shade)) >> 16);
        mapxvelbottom = ((scale_factor * 2 * (point3mapx - point2mapx)) >> 16);
        mapyvelbottom = ((scale_factor * 2 * (point3mapy - point2mapy)) >> 16);

        mapxveltop -= ((scale_factor * mapxhstep) >> 16);
        mapyveltop -= ((mapyhstep * scale_factor) >> 16);
        shadeveltop -= ((shadehstep * scale_factor) >> 16);
        mapxvelbottom -= ((mapxhstep * scale_factor) >> 16);
        mapyvelbottom -= ((mapyhstep * scale_factor) >> 16);
        shadevelbottom -= ((shadehstep * scale_factor) >> 16);
    }
}

// Further cleanup
void adjustVertexAttributesBasedOnFactor()
{
    int deltaY; // Variable to store differences computed along Y-axis
    int scaleFactor; // Variable to store the computed scale factor
    int adjustmentResult; // Result after applying the scale factor

    if (conditionFactor < 0) // This likely indicates a particular geometric or rendering condition
    {
        deltaY = vertexC_Y - vertexA_Y;
        scaleFactor = (deltaY > 255) ? (INT_MAX / deltaY) : repeatScaleTable[deltaY];

        // Adjust attribute for vertex C
        adjustmentResult = ((scaleFactor * 2 * (textureAdjustment_C - textureAdjustment_A)) >> 16);
        attribute_vertexC = adjustmentResult;

        // Adjust attribute 1
        adjustmentResult = ((scaleFactor * 2 * (textureCoord_C_U - textureCoord_A_U)) >> 16);
        adjustment_global1 = adjustmentResult;

        // Adjust attribute 2
        adjustmentResult = ((scaleFactor * 2 * (textureCoord_C_V - textureCoord_A_V)) >> 16);
        adjustment_global2 = adjustmentResult;

        // Applying final adjustments based on another scale factor
        adjustment_global1 -= ((globalFactor_B * scaleCondition) >> 16);
        adjustment_global2 -= ((globalFactor_A * scaleCondition) >> 16);
        attribute_vertexC -= ((globalAdjustment_C * scaleCondition) >> 16);
    }
    else
    {
        deltaY = vertexB_Y - vertexA_Y;
        scaleFactor = (deltaY > 255) ? (INT_MAX / deltaY) : repeatScaleTable[deltaY];

        // Adjust attribute for vertex C
        adjustmentResult = ((scaleFactor * 2 * (textureAdjustment_B - textureAdjustment_A)) >> 16);
        attribute_vertexC = adjustmentResult;

        // Adjust attribute 1
        adjustmentResult = ((scaleFactor * 2 * (textureCoord_B_U - textureCoord_A_U)) >> 16);
        adjustment_global1 = adjustmentResult;

        // Adjust attribute 2
        adjustmentResult = ((scaleFactor * 2 * (textureCoord_B_V - textureCoord_A_V)) >> 16);
        adjustment_global2 = adjustmentResult;

        deltaY = vertexC_Y - vertexB_Y;
        scaleFactor = (deltaY > 255) ? (INT_MAX / deltaY) : repeatScaleTable[deltaY];

        // Additional adjustments considering vertex B to C changes
        adjustment_vertexBtoC = ((scaleFactor * 2 * (textureAdjustment_C - textureAdjustment_B)) >> 16);
        adjustment_global3 = ((scaleFactor * 2 * (textureCoord_C_U - textureCoord_B_U)) >> 16);
        adjustment_global4 = ((scaleFactor * 2 * (textureCoord_C_V - textureCoord_B_V)) >> 16);

        // Applying final adjustments based on the newly computed scale factor
        adjustment_global1 -= ((globalFactor_B * scaleFactor) >> 16);
        adjustment_global2 -= ((globalFactor_A * scaleFactor) >> 16);
        attribute_vertexC -= ((globalAdjustment_C * scaleFactor) >> 16);
        adjustment_global3 -= ((globalFactor_B * scaleFactor) >> 16);
        adjustment_global4 -= ((globalFactor_A * scaleFactor) >> 16);
        adjustment_vertexBtoC -= ((globalAdjustment_C * scaleFactor) >> 16);
    }
}
#endif

void draw_gpoly_sub2c() {
  startposshadetop = (point1shade << 16) + shadehstep;
  startposmapxtop = (point1mapx << 16) + mapxhstep;
  startposmapytop = (point1mapy << 16) + mapyhstep;
  startposshadebottom = (point2shade << 16) + shadehstep;
  startposmapxbottom = (point2mapx << 16) + mapxhstep;
  startposmapybottom = (point2mapy << 16) + mapyhstep;

  unk_update_gpoly1_tri16a(&gploc_B8, &gploc_BC, gploc_word03, mapxhstep, 65536);
  unk_update_gpoly1_tri8a(&gploc_B4, &gploc_B8, gploc_B8, mapyhstep, 256);
  unk_update_gpoly1_tri16a(&gploc_2C, &gploc_5C, gploc_word03, mapxhstep, 65535);
  unk_update_gpoly1_tri8a(&gploc_28, &gploc_2C, gploc_2C, mapyhstep, 256);
  unk_update_gpoly1_tri16a(&gploc_A0, &gploc_A4, gploc_word01, mapxveltop, 65536);
  unk_update_gpoly1_tri8a(&gploc_9C, &gploc_A0, gploc_A0, mapyveltop, 256);
  unk_update_gpoly1_tri16b(&gploc_84, &gploc_88, &gploc_8C, startposmapytop, startposmapxtop,
                           startposshadetop);

  if (crease_len >= 0) {
    unk_update_gpoly1_tri16a(&gploc_94, &gploc_98, gploc_word02, mapxvelbottom, 65536);
    unk_update_gpoly1_tri8a(&gploc_90, &gploc_94, gploc_94, mapyvelbottom, 256);
    unk_update_gpoly1_tri16b(&gploc_78, &gploc_7C, &gploc_80, startposmapybottom,
                             startposmapxbottom, startposshadebottom);
  }
}

void draw_gpoly_sub3a() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _gploc_pt_bx,%%esi\n \
    movl    _crease_len,%%edi\n \
    orl %%edi,%%edi\n \
    subl    $0,%%esi\n \
    addl    $0,%%esi\n \
    movl    _gploc_pt_ax,%%eax\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_cx,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_by,%%ebx\n \
    subl    %%eax,%%ebx\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    %%eax,%%ecx\n \
    movl    %%ecx,%%eax\n \
    imull   %%esi,%%ecx\n \
    movl    _crease_len,%%ebp\n \
    orl %%ebp,%%ebp\n \
    js  gpo_loc_0FAC\n \
    subl    %%eax,%%ecx\n \
    subl    %%eax,%%ecx\n \
\n \
gpo_loc_0FAC:         # DB6\n \
    addl    %%eax,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    jz  gpo_loc_1002\n \
    xorl    %%edx,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ebx\n \
    movl    %%eax,%%ebp\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_cy,%%esi\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_by,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    %%ebp,%%eax\n \
    movl    _point1shade,%%edx\n \
    movl    _point3shade,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2shade,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_0FF9\n \
    incl    %%eax\n \
\n \
gpo_loc_0FF9:         # E06\n \
    movl    %%eax,_shadehstep\n \
    jmp gpo_loc_100B\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_1002:         # DC3\n \
    xorl    %%eax,%%eax\n \
    movl    %%eax,_shadehstep\n \
\n \
gpo_loc_100B:         # E10\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

#if 0
// IDA decompilation
void draw_gpoly_sub3a()
{
  int v0; // eax
  int v1; // ecx
  int v2; // ebx
  bool v4; // sf
  int v5; // eax

  v0 = gploc_pt_cy - gploc_pt_ay;
  v1 = (gploc_pt_bx - gploc_pt_ax) * (gploc_pt_cy - gploc_pt_ay);
  if ( crease_len >= 0 )
    v1 = v1 - v0 - v0;
  v2 = (gploc_pt_cx - gploc_pt_ax) * (gploc_pt_by - gploc_pt_ay) - (v0 + v1);
  if ( v2 )
  {
    _RAX = ((gploc_pt_by - gploc_pt_ay) * (point3shade - point1shade) - (gploc_pt_cy - gploc_pt_ay)
                                                                  * (point2shade - point1shade))
         * (__int64)(0x7FFFFFFF / v2);
    LODWORD(_RAX) = 2 * _RAX;
    v4 = (int)_RAX < 0;
    __asm { rcl     edx, 1 }
    LOWORD(_RAX) = WORD2(_RAX);
    v5 = __ROL4__(_RAX, 16);
    if ( v4 )
      ++v5;
    shadehstep = v5;
  }
  else
  {
    shadehstep = 0;
  }
}

// GPT-4 refactoring
void draw_gpoly_sub3a()
{
    int delta_ay_cy = gploc_pt_cy - gploc_pt_ay; // Change in Y from A to C
    int calculation = (gploc_pt_bx - gploc_pt_ax) * delta_ay_cy; // Initial calculation using differences in X (B to A) and Y (C to A)

    // Adjust calculation based on crease_len
    if (crease_len >= 0)
        calculation -= 2 * delta_ay_cy;

    int result_factor = (gploc_pt_cx - gploc_pt_ax) * (gploc_pt_by - gploc_pt_ay) - (delta_ay_cy + calculation);

    if (result_factor != 0)
    {
        int64_t part_result = ((int64_t)(gploc_pt_by - gploc_pt_ay) * (point3shade - point1shade) -
                              (int64_t)delta_ay_cy * (point2shade - point1shade)) * (0x7FFFFFFF / result_factor);

        part_result *= 2; // Double the part_result as per original logic

        // Since we want to preserve the high 32 bits of the result into shadehstep, perform a right shift
        shadehstep = (int)(part_result >> 32); // Casting to int ensures we're working with 32-bit precision
    }
    else
    {
        shadehstep = 0;
    }
}

// More refactoring
void calculateAttributeAdjustmentForVertexA()
{
    int deltaY_AC = vertexC_Y - vertexA_Y; // Change in Y from vertex A to C
    int preliminaryCalculation = (vertexB_X - vertexA_X) * deltaY_AC; // Preliminary calculation using delta X (B to A) and delta Y (C to A)

    // Adjust preliminaryCalculation based on 'conditionFactor'
    if (conditionFactor >= 0)
        preliminaryCalculation -= 2 * deltaY_AC;

    int adjustedFactor = (vertexC_X - vertexA_X) * (vertexB_Y - vertexA_Y) - (deltaY_AC + preliminaryCalculation);

    if (adjustedFactor != 0)
    {
        int64_t scaledResult = ((int64_t)(vertexB_Y - vertexA_Y) * (adjustmentValue_C - adjustmentValue_A) -
                               (int64_t)deltaY_AC * (adjustmentValue_B - adjustmentValue_A)) * (INT_MAX / adjustedFactor);

        scaledResult *= 2; // Double the scaledResult to adhere to original logic

        // Preserving the higher 32 bits of scaledResult into adjustmentAttribute_A
        adjustmentAttribute_A = (int)(scaledResult >> 32); // Casting to int to ensure 32-bit precision is maintained
    }
    else
    {
        adjustmentAttribute_A = 0;
    }
}
#endif

void draw_gpoly_sub3b() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_loc_1099\n \
    movl    _gploc_pt_by,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_1030\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_103E\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_1030:         # E35\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_103E:         # E3E\n \
    movl    _point2shade,%%eax\n \
    subl    _point1shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1053\n \
    incl    %%eax\n \
\n \
gpo_loc_1053:         # E60\n \
    movl    %%eax,_shadeveltop\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_by,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_1070\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_107E\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_1070:         # E75\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_107E:         # E7E\n \
    movl    _point3shade,%%eax\n \
    subl    _point2shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1093\n \
    incl    %%eax\n \
\n \
gpo_loc_1093:         # EA0\n \
    movl    %%eax,_shadevelbottom\n \
    jmp gpo_loc_10D9\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_1099:         # E2\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_10B2\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_10C0\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_10B2:         # EB7\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_10C0:         # EC0\n \
    movl    _point3shade,%%eax\n \
    subl    _point1shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_10D5\n \
    incl    %%eax\n \
\n \
gpo_loc_10D5:         # EE2\n \
    movl    %%eax,_shadeveltop\n \
\n \
gpo_loc_10D9:         # EA7\n \
    movl    _point1shade,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposshadetop\n \
    movl    _point2shade,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposshadebottom\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

#if 0
// IDA decompilation
void draw_gpoly_sub3b()
{
  int v0; // ecx
  int v1; // ebx
  char v2; // sf
  int v3; // eax
  int v4; // eax
  int v5; // ecx
  int v6; // ebx
  int v7; // eax
  int v8; // eax
  int v9; // ecx
  int v10; // ebx
  int v11; // eax
  int v12; // eax

  if ( crease_len < 0 )
  {
    v9 = gploc_pt_cy - gploc_pt_ay;
    if ( gploc_pt_cy - gploc_pt_ay > 255 )
      v10 = 0x7FFFFFFF / v9;
    else
      v10 = gpoly_reptable[v9];
    HIWORD(v11) = (unsigned int)(v10 * 2 * (point3shade - point1shade)) >> 16;
    LOWORD(v11) = (unsigned __int64)(v10 * (__int64)(2 * (point3shade - point1shade))) >> 32;
    v12 = __ROL4__(v11, 16);
    if ( v2 )
      ++v12;
    shadeveltop = v12;
  }
  else
  {
    v0 = gploc_pt_by - gploc_pt_ay;
    if ( gploc_pt_by - gploc_pt_ay > 255 )
      v1 = 0x7FFFFFFF / v0;
    else
      v1 = gpoly_reptable[v0];
    HIWORD(v3) = (unsigned int)(v1 * 2 * (point2shade - point1shade)) >> 16;
    LOWORD(v3) = (unsigned __int64)(v1 * (__int64)(2 * (point2shade - point1shade))) >> 32;
    v4 = __ROL4__(v3, 16);
    if ( v2 )
      ++v4;
    shadeveltop = v4;
    v5 = gploc_pt_cy - gploc_pt_by;
    if ( gploc_pt_cy - gploc_pt_by > 255 )
      v6 = 0x7FFFFFFF / v5;
    else
      v6 = gpoly_reptable[v5];
    HIWORD(v7) = (unsigned int)(v6 * 2 * (point3shade - point2shade)) >> 16;
    LOWORD(v7) = (unsigned __int64)(v6 * (__int64)(2 * (point3shade - point2shade))) >> 32;
    v8 = __ROL4__(v7, 16);
    if ( v2 )
      ++v8;
    shadevelbottom = v8;
  }
  startposshadetop = point1shade << 16;
  startposshadebottom = point2shade << 16;
}

// GPT-4 refactored
void draw_gpoly_sub3b()
{
    int delta;
    int result;

    if (crease_len < 0)
    {
        delta = gploc_pt_cy - gploc_pt_ay;
        int v10 = (delta > 255) ? (0x7FFFFFFF / delta) : gpoly_reptable[delta];
        int calculation = v10 * 2 * (point3shade - point1shade);
        result = (calculation >> 16) & 0xFFFF; // Take HiWord of the calculation
        // If the original code used a check (v2) to conditionally increment result, assume similar logic is needed
        shadeveltop = result;
    }
    else
    {
        // Branch when the crease_len is >= 0
        delta = gploc_pt_by - gploc_pt_ay;
        int v1 = (delta > 255) ? (0x7FFFFFFF / delta) : gpoly_reptable[delta];
        int calculation = v1 * 2 * (point2shade - point1shade);
        result = (calculation >> 16) & 0xFFFF; // Take HiWord
        shadeveltop = result;

        delta = gploc_pt_cy - gploc_pt_by;
        int v6 = (delta > 255) ? (0x7FFFFFFF / delta) : gpoly_reptable[delta];
        calculation = v6 * 2 * (point3shade - point2shade);
        result = (calculation >> 16) & 0xFFFF; // Take HiWord
        shadevelbottom = result;
    }

    // Direct assignments to global variables, assuming these are correctly calculating a shifted value
    startposshadetop = point1shade << 16;
    startposshadebottom = point2shade << 16;
}

// More refactoring
void calculateVertexAttributeAdjustments()
{
    int deltaY;
    int attributeAdjustmentResult;

    if (conditionFactor < 0) // Negative condition factor logic
    {
        deltaY = vertexC_Y - vertexA_Y;
        int scaleCorrection = (deltaY > 255) ? (INT_MAX / deltaY) : repetitionScaleTable[deltaY];
        int intermediateCalculation = scaleCorrection * 2 * (vertexAdjustment_C - vertexAdjustment_A);
        attributeAdjustmentResult = (intermediateCalculation >> 16) & 0xFFFF; // Extracting higher word
        adjustedAttribute_vertexC = attributeAdjustmentResult; // Adjusted attribute result for vertex C
    }
    else // Non-negative condition factor logic
    {
        deltaY = vertexB_Y - vertexA_Y;
        int scaleCorrectionBtoA = (deltaY > 255) ? (INT_MAX / deltaY) : repetitionScaleTable[deltaY];
        int intermediateCalcBtoA = scaleCorrectionBtoA * 2 * (vertexAdjustment_B - vertexAdjustment_A);
        attributeAdjustmentResult = (intermediateCalcBtoA >> 16) & 0xFFFF; // Extracting higher word
        adjustedAttribute_vertexC = attributeAdjustmentResult; // Adjusted attribute result for vertex C variant

        deltaY = vertexC_Y - vertexB_Y;
        int scaleCorrectionCtoB = (deltaY > 255) ? (INT_MAX / deltaY) : repetitionScaleTable[deltaY];
        int intermediateCalcCtoB = scaleCorrectionCtoB * 2 * (vertexAdjustment_C - vertexAdjustment_B);
        attributeAdjustmentResult = (intermediateCalcCtoB >> 16) & 0xFFFF; // Extracting higher word again
        adjustedAttribute_other = attributeAdjustmentResult; // Another adjusted attribute, potentially for a different vertex or property
    }

    // Assigning shifted attribute values, likely preparing for further processing or rendering
    shiftedAttribute_A = vertexAdjustment_A << 16;
    shiftedAttribute_B = vertexAdjustment_B << 16;
}
#endif

void draw_gpoly_sub4() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _gploc_pt_bx,%%esi\n \
    movl    _crease_len,%%edi\n \
    orl %%edi,%%edi\n \
    subl    $0,%%esi\n \
    addl    $0,%%esi\n \
    movl    _gploc_pt_ax,%%eax\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_cx,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_by,%%ebx\n \
    subl    %%eax,%%ebx\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    %%eax,%%ecx\n \
    movl    %%ecx,%%eax\n \
    imull   %%esi,%%ecx\n \
    movl    _crease_len,%%ebp\n \
    orl %%ebp,%%ebp\n \
    js  gpo_loc_1137\n \
    subl    %%eax,%%ecx\n \
    subl    %%eax,%%ecx\n \
\n \
gpo_loc_1137:         # F4\n \
    addl    %%eax,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    jz  gpo_loc_11BD\n \
    xorl    %%edx,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ebx\n \
    movl    %%eax,%%ebp\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_cy,%%esi\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_by,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapx,%%edx\n \
    movl    _point3mapx,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapx,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1184\n \
    incl    %%eax\n \
\n \
gpo_loc_1184:         # F9\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapy,%%edx\n \
    movl    _point3mapy,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapy,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_11B4\n \
    incl    %%eax\n \
\n \
gpo_loc_11B4:         # FC\n \
    movl    %%eax,_mapyhstep\n \
    jmp gpo_loc_11CD\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_11BD:         # F4E\n \
    xorl    %%eax,%%eax\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%eax,_mapyhstep\n \
\n \
gpo_loc_11CD:         # FCB\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif

#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_loc_128D\n \
    movl    _gploc_pt_by,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_11F2\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_1200\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_11F2:         # FF7\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_1200:         # 1000\n \
    movl    _point2mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1215\n \
    incl    %%eax\n \
\n \
gpo_loc_1215:         # 1022\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point2mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_122E\n \
    incl    %%eax\n \
\n \
gpo_loc_122E:         # 103B\n \
    movl    %%eax,_mapyveltop\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_by,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_124B\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_1259\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_124B:         # 1050\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_1259:         # 105\n \
    movl    _point3mapx,%%eax\n \
    subl    _point2mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_126E\n \
    incl    %%eax\n \
\n \
gpo_loc_126E:         # 107B\n \
    movl    %%eax,_mapxvelbottom\n \
    movl    _point3mapy,%%eax\n \
    subl    _point2mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1287\n \
    incl    %%eax\n \
\n \
gpo_loc_1287:         # 109\n \
    movl    %%eax,_mapyvelbottom\n \
    jmp gpo_loc_12E6\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_128D:         # FE3\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_12A6\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_12B4\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_12A6:         # 10AB\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_12B4:         # 10B\n \
    movl    _point3mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_12C9\n \
    incl    %%eax\n \
\n \
gpo_loc_12C9:         # 10D6\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point3mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_12E2\n \
    incl    %%eax\n \
\n \
gpo_loc_12E2:         # 10E\n \
    movl    %%eax,_mapyveltop\n \
\n \
gpo_loc_12E6:         # 109B\n \
    movl    _point1mapx,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapxtop\n \
    movl    _point1mapy,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapytop\n \
    movl    _point2mapx,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapxbottom\n \
    movl    _point2mapy,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapybottom\n \
    movl    _mapxhstep,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movl    %%eax,_gploc_BC\n \
    movl    %%edx,_gploc_B8\n \
    movl    _mapyhstep,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    _mapyhstep,%%edx\n \
    sarl    $0x10,%%edx\n \
    movb    _gploc_B8,%%al\n \
    orb %%al,%%al\n \
    jns gpo_loc_1362\n \
    subl    $0x100,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_1362:         # 1168\n \
    movl    %%eax,_gploc_B8\n \
    movl    %%edx,_gploc_B4\n \
    movl    _mapxveltop,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movl    %%eax,_gploc_A4\n \
    movl    %%edx,_gploc_A0\n \
    movl    _mapyveltop,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    _mapyveltop,%%edx\n \
    sarl    $0x10,%%edx\n \
    movb    _gploc_A0,%%al\n \
    orb %%al,%%al\n \
    jns gpo_loc_13AB\n \
    subl    $0x100,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_13AB:         # 11B\n \
    movl    %%eax,_gploc_A0\n \
    movl    %%edx,_gploc_9C\n \
    movl    _startposmapxtop,%%ecx\n \
    movl    _startposmapytop,%%edx\n \
    movl    %%ecx,%%eax\n \
    shll    $0x10,%%ecx\n \
    shrl    $0x10,%%eax\n \
    movl    %%ecx,_gploc_8C\n \
    movl    %%edx,%%ecx\n \
    shll    $0x10,%%ecx\n \
    shll    $8,%%edx\n \
    shrl    $0x18,%%edx\n \
    shll    $8,%%edx\n \
    movb    %%al,%%cl\n \
    movl    %%ecx,_gploc_88\n \
    movl    %%edx,_gploc_84\n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_loc_1484\n \
    movl    _mapxvelbottom,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movl    %%eax,_gploc_98\n \
    movl    %%edx,_gploc_94\n \
    movl    _mapyvelbottom,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    _mapyvelbottom,%%edx\n \
    sarl    $0x10,%%edx\n \
    movb    _gploc_94,%%al\n \
    orb %%al,%%al\n \
    jns gpo_loc_143B\n \
    subl    $0x100,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_143B:         # 124\n \
    movl    %%eax,_gploc_94\n \
    movl    %%edx,_gploc_90\n \
    movl    _startposmapxbottom,%%ecx\n \
    movl    _startposmapybottom,%%edx\n \
    movl    %%ecx,%%eax\n \
    shll    $0x10,%%ecx\n \
    shrl    $0x10,%%eax\n \
    movl    %%ecx,_gploc_80\n \
    movl    %%edx,%%ecx\n \
    shll    $0x10,%%ecx\n \
    shll    $8,%%edx\n \
    shrl    $0x18,%%edx\n \
    shll    $8,%%edx\n \
    movb    %%al,%%cl\n \
    movl    %%ecx,_gploc_7C\n \
    movl    %%edx,_gploc_78\n \
\n \
gpo_loc_1484:         # 120A\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

#if 0
// IDA decompilation
void draw_gpoly_sub4()
{
  int v1; // eax
  int v2; // ecx
  int v3; // ebx
  bool v5; // sf
  int v6; // eax
  int v8; // eax
  int v9; // ecx
  int v10; // ebx
  int v11; // eax
  int v12; // eax
  int v13; // eax
  int v14; // eax
  int v15; // ecx
  int v16; // ebx
  int v17; // eax
  int v18; // eax
  int v19; // eax
  int v20; // eax
  int v21; // ecx
  int v22; // ebx
  int v23; // eax
  int v24; // eax
  int v25; // eax
  int v26; // eax
  unsigned __int64 v27; // kr00_8
  int v28; // edx
  unsigned int v29; // eax
  bool v30; // cf
  unsigned __int64 v31; // kr08_8
  int v32; // edx
  unsigned int v33; // eax
  int v34; // ecx
  unsigned __int64 v35; // kr10_8
  int v36; // edx
  unsigned int v37; // eax
  int v38; // ecx

  v1 = gploc_pt_cy - gploc_pt_ay;
  v2 = (gploc_pt_bx - gploc_pt_ax) * (gploc_pt_cy - gploc_pt_ay);
  if ( crease_len >= 0 )
    v2 = v2 - v1 - v1;
  v3 = (gploc_pt_cx - gploc_pt_ax) * (gploc_pt_by - gploc_pt_ay) - (v1 + v2);
  if ( v3 )
  {
    _RAX = ((gploc_pt_by - gploc_pt_ay) * (point3mapx - point1mapx) - (gploc_pt_cy - gploc_pt_ay)
                                                                  * (point2mapx - point1mapx))
         * (__int64)(0x7FFFFFFF / v3);
    LODWORD(_RAX) = 2 * _RAX;
    v5 = (int)_RAX < 0;
    __asm { rcl     edx, 1 }
    LOWORD(_RAX) = WORD2(_RAX);
    v6 = __ROL4__(_RAX, 16);
    if ( v5 )
      ++v6;
    mapxhstep = v6;
    _RAX = ((gploc_pt_by - gploc_pt_ay) * (point3mapy - point1mapy) - (gploc_pt_cy - gploc_pt_ay)
                                                                  * (point2mapy - point1mapy))
         * (__int64)(0x7FFFFFFF / v3);
    LODWORD(_RAX) = 2 * _RAX;
    v5 = (int)_RAX < 0;
    __asm { rcl     edx, 1 }
    LOWORD(_RAX) = WORD2(_RAX);
    v8 = __ROL4__(_RAX, 16);
    if ( v5 )
      ++v8;
    mapyhstep = v8;
  }
  else
  {
    mapxhstep = 0;
    mapyhstep = 0;
  }
  if ( crease_len < 0 )
  {
    v21 = gploc_pt_cy - gploc_pt_ay;
    if ( gploc_pt_cy - gploc_pt_ay > 255 )
      v22 = 0x7FFFFFFF / v21;
    else
      v22 = gpoly_reptable[v21];
    HIWORD(v23) = (unsigned int)(v22 * 2 * (point3mapx - point1mapx)) >> 16;
    LOWORD(v23) = (unsigned __int64)(v22 * (__int64)(2 * (point3mapx - point1mapx))) >> 32;
    v24 = __ROL4__(v23, 16);
    if ( v5 )
      ++v24;
    mapxveltop = v24;
    HIWORD(v25) = (unsigned int)(v22 * 2 * (point3mapy - point1mapy)) >> 16;
    LOWORD(v25) = (unsigned __int64)(v22 * (__int64)(2 * (point3mapy - point1mapy))) >> 32;
    v26 = __ROL4__(v25, 16);
    if ( v5 )
      ++v26;
    mapyveltop = v26;
  }
  else
  {
    v9 = gploc_pt_by - gploc_pt_ay;
    if ( gploc_pt_by - gploc_pt_ay > 255 )
      v10 = 0x7FFFFFFF / v9;
    else
      v10 = gpoly_reptable[v9];
    HIWORD(v11) = (unsigned int)(v10 * 2 * (point2mapx - point1mapx)) >> 16;
    LOWORD(v11) = (unsigned __int64)(v10 * (__int64)(2 * (point2mapx - point1mapx))) >> 32;
    v12 = __ROL4__(v11, 16);
    if ( v5 )
      ++v12;
    mapxveltop = v12;
    HIWORD(v13) = (unsigned int)(v10 * 2 * (point2mapy - point1mapy)) >> 16;
    LOWORD(v13) = (unsigned __int64)(v10 * (__int64)(2 * (point2mapy - point1mapy))) >> 32;
    v14 = __ROL4__(v13, 16);
    if ( v5 )
      ++v14;
    mapyveltop = v14;
    v15 = gploc_pt_cy - gploc_pt_by;
    if ( gploc_pt_cy - gploc_pt_by > 255 )
      v16 = 0x7FFFFFFF / v15;
    else
      v16 = gpoly_reptable[v15];
    HIWORD(v17) = (unsigned int)(v16 * 2 * (point3mapx - point2mapx)) >> 16;
    LOWORD(v17) = (unsigned __int64)(v16 * (__int64)(2 * (point3mapx - point2mapx))) >> 32;
    v18 = __ROL4__(v17, 16);
    if ( v5 )
      ++v18;
    mapxvelbottom = v18;
    HIWORD(v19) = (unsigned int)(v16 * 2 * (point3mapy - point2mapy)) >> 16;
    LOWORD(v19) = (unsigned __int64)(v16 * (__int64)(2 * (point3mapy - point2mapy))) >> 32;
    v20 = __ROL4__(v19, 16);
    if ( v5 )
      ++v20;
    mapyvelbottom = v20;
  }
  startposmapxtop = point1mapx << 16;
  startposmapytop = point1mapy << 16;
  startposmapxbottom = point2mapx << 16;
  startposmapybottom = point2mapy << 16;
  v27 = (__int64)mapxhstep << 16;
  gploc_B8 = HIDWORD(v27);
  gploc_BC = v27;
  v29 = mapyhstep << 16;
  v28 = mapyhstep >> 16;
  LOBYTE(v29) = BYTE4(v27);
  if ( (v27 & 0x8000000000LL) != 0 )
  {
    v30 = v29 < 0x100;
    v29 -= 256;
    LOBYTE(v28) = BYTE2(mapyhstep) - v30;
  }
  gploc_B8 = v29;
  gploc_B4 = v28;
  v31 = (__int64)mapxveltop << 16;
  gploc_A0 = HIDWORD(v31);
  gploc_A4 = v31;
  v33 = mapyveltop << 16;
  v32 = mapyveltop >> 16;
  LOBYTE(v33) = BYTE4(v31);
  if ( (v31 & 0x8000000000LL) != 0 )
  {
    v30 = v33 < 0x100;
    v33 -= 256;
    LOBYTE(v32) = BYTE2(mapyveltop) - v30;
  }
  gploc_A0 = v33;
  gploc_9C = v32;
  gploc_8C = startposmapxtop << 16;
  v34 = startposmapytop << 16;
  LOBYTE(v34) = (unsigned __int64)(unsigned int)startposmapxtop >> 16;
  gploc_88 = v34;
  gploc_84 = (unsigned int)(startposmapytop << 8) >> 24 << 8;
  if ( crease_len >= 0 )
  {
    v35 = (__int64)mapxvelbottom << 16;
    gploc_94 = HIDWORD(v35);
    gploc_98 = v35;
    v37 = mapyvelbottom << 16;
    v36 = mapyvelbottom >> 16;
    LOBYTE(v37) = BYTE4(v35);
    if ( (v35 & 0x8000000000LL) != 0 )
    {
      v30 = v37 < 0x100;
      v37 -= 256;
      LOBYTE(v36) = BYTE2(mapyvelbottom) - v30;
    }
    gploc_94 = v37;
    gploc_90 = v36;
    gploc_80 = startposmapxbottom << 16;
    v38 = startposmapybottom << 16;
    LOBYTE(v38) = (unsigned __int64)(unsigned int)startposmapxbottom >> 16;
    gploc_7C = v38;
    gploc_78 = (unsigned int)(startposmapybottom << 8) >> 24 << 8;
  }
  return a1;
}
#endif

void draw_gpoly_sub5() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _gploc_pt_bx,%%esi\n \
    movl    _crease_len,%%edi\n \
    orl %%edi,%%edi\n \
    subl    $0,%%esi\n \
    addl    $0,%%esi\n \
    movl    _gploc_pt_ax,%%eax\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_cx,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_by,%%ebx\n \
    subl    %%eax,%%ebx\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    %%eax,%%ecx\n \
    movl    %%ecx,%%eax\n \
    imull   %%esi,%%ecx\n \
    movl    _crease_len,%%ebp\n \
    orl %%ebp,%%ebp\n \
    js  gpo_loc_14C6\n \
    subl    %%eax,%%ecx\n \
    subl    %%eax,%%ecx\n \
\n \
gpo_loc_14C6:         # 12D0\n \
    addl    %%eax,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    jz  gpo_loc_154C\n \
    xorl    %%edx,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ebx\n \
    movl    %%eax,%%ebp\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_cy,%%esi\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_by,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapx,%%edx\n \
    movl    _point3mapx,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapx,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1513\n \
    incl    %%eax\n \
\n \
gpo_loc_1513:         # 1320\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapy,%%edx\n \
    movl    _point3mapy,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapy,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1543\n \
    incl    %%eax\n \
\n \
gpo_loc_1543:         # 1350\n \
    movl    %%eax,_mapyhstep\n \
    jmp gpo_loc_155C\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_154C:         # 12DD\n \
    xorl    %%eax,%%eax\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%eax,_mapyhstep\n \
\n \
gpo_loc_155C:         # 135A\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif

#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_loc_161C\n \
    movl    _gploc_pt_by,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_1581\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_158F\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_1581:         # 1386\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_158F:         # 138\n \
    movl    _point2mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_15A4\n \
    incl    %%eax\n \
\n \
gpo_loc_15A4:         # 13B\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point2mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_15BD\n \
    incl    %%eax\n \
\n \
gpo_loc_15BD:         # 13CA\n \
    movl    %%eax,_mapyveltop\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_by,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_15DA\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_15E8\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_15DA:         # 13D\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_15E8:         # 13E8\n \
    movl    _point3mapx,%%eax\n \
    subl    _point2mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_15FD\n \
    incl    %%eax\n \
\n \
gpo_loc_15FD:         # 140A\n \
    movl    %%eax,_mapxvelbottom\n \
    movl    _point3mapy,%%eax\n \
    subl    _point2mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1616\n \
    incl    %%eax\n \
\n \
gpo_loc_1616:         # 1423\n \
    movl    %%eax,_mapyvelbottom\n \
    jmp gpo_loc_1675\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_161C:         # 1372\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_1635\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_1643\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_1635:         # 143A\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_1643:         # 1443\n \
    movl    _point3mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1658\n \
    incl    %%eax\n \
\n \
gpo_loc_1658:         # 1465\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point3mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1671\n \
    incl    %%eax\n \
\n \
gpo_loc_1671:         # 147E\n \
    movl    %%eax,_mapyveltop\n \
\n \
gpo_loc_1675:         # 142A\n \
    movl    _point1mapx,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapxtop\n \
    movl    _point1mapy,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapytop\n \
    movl    _point2mapx,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapxbottom\n \
    movl    _point2mapy,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapybottom\n \
    movl    _mapyhstep,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_gploc_BC\n \
    movl    _mapxhstep,%%eax\n \
    orl %%edx,%%edx\n \
    jns gpo_loc_16CC\n \
    decl    %%eax\n \
\n \
gpo_loc_16CC:         # 14D\n \
    shll    $8,%%eax\n \
    shrl    $0x10,%%edx\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_B8\n \
    movl    _mapyveltop,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_gploc_A4\n \
    movl    _mapxveltop,%%eax\n \
    orl %%edx,%%edx\n \
    jns gpo_loc_16FA\n \
    decl    %%eax\n \
\n \
gpo_loc_16FA:         # 1507\n \
    shll    $8,%%eax\n \
    shrl    $0x10,%%edx\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_A0\n \
    movl    _startposmapytop,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_gploc_8C\n \
    movl    _startposmapxtop,%%eax\n \
    shll    $8,%%eax\n \
    shrl    $0x10,%%edx\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_88\n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_loc_17A3\n \
    movl    _mapyvelbottom,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_gploc_98\n \
    movl    _mapxvelbottom,%%eax\n \
    orl %%edx,%%edx\n \
    jns gpo_loc_175F\n \
    decl    %%eax\n \
\n \
gpo_loc_175F:         # 156C\n \
    shll    $8,%%eax\n \
    shrl    $0x10,%%edx\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_94\n \
    movl    _startposmapybottom,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_gploc_80\n \
    movl    _startposmapxbottom,%%eax\n \
    shll    $8,%%eax\n \
    shrl    $0x10,%%edx\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_7C\n \
\n \
gpo_loc_17A3:         # 155\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

void draw_gpoly_sub6() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _gploc_pt_bx,%%esi\n \
    movl    _crease_len,%%edi\n \
    orl %%edi,%%edi\n \
    subl    $0,%%esi\n \
    addl    $0,%%esi\n \
    movl    _gploc_pt_ax,%%eax\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_cx,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_by,%%ebx\n \
    subl    %%eax,%%ebx\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    %%eax,%%ecx\n \
    movl    %%ecx,%%eax\n \
    imull   %%esi,%%ecx\n \
    movl    _crease_len,%%ebp\n \
    orl %%ebp,%%ebp\n \
    js  gpo_loc_17E5\n \
    subl    %%eax,%%ecx\n \
    subl    %%eax,%%ecx\n \
\n \
gpo_loc_17E5:         # 15E\n \
    addl    %%eax,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    jz  gpo_loc_189F\n \
    xorl    %%edx,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ebx\n \
    movl    %%eax,%%ebp\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_cy,%%esi\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_by,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    %%ebp,%%eax\n \
    movl    _point1shade,%%edx\n \
    movl    _point3shade,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2shade,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1836\n \
    incl    %%eax\n \
\n \
gpo_loc_1836:         # 1643\n \
    movl    %%eax,_shadehstep\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapx,%%edx\n \
    movl    _point3mapx,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapx,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1866\n \
    incl    %%eax\n \
\n \
gpo_loc_1866:         # 1673\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapy,%%edx\n \
    movl    _point3mapy,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapy,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1896\n \
    incl    %%eax\n \
\n \
gpo_loc_1896:         # 16A3\n \
    movl    %%eax,_mapyhstep\n \
    jmp gpo_loc_18B6\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_189F:         # 15FC\n \
    xorl    %%eax,%%eax\n \
    movl    %%eax,_shadehstep\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%eax,_mapyhstep\n \
\n \
gpo_loc_18B6:         # 16AD\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif

#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_loc_19A8\n \
    movl    _gploc_pt_by,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_18DB\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_18E9\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_18DB:         # 16E0\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_18E9:         # 16E\n \
    movl    _point2shade,%%eax\n \
    subl    _point1shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_18FE\n \
    incl    %%eax\n \
\n \
gpo_loc_18FE:         # 170B\n \
    movl    %%eax,_shadeveltop\n \
    movl    _point2mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1917\n \
    incl    %%eax\n \
\n \
gpo_loc_1917:         # 172\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point2mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1930\n \
    incl    %%eax\n \
\n \
gpo_loc_1930:         # 173D\n \
    movl    %%eax,_mapyveltop\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_by,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_194D\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_195B\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_194D:         # 1752\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_195B:         # 175B\n \
    movl    _point3shade,%%eax\n \
    subl    _point2shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1970\n \
    incl    %%eax\n \
\n \
gpo_loc_1970:         # 177D\n \
    movl    %%eax,_shadevelbottom\n \
    movl    _point3mapx,%%eax\n \
    subl    _point2mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1989\n \
    incl    %%eax\n \
\n \
gpo_loc_1989:         # 1796\n \
    movl    %%eax,_mapxvelbottom\n \
    movl    _point3mapy,%%eax\n \
    subl    _point2mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_19A2\n \
    incl    %%eax\n \
\n \
gpo_loc_19A2:         # 17A\n \
    movl    %%eax,_mapyvelbottom\n \
    jmp gpo_loc_1A1A\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_19A8:         # 16CC\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    _gploc_pt_ay,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_19C1\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_19CF\n \
# ---------------------------------------------------------------------------\n \
\n \
gpo_loc_19C1:         # 17C6\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_19CF:         # 17C\n \
    movl    _point3shade,%%eax\n \
    subl    _point1shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_19E4\n \
    incl    %%eax\n \
\n \
gpo_loc_19E4:         # 17F\n \
    movl    %%eax,_shadeveltop\n \
    movl    _point3mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_19FD\n \
    incl    %%eax\n \
\n \
gpo_loc_19FD:         # 180A\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point3mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1A16\n \
    incl    %%eax\n \
\n \
gpo_loc_1A16:         # 1823\n \
    movl    %%eax,_mapyveltop\n \
\n \
gpo_loc_1A1A:         # 17B6\n \
    movl    _point1shade,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposshadetop\n \
    movl    _point1mapx,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapxtop\n \
    movl    _point1mapy,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapytop\n \
    movl    _point2shade,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposshadebottom\n \
    movl    _point2mapx,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapxbottom\n \
    movl    _point2mapy,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapybottom\n \
    movl    _mapxhstep,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movw    _gploc_word03,%%ax\n \
    orw %%ax,%%ax\n \
    jns gpo_loc_1A92\n \
    subl    $0x10000,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_1A92:         # 1898\n \
    movl    %%eax,_gploc_BC\n \
    movl    %%edx,_gploc_B8\n \
    movl    _mapyhstep,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    _mapyhstep,%%edx\n \
    sarl    $0x10,%%edx\n \
    movb    _gploc_B8,%%al\n \
    orb %%al,%%al\n \
    jns gpo_loc_1AC7\n \
    subl    $0x100,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_1AC7:         # 18CD\n \
    movl    %%eax,_gploc_B8\n \
    movl    %%edx,_gploc_B4\n \
    movl    _mapxhstep,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movw    _gploc_word03,%%ax\n \
    orw %%ax,%%ax\n \
    jns gpo_loc_1AF9\n \
    subl    $0x0FFFF,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_1AF9:         # 18F\n \
    movl    %%eax,_gploc_5C\n \
    movl    %%edx,_gploc_2C\n \
    movl    _mapyhstep,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    _mapyhstep,%%edx\n \
    sarl    $0x10,%%edx\n \
    movb    _gploc_2C,%%al\n \
    orb %%al,%%al\n \
    jns gpo_loc_1B2E\n \
    subl    $0x100,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_1B2E:         # 193\n \
    movl    %%eax,_gploc_2C\n \
    movl    %%edx,_gploc_28\n \
    movl    _mapxveltop,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movw    _gploc_word01,%%ax\n \
    orw %%ax,%%ax\n \
    jns gpo_loc_1B5A\n \
    subl    $0x10000,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_1B5A:         # 1960\n \
    movl    %%eax,_gploc_A4\n \
    movl    %%edx,_gploc_A0\n \
    movl    _mapyveltop,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    _mapyveltop,%%edx\n \
    sarl    $0x10,%%edx\n \
    movb    _gploc_A0,%%al\n \
    orb %%al,%%al\n \
    jns gpo_loc_1B89\n \
    subl    $0x100,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_1B89:         # 198\n \
    movl    %%eax,_gploc_A0\n \
    movl    %%edx,_gploc_9C\n \
    movl    _startposshadetop,%%ebx\n \
    movl    _startposmapxtop,%%ecx\n \
    movl    _startposmapytop,%%edx\n \
    movb    %%bl,_gploc_84\n \
    shrl    $8,%%ebx\n \
    movl    %%ecx,%%eax\n \
    shll    $0x10,%%ecx\n \
    shrl    $0x10,%%eax\n \
    movw    %%bx,%%cx\n \
    movl    %%ecx,_gploc_8C\n \
    movl    %%edx,%%ecx\n \
    shll    $0x10,%%ecx\n \
    shll    $8,%%edx\n \
    shrl    $0x18,%%edx\n \
    shll    $8,%%edx\n \
    movb    %%al,%%cl\n \
    movl    %%ecx,_gploc_88\n \
    movb    _gploc_84,%%dl\n \
    movl    %%edx,_gploc_84\n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_loc_1CAA\n \
    movl    _mapxvelbottom,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movw    _gploc_word02,%%ax\n \
    orw %%ax,%%ax\n \
    jns gpo_loc_1C17\n \
    subl    $0x10000,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_1C17:         # 1A1D\n \
    movl    %%eax,_gploc_98\n \
    movl    %%edx,_gploc_94\n \
    movl    _mapyvelbottom,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    _mapyvelbottom,%%edx\n \
    sarl    $0x10,%%edx\n \
    movb    _gploc_94,%%al\n \
    orb %%al,%%al\n \
    jns gpo_loc_1C46\n \
    subl    $0x100,%%eax\n \
    sbbb    $0,%%dl\n \
\n \
gpo_loc_1C46:         # 1A4C\n \
    movl    %%eax,_gploc_94\n \
    movl    %%edx,_gploc_90\n \
    movl    _startposshadebottom,%%ebx\n \
    movl    _startposmapxbottom,%%ecx\n \
    movl    _startposmapybottom,%%edx\n \
    movb    %%bl,_gploc_78\n \
    shrl    $8,%%ebx\n \
    movl    %%ecx,%%eax\n \
    shll    $0x10,%%ecx\n \
    shrl    $0x10,%%eax\n \
    movw    %%bx,%%cx\n \
    movl    %%ecx,_gploc_80\n \
    movl    %%edx,%%ecx\n \
    shll    $0x10,%%ecx\n \
    shll    $8,%%edx\n \
    shrl    $0x18,%%edx\n \
    shll    $8,%%edx\n \
    movb    %%al,%%cl\n \
    movl    %%ecx,_gploc_7C\n \
    movb    _gploc_78,%%dl\n \
    movl    %%edx,_gploc_78\n \
\n \
gpo_loc_1CAA:\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

const int test_gploc_pt_ax[] = {
    253, 253, 251, 252, 252, 251, 252, 251, 253, 253, 252, 253, 251, 254, 254, 252, 254, 254, 254,
    252, 254, 253, 254, 252, 255, 255, 252, 255, 255, 255, 255, 253, 253, 257, 254, 257, 257, 257,
    253, 258, 254, 257, 258, 258, 254, 259, 255, 259, 255, 258, 256, 260, 256, 261, 261, 261, 255,
    262, 256, 262, 257, 260, 256, 256, 256, 263, 256, 261, 258, 264, 262, 262, 257, 258, 265, 257,
    263, 266, 264, 257, 259, 264, 258, 258, 267, 258, 258, 260, 268, 259, 265, 269, 267, 259, 261,
    270, 260, 267, 271, 271, 268, 260, 263, 273, 261, 269, 262, 273, 271, 261, 264, 276, 263, 272,
    277, 277, 274, 263, 266, 280, 265, 275, 266, 281, 278, 265, 269, 285, 267, 280, 287, 287, 283,
    268, 273, 292, 271, 287, 296, 296, 292, 273, 280, 304, 278, 297, 310, 310, 306, 280};
const int test_gploc_pt_ay[] = {
    234, 234, 234, 234, 234, 234, 234, 234, 233, 233, 233, 233, 233, 232, 232, 232, 232, 231, 231,
    231, 231, 230, 230, 230, 229, 229, 229, 229, 228, 228, 228, 228, 227, 227, 226, 226, 225, 225,
    225, 225, 224, 224, 223, 223, 223, 223, 222, 222, 221, 221, 220, 220, 219, 219, 218, 218, 218,
    218, 217, 217, 216, 216, 216, 215, 215, 215, 214, 214, 213, 213, 212, 212, 212, 211, 211, 210,
    210, 209, 209, 209, 208, 208, 208, 207, 207, 206, 206, 205, 206, 204, 204, 203, 203, 203, 202,
    202, 201, 201, 199, 199, 199, 199, 197, 198, 196, 196, 195, 195, 194, 194, 192, 192, 190, 190,
    188, 188, 187, 187, 184, 185, 182, 182, 180, 180, 178, 178, 175, 175, 172, 172, 168, 168, 166,
    166, 161, 161, 156, 156, 151, 151, 147, 147, 140, 140, 132, 132, 124, 124, 117, 117};
const int test_gploc_pt_bx[] = {
    252, 252, 251, 251, 253, 252, 253, 252, 253, 253, 251, 252, 252, 253, 253, 254, 253, 253, 254,
    252, 252, 252, 253, 253, 254, 252, 252, 252, 256, 256, 256, 255, 257, 255, 257, 256, 256, 253,
    253, 253, 257, 258, 257, 254, 254, 254, 259, 258, 254, 255, 260, 258, 261, 259, 259, 255, 255,
    255, 262, 260, 255, 257, 257, 261, 256, 256, 261, 263, 264, 262, 257, 264, 264, 265, 263, 263,
    265, 263, 266, 264, 257, 259, 259, 267, 264, 265, 258, 258, 258, 265, 260, 265, 269, 267, 270,
    267, 267, 270, 260, 267, 271, 268, 273, 268, 269, 263, 273, 269, 273, 271, 276, 271, 272, 276,
    263, 272, 277, 274, 280, 274, 275, 266, 281, 275, 281, 278, 285, 278, 280, 285, 269, 280, 287,
    283, 292, 283, 287, 292, 273, 287, 296, 292, 304, 292, 297, 304, 280, 297, 310, 306};
const int test_gploc_pt_by[] = {
    235, 235, 235, 234, 234, 234, 234, 234, 234, 234, 234, 233, 233, 233, 233, 232, 233, 232, 232,
    232, 231, 231, 230, 230, 230, 229, 230, 229, 229, 228, 228, 228, 227, 228, 226, 227, 226, 225,
    226, 225, 224, 225, 224, 223, 224, 223, 222, 223, 222, 221, 220, 221, 219, 220, 219, 218, 219,
    218, 217, 218, 217, 216, 216, 216, 216, 215, 214, 215, 213, 214, 213, 213, 212, 211, 212, 210,
    211, 210, 209, 209, 209, 208, 208, 207, 208, 207, 207, 206, 206, 204, 205, 204, 203, 203, 202,
    203, 201, 202, 200, 201, 199, 199, 198, 199, 196, 197, 195, 196, 195, 194, 192, 194, 190, 192,
    189, 190, 188, 187, 185, 187, 182, 184, 180, 182, 180, 178, 175, 178, 172, 175, 169, 172, 168,
    166, 161, 166, 156, 161, 152, 156, 151, 147, 140, 147, 132, 140, 125, 132, 124, 117};
const int test_gploc_pt_cx[] = {
    251, 251, 253, 253, 251, 251, 251, 251, 251, 251, 253, 253, 253, 252, 251, 251, 251, 252, 252,
    254, 254, 255, 255, 255, 252, 255, 255, 255, 253, 253, 253, 253, 253, 253, 253, 253, 253, 256,
    256, 256, 254, 254, 254, 257, 257, 257, 254, 254, 260, 260, 254, 254, 255, 255, 255, 260, 260,
    260, 255, 255, 262, 262, 262, 262, 261, 261, 257, 257, 256, 256, 258, 258, 257, 257, 257, 258,
    258, 257, 258, 258, 267, 267, 267, 258, 258, 267, 265, 268, 265, 260, 268, 259, 259, 259, 259,
    259, 261, 261, 260, 260, 260, 260, 260, 260, 263, 273, 261, 261, 262, 262, 261, 261, 264, 264,
    263, 263, 263, 263, 263, 263, 266, 280, 265, 265, 266, 266, 265, 265, 269, 269, 267, 267, 269,
    269, 268, 268, 273, 273, 271, 271, 273, 273, 273, 273, 280, 280, 278, 278, 280, 280};
const int test_gploc_pt_cy[] = {
    235, 235, 235, 235, 235, 235, 235, 235, 234, 234, 234, 234, 234, 233, 233, 233, 233, 232, 232,
    232, 232, 231, 231, 231, 230, 230, 230, 230, 229, 229, 229, 229, 228, 228, 227, 227, 226, 226,
    226, 226, 225, 225, 224, 224, 224, 224, 223, 223, 222, 222, 221, 221, 220, 220, 219, 219, 219,
    219, 218, 218, 217, 217, 217, 216, 216, 216, 215, 215, 214, 214, 213, 213, 213, 212, 212, 211,
    211, 210, 210, 210, 209, 209, 209, 208, 208, 207, 207, 206, 207, 205, 206, 204, 204, 204, 203,
    203, 202, 202, 201, 201, 200, 200, 199, 199, 197, 198, 196, 196, 195, 195, 194, 194, 192, 192,
    190, 190, 189, 189, 187, 187, 184, 185, 182, 182, 180, 180, 178, 178, 175, 175, 172, 172, 169,
    169, 166, 166, 161, 161, 156, 156, 152, 152, 147, 147, 140, 140, 132, 132, 125, 125};
const int test_point3shade[]
    = {0,  0,  0,  0,  0,  0,  0,  0,  5,  5,  7,  7,  7,  9,  10, 10, 10, 9,  10, 13, 13, 13,
       13, 13, 9,  6,  6,  6,  7,  13, 13, 13, 10, 10, 10, 10, 9,  6,  6,  6,  7,  7,  9,  6,
       6,  6,  10, 10, 13, 13, 10, 10, 10, 10, 9,  6,  6,  6,  9,  9,  13, 13, 13, 7,  6,  6,
       7,  7,  10, 10, 7,  7,  9,  10, 10, 7,  7,  9,  13, 13, 13, 13, 13, 9,  9,  7,  6,  13,
       6,  7,  13, 9,  13, 13, 10, 10, 7,  7,  9,  9,  13, 13, 10, 10, 7,  13, 9,  9,  13, 13,
       10, 10, 7,  7,  9,  9,  13, 13, 10, 10, 7,  13, 9,  9,  13, 13, 10, 10, 7,  7,  9,  9,
       13, 13, 10, 10, 7,  7,  9,  9,  13, 13, 10, 10, 7,  7,  9,  9,  13, 13};
const int test_point1shade[] = {
    0,  0,  0,  0,  0,  0, 0,  0,  4,  4,  4,  5,  5, 7,  7,  7,  13, 7,  7,  7,  9,  7, 9,  9, 7,
    7,  10, 13, 9,  9,  6, 10, 7,  13, 7,  13, 7,  7, 10, 13, 9,  9,  7,  7,  10, 13, 7, 13, 7, 9,
    7,  13, 7,  13, 7,  7, 10, 13, 13, 7,  7,  9,  9, 10, 10, 13, 9,  9,  7,  13, 9,  9, 13, 7, 13,
    9,  9,  7,  6,  10, 7, 9,  9,  13, 7,  10, 10, 7, 13, 9,  9,  7,  6,  10, 7,  13, 9, 9,  7, 7,
    6,  10, 7,  13, 9,  9, 13, 7,  6,  10, 7,  13, 9, 9,  7,  7,  6,  10, 7,  13, 9,  9, 13, 7, 6,
    10, 7,  13, 9,  9,  7, 7,  6,  10, 7,  13, 9,  9, 7,  7,  6,  10, 7,  13, 9,  9,  7, 7,  6, 10};
const int test_point2shade[]
    = {0,  0,  0,  0,  0,  0,  0,  0,  5,  3,  5,  4,  4,  9,  6,  13, 6,  9,  6, 10, 7,  10,
       7,  7,  9,  10, 13, 10, 13, 7,  7,  6,  13, 6,  13, 6,  9,  10, 13, 10, 9, 13, 9,  10,
       13, 10, 13, 6,  10, 7,  13, 6,  13, 6,  9,  10, 13, 10, 7,  9,  10, 7,  7, 6,  13, 10,
       9,  13, 13, 6,  9,  13, 7,  13, 6,  9,  13, 9,  7,  6,  10, 7,  7,  7,  9, 6,  13, 10,
       10, 9,  7,  9,  7,  6,  13, 6,  9,  13, 13, 9,  7,  6,  13, 6,  9,  7,  7, 9,  7,  6,
       13, 6,  9,  13, 13, 9,  7,  6,  13, 6,  9,  7,  7,  9,  7,  6,  13, 6,  9, 13, 13, 9,
       7,  6,  13, 6,  9,  13, 13, 9,  7,  6,  13, 6,  9,  13, 13, 9,  7,  6};
const int test_point3mapx[]
    = {31, 0,  0,  31, 0,  0,  31, 31, 31, 0,  0,  31, 31, 31, 0,  31, 31, 31, 0,  0,  31, 0,
       31, 31, 31, 0,  0,  31, 31, 0,  31, 31, 31, 31, 31, 31, 31, 0,  0,  31, 31, 31, 31, 0,
       0,  31, 31, 31, 0,  31, 31, 31, 31, 31, 31, 0,  0,  31, 31, 31, 0,  31, 31, 31, 0,  31,
       31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 0,  31, 31, 31, 31, 31, 0,  0,
       31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
       31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
       31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31};
const int test_point1mapx[]
    = {0,  31, 0, 0, 31, 31, 0, 0, 0, 31, 0, 0, 0,  0, 31, 0, 0, 0, 31, 0, 0, 0, 0, 0, 0,
       31, 0,  0, 0, 31, 0,  0, 0, 0, 0,  0, 0, 31, 0, 0,  0, 0, 0, 31, 0, 0, 0, 0, 0, 0,
       0,  0,  0, 0, 0,  31, 0, 0, 0, 0,  0, 0, 0,  0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0,
       0,  0,  0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0,
       0,  0,  0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0,
       0,  0,  0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0,  0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0};
const int test_point3mapy[]
    = {31, 0,  0,  0,  0,  0,  31, 31, 31, 0,  0,  0,  31, 31, 0,  31, 31, 31, 0,  0,  0,  0,
       0,  31, 31, 31, 0,  0,  31, 0,  31, 31, 31, 31, 31, 31, 31, 31, 0,  0,  31, 31, 31, 31,
       0,  0,  31, 31, 0,  0,  31, 31, 31, 31, 31, 31, 0,  0,  31, 31, 0,  0,  31, 31, 0,  0,
       31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 0,  0,  31, 31, 31, 31, 0,  0,
       0,  31, 0,  31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 0,  31, 31, 31, 31,
       31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 0,  31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
       31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31};
const int test_point1mapy[]
    = {0,  31, 31, 0,  31, 0,  0,  31, 0,  31, 31, 0,  0,  0,  31, 31, 0,  0,  31, 31, 0,  31,
       0,  0,  0,  31, 31, 0,  0,  31, 0,  31, 31, 0,  31, 0,  0,  31, 31, 0,  31, 0,  0,  31,
       31, 0,  31, 0,  31, 0,  31, 0,  31, 0,  0,  31, 31, 0,  31, 0,  31, 0,  0,  0,  31, 0,
       31, 0,  31, 0,  0,  0,  31, 31, 0,  31, 0,  0,  0,  31, 31, 0,  0,  31, 0,  0,  31, 31,
       0,  31, 0,  0,  0,  31, 31, 0,  31, 0,  0,  0,  0,  31, 31, 0,  31, 0,  31, 0,  0,  31,
       31, 0,  31, 0,  0,  0,  0,  31, 31, 0,  31, 0,  31, 0,  0,  31, 31, 0,  31, 0,  0,  0,
       0,  31, 31, 0,  31, 0,  0,  0,  0,  31, 31, 0,  31, 0,  0,  0,  0,  31};
const int test_point2mapx[]
    = {31, 0,  31, 31, 0,  31, 31, 0,  31, 0,  31, 31, 31, 31, 0,  0,  31, 31, 0,  31, 31, 31,
       31, 31, 31, 0,  31, 31, 31, 0,  31, 0,  0,  31, 0,  31, 31, 0,  31, 31, 0,  31, 31, 0,
       31, 31, 0,  31, 31, 31, 0,  31, 0,  31, 31, 0,  31, 31, 0,  31, 31, 31, 31, 0,  31, 31,
       0,  31, 0,  31, 0,  31, 0,  0,  31, 0,  31, 31, 31, 0,  31, 31, 31, 0,  31, 0,  31, 31,
       31, 0,  31, 31, 31, 0,  0,  31, 0,  31, 0,  31, 31, 0,  0,  31, 0,  31, 0,  31, 31, 0,
       0,  31, 0,  31, 0,  31, 31, 0,  0,  31, 0,  31, 0,  31, 31, 0,  0,  31, 0,  31, 0,  31,
       31, 0,  0,  31, 0,  31, 0,  31, 31, 0,  0,  31, 0,  31, 0,  31, 31, 0};
const int test_crease_len[]
    = {-65536,   -65536,   131072,   65536,    -65536,   -65536,   -65536,   -65536,   -131072,
       -131072,  131072,   65536,    -65536,   -65536,   -131072,  -131072,  -131072,  -65536,
       -131072,  131072,   131072,   196608,   65536,    -65536,   -131072,  196608,   196608,
       196608,   -196608,  -65536,   -65536,   -131072,  -262144,  -131072,  -196608,  -196608,
       -196608,  262144,   196608,   327680,   -196608,  -262144,  -196608,  262144,   196608,
       327680,   -262144,  -262144,  393216,   196608,   -262144,  -262144,  -327680,  -262144,
       -262144,  393216,   327680,   458752,   -393216,  -327680,  458752,   196608,   -65536,
       65536,    327680,   458752,   -327680,  -393216,  -393216,  -393216,  65536,    -393216,
       -458752,  -458752,  -393216,  -393216,  -458752,  -393216,  -131072,  -458752,  655360,
       327680,   -65536,   -589824,  -393216,  131072,   458752,   655360,   655360,   -393216,
       425984,   -393216,  -131072,  -524288,  -589824,  -524288,  -458752,  -589824,  360448,
       -458752,  -196608,  -524288,  -753664,  -524288,  -524288,  524288,   -720896,  -524288,
       -720896,  -655360,  -786432,  -655360,  -589824,  -786432,  458752,   -589824,  -557056,
       -720896,  -983040,  -720896,  -655360,  808276,   -983040,  -655360,  -983040,  -851968,
       -1048576, -851966,  -851968,  -1048574, 851968,   -851968,  -873812,  -983040,  -1245184,
       -983036,  -1048576, -1245180, 1179648,  -1048576, -1258288, -1245184, -1572864, -1245178,
       -1245184, -1572864, 1703936,  -1245184, -1753088, -1703936};
const int test_shadehstep[]
    = {0,       0,      0,      0,      0,       0,       0,      0,      0,      -43691, 43691,
       32767,   -32767, 0,      -87381, 131072,  -87381,  0,      -87381, 65536,  43691,  49152,
       65536,   -65536, 0,      -49152, -114688, 49152,   98303,  -65536, 32767,  -87381, 78643,
       -87381,  98303,  -65536, 0,      -39322,  -114688, 32767,  0,      78643,  0,      -39322,
       -114688, 32767,  78643,  -52429, 28086,   32767,   78643,  -52429, 65536,  -52429, 0,
       -28086,  -76458, 24575,  -56174, 0,       24575,   32767,  -65536, 32767,  -76458, 24575,
       0,       56174,  56174,  -37450, -65536,  56174,   -49152, 49152,  -37450, 0,      49152,
       0,       21845,  -32767, 17873,  21845,   -65536,  -39322, 0,      21845,  -57344, 17873,
       17873,   0,      34953,  0,      21845,   -29127,  39322,  -29127, 0,      39322,  -50413,
       0,       16383,  -29127, 23592,  -29127,  0,       29127,  -32767, 0,      -32767, -23831,
       30247,   -23831, 0,      30247,  -40960,  0,       -17246, -21845, 20479,  -21845, 0,
       22937,   -24575, 0,      -24575, -18724,  23130,   -18724, 0,      23130,  -25746, 0,
       -16765,  -16383, 19660,  -16383, 0,       19660,   -19315, 0,      -14924, -13107, 15728,
       -13107,  0,      15728,  -13956, 0,       -12103,  -9709};
const int test_mapxhstep[] = {
    0,       0,        -677205,  -1015807, -1015807, 0,       1015807, 0,       0,       0,
    -677205, -1015807, 1015807,  0,        0,        0,       0,       0,       0,       -677205,
    -677205, -507904,  -1015807, 1015807,  0,        507904,  -507904, -507904, 0,       -1015807,
    1015807, 0,        0,        0,        0,        0,       0,       406323,  -507904, -338602,
    0,       0,        0,        406323,   -507904,  -338602, 0,       0,       -290230, -507904,
    0,       0,        0,        0,        0,        290230,  -338602, -253952, 0,       0,
    -253952, -507904,  1015807,  1015807,  -338602,  -253952, 0,       0,       0,       0,
    1015807, 0,        0,        0,        0,        0,       0,       0,       677205,  0,
    -184693, -338602,  1015807,  0,        0,        677205,  -253952, -184693, -184693, 0,
    -135441, 0,        677205,   0,        0,        0,       0,       0,       156278,  0,
    507904,  0,        -81264,   0,        0,        -112868, 0,       0,       0,       0,
    0,       0,        0,        0,        126976,   0,       106928,  0,       -42326,  0,
    0,       -50791,   0,        0,        0,        0,       0,       0,       0,       0,
    36279,   0,        47247,    0,        0,        0,       0,       0,       21385,   0,
    20115,   0,        0,        0,        0,        0,       9405,    0,       9151,    0};
const int test_mapyhstep[] = {
    -1015807, 1015807,  -677205,  -1015807, 0,       1015807, 0,       -1015807, -677205, 677205,
    -677205,  -1015807, 0,        -1015807, 677205,  -677205, -677205, -1015807, 677205,  -677205,
    -677205,  -507904,  -1015807, 0,        -677205, 507904,  -507904, -507904,  -507904, 0,
    0,        -677205,  -406323,  -677205,  -507904, -507904, -507904, 406323,   -507904, -338602,
    -507904,  -406323,  -507904,  406323,   -507904, -338602, -406323, -406323,  -290230, -507904,
    -406323,  -406323,  -338602,  -406323,  -406323, 290230,  -338602, -253952,  -290230, -338602,
    -253952,  -507904,  0,        0,        -338602, -253952, -338602, -290230,  -290230, -290230,
    0,        -290230,  -253952,  -253952,  -290230, -290230, -253952, -290230,  0,       -253952,
    -184693,  -338602,  0,        -203161,  -290230, 0,       -253952, -184693,  -184693, -290230,
    -270882,  -290230,  0,        -225735,  -203161, -225735, -253952, -203161,  -156278, -253952,
    0,        -225735,  -162529,  -225735,  -225735, -225735, -169302, -225735,  -169302, -184693,
    -156278,  -184693,  -203161,  -156278,  -126976, -203161, -106928, -169302,  -126976, -169302,
    -184693,  -152371,  -126976,  -184693,  -126976, -145115, -119507, -145115,  -145115, -119507,
    -108837,  -145115,  -94493,   -126976,  -101581, -126976, -119507, -101581,  -85541,  -119507,
    -80460,   -101581,  -81264,   -101581,  -101581, -81264,  -65839,  -101581,  -64060,  -75245};

const int BIT_SHIFT_16 = 16;
const int BIT_SHIFT_32 = 32;
const int MAX_INT_DIV = 0x7FFFFFFF;  // Max int value for normalization/division

/**
 * Concatenates two 16-bit integers into a 32-bit integer.
 *
 * This most likely builds a 16.16 fixed point number from a whole and fractional part.
 *
 * @param high The high 16 bits.
 * @param low The low 16 bits.
 * @return The combined 32-bit integer.
 */
static inline uint32_t combineHighLowBits(uint16_t high, uint16_t low) {
  return ((uint32_t)high << BIT_SHIFT_16) | (uint32_t)low;
}

/**
 * Calculates the barycentric coordinates for affine texture mapping
 *
 * @param scaleFactor The inverse determinant (for multiplication instead of divide).
 * @param deltaP_C_A The delta parameter from vertex A to vertex C.
 * @param deltaP_B_A The delta parameter from vertex A to vertex B.
 * @param deltaY_B_A The delta Y from vertex A to vertex B.
 * @param deltaY_C_A The delta Y from vertex A to vertex C.
 * @return The calculated thingy.
 */
static int calculateParameter(int scaleFactor, int deltaP_C_A, int deltaP_B_A, int deltaY_B_A,
                              int deltaY_C_A) {
  long long int lVar1;
  int iVar5;
  int iVar2;
  ushort upperHalf;
  int result;

  // This line computes a scaled difference concerning both the Y-deltas between the vertex A and
  // the other vertices B and C. This kind of operation is reminiscent of how weights are calculated
  // in barycentric coordinates or how gradients are derived in affine texture mapping. This
  // particular multiplication can be seen as determining the influence of the vertex position
  // deltas on the texture coordinates, modified by the scaleFactor (related to the determinant
  // which, in graphical terms, could relate to the area of the triangle or a similar geometric
  // factor influencing texture coordinate scaling).
  lVar1 = (long long int)scaleFactor
          * (long long int)(deltaP_C_A * deltaY_B_A - deltaP_B_A * deltaY_C_A);

  // The resulting value `lVar1` serves as an intermediary step that could relate to an incremental
  // differential area calculation concerning texturing, which might then be used to establish
  // actual texture coordinate mappings across the triangle by offsetting base coordinates by these
  // computed values.

  iVar5 = (int)lVar1;
  iVar2 = iVar5 << 1;
  upperHalf = (ushort)((uint)iVar2 >> BIT_SHIFT_16);
  result = combineHighLowBits(upperHalf,
                              (ushort)((int)((unsigned long long int)lVar1 >> BIT_SHIFT_32) << 1)
                                  | (ushort)(iVar5 < 0))
               << BIT_SHIFT_16
           | (uint)upperHalf;

  // Correction step
  if (iVar2 < 0) {
    result++;
  }

  return result;
}

/*
### Algorithm Insights

The code calculates properties for triangle rasterization, possibly interpolation values or
coordinates using affine or similar transformations based on triangle vertex positions. This process
typically involves the following steps in graphical computations:

- **Determinant Calculation**: The determinant essentially informs about the orientation and the
area spanned by the triangle in 2D space. A zero determinant indicates collinear points, hence no
valid triangle.

- **Scaling and Interpolation**:
   - **Scale Factor Calculation**: This involves normalization with `0x7FFFFFFF`, possibly to
maintain precision or manage overflow in subsequent calculations. This scale factor is then used to
determine how the linear interpolation should be scaled across the triangle’s area.
   - **Interpolation Calculation**: `calculateParameter()` uses affine transformations or similar
math to project properties (like texture coordinates, colors, etc.) across the rasterized triangle.
The combination of subtraction and multiplication likely corresponds to a form of barycentric
coordinate computation or a direct calculation for interpolating vertex attributes.

- **Bit Manipulation for Result Adjustment**: This ensures that calculated values are properly
aligned and formatted, likely adjusting for specific requirements of the rasterization hardware or
software algorithm, or to pack multiple small values into a single integer for performance reasons.

### Detailed Algorithm Insights:

Each vertex seems to be associated with three parameters: S, U, and V. While U and V are
traditionally used in graphics programming to represent texture coordinates mapped onto a 3D model's
surface, S could potentially be another dimension or attribute used in texturing (like a secondary
set of texture coordinates or special shading/scaling factor). However, without further context,
it's conjecture.

1. **Affine Texture Mapping**: The algorithm apparently performs calculations required for affine
texture mapping by computing transformed parameters (S, U, V) to apply textures to a 2D projection
of a 3D triangle. The affine transformation ensures that texture coordinates change linearly over
the triangle’s surface.

2. **Barycentric Coordinate Computation**:
   - The computation carried out in `calculateParameter` looks like a form of barycentric coordinate
computation which is used for interpolating vertex attributes like textures across the surface of a
triangle.
   - Barycentric coordinates allow an arbitrary point within a triangle to be expressed as a sum of
scaled vertex positions. Here, `delta1`, `delta2`, `deltaY_B_A`, `deltaY_C_A` likely assist in
calculating how much each vertex contributes to the final pixel position in the transformed texture
space.

3. **Bit Manipulations**:
   - The use of bit shifts and combinations (`combineHighLowBits`) within `calculateParameter` might
be needed to accommodate precision requirements or hardware-specific optimization. These operations
might package the outputs into a format that's suitable for quick retrieval and use by a rasterizer
or texture mapping unit in a graphics pipeline.

https://mtrebi.github.io/2017/03/15/texture-mapping-affine-perspective.html
*/
static void calculateTriangleProperties() {
  int deltaProduct;
  int deltaX_B_A;  // Delta X from vertex A to vertex B
  int deltaY_B_A;  // Delta Y from vertex A to vertex B
  int deltaX_C_A;  // Delta X from vertex A to vertex C
  int deltaY_C_A;  // Delta Y from vertex A to vertex C
  int determinant;
  int scaleFactor;

  //   #### Variable Renaming Suggestions:
  // 1. **Vertices related (`gploc_pt_*`)**:
  //    - `gploc_pt_ax`, `gploc_pt_ay` -> `vertexA_x`, `vertexA_y`
  //    - `gploc_pt_bx`, `gploc_pt_by` -> `vertexB_x`, `vertexB_y`
  //    - `gploc_pt_cx`, `gploc_pt_cy` -> `vertexC_x`, `vertexC_y`

  // Calculate triangle delta values
  deltaX_B_A = gploc_pt_bx - gploc_pt_ax;
  deltaY_B_A = gploc_pt_by - gploc_pt_ay;
  deltaX_C_A = gploc_pt_cx - gploc_pt_ax;
  deltaY_C_A = gploc_pt_cy - gploc_pt_ay;

  deltaProduct = deltaY_C_A * deltaX_B_A;
  if (-1 < crease_len) {
    deltaProduct = (deltaProduct - deltaY_C_A) - deltaY_C_A;
  }

  // Calculate determinant
  determinant = deltaY_B_A * deltaX_C_A - (deltaProduct + deltaY_C_A);
  if (determinant == 0) {  // Invalid triangle
    shadehstep = 0;
    mapxhstep = 0;
    mapyhstep = 0;
  } else {
    scaleFactor = (int)(MAX_INT_DIV / (long long int)determinant);
    // Variable rename suggestions:
    // point3shade -> vertexC_s
    // point1shade -> vertexA_s
    // point2shade -> vertexB_s
    // point3mapx -> vertexC_u
    // point1mapx -> vertexA_u
    // point2mapx -> vertexB_u
    // point3mapy -> vertexC_v
    // point1mapy -> vertexA_v
    // point2mapy -> vertexB_v
    // shadehstep -> factorS
    // mapxhstep -> factorU
    // mapyhstep -> factorV
    shadehstep = calculateParameter(scaleFactor, point3shade - point1shade,
                                    point2shade - point1shade, deltaY_B_A, deltaY_C_A);
    mapxhstep = calculateParameter(scaleFactor, point3mapx - point1mapx, point2mapx - point1mapx,
                                   deltaY_B_A, deltaY_C_A);
    mapyhstep = calculateParameter(scaleFactor, point3mapy - point1mapy, point2mapy - point1mapy,
                                   deltaY_B_A, deltaY_C_A);
  }
}

/*
 1. **Global Vertex Coordinates**:
    - `gploc_pt_ax`, `gploc_pt_ay`
    - `gploc_pt_bx`, `gploc_pt_by`
    - `gploc_pt_cx`, `gploc_pt_cy`

2. **Global Control Variables**:
   - `point3shade`, `point1shade`
   - `point2shade`
   - `point3mapx`, `point1mapx`
   - `point3mapy`, `point1mapy`
   - `point2mapx`

3. **Condition Check Variable**:
   - `crease_len`: Determines an adaptation in the computation of `tempCalculation`.

These global variables will need to be defined and initialized appropriately for tests. Since
they are external to the function, it might be beneficial to encapsulate them or pass them as
parameters for a more testable, self-contained function.

### Output Variables:
The function primarily alters three global variables, which are the direct outputs:
- `shadehstep`
- `mapxhstep`
- `mapyhstep`
*/

static void calc_hstep();

void test_calculateTriangleProperties() {
  JUSTLOG("Starting tests");

  for (int i = 0; i < 150; i++) {
    gploc_pt_ax = test_gploc_pt_ax[i];
    gploc_pt_ay = test_gploc_pt_ay[i];
    gploc_pt_bx = test_gploc_pt_bx[i];
    gploc_pt_by = test_gploc_pt_by[i];
    gploc_pt_cx = test_gploc_pt_cx[i];
    gploc_pt_cy = test_gploc_pt_cy[i];
    point3shade = test_point3shade[i];
    point1shade = test_point1shade[i];
    point2shade = test_point2shade[i];
    point3mapx = test_point3mapx[i];
    point1mapx = test_point1mapx[i];
    point3mapy = test_point3mapy[i];
    point1mapy = test_point1mapy[i];
    point2mapx = test_point2mapx[i];
    crease_len = test_crease_len[i];

    // JUSTLOG(
    //     "[test-inputs] gploc_pt_ax=%d, gploc_pt_ay=%d, gploc_pt_bx=%d, gploc_pt_by=%d, "
    //     "gploc_pt_cx=%d, "
    //     "gploc_pt_cy=%d, point3shade=%d, point1shade=%d, point2shade=%d, point3mapx=%d,
    //     point1mapx=%d, " "point3mapy=%d, point1mapy=%d, point2mapx=%d, crease_len=%d",
    //     gploc_pt_ax, gploc_pt_ay, gploc_pt_bx, gploc_pt_by, gploc_pt_cx, gploc_pt_cy,
    //     point3shade, point1shade, point2shade, point3mapx, point1mapx, point3mapy, point1mapy,
    //     point2mapx, crease_len);

    calc_hstep();

    int valid_shadehstep = shadehstep;
    int valid_mapxhstep = mapxhstep;
    int valid_mapyhstep = mapyhstep;

    calculateTriangleProperties();

    // JUSTLOG("[test-outputs] shadehstep=%d, mapxhstep=%d, mapyhstep=%d", shadehstep, mapxhstep,
    // mapyhstep);

    if (shadehstep != valid_shadehstep) {
      JUSTLOG("Test %d failed for shadehstep. Expected=%d, Got=%d", i, valid_shadehstep,
              shadehstep);
    }

    if (mapxhstep != valid_mapxhstep) {
      JUSTLOG("Test %d failed for mapxhstep. Expected=%d, Got=%d", i, valid_mapxhstep, mapxhstep);
    }

    if (mapyhstep != valid_mapyhstep) {
      JUSTLOG("Test %d failed for mapyhstep. Expected=%d, Got=%d", i, valid_mapyhstep, mapyhstep);
    }
  }
}

#if 0
static int calculateInterpolatedResult(int factor, int difference) {
  long long temp = factor * (long long)(difference * 2);
  int result = ((int)(temp >> 16)
                + (temp < 0));  // incorporating high bits shift and handling potential negatives
  return result;
}

static int processHighLowBits(int value) {
  unsigned int low = value & 0xFFFF;
  unsigned int high = (value >> 16) & 0xFFFF;
  return (low | (high << 16));  // Example: swap high and low halves of the value
}

static int combineHighLowResults(int highBits, int lowBits) {
  return ((highBits << 16)
          | (lowBits & 0xFFFF));  // Or any other formula that combines two integer values
}

// TODO: Implement
static int processCombinedResults(int foo, int bar) { return 0; }

static void draw_gpoly_sub7_subfunc2() {
  // Assume all integers unless specified
  int deltaA, deltaB, deltaC;
  int factor;

  if (crease_len < 0) {
    deltaA = gploc_pt_cy - gploc_pt_ay;
    factor = (deltaA > 255) ? (0x7FFFFFFF / deltaA) : gpoly_reptable[deltaA];
    // Using function to mimic rotation operation and conditional increment
    shadeveltop = calculateInterpolatedResult(factor, point3shade - point1shade);
    mapxveltop = calculateInterpolatedResult(factor, point3mapx - point1mapx);
    mapyveltop = calculateInterpolatedResult(factor, point3mapy - point1mapy);
  } else {
    deltaB = gploc_pt_by - gploc_pt_ay;
    factor = (deltaB > 255) ? (0x7FFFFFFF / deltaB) : gpoly_reptable[deltaB];
    shadeveltop = calculateInterpolatedResult(factor, point2shade - point1shade);
    mapxveltop = calculateInterpolatedResult(factor, point2mapx - point1mapx);
    mapyveltop = calculateInterpolatedResult(factor, point2mapy - point1mapy);

    deltaC = gploc_pt_cy - gploc_pt_by;
    factor = (deltaC > 255) ? (0x7FFFFFFF / deltaC) : gpoly_reptable[deltaC];
    shadevelbottom = calculateInterpolatedResult(factor, point3shade - point2shade);
    mapxvelbottom = calculateInterpolatedResult(factor, point3mapx - point2mapx);
    mapyvelbottom = calculateInterpolatedResult(factor, point3mapy - point2mapy);
  }

  startposshadetop = point1shade << 16;
  startposmapxtop = point1mapx << 16;
  startposmapytop = point1mapy << 16;
  startposshadebottom = point2shade << 16;
  startposmapxbottom = point2mapx << 16;
  startposmapybottom = point2mapy << 16;

  // Processing results with presumed utility functions to handle the bit manipulations
  // These functions will require definition based on specific handling or masking requirements
  // observed in original code
  gploc_BC = combineHighLowResults(mapyhstep, shadehstep);
  gploc_B8 = processHighLowBits(mapxhstep);
  gploc_5C = combineHighLowResults(mapyhstep, shadehstep);
  gploc_2C = processHighLowBits(mapxhstep);
  gploc_A4 = combineHighLowResults(mapyveltop, shadeveltop);
  gploc_A0 = processHighLowBits(mapxveltop);
  gploc_8C = processCombinedResults(startposshadetop, startposmapytop);
  gploc_88 = processCombinedResults(startposmapxtop, startposmapytop);

  if (crease_len >= 0) {
    gploc_98 = combineHighLowResults(mapyvelbottom, shadevelbottom);
    gploc_94 = processHighLowBits(mapxvelbottom);
    gploc_80 = processCombinedResults(startposshadebottom, startposmapybottom);
    gploc_7C = processCombinedResults(startposmapxbottom, startposmapybottom);
  }
}
#endif

#include <stdbool.h>  // TODO: Move this

// Emulate CARRY4 by checking for overflow after adding two 32-bit integers
static inline bool CARRY4(uint32_t a, uint32_t b) {
  uint32_t result = a + b;
  return a > UINT32_MAX - b;
}

static inline void pack_startpos_textshade_bottom() {
  gploc_80 = startposmapybottom << 0x10 | startposshadebottom >> 8;
  gploc_7C = startposmapybottom >> (0x10 & 0xff) | startposmapxbottom << 8;
}

/*
static inline uint32_t processFixedPointMultiplication(int operand1, int operand2) {
    long long int product = (long long int)operand1 * (long long int)operand2;
    uint16_t high = (uint16_t)(product >> 16);  // Extract high and low parts, isolate
    uint16_t low = (uint16_t)(product >> 32);
    return combineHighLowBits(high, low);  // Creating a fixed-point result in 16.16 format
}

// Usage in modified context
int step1Diff = (point3shade - point1shade) * 2;
uint32_t fixedPointResult = processFixedPointMultiplication(step1Diff, diffModifier);
shadeveltop = fixedPointResult;  // Now as a uint32_t, not a pointer
*/

void draw_gpoly_sub7_subfunc2_refactor() {
  long long int lVar1;
  int iVar2;
  ushort uVar4;
  uint uVar3;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  bool bVar9;

  // Variable rename suggestions:
  // point3shade -> vertexC_s
  // point1shade -> vertexA_s
  // point2shade -> vertexB_s
  // point3mapx -> vertexC_u
  // point1mapx -> vertexA_u
  // point2mapx -> vertexB_u
  // point3mapy -> vertexC_v
  // point1mapy -> vertexA_v
  // point2mapy -> vertexB_v
  // shadehstep -> factorS
  // mapxhstep -> factorU
  // mapyhstep -> factorV

  if (crease_len < 0) {
    iVar5 = gploc_pt_cy - gploc_pt_ay;
    if (iVar5 < 0x100) {
      iVar5 = *(int *)(&gpoly_reptable + iVar5 * 4);
    } else {
      iVar5 = (int)(0x7fffffff / (long long int)iVar5);
    }
    // int step1Diff = (point3shade - point1shade) * 2;
    iVar2 = (point3shade - point1shade) * 2;
    lVar1
        = (long long int)iVar2
          * (long long int)iVar5;  // Multiply two 16.16 fixed point numbers which results in 32.32
    uVar4 = (ushort)((unsigned long long int)lVar1
                     >> 0x10);  // Normalize it back into 16.16 and isolate the fractional part
    shadeveltop = (struct PolyPoint *)(combineHighLowBits(
                                           uVar4, (short)((unsigned long long int)lVar1 >> 0x20))
                                           << 0x10
                                       | (uint)uVar4);
    // uint32_t fixedPointResult = processFixedPointMultiplication(step1Diff, diffModifier);
    // shadeveltop = fixedPointResult;  // Now as a uint32_t, not a pointer
    if (iVar2 < 0) {
      shadeveltop = (struct PolyPoint *)((int)&shadeveltop->X + 1);
    }
    iVar2 = (point3mapx - point1mapx) * 2;
    lVar1 = (long long int)iVar2 * (long long int)iVar5;
    uVar4 = (ushort)((unsigned long long int)lVar1 >> 0x10);
    mapxveltop = combineHighLowBits(uVar4, (short)((unsigned long long int)lVar1 >> 0x20)) << 0x10
                 | (uint)uVar4;
    if (iVar2 < 0) {
      mapxveltop++;
    }
    iVar2 = (point3mapy - point1mapy) * 2;
    lVar1 = (long long int)iVar2 * (long long int)iVar5;
    uVar4 = (ushort)((unsigned long long int)lVar1 >> 0x10);
    mapyveltop = combineHighLowBits(uVar4, (short)((unsigned long long int)lVar1 >> 0x20)) << 0x10
                 | (uint)uVar4;
    if (iVar2 < 0) {
      mapyveltop++;
    }
  } else {
    iVar5 = gploc_pt_by - gploc_pt_ay;
    if (iVar5 < 0x100) {
      iVar5 = *(int *)(&gpoly_reptable + iVar5 * 4);
    } else {
      iVar5 = (int)(0x7fffffff / (long long int)iVar5);
    }
    iVar2 = (point2shade - point1shade) * 2;
    lVar1 = (long long int)iVar2 * (long long int)iVar5;
    uVar4 = (ushort)((unsigned long long int)lVar1 >> 0x10);
    shadeveltop = (struct PolyPoint *)(combineHighLowBits(
                                           uVar4, (short)((unsigned long long int)lVar1 >> 0x20))
                                           << 0x10
                                       | (uint)uVar4);
    if (iVar2 < 0) {
      shadeveltop = (struct PolyPoint *)((int)&shadeveltop->X + 1);
    }
    iVar2 = (point2mapx - point1mapx) * 2;
    lVar1 = (long long int)iVar2 * (long long int)iVar5;
    uVar4 = (ushort)((unsigned long long int)lVar1 >> 0x10);
    mapxveltop = combineHighLowBits(uVar4, (short)((unsigned long long int)lVar1 >> 0x20)) << 0x10
                 | (uint)uVar4;
    if (iVar2 < 0) {
      mapxveltop++;
    }
    iVar2 = (point2mapy - point1mapy) * 2;
    lVar1 = (long long int)iVar2 * (long long int)iVar5;
    uVar4 = (ushort)((unsigned long long int)lVar1 >> 0x10);
    mapyveltop = combineHighLowBits(uVar4, (short)((unsigned long long int)lVar1 >> 0x20)) << 0x10
                 | (uint)uVar4;
    if (iVar2 < 0) {
      mapyveltop++;
    }
    iVar5 = gploc_pt_cy - gploc_pt_by;
    if (iVar5 < 0x100) {
      iVar5 = *(int *)(&gpoly_reptable + iVar5 * 4);
    } else {
      iVar5 = (int)(0x7fffffff / (long long int)iVar5);
    }
    iVar2 = (point3shade - point2shade) * 2;
    lVar1 = (long long int)iVar2 * (long long int)iVar5;
    uVar4 = (ushort)((unsigned long long int)lVar1 >> 0x10);
    shadevelbottom = combineHighLowBits(uVar4, (short)((unsigned long long int)lVar1 >> 0x20))
                         << 0x10
                     | (uint)uVar4;
    if (iVar2 < 0) {
      shadevelbottom++;
    }
    iVar2 = (point3mapx - point2mapx) * 2;
    lVar1 = (long long int)iVar2 * (long long int)iVar5;
    uVar4 = (ushort)((unsigned long long int)lVar1 >> 0x10);
    mapxvelbottom = combineHighLowBits(uVar4, (short)((unsigned long long int)lVar1 >> 0x20))
                        << 0x10
                    | (uint)uVar4;
    if (iVar2 < 0) {
      mapxvelbottom++;
    }
    iVar2 = (point3mapy - point2mapy) * 2;
    lVar1 = (long long int)iVar2 * (long long int)iVar5;
    uVar4 = (ushort)((unsigned long long int)lVar1 >> 0x10);
    mapyvelbottom = combineHighLowBits(uVar4, (short)((unsigned long long int)lVar1 >> 0x20))
                        << 0x10
                    | (uint)uVar4;
    if (iVar2 < 0) {
      mapyvelbottom++;
    }
  }
  startposshadetop = point1shade << 0x10;
  startposmapxtop = point1mapx << 0x10;
  startposmapytop = point1mapy << 0x10;
  startposshadebottom = point2shade << 0x10;
  startposmapxbottom = point2mapx << 0x10;
  startposmapybottom = point2mapy << 0x10;
  uVar3 = mapyhstep * 0x10000;
  iVar2 = mapyhstep >> 0x10;
  gploc_30 = shadehstep << 0x18;
  uVar7 = shadehstep >> 8;
  iVar5 = iVar2;
  uVar8 = uVar7;
  if ((int)uVar7 < 0) {
    uVar8 = uVar7 & 0xffff;
    bVar9 = uVar3 < 0x10000;
    uVar3 = uVar3 - 0x10000;
    iVar5 = iVar2 - (uint)bVar9;
  }
  uVar6 = iVar5 + (uint)CARRY4(uVar3, uVar8);
  gploc_BC = uVar3 + uVar8;
  iVar5 = mapxhstep;
  if ((int)uVar6 < 0) {
    iVar5 = mapxhstep - 1;
  }
  gploc_B8 = (uVar6 & 0xff) | iVar5 << 8;
  uVar8 = mapyhstep * 0x10000;
  if ((int)uVar7 < 0) {
    uVar7 = uVar7 & 0xffff;
    bVar9 = uVar8 < 0xffff;
    uVar8 = uVar8 - 0xffff;
    iVar2 = iVar2 - (uint)bVar9;
  }
  uVar3 = iVar2 + (uint)CARRY4(uVar8, uVar7);
  gploc_5C = uVar8 + uVar7;
  iVar5 = mapxhstep;
  if ((int)uVar3 < 0) {
    iVar5 = mapxhstep - 1;
  }
  gploc_2C = (uVar3 & 0xff) | iVar5 << 8;
  uVar8 = mapyveltop * 0x10000;
  iVar5 = (int)mapyveltop >> 0x10;
  gploc_68 = (int)shadeveltop << 0x18;
  uVar3 = (int)shadeveltop >> 8;
  if ((int)uVar3 < 0) {
    uVar3 = uVar3 & 0xffff;
    bVar9 = uVar8 < 0x10000;
    uVar8 = uVar8 - 0x10000;
    iVar5 = iVar5 - (uint)bVar9;
  }
  uVar7 = iVar5 + (uint)CARRY4(uVar8, uVar3);
  gploc_A4 = uVar8 + uVar3;
  uVar8 = mapxveltop;
  if ((int)uVar7 < 0) {
    uVar8 = mapxveltop - 1;
  }
  gploc_A0 = (uVar7 & 0xff) | uVar8 << 8;
  gploc_8C = (uint)(point1shade << 0x10) >> 8;
  gploc_88 = (point1mapy & 0xff) | point1mapx << 0x18;
  if (-1 < crease_len) {
    uVar8 = mapyvelbottom * 0x10000;
    iVar5 = (int)mapyvelbottom >> 0x10;
    gploc_64 = shadevelbottom << 0x18;
    uVar3 = (int)shadevelbottom >> 8;
    if ((int)uVar3 < 0) {
      uVar3 = uVar3 & 0xffff;
      bVar9 = uVar8 < 0x10000;
      uVar8 = uVar8 - 0x10000;
      iVar5 = iVar5 - (uint)bVar9;
    }
    gploc_98 = uVar8 + uVar3;
    uVar3 = iVar5 + (uint)CARRY4(uVar8, uVar3);
    uVar8 = mapxvelbottom;
    if ((int)uVar3 < 0) {
      uVar8 = mapxvelbottom - 1;
    }
    gploc_94 = (uVar3 & 0xff) | uVar8 << 8;
    gploc_80 = (uint)(point2shade << 0x10) >> 8;
    gploc_7C = (point2mapy & 0xff) | point2mapx << 0x18;
  }
}

// Calculates the vectors needed to step across the triangle
// returns vel variables
static void calc_hstep() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _gploc_pt_bx,%%esi\n \
    movl    _crease_len,%%edi\n \
    orl     %%edi,%%edi\n \
    subl    $0x0,%%esi\n \
    addl    $0x0,%%esi\n \
    movl    _gploc_pt_ax,%%eax    # find denom cross product\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_cx,%%edi\n \
    subl    %%eax,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_by,%%ebx\n \
    subl    %%eax,%%ebx\n \
    movl    _gploc_pt_cy,%%ecx\n \
    subl    %%eax,%%ecx\n \
    movl    %%ecx,%%eax\n \
    imull   %%esi,%%ecx\n \
    movl    _crease_len,%%ebp\n \
    orl     %%ebp,%%ebp\n \
    js      bendonright\n \
    subl    %%eax,%%ecx\n \
    subl    %%eax,%%ecx\n \
\n \
bendonright:         # 1AF6\n \
    addl    %%eax,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    jz      zerocalc\n \
    xorl    %%edx,%%edx\n \
    movl    $0x7FFFFFFF,%%eax       # 32768 * 65536 - 1\n \
    idivl   %%ebx\n \
    movl    %%eax,%%ebp\n \
    movl    _gploc_pt_ay,%%eax\n \
    movl    _gploc_pt_cy,%%esi\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_pt_by,%%edi\n \
    subl    %%eax,%%edi\n \
\n \
    # IF ShadeOn\n \
    movl    %%ebp,%%eax\n \
    movl    _point1shade,%%edx\n \
    movl    _point3shade,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2shade,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns posshade\n \
    incl    %%eax\n \
\n \
posshade:         # 1B4A\n \
    movl    %%eax,_shadehstep\n \
    # ENDIF ShadeOn\n \
\n \
    # IF TextOn\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapx,%%edx  # calc x mapx step\n \
    movl    _point3mapx,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapx,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns posmapx\n \
    incl    %%eax\n \
\n \
posmapx:         # 1B7A\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%ebp,%%eax\n \
    movl    _point1mapy,%%edx    # calc x mapy step\n \
    movl    _point3mapy,%%ebx\n \
    subl    %%edx,%%ebx\n \
    movl    _point2mapy,%%ecx\n \
    subl    %%edx,%%ecx\n \
    imull   %%esi,%%ecx\n \
    imull   %%edi,%%ebx\n \
    subl    %%ecx,%%ebx\n \
    imull   %%ebx\n \
    shll    $1,%%eax\n \
    rcll    $1,%%edx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns posmapy\n \
    incl    %%eax\n \
\n \
posmapy:         # 1BAA\n \
    movl    %%eax,_mapyhstep\n \
    # ENDIF TextOn\n \
    jmp calcend\n \
# ---------------------------------------------------------------------------\n \
\n \
zerocalc:         # 1B03\n \
    xorl    %%eax,%%eax\n \
    movl    %%eax,_shadehstep\n \
    movl    %%eax,_mapxhstep\n \
    movl    %%eax,_mapyhstep\n \
\n \
calcend:\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

static void calc_data_edge_step() {}

static void calc_data_for_edge() {
  long long lVar1;
  int iVar2;
  ushort uVar3;
  int iVar4;

  iVar4 = point2y - point1y;
  if (iVar4 < 0x100) {
    iVar4 = *(int *)(&gpoly_reptable + iVar4 * 4);
  } else {
    iVar4 = (int)(0x7fffffff / (long long)iVar4);
  }
  iVar2 = (point2shade - point1shade) * 2;
  lVar1 = (long long)iVar2 * (long long)iVar4;
  uVar3 = (ushort)((unsigned long long)lVar1 >> 0x10);
  shadeveltop
      = combineHighLowBits(uVar3, (short)((unsigned long long)lVar1 >> 0x20)) << 0x10 | (uint)uVar3;
  if (iVar2 < 0) {
    shadeveltop++;
  }
  iVar2 = (point2mapx - point1mapx) * 2;
  lVar1 = (long long)iVar2 * (long long)iVar4;
  uVar3 = (ushort)((unsigned long long)lVar1 >> 0x10);
  mapxveltop
      = combineHighLowBits(uVar3, (short)((unsigned long long)lVar1 >> 0x20)) << 0x10 | (uint)uVar3;
  if (iVar2 < 0) {
    mapxveltop++;
  }
  iVar2 = (point2mapy - point1mapy) * 2;
  lVar1 = (long long)iVar2 * (long long)iVar4;
  uVar3 = (ushort)((unsigned long long)lVar1 >> 0x10);
  mapyveltop
      = combineHighLowBits(uVar3, (short)((unsigned long long)lVar1 >> 0x20)) << 0x10 | (uint)uVar3;
  if (iVar2 < 0) {
    mapyveltop++;
  }
}

void calc_data_for_edge_clean() {
  int deltaY = point2y - point1y;
  int reciprocal;
  long long product;

  // Calculate reciprocal based on deltaY
  if (deltaY < 256) {
    reciprocal = gpoly_reptable[deltaY];
  } else {
    reciprocal = 0x7FFFFFFF / deltaY;
  }

  // Calculate shade velocity
  int deltaShade = (point2shade - point1shade) * 2;
  product = (long long)deltaShade * reciprocal;
  shadeveltop = product >> 16;
  if (deltaShade < 0) {
    shadeveltop++;
  }

  // Calculate map X velocity
  int deltaMapX = (point2mapx - point1mapx) * 2;
  product = (long long)deltaMapX * reciprocal;
  mapxveltop = product >> 16;
  if (deltaMapX < 0) {
    mapxveltop++;
  }

  // Calculate map Y velocity
  int deltaMapY = (point2mapy - point1mapy) * 2;
  product = (long long)deltaMapY * reciprocal;
  mapyveltop = product >> 16;
  if (deltaMapY < 0) {
    mapyveltop++;
  }
}

static void draw_gpoly_sub7_subfunc2() {
#if __GNUC__
  asm volatile(
      " \
# CALC_DATA_EDGE_STEP -------------------------------------------------------\n \
    pusha   \n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  bendonright1\n \
\n \
# --------------------------------------------------------------------------\n \
# CALC_DATA_FOR_EDGE -------------------------------------------------------\n \
    movl    _point2y,%%ecx\n \
    subl    _point1y,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  largey\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp smally\n \
\n \
largey:\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
smally:\n \
    movl    _point2shade,%%eax\n \
    subl    _point1shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns posshade1\n \
    incl    %%eax\n \
\n \
posshade1:\n \
    movl    %%eax,_shadeveltop\n \
    movl    _point2mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns posmapx1\n \
    incl    %%eax\n \
\n \
posmapx1:\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point2mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns posmapy1\n \
    incl    %%eax\n \
\n \
posmapy1:\n \
    movl    %%eax,_mapyveltop\n \
# END CALC_DATA_FOR_EDGE ----------------------------------------------------\n \
# ---------------------------------------------------------------------------\n \
\n \
# --------------------------------------------------------------------------\n \
# CALC_DATA_FOR_EDGE -------------------------------------------------------\n \
    movl    _point3y,%%ecx\n \
    subl    _point2y,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  largey1\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp smally1\n \
\n \
largey1:\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
smally1:\n \
    movl    _point3shade,%%eax\n \
    subl    _point2shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns posshade2\n \
    incl    %%eax\n \
\n \
posshade2:\n \
    movl    %%eax,_shadevelbottom\n \
    movl    _point3mapx,%%eax\n \
    subl    _point2mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns posmapx2\n \
    incl    %%eax\n \
\n \
posmapx2:\n \
    movl    %%eax,_mapxvelbottom\n \
    movl    _point3mapy,%%eax\n \
    subl    _point2mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns posmapy2\n \
    incl    %%eax\n \
\n \
posmapy2:\n \
    movl    %%eax,_mapyvelbottom\n \
# END CALC_DATA_FOR_EDGE ----------------------------------------------------\n \
# ---------------------------------------------------------------------------\n \
\n \
    jmp calcend1\n \
# ---------------------------------------------------------------------------\n \
\n \
bendonright1:\n \
# --------------------------------------------------------------------------\n \
# CALC_DATA_FOR_EDGE -------------------------------------------------------\n \
    movl    _point3y,%%ecx\n \
    subl    _point1y,%%ecx\n \
    cmpl    $0x0FF,%%ecx\n \
    jg  gpo_loc_1EC8\n \
    movl    _gpoly_reptable(,%%ecx,4),%%ebx\n \
    jmp gpo_loc_1ED6\n \
\n \
gpo_loc_1EC8:         # 1CCD\n \
    movl    $0,%%edx\n \
    movl    $0x7FFFFFFF,%%eax\n \
    idivl   %%ecx\n \
    movl    %%eax,%%ebx\n \
\n \
gpo_loc_1ED6:         # 1CD6\n \
    movl    _point3shade,%%eax\n \
    subl    _point1shade,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1EEB\n \
    incl    %%eax\n \
\n \
gpo_loc_1EEB:         # 1CF8\n \
    movl    %%eax,_shadeveltop\n \
    movl    _point3mapx,%%eax\n \
    subl    _point1mapx,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1F04\n \
    incl    %%eax\n \
\n \
gpo_loc_1F04:         # 1D1\n \
    movl    %%eax,_mapxveltop\n \
    movl    _point3mapy,%%eax\n \
    subl    _point1mapy,%%eax\n \
    shll    $1,%%eax\n \
    imull   %%ebx\n \
    movw    %%dx,%%ax\n \
    roll    $0x10,%%eax\n \
    jns gpo_loc_1F1D\n \
    incl    %%eax\n \
\n \
gpo_loc_1F1D:         # 1D2A\n \
    movl    %%eax,_mapyveltop\n \
# END CALC_DATA_FOR_EDGE ----------------------------------------------------\n \
# ---------------------------------------------------------------------------\n \
\n \
calcend1:\n \
# END CALC_DATA_EDGE_STEP ---------------------------------------------------\n \
# ---------------------------------------------------------------------------\n \
\n \
# --------------------------------------------------------------------------\n \
# CALC_STARTPOS ------------------------------------------------------------\n \
# --- CALC_STARTPOS_SEC ----------------------------------------------------\n \
    movl    _point1shade,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposshadetop\n \
    movl    _point1mapx,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapxtop\n \
    movl    _point1mapy,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapytop\n \
    movl    _point2shade,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposshadebottom\n \
    movl    _point2mapx,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapxbottom\n \
    movl    _point2mapy,%%eax\n \
    shll    $0x10,%%eax\n \
    movl    %%eax,_startposmapybottom\n \
    movl    _mapyhstep,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movl    _shadehstep,%%ebx\n \
    shll    $0x18,%%ebx\n \
    movl    %%ebx,_gploc_30\n \
    movl    _shadehstep,%%ebx\n \
    sarl    $8,%%ebx\n \
    orl %%ebx,%%ebx\n \
    jns gpo_loc_1FB1\n \
    andl    $0x0FFFF,%%ebx\n \
    subl    $0x10000,%%eax\n \
    sbbl    $0,%%edx\n \
\n \
gpo_loc_1FB1:         # 1DB\n \
    addl    %%ebx,%%eax\n \
    adcl    $0,%%edx\n \
    movl    %%eax,_gploc_BC\n \
    movl    _mapxhstep,%%eax\n \
    orl %%edx,%%edx\n \
    jns gpo_loc_1FC9\n \
    decl    %%eax\n \
\n \
gpo_loc_1FC9:         # 1DD6\n \
    shll    $8,%%eax\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_B8\n \
    movl    _mapyhstep,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movl    _shadehstep,%%ebx\n \
    sarl    $8,%%ebx\n \
    orl %%ebx,%%ebx\n \
    jns gpo_loc_2006\n \
    andl    $0x0FFFF,%%ebx\n \
    subl    $0x0FFFF,%%eax\n \
    sbbl    $0,%%edx\n \
\n \
gpo_loc_2006:         # 1E06\n \
    addl    %%ebx,%%eax\n \
    adcl    $0,%%edx\n \
    movl    %%eax,_gploc_5C\n \
    movl    _mapxhstep,%%eax\n \
    orl %%edx,%%edx\n \
    jns gpo_loc_201E\n \
    decl    %%eax\n \
\n \
gpo_loc_201E:         # 1E2B\n \
    shll    $8,%%eax\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_2C\n \
    movl    _mapyveltop,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movl    _shadeveltop,%%ebx\n \
    shll    $0x18,%%ebx\n \
    movl    %%ebx,_gploc_68\n \
    movl    _shadeveltop,%%ebx\n \
    sarl    $8,%%ebx\n \
    orl %%ebx,%%ebx\n \
    jns gpo_loc_2063\n \
    andl    $0x0FFFF,%%ebx\n \
    subl    $0x10000,%%eax\n \
    sbbl    $0,%%edx\n \
\n \
gpo_loc_2063:         # 1E63\n \
    addl    %%ebx,%%eax\n \
    adcl    $0,%%edx\n \
    movl    %%eax,_gploc_A4\n \
    movl    _mapxveltop,%%eax\n \
    orl %%edx,%%edx\n \
    jns gpo_loc_2078\n \
    decl    %%eax\n \
\n \
gpo_loc_2078:         # 1E85\n \
    shll    $8,%%eax\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_A0\n \
    movl    _startposmapytop,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    movl    _startposshadetop,%%ebx\n \
    shrl    $8,%%ebx\n \
    orl %%ebx,%%eax\n \
    movl    %%eax,_gploc_8C\n \
    movl    _startposmapxtop,%%eax\n \
    shll    $8,%%eax\n \
    shrl    $0x10,%%edx\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_88\n \
    movl    _crease_len,%%esi\n \
    orl %%esi,%%esi\n \
    js  gpo_case69_break\n \
    movl    _mapyvelbottom,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    sarl    $0x10,%%edx\n \
    movl    _shadevelbottom,%%ebx\n \
    shll    $0x18,%%ebx\n \
    movl    %%ebx,_gploc_64\n \
    movl    _shadevelbottom,%%ebx\n \
    sarl    $8,%%ebx\n \
    orl %%ebx,%%ebx\n \
    jns gpo_loc_2104\n \
    andl    $0x0FFFF,%%ebx\n \
    subl    $0x10000,%%eax\n \
    sbbl    $0,%%edx\n \
\n \
gpo_loc_2104:         # 1F0\n \
    addl    %%ebx,%%eax\n \
    adcl    $0,%%edx\n \
    movl    %%eax,_gploc_98\n \
    movl    _mapxvelbottom,%%eax\n \
    orl %%edx,%%edx\n \
    jns gpo_loc_2119\n \
    decl    %%eax\n \
\n \
gpo_loc_2119:         # 1F26\n \
    shll    $8,%%eax\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_94\n \
    movl    _startposmapybottom,%%eax\n \
    movl    %%eax,%%edx\n \
    shll    $0x10,%%eax\n \
    movl    _startposshadebottom,%%ebx\n \
    shrl    $8,%%ebx\n \
    orl %%ebx,%%eax\n \
    movl    %%eax,_gploc_80\n \
    movl    _startposmapxbottom,%%eax\n \
    shll    $8,%%eax\n \
    shrl    $0x10,%%edx\n \
    andl    $0x0FF,%%edx\n \
    orl %%eax,%%edx\n \
    movl    %%edx,_gploc_7C\n \
\n \
gpo_case69_break:\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

/*
CALC_STARTPOS_SEC MACRO ShadeOn, TextOn, pt, sec, PixFix
        mov eax, point&pt&shade
        shl eax, 16
        mov startposshade&sec, eax

        mov eax, point&pt&mapx
        shl eax, 16
        mov startposmapx&sec, eax
        mov eax, point&pt&mapy
        shl eax, 16
        mov startposmapy&sec, eax
ENDM
*/

// #if 0
// Legacy implementation
void draw_gpoly_sub7() {
  // JUSTLOG(
  //     "[test-inputs] gploc_pt_ax=%d, gploc_pt_ay=%d, gploc_pt_bx=%d, gploc_pt_by=%d, "
  //     "gploc_pt_cx=%d, "
  //     "gploc_pt_cy=%d, point3shade=%d, point1shade=%d, point2shade=%d, point3mapx=%d,
  //     point1mapx=%d, " "point3mapy=%d, point1mapy=%d, point2mapx=%d, crease_len=%d", gploc_pt_ax,
  //     gploc_pt_ay, gploc_pt_bx, gploc_pt_by, gploc_pt_cx, gploc_pt_cy, point3shade, point1shade,
  //     point2shade, point3mapx, point1mapx, point3mapy, point1mapy, point2mapx, crease_len);

  // calculateTriangleProperties();
  calc_hstep();

  // JUSTLOG("[test-outputs] shadehstep=%d, mapxhstep=%d, mapyhstep=%d", shadehstep, mapxhstep,
  // mapyhstep);

  /*
  ### Global Input Variables
  These are variables that are read within the function to compute other values or influence
  decision paths:

  1. `crease_len`
  2. `gploc_pt_cy`
  3. `gploc_pt_ay`
  4. `point3shade`
  5. `point1shade`
  6. `point3mapx`
  7. `point1mapx`
  8. `point3mapy`
  9. `point1mapy`
  10. `gploc_pt_by`
  11. `point2shade`
  12. `point2mapx`
  13. `point2mapy`
  14. `mapyhstep`
  15. `shadehstep`
  16. `mapxhstep`
  17. `mapyveltop`
  19. `mapxveltop`
  20. `shadevelbottom`
  21. `mapxvelbottom`
  22. `mapyvelbottom`

  ### Global Output Variables
  These are variables that are written or potentially modified by the function:

  1. `shadeveltop`
  2. `mapxveltop`
  3. `mapyveltop`
  4. `startposshadetop`
  5. `startposmapxtop`
  6. `startposmapytop`
  7. `startposshadebottom`
  8. `startposmapxbottom`
  9. `startposmapybottom`
  10. `gploc_BC`
  11. `gploc_B8`
  12. `gploc_5C`
  13. `gploc_2C`
  14. `gploc_A4`
  15. `gploc_A0`
  16. `gploc_8C`
  17. `gploc_88`
  18. `gploc_98`
  19. `gploc_94`
  20. `gploc_80`
  21. `gploc_7C`
  22. `gploc_64`
  23. `shadevelbottom` (conditional overwrite based on branch conditions)
  24. `mapxvelbottom` (conditional overwrite)
  25. `mapyvelbottom` (conditional overwrite)
  */

  // JUSTLOG(
  //     "[test-inputs] crease_len=%d, gploc_pt_cy=%d, gploc_pt_ay=%d, point3shade=%d, "
  //     "point1shade=%d, "
  //     "point3mapx=%d, "
  //     "point1mapx=%d, point3mapy=%d, point1mapy=%d, gploc_pt_by=%d, point2shade=%d,
  //     point2mapx=%d, " "point2mapy=%d, mapyhstep=%d, shadehstep=%d, mapxhstep=%d, mapyveltop=%d,
  //     mapxveltop=%d, " "shadevelbottom=%d, mapxvelbottom=%d, mapyvelbottom=%d", crease_len,
  //     gploc_pt_cy, gploc_pt_ay, point3shade, point1shade, point3mapx, point1mapx, point3mapy,
  //     point1mapy, gploc_pt_by, point2shade, point2mapx, point2mapy, mapyhstep, shadehstep,
  //     mapxhstep, mapyveltop, mapxveltop, shadevelbottom, mapxvelbottom, mapyvelbottom);

  draw_gpoly_sub7_subfunc2();
}
// #endif  // End legacy implementation

#if 0
void draw_gpoly_sub7_subfunc2()
{
  int v0; // ecx
  int v1; // ebx
  char v2; // sf
  int v3; // eax
  int v4; // eax
  int v5; // eax
  int v6; // eax
  int v7; // eax
  int v8; // eax
  int v9; // ecx
  int v10; // ebx
  int v11; // eax
  int v12; // eax
  int v13; // eax
  int v14; // eax
  int v15; // eax
  int v16; // eax
  int v17; // ecx
  int v18; // ebx
  int v19; // eax
  int v20; // eax
  int v21; // eax
  int v22; // eax
  int v23; // eax
  int v24; // eax
  __int64 v25; // rax
  int v26; // ebx
  __int64 v27; // rax
  __int64 v28; // rax
  int v29; // ebx
  __int64 v30; // rax
  __int64 v31; // rax
  int v32; // ebx
  __int64 v33; // rax
  __int64 v34; // rax
  int v35; // ebx
  __int64 v36; // rax

  if ( crease_len < 0 )
  {
    v17 = gploc_pt_cy - gploc_pt_ay;
    if ( gploc_pt_cy - gploc_pt_ay > 255 )
      v18 = 0x7FFFFFFF / v17;
    else
      v18 = gpoly_reptable[v17];
    HIWORD(v19) = (unsigned int)(v18 * 2 * (point3shade - point1shade)) >> 16;
    LOWORD(v19) = (unsigned __int64)(v18 * (__int64)(2 * (point3shade - point1shade))) >> 32;
    v20 = __ROL4__(v19, 16);
    if ( v2 )
      ++v20;
    shadeveltop = v20;
    HIWORD(v21) = (unsigned int)(v18 * 2 * (point3mapx - point1mapx)) >> 16;
    LOWORD(v21) = (unsigned __int64)(v18 * (__int64)(2 * (point3mapx - point1mapx))) >> 32;
    v22 = __ROL4__(v21, 16);
    if ( v2 )
      ++v22;
    mapxveltop = v22;
    HIWORD(v23) = (unsigned int)(v18 * 2 * (point3mapy - point1mapy)) >> 16;
    LOWORD(v23) = (unsigned __int64)(v18 * (__int64)(2 * (point3mapy - point1mapy))) >> 32;
    v24 = __ROL4__(v23, 16);
    if ( v2 )
      ++v24;
    mapyveltop = v24;
  }
  else
  {
    v0 = gploc_pt_by - gploc_pt_ay;
    if ( gploc_pt_by - gploc_pt_ay > 255 )
      v1 = 0x7FFFFFFF / v0;
    else
      v1 = gpoly_reptable[v0];
    HIWORD(v3) = (unsigned int)(v1 * 2 * (point2shade - point1shade)) >> 16;
    LOWORD(v3) = (unsigned __int64)(v1 * (__int64)(2 * (point2shade - point1shade))) >> 32;
    v4 = __ROL4__(v3, 16);
    if ( v2 )
      ++v4;
    shadeveltop = v4;
    HIWORD(v5) = (unsigned int)(v1 * 2 * (point2mapx - point1mapx)) >> 16;
    LOWORD(v5) = (unsigned __int64)(v1 * (__int64)(2 * (point2mapx - point1mapx))) >> 32;
    v6 = __ROL4__(v5, 16);
    if ( v2 )
      ++v6;
    mapxveltop = v6;
    HIWORD(v7) = (unsigned int)(v1 * 2 * (point2mapy - point1mapy)) >> 16;
    LOWORD(v7) = (unsigned __int64)(v1 * (__int64)(2 * (point2mapy - point1mapy))) >> 32;
    v8 = __ROL4__(v7, 16);
    if ( v2 )
      ++v8;
    mapyveltop = v8;
    v9 = gploc_pt_cy - gploc_pt_by;
    if ( gploc_pt_cy - gploc_pt_by > 255 )
      v10 = 0x7FFFFFFF / v9;
    else
      v10 = gpoly_reptable[v9];
    HIWORD(v11) = (unsigned int)(v10 * 2 * (point3shade - point2shade)) >> 16;
    LOWORD(v11) = (unsigned __int64)(v10 * (__int64)(2 * (point3shade - point2shade))) >> 32;
    v12 = __ROL4__(v11, 16);
    if ( v2 )
      ++v12;
    shadevelbottom = v12;
    HIWORD(v13) = (unsigned int)(v10 * 2 * (point3mapx - point2mapx)) >> 16;
    LOWORD(v13) = (unsigned __int64)(v10 * (__int64)(2 * (point3mapx - point2mapx))) >> 32;
    v14 = __ROL4__(v13, 16);
    if ( v2 )
      ++v14;
    mapxvelbottom = v14;
    HIWORD(v15) = (unsigned int)(v10 * 2 * (point3mapy - point2mapy)) >> 16;
    LOWORD(v15) = (unsigned __int64)(v10 * (__int64)(2 * (point3mapy - point2mapy))) >> 32;
    v16 = __ROL4__(v15, 16);
    if ( v2 )
      ++v16;
    mapyvelbottom = v16;
  }
  startposshadetop = point1shade << 16;
  startposmapxtop = point1mapx << 16;
  startposmapytop = point1mapy << 16;
  startposshadebottom = point2shade << 16;
  startposmapxbottom = point2mapx << 16;
  startposmapybottom = point2mapy << 16;
  LODWORD(v25) = mapyhstep << 16;
  HIDWORD(v25) = mapyhstep >> 16;
  gploc_30 = shadehstep << 24;
  v26 = shadehstep >> 8;
  if ( shadehstep >> 8 < 0 )
  {
    v26 = (unsigned __int16)(shadehstep >> 8);
    v25 -= 0x10000LL;
  }
  v27 = (unsigned int)v26 + v25;
  gploc_BC = v27;
  LODWORD(v27) = mapxhstep;
  if ( v27 < 0 )
    LODWORD(v27) = mapxhstep - 1;
  gploc_B8 = ((_DWORD)v27 << 8) | BYTE4(v27);
  LODWORD(v28) = mapyhstep << 16;
  HIDWORD(v28) = mapyhstep >> 16;
  v29 = shadehstep >> 8;
  if ( shadehstep >> 8 < 0 )
  {
    v29 = (unsigned __int16)(shadehstep >> 8);
    v28 -= 0xFFFFLL;
  }
  v30 = (unsigned int)v29 + v28;
  gploc_5C = v30;
  LODWORD(v30) = mapxhstep;
  if ( v30 < 0 )
    LODWORD(v30) = mapxhstep - 1;
  gploc_2C = ((_DWORD)v30 << 8) | BYTE4(v30);
  LODWORD(v31) = mapyveltop << 16;
  HIDWORD(v31) = mapyveltop >> 16;
  gploc_68 = shadeveltop << 24;
  v32 = shadeveltop >> 8;
  if ( shadeveltop >> 8 < 0 )
  {
    v32 = (unsigned __int16)(shadeveltop >> 8);
    v31 -= 0x10000LL;
  }
  v33 = (unsigned int)v32 + v31;
  gploc_A4 = v33;
  LODWORD(v33) = mapxveltop;
  if ( v33 < 0 )
    LODWORD(v33) = mapxveltop - 1;
  gploc_A0 = ((_DWORD)v33 << 8) | BYTE4(v33);
  gploc_8C = ((unsigned int)startposshadetop >> 8) | (startposmapytop << 16);
  gploc_88 = (startposmapxtop << 8) | BYTE2(startposmapytop);
  if ( crease_len >= 0 )
  {
    LODWORD(v34) = mapyvelbottom << 16;
    HIDWORD(v34) = mapyvelbottom >> 16;
    gploc_64 = shadevelbottom << 24;
    v35 = shadevelbottom >> 8;
    if ( shadevelbottom >> 8 < 0 )
    {
      v35 = (unsigned __int16)(shadevelbottom >> 8);
      v34 -= 0x10000LL;
    }
    v36 = (unsigned int)v35 + v34;
    gploc_98 = v36;
    LODWORD(v36) = mapxvelbottom;
    if ( v36 < 0 )
      LODWORD(v36) = mapxvelbottom - 1;
    gploc_94 = ((_DWORD)v36 << 8) | BYTE4(v36);
    gploc_80 = ((unsigned int)startposshadebottom >> 8) | (startposmapybottom << 16);
    gploc_7C = (startposmapxbottom << 8) | BYTE2(startposmapybottom);
  }
}

void draw_gpoly_sub7_subfunc2() {
    // Assume all integers unless specified
    int v0, v1, deltaA, deltaB, deltaC;
    int resultA, resultB, resultC;
    int factor;
    bool negativeCondition;
    int prodResult;
    __int64 extendedResult;

    if (crease_len < 0) {
        deltaA = gploc_pt_cy - gploc_pt_ay;
        factor = (deltaA > 255) ? (0x7FFFFFFF / deltaA) : gpoly_reptable[deltaA];
        // Using function to mimic rotation operation and conditional increment
        shadeveltop = calculateResult(factor, point3shade - point1shade);
        mapxveltop = calculateResult(factor, point3mapx - point1mapx);
        mapyveltop = calculateResult(factor, point3mapy - point1mapy);
    } else {
        deltaB = gploc_pt_by - gploc_pt_ay;
        factor = (deltaB > 255) ? (0x7FFFFFFF / deltaB) : gpoly_reptable[deltaB];
        shadeveltop = calculateResult(factor, point2shade - point1shade);
        mapxveltop = calculateResult(factor, point2mapx - point1mapx);
        mapyveltop = calculateResult(factor, point2mapy - point1mapy);

        deltaC = gploc_pt_cy - gploc_pt_by;
        factor = (deltaC > 255) ? (0x7FFFFFFF / deltaC) : gpoly_reptable[deltaC];
        shadevelbottom = calculateResult(factor, point3shade - point2shade);
        mapxvelbottom = calculateResult(factor, point3mapx - point2mapx);
        mapyvelbottom = calculateResult(factor, point3mapy - point2mapy);
    }

    startposshadetop = point1shade << 16;
    startposmapxtop = point1mapx << 16;
    startposmapytop = point1mapy << 16;
    startposshadebottom = point2shade << 16;
    startposmapxbottom = point2mapx << 16;
    startposmapybottom = point2mapy << 16;

    // Processing results with presumed utility functions to handle the bit manipulations
    // These functions will require definition based on specific handling or masking requirements observed in original code
    gploc_BC = processResult(mapyhstep, shadehstep);
    gploc_B8 = processResult(mapxhstep);
    gploc_5C = processResult(mapyhstep, shadehstep);
    gploc_2C = processResult(mapxhstep);
    gploc_A4 = processResult(mapyveltop, shadeveltop);
    gploc_A0 = processResult(mapxveltop);
    gploc_8C = processCombinedResults(startposshadetop, startposmapytop);
    gploc_88 = processCombinedResults(startposmapxtop, startposmapytop);

    if (crease_len >= 0) {
        gploc_98 = processResult(mapyvelbottom, shadevelbottom);
        gploc_94 = processResult(mapxvelbottom);
        gploc_80 = processCombinedResults(startposshadebottom, startposmapybottom);
        gploc_7C = processCombinedResults(startposmapxbottom, startposmapybottom);
    }
}

int calculateResult(int factor, int delta) {
    int result = ((factor * (2 * delta) + (factor < 0)) >> 16);  // Assuming rotate-like behavior and sign correction
    return result;  // Return rollover-adjusted and conditionally incremented result
}

// Hypothetical function definitions will need the actual implementation specifics.
#endif

void draw_gpoly_sub11() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _gploc_8C,%%ecx\n \
    movl    _gploc_88,%%edx\n \
    movl    _gploc_84,%%ebx\n \
    movl    _gploc_pt_ay,%%esi\n \
    movl    _LOC_vec_screen_width,%%edi\n \
    imull   %%esi,%%edi\n \
    addl    _LOC_vec_screen,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jg  locret5a\n \
    movl    _gploc_pt_by,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_782808\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_782808:         # 261\n \
    subl    _gploc_pt_ay,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_pt_ax,%%esi\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_pt_shax,%%eax\n \
    movl    %%eax,%%ebp\n \
    jz  loc_782CD8\n \
    movl    _gploc_pt_ay,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_782C50\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_loop02\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782B70:         # 2910\n \
    addb    _shadehstep,%%bl\n \
    adcl    _gploc_BC,%%ecx\n \
    adcl    _gploc_B8,%%edx\n \
    adcb    _gploc_B4,%%bh\n \
    incl    %%esi\n \
    cmpl    %%esi,%%eax\n \
    jle loc_782B08\n \
    jmp loc_782B70\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782BA0:         # 2912\n \
    subb    _shadehstep,%%bl\n \
    sbbl    _gploc_BC,%%ecx\n \
    sbbl    _gploc_B8,%%edx\n \
    sbbb    _gploc_B4,%%bh\n \
    decl    %%esi\n \
    cmpl    %%esi,%%eax\n \
    jge loc_782B08\n \
    jmp loc_782BA0\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782BD0:         # 2908\n \
    orl %%esi,%%esi\n \
    jz  loc_782B08\n \
    js  loc_782BE0\n \
    jmp loc_782C10\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782BE0:         # 29E8\n \
    addb    _shadehstep,%%bl\n \
    adcl    _gploc_BC,%%ecx\n \
    adcl    _gploc_B8,%%edx\n \
    adcb    _gploc_B4,%%bh\n \
    incl    %%esi\n \
    jz  loc_782B08\n \
    jmp loc_782BE0\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782C10:         # 29EA\n \
    subb    _shadehstep,%%bl\n \
    sbbl    _gploc_BC,%%ecx\n \
    sbbl    _gploc_B8,%%edx\n \
    sbbb    _gploc_B4,%%bh\n \
    decl    %%esi\n \
    jz  loc_782B08\n \
    jmp loc_782C10\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782C40:         # 294\n \
    movl    _LOC_vec_window_width,%%ebp\n \
    jmp loc_782B3A\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782C50:         # 2640\n \
    addb    _gploc_60,%%bl\n \
    adcl    _gploc_CC,%%ecx\n \
    adcl    _gploc_C4,%%edx\n \
    adcb    _gploc_C8,%%bh\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    subl    %%eax,_gploc_74\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_12C,%%eax\n \
    addl    _gploc_128,%%ebp\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    addl    %%eax,_gploc_74\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_104,%%edi\n \
    decl _gploc_C0\n \
    jz  loc_782CD0\n \
    incl    %%esi\n \
    js  loc_782C50\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_loop02\n \
# ---------------------------------------------------------------------------\n \
\
\n \
loc_782CD0:         # 2AC8\n \
    movl    _gploc_74,%%esi\n \
    nop \n \
\n \
loc_782CD8:         # 263\n \
    decl _gploc_180\n \
    jz  locret5a\n \
    movl    %%eax,_gploc_FC\n \
    movl    _crease_len,%%eax\n \
    orl %%eax,%%eax\n \
    js  loc_782D90\n \
    movl    _factor_cb,%%eax\n \
    movl    %%eax,_gploc_12C\n \
    movl    _shadevelbottom,%%eax\n \
    movl    %%eax,_gploc_60\n \
    movl    _gploc_98,%%eax\n \
    movl    %%eax,_gploc_CC\n \
    movl    _gploc_94,%%eax\n \
    movl    %%eax,_gploc_C4\n \
    movl    _gploc_90,%%eax\n \
    movl    %%eax,_gploc_C8\n \
    movl    _gploc_80,%%ecx\n \
    movl    _gploc_7C,%%edx\n \
    movl    _gploc_78,%%ebx\n \
    movl    _gploc_pt_cy,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_782D5B\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_782D5B:         # 2B6\n \
    subl    _gploc_pt_by,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_pt_bx,%%eax\n \
    movl    %%eax,_gploc_74\n \
    movl    _gploc_pt_shbx,%%eax\n \
    jle locret5a\n \
    movl    _gploc_pt_by,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_782C50\n \
    movl    _gploc_pt_bx,%%esi\n \
    jmp loc_loop02\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782D90:         # 2AF\n \
    movl    _factor_cb,%%ebp\n \
    movl    %%ebp,_gploc_128\n \
    movl    _gploc_pt_shbx,%%ebp\n \
    movl    _gploc_pt_cy,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_782DB0\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_782DB0:         # 2BB\n \
    subl    _gploc_pt_by,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_FC,%%eax\n \
    jle locret5a\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_pt_by,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_782C50\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_loop02\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_loop01:         # 294E\n \
    movl    _gploc_FC,%%eax\n \
    movl    _gploc_F8,%%ebp\n \
    movl    _gploc_F4,%%edi\n \
    movl    _gploc_74,%%esi\n \
    sarl    $0x10,%%eax\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_12C,%%eax\n \
    addl    _gploc_128,%%ebp\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    addl    %%eax,%%esi\n \
    movl    _gploc_FC,%%eax\n \
    movl    _gploc_E0,%%ecx\n \
    movl    _gploc_E4,%%ebx\n \
    movl    _gploc_D8,%%edx\n \
    addb    _gploc_60,%%bl\n \
    adcl    _gploc_CC,%%ecx\n \
    adcl    _gploc_C4,%%edx\n \
    adcb    _gploc_C8,%%bh\n \
    addl    _gploc_104,%%edi\n \
    decl _gploc_C0\n \
    jz  loc_782CD8\n \
\n \
loc_loop02:         # 264D\n \
    movl    %%eax,_gploc_FC\n \
    movl    %%ebp,_gploc_F8\n \
    movl    %%edi,_gploc_F4\n \
    sarl    $0x10,%%eax\n \
    js  loc_782BD0\n \
    cmpl    %%esi,%%eax\n \
    jg  loc_782B70\n \
    jl  loc_782BA0\n \
\n \
loc_782B08:         # 299F\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_F4,%%edi\n \
    movl    %%ecx,_gploc_E0\n \
    movl    %%ebx,_gploc_E4\n \
    movl    %%edx,_gploc_D8\n \
    sarl    $0x10,%%ebp\n \
    cmpl    _LOC_vec_window_width,%%ebp\n \
    jg  loc_782C40\n \
\n \
loc_782B3A:         # 2A56\n \
    addl    %%esi,%%edi\n \
    subl    %%esi,%%ebp\n \
    jle loc_loop01\n \
    movl    %%ebp,%%eax\n \
    andl    $0x0F,%%eax\n \
    addl    _gpoly_countdown(,%%eax,4),%%edi\n \
    movl    %%ebp,_gploc_D4\n \
    movl    _gploc_5C,%%ebp\n \
    movl    _LOC_vec_map,%%esi\n \
    jmp   *switch_vecmap(,%%eax,4)\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_vecmap00:         # 285C\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,(%%edi)\n \
\n \
loc_vecmap15:         # 3E6C\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_vecmap14:         # 3E68\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_vecmap13:         # 3E64\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_vecmap12:         # 3E60\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_vecmap11:         # 3E5C\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_vecmap10:         # 3E58\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_vecmap09:         # 3E54\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_vecmap08:         # 3E50\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_vecmap07:         # 3E4C\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_vecmap06:         # 3E48\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_vecmap05:         # 3E44\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_vecmap04:         # 3E40\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_vecmap03:         # 3E3C\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_vecmap02:         # 3E38\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_vecmap01:         # 3E34\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0F(%%edi)\n \
    addl    $0x10,%%edi\n \
    subl $0x10,_gploc_D4\n \
    jg  loc_vecmap00\n \
    jmp  loc_loop01\n \
# ---------------------------------------------------------------------------\n \
\n \
switch_vecmap:\n \
    .int    loc_vecmap00\n \
    .int    loc_vecmap01\n \
    .int    loc_vecmap02\n \
    .int    loc_vecmap03\n \
    .int    loc_vecmap04\n \
    .int    loc_vecmap05\n \
    .int    loc_vecmap06\n \
    .int    loc_vecmap07\n \
    .int    loc_vecmap08\n \
    .int    loc_vecmap09\n \
    .int    loc_vecmap10\n \
    .int    loc_vecmap11\n \
    .int    loc_vecmap12\n \
    .int    loc_vecmap13\n \
    .int    loc_vecmap14\n \
    .int    loc_vecmap15\n \
\n \
locret5a:\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

void draw_gpoly_sub12() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    movl    _LOC_vec_screen_width,%%ecx\n \
    movl    %%ecx,_gploc_104\n \
    movl    $2,%%ecx\n \
    movl    %%ecx,_gploc_180\n \
    movl    _gploc_1A4,%%eax\n \
    movl    %%eax,_gploc_60\n \
    movl    _gploc_A4,%%eax\n \
    movl    %%eax,_gploc_CC\n \
    movl    _gploc_A0,%%eax\n \
    movl    %%eax,_gploc_C4\n \
    movl    _gploc_9C,%%eax\n \
    movl    %%eax,_gploc_C8\n \
    movl    _crease_len,%%eax\n \
    orl %%eax,%%eax\n \
    js  loc_782209\n \
    movl    _factor_ba,%%eax\n \
    movl    %%eax,_gploc_12C\n \
    movl    _factor_ca,%%eax\n \
    movl    %%eax,_gploc_128\n \
    jmp loc_78221E\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782209:         # 2000\n \
    movl    _factor_ca,%%eax\n \
    movl    %%eax,_gploc_12C\n \
    movl    _factor_ba,%%eax\n \
    movl    %%eax,_gploc_128\n \
\n \
loc_78221E:         # 2017\n \
    movl    _gploc_8C,%%ecx\n \
    movl    _gploc_88,%%edx\n \
    movl    _gploc_84,%%ebx\n \
    movl    _gploc_pt_ay,%%esi\n \
    movl    _LOC_vec_screen_width,%%edi\n \
    imull   %%esi,%%edi\n \
    addl    _LOC_vec_screen,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jg  locret5b\n \
    movl    _gploc_pt_by,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_782267\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_782267:         # 2070\n \
    subl    _gploc_pt_ay,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_pt_ax,%%esi\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_pt_shax,%%eax\n \
    movl    %%eax,%%ebp\n \
    jz  loc_782618\n \
    movl    _gploc_pt_ay,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_782590\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_782520\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_7822A1:         # 22BB\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,(%%edi)\n \
\n \
loc_7822C0:         # 3E2C\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,1(%%edi)\n \
\n \
loc_7822E0:         # 3E28\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,2(%%edi)\n \
\n \
loc_782300:         # 3E24\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,3(%%edi)\n \
\n \
loc_782320:         # 3E20\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,4(%%edi)\n \
\n \
loc_782340:         # 3E1C\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,5(%%edi)\n \
\n \
loc_782360:         # 3E18\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,6(%%edi)\n \
\n \
loc_782380:         # 3E14\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,7(%%edi)\n \
\n \
loc_7823A0:         # 3E10\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,8(%%edi)\n \
\n \
loc_7823C0:         # 3E0C\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,9(%%edi)\n \
\n \
loc_7823E0:         # 3E08\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0A(%%edi)\n \
\n \
loc_782400:         # 3E04\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0B(%%edi)\n \
\n \
loc_782420:         # 3E00\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0C(%%edi)\n \
\n \
loc_782440:         # 3DFC\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0D(%%edi)\n \
\n \
loc_782460:         # 3DF8\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0E(%%edi)\n \
\n \
loc_782480:         # 3DF4\n \
    movb    %%ch,%%ah\n \
    movb    %%dl,%%bl\n \
    addl    %%ebp,%%ecx\n \
    movb    (%%ebx,%%esi),%%al\n \
    adcl    _gploc_2C,%%edx\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%eax),%%al\n \
    popl    %%ebx\n \
    adcb    _gploc_28,%%bh\n \
    movb    %%al,0x0F(%%edi)\n \
    addl    $0x10,%%edi\n \
    subl $0x10,_gploc_D4\n \
    jg  loc_7822A1\n \
\n \
loc_7824B1:         # 236B\n \
    movl    _gploc_FC,%%eax\n \
    movl    _gploc_F8,%%ebp\n \
    movl    _gploc_F4,%%edi\n \
    movl    _gploc_74,%%esi\n \
    addl    _gploc_12C,%%eax\n \
    addl    _gploc_128,%%ebp\n \
    movl    _gploc_E0,%%ecx\n \
    movl    _gploc_E4,%%ebx\n \
    movl    _gploc_D8,%%edx\n \
    addb    _gploc_60,%%bl\n \
    adcl    _gploc_CC,%%ecx\n \
    adcl    _gploc_C4,%%edx\n \
    adcb    _gploc_C8,%%bh\n \
    addl    _gploc_104,%%edi\n \
    decl _gploc_C0\n \
    jz  loc_782618\n \
\n \
loc_782520:         # 20AC\n \
# draw_gpoly_+2414 ...\n \
    movl    %%eax,_gploc_FC\n \
    movl    %%ebp,_gploc_F8\n \
    movl    %%edi,_gploc_F4\n \
    sarl    $0x10,%%eax\n \
    movl    _gploc_F4,%%edi\n \
    movl    %%ecx,_gploc_E0\n \
    movl    %%ebx,_gploc_E4\n \
    movl    %%edx,_gploc_D8\n \
    sarl    $0x10,%%ebp\n \
    addl    %%eax,%%edi\n \
    subl    %%eax,%%ebp\n \
    jle loc_7824B1\n \
    movl    %%ebp,%%eax\n \
    andl    $0x0F,%%eax\n \
    addl    _gpoly_countdown(,%%eax,4),%%edi\n \
    movl    %%ebp,_gploc_D4\n \
    movl    _gploc_5C,%%ebp\n \
    movl    _LOC_vec_map,%%esi\n \
    jmp   *off_783FE0(,%%eax,4)\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782590:         # 209\n \
    addb    _gploc_60,%%bl\n \
    adcl    _gploc_CC,%%ecx\n \
    adcl    _gploc_C4,%%edx\n \
    adcb    _gploc_C8,%%bh\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    subl    %%eax,_gploc_74\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_12C,%%eax\n \
    addl    _gploc_128,%%ebp\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    addl    %%eax,_gploc_74\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_104,%%edi\n \
    decl _gploc_C0\n \
    jz  loc_782610\n \
    incl    %%esi\n \
    js  loc_782590\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_782520\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782610:         # 2408\n \
    movl    _gploc_74,%%esi\n \
    nop \n \
\n \
loc_782618:         # 2093\n \
    decl _gploc_180\n \
    jz  locret5b\n \
    movl    %%eax,_gploc_FC\n \
    movl    _crease_len,%%eax\n \
    orl %%eax,%%eax\n \
    js  loc_7826D0\n \
    movl    _factor_cb,%%eax\n \
    movl    %%eax,_gploc_12C\n \
    movl    _shadevelbottom,%%eax\n \
    movl    %%eax,_gploc_60\n \
    movl    _gploc_98,%%eax\n \
    movl    %%eax,_gploc_CC\n \
    movl    _gploc_94,%%eax\n \
    movl    %%eax,_gploc_C4\n \
    movl    _gploc_90,%%eax\n \
    movl    %%eax,_gploc_C8\n \
    movl    _gploc_80,%%ecx\n \
    movl    _gploc_7C,%%edx\n \
    movl    _gploc_78,%%ebx\n \
    movl    _gploc_pt_cy,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_78269B\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_78269B:         # 24A\n \
    subl    _gploc_pt_by,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_pt_bx,%%eax\n \
    movl    %%eax,_gploc_74\n \
    movl    _gploc_pt_shbx,%%eax\n \
    jle locret5b\n \
    movl    _gploc_pt_by,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_782590\n \
    movl    _gploc_pt_bx,%%esi\n \
    jmp loc_782520\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_7826D0:         # 243\n \
    movl    _factor_cb,%%ebp\n \
    movl    %%ebp,_gploc_128\n \
    movl    _gploc_pt_shbx,%%ebp\n \
    movl    _gploc_pt_cy,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_7826F0\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_7826F0:         # 24F\n \
    subl    _gploc_pt_by,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_FC,%%eax\n \
    jle locret5b\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_pt_by,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_782590\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_782520\n \
\n \
off_783FE0:\n \
    .int    loc_7822A1\n \
    .int    loc_782480\n \
    .int    loc_782460\n \
    .int    loc_782440\n \
    .int    loc_782420\n \
    .int    loc_782400\n \
    .int    loc_7823E0\n \
    .int    loc_7823C0\n \
    .int    loc_7823A0\n \
    .int    loc_782380\n \
    .int    loc_782360\n \
    .int    loc_782340\n \
    .int    loc_782320\n \
    .int    loc_782300\n \
    .int    loc_7822E0\n \
    .int    loc_7822C0\n \
\n \
locret5b:\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

void draw_gpoly_sub13() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    xorl    %%ecx,%%ecx\n \
    movl    _gploc_8C,%%edx\n \
    movl    _gploc_88,%%ebx\n \
    movl    _gploc_pt_ay,%%esi\n \
    movl    _LOC_vec_screen_width,%%edi\n \
    imull   %%esi,%%edi\n \
    addl    _LOC_vec_screen,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jg  locret69a\n \
    movl    _gploc_pt_by,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_783508\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_783508:         # 331\n \
    subl    _gploc_pt_ay,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_pt_ax,%%esi\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_pt_shax,%%eax\n \
    movl    %%eax,%%ebp\n \
    jz  loc_783A68\n \
    movl    _gploc_pt_ay,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_7839E0\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_783899\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_783542:         # 361C\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_78356D:         # 3EEC\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,1(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783599:         # 3EE8\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,2(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_7835C5:         # 3EE4\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,3(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_7835F1:         # 3EE0\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,4(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_78361D:         # 3EDC\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,5(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783649:         # 3ED8\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,6(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783675:         # 3ED4\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,7(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_7836A1:         # 3ED0\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,8(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_7836CD:         # 3ECC\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,9(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_7836F9:         # 3EC8\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0A(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783725:         # 3EC4\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0B(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783751:         # 3EC0\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0C(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_78377D:         # 3EBC\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0D(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_7837A9:         # 3EB8\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0E(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_7837D5:         # 3EB4\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0F(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
    addl    $0x10,%%edi\n \
    subl    $0x10,_gploc_D4\n \
    jg  loc_783542\n \
\n \
loc_783812:         # 370B\n \
    movl    _gploc_FC,%%eax\n \
    movl    _gploc_F8,%%ebp\n \
    movl    _gploc_F4,%%edi\n \
    movl    _gploc_74,%%esi\n \
    sarl    $0x10,%%eax\n \
    subl    %%eax,%%esi\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_12C,%%eax\n \
    addl    _gploc_128,%%ebp\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    addl    %%eax,%%esi\n \
    movl    _gploc_FC,%%eax\n \
    movl    _gploc_34,%%ecx\n \
    movl    _gploc_D8,%%edx\n \
    movl    _gploc_E4,%%ebx\n \
    addl    _gploc_60,%%ecx\n \
    adcl    _gploc_CC,%%edx\n \
    adcl    _gploc_C4,%%ebx\n \
    addl    _gploc_104,%%edi\n \
    decl _gploc_C0\n \
    jz  loc_783A68\n \
\n \
loc_783899:         # 334D\n \
    movl    %%eax,_gploc_FC\n \
    movl    %%ebp,_gploc_F8\n \
    movl    %%edi,_gploc_F4\n \
    sarl    $0x10,%%eax\n \
    js  loc_783980\n \
    cmpl    %%esi,%%eax\n \
    jg  loc_783940\n \
    jl  loc_783960\n \
\n \
loc_7838C5:         # 3768\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_F4,%%edi\n \
    movl    %%ecx,_gploc_34\n \
    movl    %%edx,_gploc_D8\n \
    movl    %%ebx,_gploc_E4\n \
    sarl    $0x10,%%ebp\n \
    cmpl    _LOC_vec_window_width,%%ebp\n \
    jg  loc_7839D0\n \
\n \
loc_7838F7:         # 37E6\n \
    addl    %%esi,%%edi\n \
    subl    %%esi,%%ebp\n \
    jle loc_783812\n \
    movl    %%ebp,%%eax\n \
    andl    $0x0F,%%eax\n \
    addl    _gpoly_countdown(,%%eax,4),%%edi\n \
    movl    %%ebp,_gploc_D4\n \
    movl    $0x0FF0000FF,%%ecx\n \
    andl    %%ebx,%%ecx\n \
    roll    $8,%%ecx\n \
    movl    _LOC_vec_map,%%esi\n \
    movl    _gploc_5C,%%ebp\n \
    jmp     *off_7840A0(,%%eax,4)\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_783940:         # 36C\n \
    addl    _gploc_30,%%ecx\n \
    adcl    _gploc_BC,%%edx\n \
    adcl    _gploc_B8,%%ebx\n \
    incl    %%esi\n \
    cmpl    %%esi,%%eax\n \
    jle loc_7838C5\n \
    jmp loc_783940\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_783960:         # 36C\n \
    subl    _gploc_30,%%ecx\n \
    sbbl    _gploc_BC,%%edx\n \
    sbbl    _gploc_B8,%%ebx\n \
    decl    %%esi\n \
    cmpl    %%esi,%%eax\n \
    jge loc_7838C5\n \
    jmp loc_783960\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_783980:         # 36C\n \
    orl %%esi,%%esi\n \
    jz  loc_7838C5\n \
    js  loc_783990\n \
    jmp loc_7839B0\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_783990:         # 3798\n \
    addl    _gploc_30,%%ecx\n \
    adcl    _gploc_BC,%%edx\n \
    adcl    _gploc_B8,%%ebx\n \
    incl    %%esi\n \
    jz  loc_7838C5\n \
    jmp loc_783990\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_7839B0:         # 379A\n \
    subl    _gploc_30,%%ecx\n \
    sbbl    _gploc_BC,%%edx\n \
    sbbl    _gploc_B8,%%ebx\n \
    decl    %%esi\n \
    jz  loc_7838C5\n \
    jmp loc_7839B0\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_7839D0:         # 370\n \
    movl    _LOC_vec_window_width,%%ebp\n \
    jmp loc_7838F7\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_7839E0:         # 3340\n \
    addl    _gploc_60,%%ecx\n \
    adcl    _gploc_CC,%%edx\n \
    adcl    _gploc_C4,%%ebx\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    subl    %%eax,_gploc_74\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_12C,%%eax\n \
    addl    _gploc_128,%%ebp\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    addl    %%eax,_gploc_74\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_104,%%edi\n \
    decl _gploc_C0\n \
    jz  loc_783A60\n \
    incl    %%esi\n \
    js  loc_7839E0\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_783899\n \
# ---------------------------------------------------------------------------\n \
\
\n \
loc_783A60:         # 385\n \
    movl    _gploc_74,%%esi\n \
    nop \n \
\n \
loc_783A68:         # 333\n \
    decl _gploc_180\n \
    jz  locret69a\n \
    movl    %%eax,_gploc_FC\n \
    movl    _crease_len,%%eax\n \
    orl %%eax,%%eax\n \
    js  loc_783B10\n \
    movl    _factor_cb,%%eax\n \
    movl    %%eax,_gploc_12C\n \
    movl    _gploc_64,%%eax\n \
    movl    %%eax,_gploc_60\n \
    movl    _gploc_98,%%eax\n \
    movl    %%eax,_gploc_CC\n \
    movl    _gploc_94,%%eax\n \
    movl    %%eax,_gploc_C4\n \
    xorl    %%ecx,%%ecx\n \
    movl    _gploc_80,%%edx\n \
    movl    _gploc_7C,%%ebx\n \
    movl    _gploc_pt_cy,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_783ADB\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_783ADB:         # 38E\n \
    subl    _gploc_pt_by,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_pt_bx,%%eax\n \
    movl    %%eax,_gploc_74\n \
    movl    _gploc_pt_shbx,%%eax\n \
    jle locret69a\n \
    movl    _gploc_pt_by,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_7839E0\n \
    movl    _gploc_pt_bx,%%esi\n \
    jmp loc_783899\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_783B10:         # 388\n \
    movl    _factor_cb,%%ebp\n \
    movl    %%ebp,_gploc_128\n \
    movl    _gploc_pt_shbx,%%ebp\n \
    movl    _gploc_pt_cy,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_783B30\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_783B30:         # 393\n \
    subl    _gploc_pt_by,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_FC,%%eax\n \
    jle locret69a\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_pt_by,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_7839E0\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_783899\n \
\n \
off_7840A0:\n \
    .int    loc_783542\n \
    .int    loc_7837D5\n \
    .int    loc_7837A9\n \
    .int    loc_78377D\n \
    .int    loc_783751\n \
    .int    loc_783725\n \
    .int    loc_7836F9\n \
    .int    loc_7836CD\n \
    .int    loc_7836A1\n \
    .int    loc_783675\n \
    .int    loc_783649\n \
    .int    loc_78361D\n \
    .int    loc_7835F1\n \
    .int    loc_7835C5\n \
    .int    loc_783599\n \
    .int    loc_78356D\n \
\n \
locret69a:\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

void draw_gpoly_sub14() {
#if __GNUC__
  asm volatile(
      " \
    pusha   \n \
    xorl    %%ecx,%%ecx\n \
    movl    _gploc_8C,%%edx\n \
    movl    _gploc_88,%%ebx\n \
    movl    _gploc_pt_ay,%%esi\n \
    movl    _LOC_vec_screen_width,%%edi\n \
    imull   %%esi,%%edi\n \
    addl    _LOC_vec_screen,%%edi\n \
    movl    _gploc_pt_ay,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jg  locret69b\n \
    movl    _gploc_pt_by,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_782EC7\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_782EC7:         # 2CD0\n \
    subl    _gploc_pt_ay,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_pt_ax,%%esi\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_pt_shax,%%eax\n \
    movl    %%eax,%%ebp\n \
    jz  loc_783338\n \
    movl    _gploc_pt_ay,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_7832B0\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_783239\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_782F01:         # 2FDB\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_782F2C:         # 3EAC\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,1(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_782F58:         # 3EA8\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,2(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_782F84:         # 3EA4\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,3(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_782FB0:         # 3EA0\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,4(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_782FDC:         # 3E9C\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,5(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783008:         # 3E98\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,6(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783034:         # 3E94\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,7(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783060:         # 3E90\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl     %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,8(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_78308C:         # 3E8C\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,9(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_7830B8:         # 3E88\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0A(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_7830E4:         # 3E84\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0B(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783110:         # 3E80\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0C(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_78313C:         # 3E7C\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0D(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783168:         # 3E78\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl     %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0E(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
\n \
loc_783194:         # 3E74\n \
    xorl    %%eax,%%eax\n \
    movb    (%%ecx,%%esi),%%al\n \
    movl    $0x0FF00,%%ecx\n \
    andl    %%edx,%%ecx\n \
    orl %%eax,%%ecx\n \
    xorl    %%eax,%%eax\n \
    pushl   %%ebx\n \
    movl    _render_fade_tables,%%ebx\n \
    movb    (%%ebx,%%ecx),%%al\n \
    popl    %%ebx\n \
    movl    $0x0FF0000FF,%%ecx\n \
    movb    %%al,0x0F(%%edi)\n \
    andl    %%ebx,%%ecx\n \
    addl    %%ebp,%%edx\n \
    adcl    _gploc_2C,%%ebx\n \
    roll    $8,%%ecx\n \
    addl    $0x10,%%edi\n \
    subl $0x10,_gploc_D4\n \
    jg  loc_782F01\n \
\n \
loc_7831D1:         # 3084\n \
    movl    _gploc_FC,%%eax\n \
    movl    _gploc_F8,%%ebp\n \
    movl    _gploc_F4,%%edi\n \
    movl    _gploc_74,%%esi\n \
    addl    _gploc_12C,%%eax\n \
    addl    _gploc_128,%%ebp\n \
    movl    _gploc_34,%%ecx\n \
    movl    _gploc_D8,%%edx\n \
    movl    _gploc_E4,%%ebx\n \
    addl    _gploc_60,%%ecx\n \
    adcl    _gploc_CC,%%edx\n \
    adcl    _gploc_C4,%%ebx\n \
    addl    _gploc_104,%%edi\n \
    decl _gploc_C0\n \
    jz  loc_783338\n \
\n \
loc_783239:         # 2D0C\n \
    movl    %%eax,_gploc_FC\n \
    movl    %%ebp,_gploc_F8\n \
    movl    %%edi,_gploc_F4\n \
    sarl    $0x10,%%eax\n \
    movl    _gploc_F4,%%edi\n \
    movl    %%ecx,_gploc_34\n \
    movl    %%edx,_gploc_D8\n \
    movl    %%ebx,_gploc_E4\n \
    sarl    $0x10,%%ebp\n \
    addl    %%eax,%%edi\n \
    subl    %%eax,%%ebp\n \
    jle loc_7831D1\n \
    movl    %%ebp,%%eax\n \
    andl    $0x0F,%%eax\n \
    addl    _gpoly_countdown(,%%eax,4),%%edi\n \
    movl    %%ebp,_gploc_D4\n \
    movl    $0x0FF0000FF,%%ecx\n \
    andl    %%ebx,%%ecx\n \
    roll    $8,%%ecx\n \
    movl    _LOC_vec_map,%%esi\n \
    movl    _gploc_5C,%%ebp\n \
    jmp   *off_784060(,%%eax,4)\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_7832B0:         # 2CF\n \
    addl    _gploc_60,%%ecx\n \
    adcl    _gploc_CC,%%edx\n \
    adcl    _gploc_C4,%%ebx\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    subl    %%eax,_gploc_74\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_12C,%%eax\n \
    addl    _gploc_128,%%ebp\n \
    movl    %%eax,_gploc_FC\n \
    sarl    $0x10,%%eax\n \
    addl    %%eax,_gploc_74\n \
    movl    _gploc_FC,%%eax\n \
    addl    _gploc_104,%%edi\n \
    decl _gploc_C0\n \
    jz  loc_783330\n \
    incl    %%esi\n \
    js  loc_7832B0\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_783239\n \
# ---------------------------------------------------------------------------\n \
\
\n \
loc_783330:         # 312\n \
    movl    _gploc_74,%%esi\n \
    nop \n \
\n \
loc_783338:         # 2CF3\n \
    decl _gploc_180\n \
    jz  locret69b\n \
    movl    %%eax,_gploc_FC\n \
    movl    _crease_len,%%eax\n \
    orl %%eax,%%eax\n \
    js  loc_7833E0\n \
    movl    _factor_cb,%%eax\n \
    movl    %%eax,_gploc_12C\n \
    movl    _gploc_64,%%eax\n \
    movl    %%eax,_gploc_60\n \
    movl    _gploc_98,%%eax\n \
    movl    %%eax,_gploc_CC\n \
    movl    _gploc_94,%%eax\n \
    movl    %%eax,_gploc_C4\n \
    xorl    %%ecx,%%ecx\n \
    movl    _gploc_80,%%edx\n \
    movl    _gploc_7C,%%ebx\n \
    movl    _gploc_pt_cy,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_7833AB\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_7833AB:         # 31B\n \
    subl    _gploc_pt_by,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_pt_bx,%%eax\n \
    movl    %%eax,_gploc_74\n \
    movl    _gploc_pt_shbx,%%eax\n \
    jle locret69b\n \
    movl    _gploc_pt_by,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_7832B0\n \
    movl    _gploc_pt_bx,%%esi\n \
    jmp loc_783239\n \
# ---------------------------------------------------------------------------\n \
\n \
loc_7833E0:         # 315\n \
    movl    _factor_cb,%%ebp\n \
    movl    %%ebp,_gploc_128\n \
    movl    _gploc_pt_shbx,%%ebp\n \
    movl    _gploc_pt_cy,%%eax\n \
    cmpl    _LOC_vec_window_height,%%eax\n \
    jle loc_783400\n \
    movl    _LOC_vec_window_height,%%eax\n \
\n \
loc_783400:         # 320\n \
    subl    _gploc_pt_by,%%eax\n \
    movl    %%eax,_gploc_C0\n \
    movl    _gploc_FC,%%eax\n \
    jle locret69b\n \
    movl    %%esi,_gploc_74\n \
    movl    _gploc_pt_by,%%esi\n \
    orl %%esi,%%esi\n \
    js  loc_7832B0\n \
    movl    _gploc_74,%%esi\n \
    jmp loc_783239\n \
\n \
off_784060:\n \
    .int    loc_782F01\n \
    .int    loc_783194\n \
    .int    loc_783168\n \
    .int    loc_78313C\n \
    .int    loc_783110\n \
    .int    loc_7830E4\n \
    .int    loc_7830B8\n \
    .int    loc_78308C\n \
    .int    loc_783060\n \
    .int    loc_783034\n \
    .int    loc_783008\n \
    .int    loc_782FDC\n \
    .int    loc_782FB0\n \
    .int    loc_782F84\n \
    .int    loc_782F58\n \
    .int    loc_782F2C\n \
\n \
locret69b:\n \
    popa    \n \
"
      :
      :
      : "memory", "cc");
#endif
}

/******************************************************************************/
