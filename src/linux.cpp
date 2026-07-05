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
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#include <cstring>
#endif

extern "C" const char * get_os_version() {
    return "Linux";
}

extern "C" const void * get_image_base()
{
    return nullptr;
}

extern "C" const char * get_wine_version()
{
    return nullptr; // we're running native
}

extern "C" const char * get_wine_host()
{
    return nullptr; // we're running native
}

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

struct TbFileFind {
	std::vector<std::pair<std::string, std::string>> names;
	size_t index = 0;
};

bool filespec_is_pattern(const char * filespec) {
	return strchr(filespec, '*') != nullptr;
}

std::string directory_from_filespec(const char * filespec) {
	const auto sep = strrchr(filespec, '/');
	if (sep && sep != filespec) {
		return std::string(filespec, sep - filespec);
	} else {
		return ".";
	}
}

extern "C" TbFileFind * LbFileFindFirst(const char * filespec, TbFileEntry * fe)
{
	try {
		auto ff = std::make_unique<TbFileFind>();
		bool is_pattern = filespec_is_pattern(filespec);
		std::string path;
		if (is_pattern) {
			path = directory_from_filespec(filespec);
		} else {
			path = filespec;
		}
		DIR *handle = opendir(path.c_str());
		if (handle) {
			while (true) {
				auto de = readdir(handle);
				if (!de) {
					break;
				}
				if (strcmp(de->d_name, ".") == 0) {
					continue;
				}
				if (strcmp(de->d_name, "..") == 0) {
					continue;
				}
				const std::string file_path = path + "/" + de->d_name;
				if (is_pattern) {
					if (fnmatch(filespec, file_path.c_str(), FNM_FILE_NAME | FNM_CASEFOLD) != 0) {
						continue;
					}
				}
				struct stat sb;
				if (stat(file_path.c_str(), &sb) < 0) {
					continue;
				}
				if (!S_ISREG(sb.st_mode)) {
					continue;
				}
				std::string key = de->d_name;
				for (size_t i = 0; i < key.size(); i++) {
					key[i] = (char)tolower((unsigned char)key[i]);
				}
				ff->names.emplace_back(key, de->d_name);
			}
			closedir(handle);
		}
		if (!ff->names.empty()) {
			std::sort(ff->names.begin(), ff->names.end());
			fe->Filename = ff->names[0].second.c_str();
			return ff.release();
		}
	} catch (...) {}
	return nullptr;
}

extern "C" int32_t LbFileFindNext(TbFileFind * ff, TbFileEntry * fe)
{
	try {
		if (ff) {
			ff->index++;
			if (ff->index < ff->names.size()) {
				fe->Filename = ff->names[ff->index].second.c_str();
				return 1;
			}
		}
	} catch (...) {}
	return -1;
}

extern "C" void LbFileFindEnd(TbFileFind * ff)
{
	delete ff;
}

#ifdef __APPLE__
// On macOS the engine is the .app's main executable. Finder launches apps with
// the working directory set to "/", while the game data (data/, sound/, fxdata/,
// campgns/, ...) lives in the folder the user dropped the .app into. Locate our
// own executable and, when we are running from inside a <name>.app/Contents/
// MacOS/ bundle, chdir to the folder that contains the .app so those files are
// found. Run outside a bundle (e.g. a developer launching bin/keeperfx from the
// game directory) and the working directory is left untouched.
static void macos_chdir_to_bundle_parent(void)
{
	char exe[PATH_MAX];
	uint32_t size = sizeof(exe);
	if (_NSGetExecutablePath(exe, &size) != 0)
		return;
	char resolved[PATH_MAX];
	if (realpath(exe, resolved) == nullptr)
		return;
	char *marker = strstr(resolved, "/Contents/MacOS/");
	if (marker == nullptr)
		return; // not inside an .app bundle; leave the working directory alone
	*marker = '\0';                       // resolved -> ".../<name>.app"
	char *slash = strrchr(resolved, '/');
	if (slash == nullptr)
		return;
	*slash = '\0';                        // resolved -> folder containing the .app
	if (chdir(resolved) != 0)
		return;
}
#endif

extern "C" int main(int argc, char *argv[]) {
#ifdef __APPLE__
	macos_chdir_to_bundle_parent();
#endif
	return kfxmain(argc, argv);
}
