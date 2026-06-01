# MSVC Toolchain Configuration
# Configures CMake to use Microsoft Visual C++ (cl.exe) for Windows native builds.
# Triplets: x86-windows-static, x64-windows-static

set(CMAKE_SYSTEM_NAME Windows)

# Compiler Configuration
set(CMAKE_C_COMPILER cl.exe CACHE STRING "C compiler" FORCE)
set(CMAKE_CXX_COMPILER cl.exe CACHE STRING "C++ compiler" FORCE)
set(CMAKE_RC_COMPILER rc.exe CACHE STRING "Resource compiler" FORCE)

# Force MSVC detection
set(MSVC TRUE CACHE BOOL "Force MSVC detection" FORCE)
set(CMAKE_CXX_COMPILER_ID "MSVC" CACHE STRING "CXX compiler ID" FORCE)
set(CMAKE_C_COMPILER_ID "MSVC" CACHE STRING "C compiler ID" FORCE)

# Disable compiler checks (assumes MSVC environment already set up)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# Language Configuration
set(CMAKE_C_STANDARD 11 CACHE STRING "C standard")
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")

# Compiler Flags
# /J: unsigned char by default (GCC compatibility); matches MinGW behavior
# /fp:precise: Floating point model (default, but explicit)
# /W4: Warning level 4 (most rigorous)
# /WX: Treat warnings as errors (matches MinGW -Werror behavior, can be relaxed)

# /std:clatest: Enable latest C standard preview (C23) — required for typeof keyword
# used in config.h field() macro (offsetof(typeof(expr), member) is a C23 pattern).
set(MSVC_COMMON_FLAGS "/J /fp:precise /W4 /std:clatest")

# For Release: /O2 (optimize for speed), /Gy (function-level linking)
# For Debug: /Od (disable optimizations), /Zi (generate program database)
string(APPEND CMAKE_C_FLAGS "${MSVC_COMMON_FLAGS}")
string(APPEND CMAKE_CXX_FLAGS "${MSVC_COMMON_FLAGS}")

string(APPEND CMAKE_C_FLAGS_RELEASE " /O2 /Gy")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " /O2 /Gy")

string(APPEND CMAKE_C_FLAGS_DEBUG " /Od /Zi /RTC1")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " /Od /Zi /RTC1")

# C4996: 'function': was declared deprecated
#   MSVC often warns about POSIX functions like fopen (prefer fopen_s).
#   For compatibility with cross-platform code (GCC doesn't warn), suppress this.
# C4267: 'var': conversion from 'size_t' to 'type', possible loss of data
#   Intentionally suppressed for cross-compilation compatibility.
string(APPEND CMAKE_C_FLAGS " /wd4996 /wd4267")
string(APPEND CMAKE_CXX_FLAGS " /wd4996 /wd4267")

# Runtime Library Linking
# /MT: Multithreaded static runtime (Release)
# /MD: Multithreaded dynamic runtime (Debug with DLL)
# CMake handles most of this automatically via CMAKE_MSVC_RUNTIME_LIBRARY,
# but we can explicitly set it for consistency.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Linker Configuration
# /SUBSYSTEM:CONSOLE (default for executables)
# /OPT:REF (remove unreferenced functions)
string(APPEND CMAKE_EXE_LINKER_FLAGS " /OPT:REF")
string(APPEND CMAKE_SHARED_LINKER_FLAGS " /OPT:REF")

# vcpkg triplet configuration (auto-set by CMakePresets.json, but documented here)
# When using this toolchain with vcpkg, ensure CMakePresets.json sets:
#   "cacheVariables": {
#       "CMAKE_TOOLCHAIN_FILE": "${sourceDir}/vcpkg/scripts/buildsystems/cmake/vcpkg.cmake",
#       "VCPKG_TARGET_TRIPLET": "x86-windows-static" or "x64-windows-static"
#   }

message(STATUS "MSVC Toolchain: cl.exe, C11/C++17, /MT (static runtime), triplet: ${VCPKG_TARGET_TRIPLET}")
