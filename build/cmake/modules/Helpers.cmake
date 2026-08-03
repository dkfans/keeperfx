# Helpers.cmake - shared functions for the desktop (Windows/MinGW + Linux) build.

function(kfx_status PREFIX MESSAGE)
    message(STATUS "[${PREFIX}] ${MESSAGE}")
endfunction()

# Warning + optimisation flags, per platform (mirrors the hand Makefiles).
function(apply_keeperfx_warnings TARGET)
    if(WIN32)
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
    if(WIN32)
        target_link_options(${TARGET} PRIVATE
            -mwindows -Wl,--enable-auto-import -Wl,-Map,${TARGET}.map)
        target_link_libraries(${TARGET} PUBLIC -static stdc++ winpthread -dynamic)
    else()
        target_link_options(${TARGET} PRIVATE -g -rdynamic)
    endif()
endfunction()

# Windows system libraries (matches the Makefile LINKLIB trailer). No-op elsewhere.
function(apply_windows_system_libs TARGET)
    if(WIN32)
        target_link_libraries(${TARGET} PRIVATE
            winmm mingw32 imagehlp ws2_32 dbghelp bcrypt ole32 uuid)
    endif()
endfunction()
