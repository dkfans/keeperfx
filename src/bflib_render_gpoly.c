/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_render_gpoly.c
 *     Rendering function draw_gpoly() for drawing 3D view elements.
 * @par Purpose:
 *     Function for rendering 3D elements.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 14 Feb 2010
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

#ifdef __GNUC__
 #pragma GCC optimize "Ofast", "omit-frame-pointer"
 #define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
 #define ALWAYS_INLINE inline
#endif

/******************************************************************************/
static const int32_t gpoly_reptable[] = {
  0x00000000, 0x7FFFFFFF, 0x3FFFFFFF, 0x2AAAAAAA, 0x1FFFFFFF, 0x19999999, 0x15555555, 0x12492492,
  0x0FFFFFFF, 0x0E38E38E, 0x0CCCCCCC, 0x0BA2E8BA, 0x0AAAAAAA, 0x09D89D89, 0x09249249, 0x08888888,
  0x07FFFFFF, 0x07878787, 0x071C71C7, 0x06BCA1AF, 0x06666666, 0x06186186, 0x05D1745D, 0x0590B216,
  0x05555555, 0x051EB851, 0x04EC4EC4, 0x04BDA12F, 0x04924924, 0x0469EE58, 0x04444444, 0x04210842,
  0x03FFFFFF, 0x03E0F83E, 0x03C3C3C3, 0x03A83A83, 0x038E38E3, 0x03759F22, 0x035E50D7, 0x03483483,
  0x03333333, 0x031F3831, 0x030C30C3, 0x02FA0BE8, 0x02E8BA2E, 0x02D82D82, 0x02C8590B, 0x02B93105,
  0x02AAAAAA, 0x029CBC14, 0x028F5C28, 0x02828282, 0x02762762, 0x026A439F, 0x025ED097, 0x0253C825,
  0x02492492, 0x023EE08F, 0x0234F72C, 0x022B63CB, 0x02222222, 0x02192E29, 0x02108421, 0x02082082,
  0x01FFFFFF, 0x01F81F81, 0x01F07C1F, 0x01E9131A, 0x01E1E1E1, 0x01DAE607, 0x01D41D41, 0x01CD8568,
  0x01C71C71, 0x01C0E070, 0x01BACF91, 0x01B4E81B, 0x01AF286B, 0x01A98EF6, 0x01A41A41, 0x019EC8E9,
  0x01999999, 0x01948B0F, 0x018F9C18, 0x018ACB90, 0x01861861, 0x01818181, 0x017D05F4, 0x0178A4C8,
  0x01745D17, 0x01702E05, 0x016C16C1, 0x01681681, 0x01642C85, 0x01605816, 0x015C9882, 0x0158ED23,
  0x01555555, 0x0151D07E, 0x014E5E0A, 0x014AFD6A, 0x0147AE14, 0x01446F86, 0x01414141, 0x013E22CB,
  0x013B13B1, 0x01381381, 0x013521CF, 0x01323E34, 0x012F684B, 0x012C9FB4, 0x0129E412, 0x0127350B,
  0x01249249, 0x0121FB78, 0x011F7047, 0x011CF06A, 0x011A7B96, 0x01181181, 0x0115B1E5, 0x01135C81,
  0x01111111, 0x010ECF56, 0x010C9714, 0x010A6810, 0x01084210, 0x010624DD, 0x01041041, 0x01020408,
  0x00FFFFFF, 0x00FE03F8, 0x00FC0FC0, 0x00FA232C, 0x00F83E0F, 0x00F6603D, 0x00F4898D, 0x00F2B9D6,
  0x00F0F0F0, 0x00EF2EB7, 0x00ED7303, 0x00EBBDB2, 0x00EA0EA0, 0x00E865AC, 0x00E6C2B4, 0x00E52598,
  0x00E38E38, 0x00E1FC78, 0x00E07038, 0x00DEE95C, 0x00DD67C8, 0x00DBEB61, 0x00DA740D, 0x00D901B2,
  0x00D79435, 0x00D62B80, 0x00D4C77B, 0x00D3680D, 0x00D20D20, 0x00D0B69F, 0x00CF6474, 0x00CE168A,
  0x00CCCCCC, 0x00CB8727, 0x00CA4587, 0x00C907DA, 0x00C7CE0C, 0x00C6980C, 0x00C565C8, 0x00C4372F,
  0x00C30C30, 0x00C1E4BB, 0x00C0C0C0, 0x00BFA02F, 0x00BE82FA, 0x00BD6910, 0x00BC5264, 0x00BB3EE7,
  0x00BA2E8B, 0x00B92143, 0x00B81702, 0x00B70FBB, 0x00B60B60, 0x00B509E6, 0x00B40B40, 0x00B30F63,
  0x00B21642, 0x00B11FD3, 0x00B02C0B, 0x00AF3ADD, 0x00AE4C41, 0x00AD602B, 0x00AC7691, 0x00AB8F69,
  0x00AAAAAA, 0x00A9C84A, 0x00A8E83F, 0x00A80A80, 0x00A72F05, 0x00A655C4, 0x00A57EB5, 0x00A4A9CF,
  0x00A3D70A, 0x00A3065E, 0x00A237C3, 0x00A16B31, 0x00A0A0A0, 0x009FD809, 0x009F1165, 0x009E4CAD,
  0x009D89D8, 0x009CC8E1, 0x009C09C0, 0x009B4C6F, 0x009A90E7, 0x0099D722, 0x00991F1A, 0x009868C8,
  0x0097B425, 0x0097012E, 0x00964FDA, 0x0095A025, 0x0094F209, 0x00944580, 0x00939A85, 0x0092F113,
  0x00924924, 0x0091A2B3, 0x0090FDBC, 0x00905A38, 0x008FB823, 0x008F1779, 0x008E7835, 0x008DDA52,
  0x008D3DCB, 0x008CA29C, 0x008C08C0, 0x008B7034, 0x008AD8F2, 0x008A42F8, 0x0089AE40, 0x00891AC7,
  0x00888888, 0x0087F780, 0x008767AB, 0x0086D905, 0x00864B8A, 0x0085BF37, 0x00853408, 0x0084A9F9,
  0x00842108, 0x00839930, 0x0083126E, 0x00828CBF, 0x00820820, 0x0081848D, 0x00810204, 0x00808080
};

