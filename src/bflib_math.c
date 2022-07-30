/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_math.c
 *     Math routines.
 * @par Purpose:
 *     Fast math routines, mostly fixed point.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     24 Jan 2009 - 08 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_math.h"

#include "bflib_basics.h"

#ifdef AUTOTESTING

#include "event_monitoring.h"

#endif
/******************************************************************************/
unsigned short const lbSqrTable[] = {
   0x0001, 0x0002, 0x0002, 0x0004, 0x0005, 0x0008, 0x000B, 0x0010,
   0x0016, 0x0020, 0x002D, 0x0040, 0x005A, 0x0080, 0x00B5, 0x0100,
   0x016A, 0x0200, 0x02D4, 0x0400, 0x05A8, 0x0800, 0x0B50, 0x1000,
   0x16A0, 0x2000, 0x2D41, 0x4000, 0x5A82, 0x8000, 0xB504, 0xFFFF,};

unsigned short const lbArcTanFactors[] = {
     0,  1,  2,  3,  5,  6,  7,  8, 10, 11, 12, 13, 15, 16, 17, 19,
    20, 21, 22, 24, 25, 26, 27, 29, 30, 31, 32, 34, 35, 36, 38, 39,
    40, 41, 43, 44, 45, 46, 48, 49, 50, 51, 53, 54, 55, 56, 57, 59,
    60, 61, 62, 64, 65, 66, 67, 68, 70, 71, 72, 73, 75, 76, 77, 78,
    79, 81, 82, 83, 84, 85, 87, 88, 89, 90, 91, 92, 94, 95, 96, 97,
    98, 99,101,102,103,104,105,106,107,109,110,111,112,113,114,115,
   116,118,119,120,121,122,123,124,125,126,127,129,130,131,132,133,
   134,135,136,137,138,139,140,141,142,143,144,145,147,148,149,150,
   151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,
   167,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,
   182,182,183,184,185,186,187,188,189,190,191,191,192,193,194,195,
   196,197,198,198,199,200,201,202,203,203,204,205,206,207,208,208,
   209,210,211,212,212,213,214,215,216,216,217,218,219,220,220,221,
   222,223,223,224,225,226,226,227,228,229,229,230,231,232,232,233,
   234,235,235,236,237,237,238,239,239,240,241,242,242,243,244,244,
   245,246,246,247,248,248,249,250,250,251,252,252,253,254,254,255,
   256 };

