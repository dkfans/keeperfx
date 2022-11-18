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

#ifndef _WIN32
#include <cpuid.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/** Issue a single request to CPUID.
 */
static void cpuid(uint32_t code, uint32_t regs[4])
{
#ifdef _MSC_VER
    __cpuid((int*)regs, (int)code);
#else
    __get_cpuid(code, &regs[0], &regs[1], &regs[2], &regs[3]);
#endif
}

/** Issue one or more requests to get a string from CPUID and copy
 *  it to the destination buffer. The buffer will be null-terminated.
 */
static void cpuid_string(uint32_t code, char* buffer, size_t len)
{
    if (len > 1)
    {
        // Zero buffer
        memset(buffer, 0, len);

        if (code == CPUID_GETVENDORSTRING)
        {
            uint32_t regs[4];
            cpuid(code, regs);
            char reordered[12];
            memcpy(&reordered[0], &regs[1], sizeof(uint32_t));
            memcpy(&reordered[4], &regs[3], sizeof(uint32_t));
            memcpy(&reordered[8], &regs[2], sizeof(uint32_t));
            memcpy(buffer, reordered, min(sizeof(reordered), len));
        }
        else if (code == CPUID_INTELBRANDSTRING)
        {
            uint32_t regs[16];
            cpuid(CPUID_INTELBRANDSTRING, &regs[0 * 4]);
            cpuid(CPUID_INTELBRANDSTRINGMORE, &regs[1 * 4]);
            cpuid(CPUID_INTELBRANDSTRINGEND, &regs[2 * 4]);
            memcpy(buffer, regs, min(sizeof(regs), len));
        }
    }

    // Ensure null terminator
    if (len > 0)
        buffer[len - 1] = '\0';
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
  {
    cpuid_string(CPUID_GETVENDORSTRING, cpu->vendor, sizeof(cpu->vendor));

    uint32_t where[4];
    cpuid(CPUID_GETFEATURES, where);
    cpu->feature_intl = where[0];
    cpu->feature_edx = where[3];
    if (cpu_get_family(cpu) >= 5)
    {
      if (cpu->feature_edx & CPUID_FEAT_EDX_TSC)
        cpu->timeStampCounter = 1;
    }

    cpuid(CPUID_INTELEXTENDED, where);
    cpu->BrandString = (where[0] >= CPUID_INTELBRANDSTRING);
    if (cpu->BrandString)
    {
      cpuid_string(CPUID_INTELBRANDSTRING, cpu->brand, sizeof(cpu->brand));
    }
  }
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
