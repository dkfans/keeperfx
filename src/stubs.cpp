#include "bflib_fmvids.h"
#include "steam_api.hpp"

extern "C" short play_smk_(char *fname, int smkflags, int plyflags) { return 0; }
extern "C" short play_smk_direct(char *fname, int smkflags, int plyflags) { return 0; }
extern "C" short play_smk_via_buffer(char *fname, int smkflags, int plyflags) { return 0; }

extern "C" short anim_open(char *fname, int arg1, short arg2, int width, int height, int arg5, unsigned int flags) { return 0; }
extern "C" short anim_stop(void) { return 0; }
extern "C" short anim_record(void) { return 0; }
extern "C" TbBool anim_record_frame(unsigned char *screenbuf, unsigned char *palette) { return 0; }

extern "C" int steam_api_init() { return 0; }
extern "C" void steam_api_shutdown() {}