const struct Proportion proportions[] = {
    {-256, 11585},
    {-255, 11563},
    {-255, 11540},
    {-254, 11518},
    {-253, 11495},
    {-253, 11473},
    {-252, 11450},
    {-251, 11428},
    {-251, 11406},
    {-250, 11383},
    {-250, 11361},
    {-249, 11339},
    {-248, 11317},
    {-248, 11295},
    {-247, 11273},
    {-246, 11251},
    {-245, 11229},
    {-245, 11207},
    {-244, 11185},
    {-243, 11164},
    {-243, 11142},
    {-242, 11120},
    {-241, 11099},
    {-241, 11077},
    {-240, 11056},
    {-239, 11034},
    {-239, 11013},
    {-238, 10991},
    {-237, 10970},
    {-236, 10949},
    {-236, 10928},
    {-235, 10906},
    {-234, 10885},
    {-234, 10864},
    {-233, 10843},
    {-232, 10822},
    {-231, 10801},
    {-231, 10781},
    {-230, 10760},
    {-229, 10739},
    {-228, 10718},
    {-228, 10698},
    {-227, 10677},
    {-226, 10657},
    {-225, 10636},
    {-225, 10616},
    {-224, 10596},
    {-223, 10575},
    {-222, 10555},
    {-222, 10535},
    {-221, 10515},
    {-220, 10495},
    {-219, 10475},
    {-219, 10455},
    {-218, 10435},
    {-217, 10415},
    {-216, 10396},
    {-215, 10376},
    {-215, 10356},
    {-214, 10337},
    {-213, 10317},
    {-212, 10298},
    {-211, 10279},
    {-211, 10259},
    {-210, 10240},
    {-209, 10221},
    {-208, 10202},
    {-207, 10183},
    {-206, 10164},
    {-206, 10145},
    {-205, 10126},
    {-204, 10107},
    {-203, 10088},
    {-202, 10070},
    {-201, 10051},
    {-201, 10033},
    {-200, 10014},
    {-199, 9996},
    {-198, 9978},
    {-197, 9959},
    {-196, 9941},
    {-195, 9923},
    {-195, 9905},
    {-194, 9887},
    {-193, 9869},
    {-192, 9851},
    {-191, 9834},
    {-190, 9816},
    {-189, 9798},
    {-188, 9781},
    {-188, 9764},
    {-187, 9746},
    {-186, 9729},
    {-185, 9712},
    {-184, 9694},
    {-183, 9677},
    {-182, 9660},
    {-181, 9643},
    {-180, 9627},
    {-179, 9610},
    {-178, 9593},
    {-177, 9577},
    {-177, 9560},
    {-176, 9544},
    {-175, 9527},
    {-174, 9511},
    {-173, 9495},
    {-172, 9479},
    {-171, 9462},
    {-170, 9447},
    {-169, 9431},
    {-168, 9415},
    {-167, 9399},
    {-166, 9383},
    {-165, 9368},
    {-164, 9352},
    {-163, 9337},
    {-162, 9322},
    {-161, 9306},
    {-160, 9291},
    {-159, 9276},
    {-158, 9261},
    {-157, 9246},
    {-156, 9232},
    {-155, 9217},
    {-154, 9202},
    {-153, 9188},
    {-152, 9173},
    {-151, 9159},
    {-150, 9145},
    {-149, 9130},
    {-148, 9116},
    {-147, 9102},
    {-146, 9089},
    {-145, 9075},
    {-144, 9061},
    {-143, 9047},
    {-142, 9034},
    {-141, 9020},
    {-140, 9007},
    {-139, 8994},
    {-138, 8981},
    {-137, 8968},
    {-135, 8955},
    {-134, 8942},
    {-133, 8929},
    {-132, 8916},
    {-131, 8904},
    {-130, 8891},
    {-129, 8879},
    {-128, 8866},
    {-127, 8854},
    {-126, 8842},
    {-125, 8830},
    {-124, 8818},
    {-122, 8807},
    {-121, 8795},
    {-120, 8783},
    {-119, 8772},
    {-118, 8760},
    {-117, 8749},
    {-116, 8738},
    {-115, 8727},
    {-114, 8716},
    {-112, 8705},
    {-111, 8694},
    {-110, 8684},
    {-109, 8673},
    {-108, 8662},
    {-107, 8652},
    {-106, 8642},
    {-104, 8632},
    {-103, 8622},
    {-102, 8612},
    {-101, 8602},
    {-100, 8592},
    {-99, 8583},
    {-98, 8573},
    {-96, 8564},
    {-95, 8555},
    {-94, 8545},
    {-93, 8536},
    {-92, 8527},
    {-91, 8519},
    {-89, 8510},
    {-88, 8501},
    {-87, 8493},
    {-86, 8484},
    {-85, 8476},
    {-83, 8468},
    {-82, 8460},
    {-81, 8452},
    {-80, 8444},
    {-79, 8436},
    {-77, 8429},
    {-76, 8421},
    {-75, 8414},
    {-74, 8407},
    {-73, 8400},
    {-71, 8393},
    {-70, 8386},
    {-69, 8379},
    {-68, 8372},
    {-67, 8366},
    {-65, 8359},
    {-64, 8353},
    {-63, 8347},
    {-62, 8341},
    {-60, 8335},
    {-59, 8329},
    {-58, 8323},
    {-57, 8318},
    {-55, 8312},
    {-54, 8307},
    {-53, 8302},
    {-52, 8296},
    {-51, 8291},
    {-49, 8287},
    {-48, 8282},
    {-47, 8277},
    {-46, 8273},
    {-44, 8268},
    {-43, 8264},
    {-42, 8260},
    {-41, 8256},
    {-39, 8252},
    {-38, 8248},
    {-37, 8244},
    {-36, 8241},
    {-34, 8237},
    {-33, 8234},
    {-32, 8231},
    {-30, 8228},
    {-29, 8225},
    {-28, 8222},
    {-27, 8220},
    {-25, 8217},
    {-24, 8215},
    {-23, 8212},
    {-22, 8210},
    {-20, 8208},
    {-19, 8206},
    {-18, 8204},
    {-17, 8203},
    {-15, 8201},
    {-14, 8200},
    {-13, 8198},
    {-11, 8197},
    {-10, 8196},
    { -9, 8195},
    { -8, 8194},
    { -6, 8194},
    { -5, 8193},
    { -4, 8193},
    { -3, 8192},
    { -1, 8192},
    {  0, 8192},
    {  1, 8192},
    {  3, 8192},
    {  4, 8193},
    {  5, 8193},
    {  6, 8194},
    {  8, 8194},
    {  9, 8195},
    { 10, 8196},
    { 11, 8197},
    { 13, 8198},
    { 14, 8200},
    { 15, 8201},
    { 17, 8203},
    { 18, 8204},
    { 19, 8206},
    { 20, 8208},
    { 22, 8210},
    { 23, 8212},
    { 24, 8215},
    { 25, 8217},
    { 27, 8220},
    { 28, 8222},
    { 29, 8225},
    { 30, 8228},
    { 32, 8231},
    { 33, 8234},
    { 34, 8237},
    { 36, 8241},
    { 37, 8244},
    { 38, 8248},
    { 39, 8252},
    { 41, 8256},
    { 42, 8260},
    { 43, 8264},
    { 44, 8268},
    { 46, 8273},
    { 47, 8277},
    { 48, 8282},
    { 49, 8287},
    { 51, 8291},
    { 52, 8296},
    { 53, 8302},
    { 54, 8307},
    { 55, 8312},
    { 57, 8318},
    { 58, 8323},
    { 59, 8329},
    { 60, 8335},
    { 62, 8341},
    { 63, 8347},
    { 64, 8353},
    { 65, 8359},
    { 67, 8366},
    { 68, 8372},
    { 69, 8379},
    { 70, 8386},
    { 71, 8393},
    { 73, 8400},
    { 74, 8407},
    { 75, 8414},
    { 76, 8421},
    { 77, 8429},
    { 79, 8436},
    { 80, 8444},
    { 81, 8452},
    { 82, 8460},
    { 83, 8468},
    { 85, 8476},
    { 86, 8484},
    { 87, 8493},
    { 88, 8501},
    { 89, 8510},
    { 91, 8519},
    { 92, 8527},
    { 93, 8536},
    { 94, 8545},
    { 95, 8555},
    { 96, 8564},
    { 98, 8573},
    { 99, 8583},
    {100, 8592},
    {101, 8602},
    {102, 8612},
    {103, 8622},
    {104, 8632},
    {106, 8642},
    {107, 8652},
    {108, 8662},
    {109, 8673},
    {110, 8684},
    {111, 8694},
    {112, 8705},
    {114, 8716},
    {115, 8727},
    {116, 8738},
    {117, 8749},
    {118, 8760},
    {119, 8772},
    {120, 8783},
    {121, 8795},
    {122, 8807},
    {124, 8818},
    {125, 8830},
    {126, 8842},
    {127, 8854},
    {128, 8866},
    {129, 8879},
    {130, 8891},
    {131, 8904},
    {132, 8916},
    {133, 8929},
    {134, 8942},
    {135, 8955},
    {137, 8968},
    {138, 8981},
    {139, 8994},
    {140, 9007},
    {141, 9020},
    {142, 9034},
    {143, 9047},
    {144, 9061},
    {145, 9075},
    {146, 9089},
    {147, 9102},
    {148, 9116},
    {149, 9130},
    {150, 9145},
    {151, 9159},
    {152, 9173},
    {153, 9188},
    {154, 9202},
    {155, 9217},
    {156, 9232},
    {157, 9246},
    {158, 9261},
    {159, 9276},
    {160, 9291},
    {161, 9306},
    {162, 9322},
    {163, 9337},
    {164, 9352},
    {165, 9368},
    {166, 9383},
    {167, 9399},
    {168, 9415},
    {169, 9431},
    {170, 9447},
    {171, 9462},
    {172, 9479},
    {173, 9495},
    {174, 9511},
    {175, 9527},
    {176, 9544},
    {177, 9560},
    {177, 9577},
    {178, 9593},
    {179, 9610},
    {180, 9627},
    {181, 9643},
    {182, 9660},
    {183, 9677},
    {184, 9694},
    {185, 9712},
    {186, 9729},
    {187, 9746},
    {188, 9764},
    {188, 9781},
    {189, 9798},
    {190, 9816},
    {191, 9834},
    {192, 9851},
    {193, 9869},
    {194, 9887},
    {195, 9905},
    {195, 9923},
    {196, 9941},
    {197, 9959},
    {198, 9978},
    {199, 9996},
    {200, 10014},
    {201, 10033},
    {201, 10051},
    {202, 10070},
    {203, 10088},
    {204, 10107},
    {205, 10126},
    {206, 10145},
    {206, 10164},
    {207, 10183},
    {208, 10202},
    {209, 10221},
    {210, 10240},
    {211, 10259},
    {211, 10279},
    {212, 10298},
    {213, 10317},
    {214, 10337},
    {215, 10356},
    {215, 10376},
    {216, 10396},
    {217, 10415},
    {218, 10435},
    {219, 10455},
    {219, 10475},
    {220, 10495},
    {221, 10515},
    {222, 10535},
    {222, 10555},
    {223, 10575},
    {224, 10596},
    {225, 10616},
    {225, 10636},
    {226, 10657},
    {227, 10677},
    {228, 10698},
    {228, 10718},
    {229, 10739},
    {230, 10760},
    {231, 10781},
    {231, 10801},
    {232, 10822},
    {233, 10843},
    {234, 10864},
    {234, 10885},
    {235, 10906},
    {236, 10928},
    {236, 10949},
    {237, 10970},
    {238, 10991},
    {239, 11013},
    {239, 11034},
    {240, 11056},
    {241, 11077},
    {241, 11099},
    {242, 11120},
    {243, 11142},
    {243, 11164},
    {244, 11185},
    {245, 11207},
    {245, 11229},
    {246, 11251},
    {247, 11273},
    {248, 11295},
    {248, 11317},
    {249, 11339},
    {250, 11361},
    {250, 11383},
    {251, 11406},
    {251, 11428},
    {252, 11450},
    {253, 11473},
    {253, 11495},
    {254, 11518},
    {255, 11540},
    {255, 11563},
    {256, 11585},
};
/******************************************************************************/
DLLIMPORT int _DK_lbCosTable[2048];
#define lbCosTable _DK_lbCosTable
DLLIMPORT int _DK_lbSinTable[2048];
#define lbSinTable _DK_lbSinTable
/******************************************************************************/
/**
 * Gives sinus of given angle.
 * @param x Angle as integer with reference to LbFPMath_PI.
 * @return Value ranged -65536 to 65536.
 */
