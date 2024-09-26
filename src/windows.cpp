#include "pre_inc.h"
#include "bflib_fileio.h"
#include "bflib_datetm.h"
#include "bflib_cpu.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <memory>
#include <string>
#include "post_inc.h"

extern "C" void log_system_info(const CPU_INFO * cpu_info)
{
	OSVERSIONINFO v;
	SYNCMSG("CPU %s type %u family %u model %u stepping %u features %08x", cpu_info->vendor,
		cpu_get_type(cpu_info), cpu_get_family(cpu_info), cpu_get_model(cpu_info),
		cpu_get_stepping(cpu_info), cpu_info->feature_edx);
	if (cpu_info->BrandString)
	{
		SYNCMSG("%s", cpu_info->brand);
	}
	SYNCMSG("Build image base: %p", GetModuleHandle(NULL));
	v.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&v))
	{
		SYNCMSG("Operating System: %s %ld.%ld.%ld", (v.dwPlatformId == VER_PLATFORM_WIN32_NT) ? "Windows NT" : "Windows", v.dwMajorVersion,v.dwMinorVersion,v.dwBuildNumber);
	}

	// Check for Wine
	#ifdef _WIN32
		HMODULE hNTDLL = GetModuleHandle("ntdll.dll");
		if(hNTDLL)
		{
			// Get Wine version
			PROC wine_get_version = (PROC) GetProcAddress(hNTDLL, "wine_get_version");
			if (wine_get_version)
			{
				SYNCMSG("Running on Wine v%s", wine_get_version());
				is_running_under_wine = true;
			}

			// Get Wine host OS
			// We have to use a union to make sure there is no weird cast warnings
			union
			{
				FARPROC func;
				void (*wine_get_host_version)(const char**, const char**);
			} wineHostVersionUnion;
			wineHostVersionUnion.func = GetProcAddress(hNTDLL, "wine_get_host_version");
			if (wineHostVersionUnion.wine_get_host_version)
			{
				const char* sys_name = NULL;
				const char* release_name = NULL;
				wineHostVersionUnion.wine_get_host_version(&sys_name, &release_name);
				SYNCMSG("Wine Host: %s %s", sys_name, release_name);
			}
		}
	#endif
}

extern "C" void get_cmdln_args(unsigned short &argc, char *argv[])
{
	char *ptr;
	const char *cmndln_orig;
	cmndln_orig = GetCommandLineA();
	char cmndline[CMDLN_MAXLEN+1];
	snprintf(cmndline, CMDLN_MAXLEN, "%s", cmndln_orig);
	ptr = cmndline;
	argc = 0;
	while (*ptr != '\0')
	{
		if ((*ptr == '\t') || (*ptr == ' '))
		{
			ptr++;
			continue;
		}
		if (*ptr == '\"')
		{
			ptr++;
			argv[argc] = ptr;
			argc++;
			while (*ptr != '\0')
			{
			  if (*ptr == '\"')
			  {
				  *ptr++ = '\0';
				  break;
			  }
			  ptr++;
			}
		} else
		{
			argv[argc] = ptr;
			argc++;
			while (*ptr != '\0')
			{
			  if ((*ptr == '\t') || (*ptr == ' '))
			  {
				  *ptr++ = '\0';
				  break;
			  }
			  ptr++;
			}
		}
	}
}

const char * exception_name(DWORD exception_code) {
    switch (exception_code) {
        case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
        case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
        case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
    }
    return "Unknown";
}

LONG __stdcall Vex_handler(
	_EXCEPTION_POINTERS *ExceptionInfo
)
{
	const auto exception_code = ExceptionInfo->ExceptionRecord->ExceptionCode;
    if (exception_code == DBG_PRINTEXCEPTION_WIDE_C) {
        return EXCEPTION_CONTINUE_EXECUTION; // Thrown by OutputDebugStringW, intended for debugger
    } else if (exception_code == DBG_PRINTEXCEPTION_C) {
        return EXCEPTION_CONTINUE_EXECUTION; // Thrown by OutputDebugStringA, intended for debugger
    }
    LbJustLog("Exception 0x%08x thrown: %s\n", exception_code, exception_name(exception_code));
    return EXCEPTION_CONTINUE_SEARCH;
}

extern "C" void platform_init(void)
{
	AddVectoredExceptionHandler(0, &Vex_handler);
}

struct TbFileFind {
	HANDLE handle = INVALID_HANDLE_VALUE;
	std::string namebuf;
};

extern "C" TbFileFind * LbFileFindFirst(const char * filespec, TbFileEntry * fentry)
{
	try {
		auto ffind = std::make_unique<TbFileFind>();
		WIN32_FIND_DATA fd;
		ffind->handle = FindFirstFile(filespec, &fd);
		if (ffind->handle == INVALID_HANDLE_VALUE) {
			return nullptr;
		}
		ffind->namebuf = fd.cFileName;
		fentry->Filename = ffind->namebuf.c_str();
		return ffind.release();
	} catch (...) {}
	return nullptr;
}

extern "C" int LbFileFindNext(TbFileFind * ffind, TbFileEntry * fentry)
{
	try {
		if (ffind == NULL) {
			return -1;
		}
		WIN32_FIND_DATA fd;
		if (!FindNextFile(ffind->handle, &fd)) {
			return -1;
		}
		ffind->namebuf = fd.cFileName;
		fentry->Filename = ffind->namebuf.c_str();
		return 1;
	} catch (...) {}
	return -1;
}

extern "C" void LbFileFindEnd(TbFileFind * ffind)
{
	if (ffind) {
		FindClose(ffind->handle);
		delete ffind;
	}
}

extern "C" void LbDoMultitasking()
{
	Sleep(LARGE_DELAY_TIME>>1); // This switches to other tasks
}


extern "C" int main(int, char *[]);

extern "C" int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	return main(0, nullptr);
}