static const int32_t gpoly_divtable[][64] = {
   {-8388607,-8388607,-8388607,-8388607,-8388607,-8388607,-8388607,-8388607,
    -8388607,-8388607,-8388607,-8388607,-8388607,-8388607,-8388607,-8388607,
    -8388607,-8388607,-8388607,-8388607,-8388607,-8388607,-8388607,-8388607,
    -8388607,-8388607,-8388607,-8388607,-8388607,-8388607,-8388607,-8388607,
           0, 8388607, 8388607, 8388607, 8388607, 8388607, 8388607, 8388607,
     8388607, 8388607, 8388607, 8388607, 8388607, 8388607, 8388607, 8388607,
     8388607, 8388607, 8388607, 8388607, 8388607, 8388607, 8388607, 8388607,
     8388607, 8388607, 8388607, 8388607, 8388607, 8388607, 8388607, 8388607,},
   {-2097152,-2031616,-1966080,-1900544,-1835008,-1769472,-1703936,-1638400,
    -1572864,-1507328,-1441792,-1376256,-1310720,-1245184,-1179648,-1114112,
    -1048576, -983040, -917504, -851968, -786432, -720896, -655360, -589824,
     -524288, -458752, -393216, -327680, -262144, -196608, -131072,  -65536,
           0,   65536,  131072,  196608,  262144,  327680,  393216,  458752,
      524288,  589824,  655360,  720896,  786432,  851968,  917504,  983040,
     1048576, 1114112, 1179648, 1245184, 1310720, 1376256, 1441792, 1507328,
     1572864, 1638400, 1703936, 1769472, 1835008, 1900544, 1966080, 2031616,},
   {-1048576,-1015808, -983040, -950272, -917504, -884736, -851968, -819200,
     -786432, -753664, -720896, -688128, -655360, -622592, -589824, -557056,
     -524288, -491520, -458752, -425984, -393216, -360448, -327680, -294912,
     -262144, -229376, -196608, -163840, -131072,  -98304,  -65536,  -32768,
           0,   32768,   65536,   98304,  131072,  163840,  196608,  229376,
      262144,  294912,  327680,  360448,  393216,  425984,  458752,  491520,
      524288,  557056,  589824,  622592,  655360,  688128,  720896,  753664,
      786432,  819200,  851968,  884736,  917504,  950272,  983040, 1015808,},
   { -699050, -677205, -655360, -633514, -611669, -589824, -567978, -546133,
     -524288, -502442, -480597, -458752, -436906, -415061, -393216, -371370,
     -349525, -327680, -305834, -283989, -262144, -240298, -218453, -196608,
     -174762, -152917, -131072, -109226,  -87381,  -65536,  -43690,  -21845,
           0,   21845,   43690,   65536,   87381,  109226,  131072,  152917,
      174762,  196608,  218453,  240298,  262144,  283989,  305834,  327680,
      349525,  371370,  393216,  415061,  436906,  458752,  480597,  502442,
      524288,  546133,  567978,  589824,  611669,  633514,  655360,  677205,},
   { -524288, -507904, -491520, -475136, -458752, -442368, -425984, -409600,
     -393216, -376832, -360448, -344064, -327680, -311296, -294912, -278528,
     -262144, -245760, -229376, -212992, -196608, -180224, -163840, -147456,
     -131072, -114688,  -98304,  -81920,  -65536,  -49152,  -32768,  -16384,
           0,   16384,   32768,   49152,   65536,   81920,   98304,  114688,
      131072,  147456,  163840,  180224,  196608,  212992,  229376,  245760,
      262144,  278528,  294912,  311296,  327680,  344064,  360448,  376832,
      393216,  409600,  425984,  442368,  458752,  475136,  491520,  507904,},
   { -419430, -406323, -393216, -380108, -367001, -353894, -340787, -327680,
     -314572, -301465, -288358, -275251, -262144, -249036, -235929, -222822,
     -209715, -196608, -183500, -170393, -157286, -144179, -131072, -117964,
     -104857,  -91750,  -78643,  -65536,  -52428,  -39321,  -26214,  -13107,
           0,   13107,   26214,   39321,   52428,   65536,   78643,   91750,
      104857,  117964,  131072,  144179,  157286,  170393,  183500,  196608,
      209715,  222822,  235929,  249036,  262144,  275251,  288358,  301465,
      314572,  327680,  340787,  353894,  367001,  380108,  393216,  406323,},
   { -349525, -338602, -327680, -316757, -305834, -294912, -283989, -273066,
     -262144, -251221, -240298, -229376, -218453, -207530, -196608, -185685,
     -174762, -163840, -152917, -141994, -131072, -120149, -109226,  -98304,
      -87381,  -76458,  -65536,  -54613,  -43690,  -32768,  -21845,  -10922,
           0,   10922,   21845,   32768,   43690,   54613,   65536,   76458,
       87381,   98304,  109226,  120149,  131072,  141994,  152917,  163840,
      174762,  185685,  196608,  207530,  218453,  229376,  240298,  251221,
      262144,  273066,  283989,  294912,  305834,  316757,  327680,  338602,},
   { -299593, -290230, -280868, -271506, -262144, -252781, -243419, -234057,
     -224694, -215332, -205970, -196608, -187245, -177883, -168521, -159158,
     -149796, -140434, -131072, -121709, -112347, -102985,  -93622,  -84260,
      -74898,  -65536,  -56173,  -46811,  -37449,  -28086,  -18724,   -9362,
           0,    9362,   18724,   28086,   37449,   46811,   56173,   65536,
       74898,   84260,   93622,  102985,  112347,  121709,  131072,  140434,
      149796,  159158,  168521,  177883,  187245,  196608,  205970,  215332,
      224694,  234057,  243419,  252781,  262144,  271506,  280868,  290230,},
   { -262144, -253952, -245760, -237568, -229376, -221184, -212992, -204800,
     -196608, -188416, -180224, -172032, -163840, -155648, -147456, -139264,
     -131072, -122880, -114688, -106496,  -98304,  -90112,  -81920,  -73728,
      -65536,  -57344,  -49152,  -40960,  -32768,  -24576,  -16384,   -8192,
           0,    8192,   16384,   24576,   32768,   40960,   49152,   57344,
       65536,   73728,   81920,   90112,   98304,  106496,  114688,  122880,
      131072,  139264,  147456,  155648,  163840,  172032,  180224,  188416,
      196608,  204800,  212992,  221184,  229376,  237568,  245760,  253952,},
   { -233016, -225735, -218453, -211171, -203889, -196608, -189326, -182044,
     -174762, -167480, -160199, -152917, -145635, -138353, -131072, -123790,
     -116508, -109226, -101944,  -94663,  -87381,  -80099,  -72817,  -65536,
      -58254,  -50972,  -43690,  -36408,  -29127,  -21845,  -14563,   -7281,
           0,    7281,   14563,   21845,   29127,   36408,   43690,   50972,
       58254,   65536,   72817,   80099,   87381,   94663,  101944,  109226,
      116508,  123790,  131072,  138353,  145635,  152917,  160199,  167480,
      174762,  182044,  189326,  196608,  203889,  211171,  218453,  225735,},
   { -209715, -203161, -196608, -190054, -183500, -176947, -170393, -163840,
     -157286, -150732, -144179, -137625, -131072, -124518, -117964, -111411,
     -104857,  -98304,  -91750,  -85196,  -78643,  -72089,  -65536,  -58982,
      -52428,  -45875,  -39321,  -32768,  -26214,  -19660,  -13107,   -6553,
           0,    6553,   13107,   19660,   26214,   32768,   39321,   45875,
       52428,   58982,   65536,   72089,   78643,   85196,   91750,   98304,
      104857,  111411,  117964,  124518,  131072,  137625,  144179,  150732,
      157286,  163840,  170393,  176947,  183500,  190054,  196608,  203161,},
   { -190650, -184692, -178734, -172776, -166818, -160861, -154903, -148945,
     -142987, -137029, -131072, -125114, -119156, -113198, -107240, -101282,
      -95325,  -89367,  -83409,  -77451,  -71493,  -65536,  -59578,  -53620,
      -47662,  -41704,  -35746,  -29789,  -23831,  -17873,  -11915,   -5957,
           0,    5957,   11915,   17873,   23831,   29789,   35746,   41704,
       47662,   53620,   59578,   65536,   71493,   77451,   83409,   89367,
       95325,  101282,  107240,  113198,  119156,  125114,  131072,  137029,
      142987,  148945,  154903,  160861,  166818,  172776,  178734,  184692,},
   { -174762, -169301, -163840, -158378, -152917, -147456, -141994, -136533,
     -131072, -125610, -120149, -114688, -109226, -103765,  -98304,  -92842,
      -87381,  -81920,  -76458,  -70997,  -65536,  -60074,  -54613,  -49152,
      -43690,  -38229,  -32768,  -27306,  -21845,  -16384,  -10922,   -5461,
           0,    5461,   10922,   16384,   21845,   27306,   32768,   38229,
       43690,   49152,   54613,   60074,   65536,   70997,   76458,   81920,
       87381,   92842,   98304,  103765,  109226,  114688,  120149,  125610,
      131072,  136533,  141994,  147456,  152917,  158378,  163840,  169301,},
   { -161319, -156278, -151236, -146195, -141154, -136113, -131072, -126030,
     -120989, -115948, -110907, -105865, -100824,  -95783,  -90742,  -85700,
      -80659,  -75618,  -70577,  -65536,  -60494,  -55453,  -50412,  -45371,
      -40329,  -35288,  -30247,  -25206,  -20164,  -15123,  -10082,   -5041,
           0,    5041,   10082,   15123,   20164,   25206,   30247,   35288,
       40329,   45371,   50412,   55453,   60494,   65536,   70577,   75618,
       80659,   85700,   90742,   95783,  100824,  105865,  110907,  115948,
      120989,  126030,  131072,  136113,  141154,  146195,  151236,  156278,},
   { -149796, -145115, -140434, -135753, -131072, -126390, -121709, -117028,
     -112347, -107666, -102985,  -98304,  -93622,  -88941,  -84260,  -79579,
      -74898,  -70217,  -65536,  -60854,  -56173,  -51492,  -46811,  -42130,
      -37449,  -32768,  -28086,  -23405,  -18724,  -14043,   -9362,   -4681,
           0,    4681,    9362,   14043,   18724,   23405,   28086,   32768,
       37449,   42130,   46811,   51492,   56173,   60854,   65536,   70217,
       74898,   79579,   84260,   88941,   93622,   98304,  102985,  107666,
      112347,  117028,  121709,  126390,  131072,  135753,  140434,  145115,},
   { -139810, -135441, -131072, -126702, -122333, -117964, -113595, -109226,
     -104857, -100488,  -96119,  -91750,  -87381,  -83012,  -78643,  -74274,
      -69905,  -65536,  -61166,  -56797,  -52428,  -48059,  -43690,  -39321,
      -34952,  -30583,  -26214,  -21845,  -17476,  -13107,   -8738,   -4369,
           0,    4369,    8738,   13107,   17476,   21845,   26214,   30583,
       34952,   39321,   43690,   48059,   52428,   56797,   61166,   65536,
       69905,   74274,   78643,   83012,   87381,   91750,   96119,  100488,
      104857,  109226,  113595,  117964,  122333,  126702,  131072,  135441,},
   { -131072, -126976, -122880, -118784, -114688, -110592, -106496, -102400,
      -98304,  -94208,  -90112,  -86016,  -81920,  -77824,  -73728,  -69632,
      -65536,  -61440,  -57344,  -53248,  -49152,  -45056,  -40960,  -36864,
      -32768,  -28672,  -24576,  -20480,  -16384,  -12288,   -8192,   -4096,
           0,    4096,    8192,   12288,   16384,   20480,   24576,   28672,
       32768,   36864,   40960,   45056,   49152,   53248,   57344,   61440,
       65536,   69632,   73728,   77824,   81920,   86016,   90112,   94208,
       98304,  102400,  106496,  110592,  114688,  118784,  122880,  126976,},
   { -123361, -119506, -115651, -111796, -107941, -104086, -100231,  -96376,
      -92521,  -88666,  -84811,  -80956,  -77101,  -73246,  -69391,  -65536,
      -61680,  -57825,  -53970,  -50115,  -46260,  -42405,  -38550,  -34695,
      -30840,  -26985,  -23130,  -19275,  -15420,  -11565,   -7710,   -3855,
           0,    3855,    7710,   11565,   15420,   19275,   23130,   26985,
       30840,   34695,   38550,   42405,   46260,   50115,   53970,   57825,
       61680,   65536,   69391,   73246,   77101,   80956,   84811,   88666,
       92521,   96376,  100231,  104086,  107941,  111796,  115651,  119506,},
   { -116508, -112867, -109226, -105585, -101944,  -98304,  -94663,  -91022,
      -87381,  -83740,  -80099,  -76458,  -72817,  -69176,  -65536,  -61895,
      -58254,  -54613,  -50972,  -47331,  -43690,  -40049,  -36408,  -32768,
      -29127,  -25486,  -21845,  -18204,  -14563,  -10922,   -7281,   -3640,
           0,    3640,    7281,   10922,   14563,   18204,   21845,   25486,
       29127,   32768,   36408,   40049,   43690,   47331,   50972,   54613,
       58254,   61895,   65536,   69176,   72817,   76458,   80099,   83740,
       87381,   91022,   94663,   98304,  101944,  105585,  109226,  112867,},
   { -110376, -106927, -103477, -100028,  -96579,  -93130,  -89680,  -86231,
      -82782,  -79333,  -75883,  -72434,  -68985,  -65536,  -62086,  -58637,
      -55188,  -51738,  -48289,  -44840,  -41391,  -37941,  -34492,  -31043,
      -27594,  -24144,  -20695,  -17246,  -13797,  -10347,   -6898,   -3449,
           0,    3449,    6898,   10347,   13797,   17246,   20695,   24144,
       27594,   31043,   34492,   37941,   41391,   44840,   48289,   51738,
       55188,   58637,   62086,   65536,   68985,   72434,   75883,   79333,
       82782,   86231,   89680,   93130,   96579,  100028,  103477,  106927,},
   { -104857, -101580,  -98304,  -95027,  -91750,  -88473,  -85196,  -81920,
      -78643,  -75366,  -72089,  -68812,  -65536,  -62259,  -58982,  -55705,
      -52428,  -49152,  -45875,  -42598,  -39321,  -36044,  -32768,  -29491,
      -26214,  -22937,  -19660,  -16384,  -13107,   -9830,   -6553,   -3276,
           0,    3276,    6553,    9830,   13107,   16384,   19660,   22937,
       26214,   29491,   32768,   36044,   39321,   42598,   45875,   49152,
       52428,   55705,   58982,   62259,   65536,   68812,   72089,   75366,
       78643,   81920,   85196,   88473,   91750,   95027,   98304,  101580,},
   {  -99864,  -96743,  -93622,  -90502,  -87381,  -84260,  -81139,  -78019,
      -74898,  -71777,  -68656,  -65536,  -62415,  -59294,  -56173,  -53052,
      -49932,  -46811,  -43690,  -40569,  -37449,  -34328,  -31207,  -28086,
      -24966,  -21845,  -18724,  -15603,  -12483,   -9362,   -6241,   -3120,
           0,    3120,    6241,    9362,   12483,   15603,   18724,   21845,
       24966,   28086,   31207,   34328,   37449,   40569,   43690,   46811,
       49932,   53052,   56173,   59294,   62415,   65536,   68656,   71777,
       74898,   78019,   81139,   84260,   87381,   90502,   93622,   96743,},
   {  -95325,  -92346,  -89367,  -86388,  -83409,  -80430,  -77451,  -74472,
      -71493,  -68514,  -65536,  -62557,  -59578,  -56599,  -53620,  -50641,
      -47662,  -44683,  -41704,  -38725,  -35746,  -32768,  -29789,  -26810,
      -23831,  -20852,  -17873,  -14894,  -11915,   -8936,   -5957,   -2978,
           0,    2978,    5957,    8936,   11915,   14894,   17873,   20852,
       23831,   26810,   29789,   32768,   35746,   38725,   41704,   44683,
       47662,   50641,   53620,   56599,   59578,   62557,   65536,   68514,
       71493,   74472,   77451,   80430,   83409,   86388,   89367,   92346,},
   {  -91180,  -88331,  -85481,  -82632,  -79782,  -76933,  -74084,  -71234,
      -68385,  -65536,  -62686,  -59837,  -56987,  -54138,  -51289,  -48439,
      -45590,  -42740,  -39891,  -37042,  -34192,  -31343,  -28493,  -25644,
      -22795,  -19945,  -17096,  -14246,  -11397,   -8548,   -5698,   -2849,
           0,    2849,    5698,    8548,   11397,   14246,   17096,   19945,
       22795,   25644,   28493,   31343,   34192,   37042,   39891,   42740,
       45590,   48439,   51289,   54138,   56987,   59837,   62686,   65536,
       68385,   71234,   74084,   76933,   79782,   82632,   85481,   88331,},
   {  -87381,  -84650,  -81920,  -79189,  -76458,  -73728,  -70997,  -68266,
      -65536,  -62805,  -60074,  -57344,  -54613,  -51882,  -49152,  -46421,
      -43690,  -40960,  -38229,  -35498,  -32768,  -30037,  -27306,  -24576,
      -21845,  -19114,  -16384,  -13653,  -10922,   -8192,   -5461,   -2730,
           0,    2730,    5461,    8192,   10922,   13653,   16384,   19114,
       21845,   24576,   27306,   30037,   32768,   35498,   38229,   40960,
       43690,   46421,   49152,   51882,   54613,   57344,   60074,   62805,
       65536,   68266,   70997,   73728,   76458,   79189,   81920,   84650,},
   {  -83886,  -81264,  -78643,  -76021,  -73400,  -70778,  -68157,  -65536,
      -62914,  -60293,  -57671,  -55050,  -52428,  -49807,  -47185,  -44564,
      -41943,  -39321,  -36700,  -34078,  -31457,  -28835,  -26214,  -23592,
      -20971,  -18350,  -15728,  -13107,  -10485,   -7864,   -5242,   -2621,
           0,    2621,    5242,    7864,   10485,   13107,   15728,   18350,
       20971,   23592,   26214,   28835,   31457,   34078,   36700,   39321,
       41943,   44564,   47185,   49807,   52428,   55050,   57671,   60293,
       62914,   65536,   68157,   70778,   73400,   76021,   78643,   81264,},
   {  -80659,  -78139,  -75618,  -73097,  -70577,  -68056,  -65536,  -63015,
      -60494,  -57974,  -55453,  -52932,  -50412,  -47891,  -45371,  -42850,
      -40329,  -37809,  -35288,  -32768,  -30247,  -27726,  -25206,  -22685,
      -20164,  -17644,  -15123,  -12603,  -10082,   -7561,   -5041,   -2520,
           0,    2520,    5041,    7561,   10082,   12603,   15123,   17644,
       20164,   22685,   25206,   27726,   30247,   32768,   35288,   37809,
       40329,   42850,   45371,   47891,   50412,   52932,   55453,   57974,
       60494,   63015,   65536,   68056,   70577,   73097,   75618,   78139,},
   {  -77672,  -75245,  -72817,  -70390,  -67963,  -65536,  -63108,  -60681,
      -58254,  -55826,  -53399,  -50972,  -48545,  -46117,  -43690,  -41263,
      -38836,  -36408,  -33981,  -31554,  -29127,  -26699,  -24272,  -21845,
      -19418,  -16990,  -14563,  -12136,   -9709,   -7281,   -4854,   -2427,
           0,    2427,    4854,    7281,    9709,   12136,   14563,   16990,
       19418,   21845,   24272,   26699,   29127,   31554,   33981,   36408,
       38836,   41263,   43690,   46117,   48545,   50972,   53399,   55826,
       58254,   60681,   63108,   65536,   67963,   70390,   72817,   75245,},
   {  -74898,  -72557,  -70217,  -67876,  -65536,  -63195,  -60854,  -58514,
      -56173,  -53833,  -51492,  -49152,  -46811,  -44470,  -42130,  -39789,
      -37449,  -35108,  -32768,  -30427,  -28086,  -25746,  -23405,  -21065,
      -18724,  -16384,  -14043,  -11702,   -9362,   -7021,   -4681,   -2340,
           0,    2340,    4681,    7021,    9362,   11702,   14043,   16384,
       18724,   21065,   23405,   25746,   28086,   30427,   32768,   35108,
       37449,   39789,   42130,   44470,   46811,   49152,   51492,   53833,
       56173,   58514,   60854,   63195,   65536,   67876,   70217,   72557,},
   {  -72315,  -70055,  -67795,  -65536,  -63276,  -61016,  -58756,  -56496,
      -54236,  -51976,  -49716,  -47457,  -45197,  -42937,  -40677,  -38417,
      -36157,  -33897,  -31638,  -29378,  -27118,  -24858,  -22598,  -20338,
      -18078,  -15819,  -13559,  -11299,   -9039,   -6779,   -4519,   -2259,
           0,    2259,    4519,    6779,    9039,   11299,   13559,   15819,
       18078,   20338,   22598,   24858,   27118,   29378,   31638,   33897,
       36157,   38417,   40677,   42937,   45197,   47457,   49716,   51976,
       54236,   56496,   58756,   61016,   63276,   65536,   67795,   70055,},
   {  -69905,  -67720,  -65536,  -63351,  -61166,  -58982,  -56797,  -54613,
      -52428,  -50244,  -48059,  -45875,  -43690,  -41506,  -39321,  -37137,
      -34952,  -32768,  -30583,  -28398,  -26214,  -24029,  -21845,  -19660,
      -17476,  -15291,  -13107,  -10922,   -8738,   -6553,   -4369,   -2184,
           0,    2184,    4369,    6553,    8738,   10922,   13107,   15291,
       17476,   19660,   21845,   24029,   26214,   28398,   30583,   32768,
       34952,   37137,   39321,   41506,   43690,   45875,   48059,   50244,
       52428,   54613,   56797,   58982,   61166,   63351,   65536,   67720,},
   {  -67650,  -65536,  -63421,  -61307,  -59193,  -57079,  -54965,  -52851,
      -50737,  -48623,  -46509,  -44395,  -42281,  -40167,  -38053,  -35939,
      -33825,  -31710,  -29596,  -27482,  -25368,  -23254,  -21140,  -19026,
      -16912,  -14798,  -12684,  -10570,   -8456,   -6342,   -4228,   -2114,
           0,    2114,    4228,    6342,    8456,   10570,   12684,   14798,
       16912,   19026,   21140,   23254,   25368,   27482,   29596,   31710,
       33825,   35939,   38053,   40167,   42281,   44395,   46509,   48623,
       50737,   52851,   54965,   57079,   59193,   61307,   63421,   65536,},
};