long LbSinL(long x)
{
    return lbSinTable[(unsigned long)x & LbFPMath_AngleMask];
}

/**
 * Gives cosinus of given angle.
 * @param x Angle as integer with reference to LbFPMath_PI.
 * @return Value ranged -65536 to 65536.
 */
long LbCosL(long x)
{
    return lbCosTable[(unsigned long)x & LbFPMath_AngleMask];
}

long LbArcTanL(long arg)
{
    if (arg < 0)
    {
        if (arg <= -sizeof(lbArcTanFactors)/sizeof(lbArcTanFactors[0]))
            arg = -sizeof(lbArcTanFactors)/sizeof(lbArcTanFactors[0]) + 1;
        return -(long)lbArcTanFactors[-arg];
    } else
    {
        if (arg >= sizeof(lbArcTanFactors)/sizeof(lbArcTanFactors[0]))
            arg = sizeof(lbArcTanFactors)/sizeof(lbArcTanFactors[0]) - 1;
        return (long)lbArcTanFactors[arg];
    }
}

/** Computes angle between negative Y axis and the line that crosses (0,0) and given (x,y).
 *  Uses arctan(x/y) with proper shift to get the angle.
 *  Returning 0 means direction towards negative y; 512 is towards positive x;
 *  1024 towards positive y, and 1536 towards negative x. Output range is between 0 (=0 rad)
 *  and 2048 (=2*pi rad), zero included.
 *  Value of the angle is properly rounded, up or down.
 *
 * @param x
 * @param y
 * @return
 */
