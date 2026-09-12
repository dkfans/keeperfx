# Platforms.cmake - platform detection + directory-scope compile definitions.

if(WIN32)
    kfx_status("PLATFORM" "Windows / MinGW-w64 (i686)")
elseif(UNIX AND NOT APPLE)
    kfx_status("PLATFORM" "Linux (x86_64)")
else()
    message(FATAL_ERROR "Unsupported platform (only Windows/MinGW and Linux are supported)")
endif()

add_compile_definitions("DEBUG=$<IF:$<CONFIG:Debug>,1,0>")

# Static-linkage defines for the prebuilt Windows dependencies.
if(WIN32)
    add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
    # AL_LIBTYPE_STATIC is deliberately not defined here: OpenAL is linked
    # dynamically for the CMake build (via vcpkg's openal-soft), unlike the
    # Makefile's static prebuilt - see Dependencies.cmake for why.
    add_compile_definitions(SPNG_STATIC=1)
    # MSVC's <math.h> only defines M_PI/M_E/etc. when this is set before the
    # first include; MinGW's <math.h> defines them unconditionally regardless,
    # so this is a no-op there. Directory-scope since math.h can be pulled in
    # transitively in an unpredictable order across source files.
    add_compile_definitions(_USE_MATH_DEFINES)
endif()