/******************************************************************************/

// Triangle vertex info.  These are sorted in Y direction, A is on top.
static int32_t vertex_a_y, vertex_a_x, vertex_a_shade, vertex_a_texture_u, vertex_a_texture_v;
static int32_t vertex_b_y, vertex_b_x, vertex_b_shade, vertex_b_texture_u, vertex_b_texture_v;
static int32_t vertex_c_y, vertex_c_x, vertex_c_shade, vertex_c_texture_u, vertex_c_texture_v;
static bool vertex_b_on_left_side;

// Slope between vertices in 16.16 (horizontal pixels per scanline).
static int32_t slope_ac, slope_ab, slope_bc, slope_left, slope_right;

// Texture mapping deltas in 16.16 for Shade, texture U, texture V.
static int32_t delta_s_x,        delta_u_x,        delta_v_x;
static int32_t delta_s_y_top,    delta_u_y_top,    delta_v_y_top;    // Along edge AB or AC
static int32_t delta_s_y_bottom, delta_u_y_bottom, delta_v_y_bottom; // Along edge BC

// Bit layout for packed texture coordinates (fractional part in lowercase):
//            msb                    lsb
// Shade    : 00000000 0000FFff ff000000
// Texture V: 000000FF ffff0000 00000000
// Texture U: FFffff00 00000000 00000000
//
// This layout was apparently chosen to minimize shifting in the inner loop:
// Texture U and V can be combined to an array index by a single rotate, and the
// shade index is already in the right position to combine with the texture
// color, which goes in the lower byte.
typedef struct { uint32_t word[3]; } TexCoord;

