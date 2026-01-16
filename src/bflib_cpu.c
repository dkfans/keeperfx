/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_cpu.c
 *     CPU detection utility.
 * @par Purpose:
 *     Allows to detect type and available features of the CPU installed.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Aug 2009 - 24 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_cpu.h"

#include "bflib_basics.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/** Issue a single request to CPUID.
 *  Fits 'intel features', for instance note that even if only "eax" and "edx"
 *  are of interest, other registers will be modified by the operation,
 *  so we need to tell the compiler about it.
 */
static inline void cpuid(int code, uint32_t *a, uint32_t *d) {
  #if defined(__i386__) || defined(__x86_64__)
    asm volatile("cpuid":"=a"(*a),"=d"(*d):"0"(code):"ecx","ebx");
  #endif
}

/** Issue a complete request, storing general registers output in an array.
 */
static inline void cpuid_string(int code, void * destination) {
  #if defined(__i386__) || defined(__x86_64__)
  uint32_t * where = (uint32_t *) destination;
  asm volatile("cpuid":"=a"(*where),"=b"(*(where+1)),
      "=c"(*(where+2)),"=d"(*(where+3)):"0"(code));
  #endif
}
/******************************************************************************/

void cpu_detect(struct CPU_INFO *cpu)
{
  static char const anonvendor[] = "AnonymousCPU";
  // Fill with defaults
  snprintf(cpu->vendor, sizeof(cpu->vendor), "%s", anonvendor);
  cpu->timeStampCounter = 0;
  cpu->feature_intl = 0;
  cpu->feature_edx = 0;
  #if defined(__i386__) || defined(__x86_64__)
  {
    uint32_t where[4];
    cpuid_string(CPUID_GETVENDORSTRING, where);
    memcpy(&cpu->vendor[0],&where[1],4);
    memcpy(&cpu->vendor[4],&where[3],4);
    memcpy(&cpu->vendor[8],&where[2],4);
    cpu->vendor[12] = '\0';
    cpuid(CPUID_GETFEATURES, &where[0], &where[3]);
    cpu->feature_intl = where[0];
    cpu->feature_edx = where[3];
    if (cpu_get_family(cpu) >= 5)
    {
      if (cpu->feature_edx & CPUID_FEAT_EDX_TSC)
        cpu->timeStampCounter = 1;
    }
    cpuid(CPUID_INTELEXTENDED, &where[0], &where[3]);
    cpu->BrandString = (where[0] >= CPUID_INTELBRANDSTRING);
    if (cpu->BrandString)
    {
        cpuid_string(CPUID_INTELBRANDSTRING, &cpu->brand[0]);
        cpuid_string(CPUID_INTELBRANDSTRINGMORE, &cpu->brand[16]);
        cpuid_string(CPUID_INTELBRANDSTRINGEND, &cpu->brand[32]);
    }
  }
  #endif
}

unsigned char cpu_get_type(struct CPU_INFO *cpu)
{
  if (cpu->feature_intl != 0)
    return (cpu->feature_intl>>12) & 0x3;
  else
    return CPUID_TYPE_OEM;
}

unsigned char cpu_get_family(struct CPU_INFO *cpu)
{
  if (cpu->feature_intl != 0)
  {
    unsigned char family = (cpu->feature_intl>>8) & 0xF;
    if (family == 15)
    {
        return ((cpu->feature_intl>>20) & 0xFF) + family;
    }
    else
    {
        return family;
    }
  }
  else
  {
    return CPUID_FAMILY_486;
  }
}

unsigned char cpu_get_model(struct CPU_INFO *cpu)
{
    unsigned char family = cpu_get_family(cpu);
    unsigned char model = ((cpu->feature_intl>>4) & 0xF);
    if ( (family == 6) || (family == 15) )
    {
        return (((cpu->feature_intl>>16 & 0xF)) << 4) + model;
    }
    else
    {
        return model;
    }
}

unsigned char cpu_get_stepping(struct CPU_INFO *cpu)
{
  return (cpu->feature_intl) & 0xF;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
