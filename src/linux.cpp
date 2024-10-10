
#include "bflib_fileio.h"
#include "bflib_cpu.h"
#include "bflib_crash.h"
#include "bflib_datetm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fnmatch.h>
#include <string>
#include <cstring>
#include <memory>
#include <ctime>

struct TbFileFind {
	std::string filespec;
	std::string path;
	std::string namebuf;
	DIR * handle = nullptr;
	bool is_pattern = false;
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

bool find_file(TbFileFind * ff, TbFileEntry * fe) {
	while (true) {
		auto de = readdir(ff->handle);
		if (!de) {
			return false;
		} else if (strcmp(de->d_name, ".") == 0) {
			continue;
		} else if (strcmp(de->d_name, "..") == 0) {
			continue;
		}
		const std::string path = ff->path + "/" + de->d_name;
		ff->namebuf = de->d_name;
		fe->Filename = ff->namebuf.c_str();
		if (ff->is_pattern) {
			if (fnmatch(ff->filespec.c_str(), path.c_str(), FNM_FILE_NAME | FNM_CASEFOLD) != 0) {
				continue;
			}
		}
		struct stat sb;
		if (stat(path.c_str(), &sb) < 0) {
			continue;
		} else if (!S_ISREG(sb.st_mode)) {
			continue;
		}
		return true;
	}
	return false;
}

extern "C" TbFileFind * LbFileFindFirst(const char * filespec, TbFileEntry * fe)
{
	try {
		auto ff = std::make_unique<TbFileFind>();
		ff->is_pattern = filespec_is_pattern(filespec);
		ff->filespec = filespec;
		if (ff->is_pattern) {
			ff->path = directory_from_filespec(filespec);
			ff->handle = opendir(ff->path.c_str());
		} else {
			ff->path = filespec;
			ff->handle = opendir(filespec);
		}
		if (ff->handle) {
			if (find_file(ff.get(), fe)) {
				return ff.release();
			}
		}
	} catch (...) {}
	return nullptr;
}

extern "C" int LbFileFindNext(TbFileFind * ff, TbFileEntry * fe)
{
	try {
		if (find_file(ff, fe)) {
			return 1;
		}
	} catch (...) {}
	return -1;
}

extern "C" void LbFileFindEnd(TbFileFind * ff)
{
	if (ff) {
		closedir(ff->handle);
	}
	delete ff;
}

extern "C" void log_system_info(const CPU_INFO *) {
	// TODO
}

extern "C" void get_cmdln_args(unsigned short &argc, char *argv[]) {
	// Nothing to do here
}

extern "C" void platform_init() {
	// Nothing to do here
}

extern "C" void LbErrorParachuteInstall() {
	// Nothing to do here
}

extern "C" void LbErrorParachuteUpdate() {
	// Nothing to do here
}

extern "C" void LbDoMultitasking()
{
	timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = (LARGE_DELAY_TIME>>1) * 1000;
	nanosleep(&ts, nullptr);
}