// Same as above, but the least significant word is omitted.
typedef struct { uint32_t word[2]; } TexCoordShort;

// Start position for vertex A
static TexCoordShort texcoord_start_a;
// Start position for vertex B
static TexCoordShort texcoord_start_b;
// X delta used in the inner loop, shorter to save one ADC instruction
static TexCoordShort texcoord_delta_x;
// X delta used for X-clipping
static TexCoord texcoord_delta_x_exact;
// Currently used Y delta (set to one of the values below)
static TexCoord texcoord_delta_y;
// Y delta along the top left edge: either AB or AC
static TexCoord texcoord_delta_y_top;
// Y delta along edge BC
static TexCoord texcoord_delta_y_bottom;

struct GPolyDrawState
{
    TexCoord texcoord;
    int32_t x_left;  // 16.16
    int32_t x_right; // 16.16
    int x;
    int y;
    int y_end;
    uint8_t *dst_line;
};

/******************************************************************************/

// Rotate left
static uint32_t rol32(uint32_t val, uint8_t shift)
{
    return (val << shift) | (val >> (32 - shift));
}

// Add with carry
static uint32_t adc32(uint32_t lhs, uint32_t rhs, bool *carry)
{
#ifdef __GNUC__
    uint32_t r;
    const bool c = __builtin_add_overflow(lhs, rhs, &r);
    *carry =   c | __builtin_add_overflow(r, *carry, &r);
    return r;
#else
 #warning "missing optimized adc32 implementation for this compiler"
    const uint32_t r1 = lhs + rhs;
    const bool c = r1 < lhs;
    const uint32_t r2 = r1 + *carry;
    *carry =  c | (r2 < r1);
    return r2;
#endif
}

