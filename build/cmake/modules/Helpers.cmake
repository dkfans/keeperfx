# Helpers.cmake - shared functions for the desktop (Windows/MinGW + Linux) build.

function(kfx_status PREFIX MESSAGE)
    message(STATUS "[${PREFIX}] ${MESSAGE}")
endfunction()

# Warning + optimisation flags, per platform (mirrors the hand Makefiles).
function(apply_keeperfx_warnings TARGET)
    if(MSVC)
        # Real cl.exe (Debug-MSVC preset) - a local-convenience build, not the
        # fork's canonical toolchain (that's GCC/MinGW-w64). The GNU-style
        # flags below aren't understood by cl.exe's native driver - e.g. a
        # bare "-W" is parsed as cl's own /W switch, which requires a number
        # ("cl : Command line error D8004: '/W' requires an argument"), and
        # -march/-fno-omit-frame-pointer/-fmessage-length have no cl.exe
        # equivalent. Skip them; CMAKE_BUILD_TYPE already supplies sane
        # /O* + /Zi flags. (Note: MSVC here means real cl.exe only - clang-cl
        # reports CMAKE_<LANG>_COMPILER_ID "Clang", so MSVC is false for it
        # and it still takes the WIN32 branch below, which it understands.)
    elseif(WIN32)
        target_compile_options(${TARGET} PRIVATE
            -Wall -W -Wshadow -Wno-sign-compare -Wno-unused-parameter
            -Wno-maybe-uninitialized -Wno-strict-aliasing -Wno-unknown-pragmas
            -Werror -Wno-format-truncation
            -march=x86-64 -fno-omit-frame-pointer -fmessage-length=0 -O3
            $<$<COMPILE_LANGUAGE:C>:-Wimplicit>)
    else()
        target_compile_options(${TARGET} PRIVATE
            -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unknown-pragmas
            -Wno-format-truncation -Wno-sign-compare
            -g -O3 -march=x86-64
            $<$<COMPILE_LANGUAGE:C>:-Wno-absolute-value>)
    endif()
endfunction()

# Link flags, per platform.
function(apply_keeperfx_link_flags TARGET)
    if(MSVC)
        # See apply_keeperfx_warnings: -mwindows/-Wl,.../-static are GNU
        # linker syntax, meaningless to link.exe. Let CMake's own MSVC
        # defaults link normally rather than passing them through unchanged.
    elseif(WIN32)
        # -static-libgcc/-static-libstdc++ are driver flags (mirrors the
        # Makefile's LINKFLAGS): they make g++ statically link its own
        # automatically-appended runtime refs. Without them the exe depends on
        # shared libgcc_s_dw2-1.dll/libstdc++-6.dll at runtime even though the
        # "-static stdc++ ... -dynamic" below (a linker -Bstatic/-Bdynamic
        # toggle for explicitly-listed libs) looks like it should cover it -
        # that toggle is back to -dynamic by the time g++ appends its own
        # implicit libs, so it doesn't apply to them.
        target_link_options(${TARGET} PRIVATE
            -mwindows -static-libgcc -static-libstdc++
            -Wl,--enable-auto-import -Wl,-Map,${TARGET}.map)
        target_link_libraries(${TARGET} PUBLIC -static stdc++ winpthread -dynamic)
    else()
        target_link_options(${TARGET} PRIVATE -g -rdynamic)
    endif()
endfunction()

# Windows system libraries (matches the Makefile LINKLIB trailer). No-op elsewhere.
function(apply_windows_system_libs TARGET)
    if(WIN32)
        target_link_libraries(${TARGET} PRIVATE
            winmm imagehlp ws2_32 dbghelp bcrypt ole32 uuid dxgi dwmapi)
        if(NOT MSVC)
            # mingw32 (CRT startup wrapper import lib) only exists for MinGW/clang-cl.
            target_link_libraries(${TARGET} PRIVATE mingw32)
        endif()
    endif()
endfunction()
