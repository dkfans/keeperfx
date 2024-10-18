#ifndef __PLATFORM_H_
#define __PLATFORM_H_

// Platform-specific stuff is declared here

#include "bflib_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

void platform_init(void);
void log_system_info(const struct CPU_INFO *);
void get_cmdln_args(unsigned short &argc, char *argv[]);

#ifdef __cplusplus
}
#endif
#endif