// Subtract with carry
static uint32_t sbc32(uint32_t lhs, uint32_t rhs, bool *carry)
{
#ifdef __GNUC__
    uint32_t r;
    const bool c = __builtin_sub_overflow(lhs, rhs, &r);
    *carry =   c | __builtin_sub_overflow(r, *carry, &r);
    return r;
#else
 #warning "missing optimized sbc32 implementation for this compiler"
    const uint32_t r1 = lhs - rhs;
    const bool c = r1 > lhs;
    const uint32_t r2 = r1 - *carry;
    *carry =  c | (r2 > r1);
    return r2;
#endif
}

static TexCoord texcoord_extend(TexCoordShort src)
{
    TexCoord result;
    result.word[0] = 0;
    result.word[1] = src.word[0];
    result.word[2] = src.word[1];
    return result;
}

static TexCoordShort texcoord_truncate(TexCoord src)
{
    TexCoordShort result;
    result.word[0] = src.word[1];
    result.word[1] = src.word[2];
    return result;
}

static TexCoord texcoord_pack(uint32_t u, uint32_t v, uint32_t s)
{
    TexCoord result;
    result.word[0] = (s << 24);
    result.word[1] = (v << 16) | (uint16_t)(s >>  8);
    result.word[2] = (u <<  8) | (uint8_t )(v >> 16);
    return result;
}