long LbArcTanAngle(long x,long y)
{
    long ux;
    long uy;
    unsigned long index;
    if ((x == 0) && (y == 0))
        return 0;
    if (x < 0)
    {
        ux = -x;
        if (y < 0)
        {
            uy = -y;
            // Make sure we'll have smaller value * 256 / greater value.
            // This way we won't exceed factors array bounds (which is 256 elements).
            if (ux < uy) {
                index = (ux << 8)/uy;
                return 2*LbFPMath_PI   - (long)lbArcTanFactors[index];
            } else {
                index = (uy << 8)/ux;
                return 3*LbFPMath_PI/2 + (long)lbArcTanFactors[index];
            }
        } else
        {
            uy = y;
            // Make sure we'll have smaller value * 256 / greater value.
            if (ux < uy) {
                index = (ux << 8)/uy;
                return   LbFPMath_PI   + (long)lbArcTanFactors[index];
            } else {
                index = (uy << 8)/ux;
                return 3*LbFPMath_PI/2 - (long)lbArcTanFactors[index];
            }
        }
    } else
    {
        ux = x;
        if (y < 0)
        {
            uy = -y;
            // Make sure we'll have smaller value * 256 / greater value.
            if (ux < uy) {
                index = (ux << 8)/uy;
                return                 (long)lbArcTanFactors[index];
            } else {
                index = (uy << 8)/ux;
                return LbFPMath_PI/2 - (long)lbArcTanFactors[index];
            }
        } else
        {
            uy = y;
            // Make sure we'll have smaller value * 256 / greater value.
            if (ux < uy) {
                index = (ux << 8)/uy;
                return LbFPMath_PI   - (long)lbArcTanFactors[index];
            } else {
                index = (uy << 8)/ux;
                return LbFPMath_PI/2 + (long)lbArcTanFactors[index];
            }
        }
    }
}

