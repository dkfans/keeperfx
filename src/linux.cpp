#include "platform.h"
#include "steam_api.hpp"
#include "bflib_crash.h"
#include "bflib_fileio.h"
#include "cdrom.h"
#include <algorithm>
#include <ctype.h>
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fnmatch.h>

extern "C" void install_exception_handler()
{
	LbErrorParachuteInstall();
}

extern "C" int steam_api_init()
{
    // Steam not supported on Linux
    return 0;
}

extern "C" void steam_api_shutdown()
{
    // Steam not supported on Linux
}

extern "C" void SetRedbookVolume(SoundVolume)
{
    // TODO: implement CDROM features
}

extern "C" TbBool PlayRedbookTrack(int)
{
    // TODO: implement CDROM features
    return false;
}

extern "C" void PauseRedbookTrack()
{
    // TODO: implement CDROM features
}

extern "C" void ResumeRedbookTrack()
{
    // TODO: implement CDROM features
}

extern "C" void StopRedbookTrack()
{
    // TODO: implement CDROM features
}

extern "C" int main(int argc, char *argv[]) {
	return kfxmain(argc, argv);
}
