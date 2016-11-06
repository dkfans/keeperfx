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
#include "bflib_cpu.h"

#include "bflib_basics.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif // _MSC_VER

/******************************************************************************/

/** Issue a single request to CPUID.
 *  Fits 'intel features', for instance note that even if only "eax" and "edx"
 *  are of interest, other registers will be modified by the operation,
 *  so we need to tell the compiler about it.
 */
static inline void cpuid(int code, unsigned long *a, unsigned long *d) {
#ifdef _MSC_VER
  unsigned long where[4];
  __cpuid(where, code);
  *a = where[0];
  *d = where[3];
#else
  asm volatile("cpuid":"=a"(*a),"=d"(*d):"0"(code):"ecx","ebx");
#endif // _MSC_VER
}

/** Issue a complete request, storing general registers output in an array.
 */
static inline void cpuid_string(int code, unsigned long where[4]) {
#ifdef _MSC_VER
  __cpuid(where, code);
#else
  asm volatile("cpuid":"=a"(*where),"=b"(*(where+1)),
               "=c"(*(where+2)),"=d"(*(where+3)):"0"(code));
#endif // _MSC_VER
}
/******************************************************************************/

void cpu_detect(struct CPU_INFO *cpu)
{
  static char const anonvendor[] = "AnonymousCPU";
  unsigned long where[4];
  unsigned long cpuflags;
  // Fill with defaults
  strncpy(cpu->vendor,anonvendor,sizeof(cpu->vendor));
  cpu->timeStampCounter = 0;
    cpu->feature_intl = 0;
    cpu->feature_edx = 0;
#ifdef _MSC_VER
    // Very unlikely cpuid is not supported, the special intrinsic functions should be
    // used anyway for determining processor capabilities
    cpuflags = 0x200000;
#else
    // Get the CPU flags
    asm volatile (
        /* See if CPUID instruction is supported ... */
        /* ... Get copies of EFLAGS into eax and ecx */
        "pushf\n\t"
        "popl %%eax\n\t"
        "movl %%eax, %%ecx\n\t"

        /* ... Toggle the ID bit in one copy and store */
        /*     to the EFLAGS reg */
        "xorl $0x200000, %%eax\n\t"
        "push %%eax\n\t"
        "popf\n\t"

        /* ... Get the (hopefully modified) EFLAGS */
        "pushf\n\t"
        "popl %%eax\n\t"

        /* ... Compare and test result */
        "xorl %%eax, %%ecx\n\t"
        "movl %%ecx, %0\n\t"
        : "=r" (cpuflags)
    );
#endif
  /* if CPUID is supported */
  if ((cpuflags & 0x200000) != 0)
  {
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
  }
}

unsigned short cpu_get_type(struct CPU_INFO *cpu)
{
  if (cpu->feature_intl != 0)
    return (cpu->feature_intl>>12) & 0x3;
  else
    return CPUID_TYPE_OEM;
}

unsigned short cpu_get_family(struct CPU_INFO *cpu)
{
  if (cpu->feature_intl != 0)
    return (cpu->feature_intl>>8) & 0xF;
  else
    return CPUID_FAMILY_486;
}

unsigned short cpu_get_model(struct CPU_INFO *cpu)
{
  return (cpu->feature_intl>>4) & 0xF;
}

unsigned short cpu_get_stepping(struct CPU_INFO *cpu)
{
  return (cpu->feature_intl) & 0x7;
}

/******************************************************************************/