long LbSqrL(long x)
{
  if (x <= 0)
    return 0;
  //
  long y;
  asm("bsrl     %1, %%eax;\n"
      "movl %%eax, %0;\n"
      : "=r"(y) // output
      : "r"(x)  // input
      : "%eax"  // clobbered register
  );
  y = lbSqrTable[y];
  while ((x/y) < y)
    y = ((x/y) + y) >> 1;
  return y;
}

long LbMathOperation(unsigned char opkind, long val1, long val2)
{
  switch (opkind)
  {
    case MOp_EQUAL:
      return val1 == val2;
    case MOp_NOT_EQUAL:
      return val1 != val2;
    case MOp_SMALLER:
      return val1 < val2;
    case MOp_GREATER:
      return val1 > val2;
    case MOp_SMALLER_EQ:
      return val1 <= val2;
    case MOp_GREATER_EQ:
      return val1 >= val2;
    case MOp_LOGIC_AND:
      return val1 && val2;
    case MOp_LOGIC_OR:
      return val1 || val2;
    case MOp_LOGIC_XOR:
      return (val1!=0) ^ (val2!=0);
    case MOp_BITWS_AND:
      return val1 & val2;
    case MOp_BITWS_OR:
      return val1 | val2;
    case MOp_BITWS_XOR:
      return val1 ^ val2;
    case MOp_SUM:
      return val1 + val2;
    case MOp_SUBTRACT:
      return val1 - val2;
    case MOp_MULTIPLY:
      return val1 * val2;
    case MOp_DIVIDE:
      return val1 / val2;
    case MOp_MODULO:
      return val1 % val2;
    default:
      return val1;
  }
}

unsigned long LbRandomSeries(unsigned long range, unsigned long *seed, const char *func_name, unsigned long place, const char *tag)
{
  if (range == 0)
    return 0;
  unsigned long i = 9377 * (*seed) + 9439;
  *seed = _lrotr(i, 13);
  i = (*seed) % range;
#ifdef AUTOTESTING
  evm_stat(0, "rnd.%s,fn=%s,range=%ld val=%ld,range=%ld", tag, func_name, range, i, range);
#endif
  return i;
}

TbBool LbNumberSignsSame(long num_a, long num_b)
{
    int sign_a;
    int sign_b;
    if (num_a >= 0)
        sign_a = (num_a != 0);
    else
        sign_a = -1;
    if (num_b >= 0)
        sign_b = (num_b != 0);
    else
        sign_b = -1;
    return (sign_a == sign_b);
}

char LbCompareMultiplications(long mul1a, long mul1b, long mul2a, long mul2b)
{
    long long mul1 = (long long)mul1a * (long long)mul1b;
    long long mul2 = (long long)mul2a * (long long)mul2b;
    if (mul1 > mul2)
        return 1;
    if (mul1 < mul2)
        return -1;
    return 0;
}

/**
 *  Optimized function for computing vector length.
 *  Returns diagonal of a rectangle with sides of given length.
 * @param a Length of one of the sides.
 * @param b Length of second of the sides.
 * @return Root of sum of squares of given lengths, sqrt(a*a + b*b).
 */
long LbDiagonalLength(long a, long b)
{
    int propidx;
    long long tmpval;
    if (a > b) {
        propidx = (b << 8)/a;
        tmpval = a;
    } else
    {
        if (b == 0) {
            return 0; // Just to avoid dividing by 0
        }
        propidx = (a << 8)/b;
        tmpval = b;
    }
    tmpval = tmpval * (long long)proportions[propidx + 256].distance_ratio;
    return (tmpval >> 13);
}

float lerp(float a, float b, float f) 
{
    return (a * (1.0 - f)) + (b * f);
}

/******************************************************************************/