static TexCoord texcoord_pack_signed(int32_t u, int32_t v, int32_t s)
{
    // Sign-extend lower fields into higher fields.
    v += s >> 31;
    u += v >> 31;
    return texcoord_pack(u, v, s);
}

static TexCoord texcoord_add(TexCoord lhs, TexCoord rhs)
{
    bool cf = 0;
    TexCoord result;
    result.word[0] = adc32(lhs.word[0], rhs.word[0], &cf);
    result.word[1] = adc32(lhs.word[1], rhs.word[1], &cf);
    result.word[2] = adc32(lhs.word[2], rhs.word[2], &cf);
    return result;
}

static TexCoord texcoord_subtract(TexCoord lhs, TexCoord rhs)
{
    bool cf = 0;
    TexCoord result;
    result.word[0] = sbc32(lhs.word[0], rhs.word[0], &cf);
    result.word[1] = sbc32(lhs.word[1], rhs.word[1], &cf);
    result.word[2] = sbc32(lhs.word[2], rhs.word[2], &cf);
    return result;
}

static uint64_t texcoord_as_uint64(TexCoordShort src)
{
    return (((uint64_t)src.word[1]) << 32) | src.word[0];
}

static bool validate_triangle(void)
{
    const int ab_x = vertex_b_x - vertex_a_x;
    const int ab_y = vertex_b_y - vertex_a_y;
    const int ac_x = vertex_c_x - vertex_a_x;
    const int ac_y = vertex_c_y - vertex_a_y;
    const int bc_x = vertex_c_x - vertex_b_x;
    const int bc_y = vertex_c_y - vertex_b_y;

    // Zero height, skip it.
    if (ac_y == 0)
        return false;

    // Range check [-16384, 16383] to prevent arithmetic overflow.
    if ((  (1u + (unsigned)(ab_x >> 14))
         | (1u + (unsigned)(ab_y >> 14))
         | (1u + (unsigned)(ac_x >> 14))
         | (1u + (unsigned)(ac_y >> 14))
         | (1u + (unsigned)(bc_x >> 14))
         | (1u + (unsigned)(bc_y >> 14))) > 1u)
    {
        return false;
    }

    return true;
}

static int32_t slope_div(int dx, int dy)
{
    assert(dy >= 0);

    const int idx_x = clamp(dx + 32, 0, 63);

    if ((dy != 0) && ((dy > 31) || (dx + 32 != idx_x)))
    {
        return (dx << 16) / dy;
    }
    else
    {
        return gpoly_divtable[dy][idx_x];
    }
}

