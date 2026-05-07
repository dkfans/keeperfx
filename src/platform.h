#ifndef PLATFORM_H
#define PLATFORM_H

// Platform-specific stuff is declared here

#ifdef __cplusplus
extern "C" {
#endif

int kfxmain(int argc, char *argv[]);
const char * get_os_version(void);
const void * get_image_base(void);
const char * get_wine_version(void);
const char * get_wine_host(void);

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_H