static void calculate_slopes(void)
{
    const int ab_x = vertex_b_x - vertex_a_x;
    const int ab_y = vertex_b_y - vertex_a_y;
    const int ac_x = vertex_c_x - vertex_a_x;
    const int ac_y = vertex_c_y - vertex_a_y;
    const int bc_x = vertex_c_x - vertex_b_x;
    const int bc_y = vertex_c_y - vertex_b_y;

    slope_ab = slope_div(ab_x, ab_y);
    slope_ac = slope_div(ac_x, ac_y);
    slope_bc = slope_div(bc_x, bc_y);

    // Check if vertex B is to the left or right of line AC.
    vertex_b_on_left_side = (ab_y * slope_ac) > (ab_x << 16);

    slope_left  = vertex_b_on_left_side ? slope_ab : slope_ac;
    slope_right = vertex_b_on_left_side ? slope_ac : slope_ab;
}

// Return 1.0/val (actually 0.999...) in signed 1.31, argument must be positive.
static int32_t reciprocal(uint32_t val)
{
    if (val < 256)
        return gpoly_reptable[val];
    else
        return 0x7FFFFFFFul / val;
}

// Multiply signed integer (32.0) by signed reciprocal (1.31), shift to 16.16.
static int32_t mul_shift(int32_t val, int32_t rcp)
{
    const int64_t result = (int64_t)val * rcp;
    const int32_t shifted = result >> 15;
    const int32_t sign = result >> 63;
    return shifted - sign;
}

static void calculate_texture_mapping(void)
{
    const int ab_x = vertex_b_x         - vertex_a_x;
    const int ab_y = vertex_b_y         - vertex_a_y;
    const int ac_x = vertex_c_x         - vertex_a_x;
    const int ac_y = vertex_c_y         - vertex_a_y;
    const int bc_y = vertex_c_y         - vertex_b_y;

    const int ab_u = vertex_b_texture_u - vertex_a_texture_u;
    const int ac_u = vertex_c_texture_u - vertex_a_texture_u;
    const int bc_u = vertex_c_texture_u - vertex_b_texture_u;
    const int ab_v = vertex_b_texture_v - vertex_a_texture_v;
    const int ac_v = vertex_c_texture_v - vertex_a_texture_v;
    const int bc_v = vertex_c_texture_v - vertex_b_texture_v;
    const int ab_s = vertex_b_shade     - vertex_a_shade;
    const int ac_s = vertex_c_shade     - vertex_a_shade;
    const int bc_s = vertex_c_shade     - vertex_b_shade;

    // Calculate texture deltas for X step.

    const int ab_x_biased = ab_x + (vertex_b_on_left_side ? -1 : +1);
    const int cross_product = ab_y * ac_x - ac_y * ab_x_biased;

    if (cross_product != 0)
    {
        const int32_t factor = 0x7FFFFFFF / cross_product;

        delta_u_x = mul_shift(ab_y * ac_u - ac_y * ab_u, factor);
        delta_v_x = mul_shift(ab_y * ac_v - ac_y * ab_v, factor);
        delta_s_x = mul_shift(ab_y * ac_s - ac_y * ab_s, factor);
    }
    else
    {
        delta_u_x = 0;
        delta_v_x = 0;
        delta_s_x = 0;
    }

    // Calculate texture deltas for Y step.

    if (vertex_b_on_left_side)
    {
        const int32_t factor1 = reciprocal(ab_y);
        delta_u_y_top = mul_shift(ab_u, factor1);
        delta_v_y_top = mul_shift(ab_v, factor1);
        delta_s_y_top = mul_shift(ab_s, factor1);

        const int32_t factor2 = reciprocal(bc_y);
        delta_u_y_bottom = mul_shift(bc_u, factor2);
        delta_v_y_bottom = mul_shift(bc_v, factor2);
        delta_s_y_bottom = mul_shift(bc_s, factor2);
    }
    else
    {
        const int32_t factor = reciprocal(ac_y);
        delta_u_y_top = mul_shift(ac_u, factor);
        delta_v_y_top = mul_shift(ac_v, factor);
        delta_s_y_top = mul_shift(ac_s, factor);
    }
}

static void pack_texcoords(void)
{
    {
        const int32_t u = delta_u_x;
        const int32_t v = delta_v_x;
        const int32_t s = delta_s_x;
        texcoord_delta_x_exact = texcoord_pack_signed(u, v, s);
    }
    {
        // Shade field is truncated by 8 bits, round towards 0.
        const int32_t u = delta_u_x;
        const int32_t v = delta_v_x;
        const int32_t s = delta_s_x - (delta_s_x >> 31 << 8);
        texcoord_delta_x = texcoord_truncate(texcoord_pack_signed(u, v, s));
    }
    {
        const int32_t u = delta_u_y_top;
        const int32_t v = delta_v_y_top;
        const int32_t s = delta_s_y_top;
        texcoord_delta_y_top = texcoord_pack_signed(u, v, s);
    }
    {
        const int32_t u = vertex_a_texture_u << 16;
        const int32_t v = vertex_a_texture_v << 16;
        const int32_t s = vertex_a_shade     << 16;
        texcoord_start_a = texcoord_truncate(texcoord_pack(u, v, s));
    }

    if (vertex_b_on_left_side)
    {
        {
            const int32_t u = delta_u_y_bottom;
            const int32_t v = delta_v_y_bottom;
            const int32_t s = delta_s_y_bottom;
            texcoord_delta_y_bottom = texcoord_pack_signed(u, v, s);
        }
        {
            const int32_t u = vertex_b_texture_u << 16;
            const int32_t v = vertex_b_texture_v << 16;
            const int32_t s = vertex_b_shade     << 16;
            texcoord_start_b = texcoord_truncate(texcoord_pack(u, v, s));
        }
    }

    texcoord_delta_y = texcoord_delta_y_top;
}

static void draw_gpoly_line(uint8_t *restrict pixel_dst, int32_t length, TexCoord texcoord)
{
    const uint8_t *const restrict texture = vec_map;
    const uint8_t *const restrict fade_table = render_fade_tables;
    const uint64_t texture_step = texcoord_as_uint64(texcoord_delta_x);
    uint64_t texture_position = texcoord_as_uint64(texcoord_truncate(texcoord));

    for (int i = 0; i < length; i++)
    {
        const uint16_t uv = rol32(texture_position >> 32, 8);
        const uint16_t shade = texture_position & 0xFF00;
        const uint8_t texel = texture[uv];
        pixel_dst[i] = fade_table[texel | shade];
        texture_position += texture_step;
    }
}

ALWAYS_INLINE
static void next_line(struct GPolyDrawState *state)
{
    state->texcoord  = texcoord_add(state->texcoord, texcoord_delta_y);
    state->x        -= state->x_left >> 16;
    state->x_left   += slope_left;
    state->x_right  += slope_right;
    state->x        += state->x_left >> 16;
    state->dst_line += vec_screen_width;
    state->y        += 1;
}

ALWAYS_INLINE
static void draw_gpoly_clipped_half(struct GPolyDrawState *state)
{
    for (; state->y < state->y_end; next_line(state))
    {
        if (state->y < 0)
            continue;

        const int x_left_int  = max(state->x_left  >> 16, 0);
        const int x_right_int = min(state->x_right >> 16, vec_window_width);
        const int length = x_right_int - x_left_int;
        uint8_t *const dst = state->dst_line + x_left_int;

        for (; x_left_int > state->x; ++state->x)
            state->texcoord = texcoord_add(state->texcoord, texcoord_delta_x_exact);

        for (; x_left_int < state->x; --state->x)
            state->texcoord = texcoord_subtract(state->texcoord, texcoord_delta_x_exact);

        draw_gpoly_line(dst, length, state->texcoord);
    }
}

ALWAYS_INLINE
static void draw_gpoly_whole_half(struct GPolyDrawState *state)
{
    for (; state->y < state->y_end; next_line(state))
    {
        if (state->y < 0)
            continue;

        const int x_left_int  = state->x_left  >> 16;
        const int x_right_int = state->x_right >> 16;
        const int length = x_right_int - x_left_int;
        uint8_t *const dst = state->dst_line + x_left_int;

        draw_gpoly_line(dst, length, state->texcoord);
    }
}

static void draw_gpoly_clipped(void)
{
    struct GPolyDrawState state;

    state.texcoord = texcoord_extend(texcoord_start_a);
    state.x_left   = vertex_a_x << 16;
    state.x_right  = vertex_a_x << 16;
    state.x        = vertex_a_x;
    state.y        = vertex_a_y;
    state.y_end    = min(vertex_b_y, vec_window_height);
    state.dst_line = &vec_screen[vec_screen_width * state.y];

    draw_gpoly_clipped_half(&state);

    if (vertex_b_on_left_side)
    {
        slope_left       = slope_bc;
        state.x_left     = vertex_b_x << 16;
        state.x          = vertex_b_x;
        texcoord_delta_y = texcoord_delta_y_bottom;
        state.texcoord   = texcoord_extend(texcoord_start_b);
    }
    else
    {
        slope_right   = slope_bc;
        state.x_right = vertex_b_x << 16;
    }

    state.y     = vertex_b_y;
    state.y_end = min(vertex_c_y, vec_window_height);

    draw_gpoly_clipped_half(&state);
}

static void draw_gpoly_whole(void)
{
    // state.x is not used here.
    struct GPolyDrawState state;

    state.texcoord = texcoord_extend(texcoord_start_a);
    state.x_left   = vertex_a_x << 16;
    state.x_right  = vertex_a_x << 16;
    state.y        = vertex_a_y;
    state.y_end    = min(vertex_b_y, vec_window_height);
    state.dst_line = &vec_screen[vec_screen_width * state.y];

    draw_gpoly_whole_half(&state);

    if (vertex_b_on_left_side)
    {
        slope_left       = slope_bc;
        state.x_left     = vertex_b_x << 16;
        texcoord_delta_y = texcoord_delta_y_bottom;
        state.texcoord   = texcoord_extend(texcoord_start_b);
    }
    else
    {
        slope_right   = slope_bc;
        state.x_right = vertex_b_x << 16;
    }

    state.y     = vertex_b_y;
    state.y_end = min(vertex_c_y, vec_window_height);

    draw_gpoly_whole_half(&state);
}

void draw_gpoly(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c)
{
    if (vec_mode != VM_QuadTextured)
    {
        ERRORLOG("unexpected vec_mode %d in draw_gpoly", vec_mode);
        return;
    }

    // Sort points: a.Y < b.Y < c.Y
    struct PolyPoint *point_tmp;
    if (point_a->Y > point_b->Y)
    {
        point_tmp = point_a;
        point_a = point_b;
        point_b = point_tmp;
    }
    if (point_a->Y > point_c->Y)
    {
        point_tmp = point_a;
        point_a = point_c;
        point_c = point_tmp;
    }
    if (point_b->Y > point_c->Y)
    {
        point_tmp = point_b;
        point_b = point_c;
        point_c = point_tmp;
    }

    vertex_a_x = point_a->X;
    vertex_a_y = point_a->Y;
    vertex_b_x = point_b->X;
    vertex_b_y = point_b->Y;
    vertex_c_x = point_c->X;
    vertex_c_y = point_c->Y;

    if (! validate_triangle())
        return;

    vertex_a_shade     = point_a->S >> 16;
    vertex_b_shade     = point_b->S >> 16;
    vertex_c_shade     = point_c->S >> 16;
    vertex_a_texture_u = point_a->U >> 16;
    vertex_a_texture_v = point_a->V >> 16;
    vertex_b_texture_u = point_b->U >> 16;
    vertex_b_texture_v = point_b->V >> 16;
    vertex_c_texture_u = point_c->U >> 16;
    vertex_c_texture_v = point_c->V >> 16;

    const bool clip_x = (  (vertex_a_x) | (vec_window_width - vertex_a_x)
                         | (vertex_b_x) | (vec_window_width - vertex_b_x)
                         | (vertex_c_x) | (vec_window_width - vertex_c_x) ) < 0;

    calculate_slopes();
    calculate_texture_mapping();
    pack_texcoords();

    if (clip_x)
        draw_gpoly_clipped();
    else
        draw_gpoly_whole();
}

/******************************************************************************/
