# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# Helpers.cmake — Common functions and macros
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

# Log a status message with [PREFIX] for clarity
function(kfx_status PREFIX MESSAGE)
    message(STATUS "[${PREFIX}] ${MESSAGE}")
endfunction()

# Apply common compilation flags to a target based on platform
function(apply_keeperfx_warnings TARGET)
    if(MSVC)
        # MSVC: use /W3, suppress common cross-platform noise
        target_compile_options(${TARGET} PRIVATE
            /W3
            /wd4996   # deprecated functions
            /wd4267   # size_t conversion
            /wd4244   # float/int conversion
            /wd4305   # truncation
            /wd4018   # signed/unsigned mismatch
            /wd4146   # unary minus on unsigned
            /wd4101   # unreferenced local variable
        )
        return()
    endif()

    set(WARNFLAGS -Wall -W -Wshadow -Wno-sign-compare -Wno-unused-parameter -Wno-strict-aliasing -Wno-unknown-pragmas -Werror)

    # Desktop: MinGW or native Linux
    set(GNU_COMPILER_FLAG -march=x86-64 -fno-omit-frame-pointer -fmessage-length=0)
    target_compile_options(${TARGET} PRIVATE ${WARNFLAGS} ${GNU_COMPILER_FLAG})
endfunction()

# Apply common link options to a target based on platform
function(apply_keeperfx_link_flags TARGET)
    if(MSVC)
        # MSVC: Windows subsystem, no GCC-style flags.
        # /MANIFEST:NO disables the auto-generated manifest because keeperfx_stdres.rc
        # already embeds the manifest via RT_MANIFEST, and duplicate manifests cause LNK error.
        target_link_options(${TARGET} PRIVATE /SUBSYSTEM:WINDOWS /MANIFEST:NO)
        return()
    endif()
    
    if(WIN32 OR MINGW)
        # MinGW: Windows subsystem + map file
        target_link_options(${TARGET} PRIVATE -mwindows -Wl,--enable-auto-import -Wl,-Map,${TARGET}.map)
        target_link_libraries(${TARGET} PUBLIC -static stdc++ winpthread -dynamic)
        # Use LLVM linker (LLD) for faster linking
        set_property(TARGET ${TARGET} PROPERTY LINKER_TYPE LLD)
    else()
        # Linux: pthreads + map file
        target_link_options(${TARGET} PRIVATE -Wl,-Map,${TARGET}.map)
        find_package(Threads REQUIRED)
        target_link_libraries(${TARGET} PUBLIC Threads::Threads)
    endif()
endfunction()

# Apply system libraries (Windows only: imagehlp, dbghelp, ole32, uuid, winmm, ws2_32)
function(apply_windows_system_libs TARGET)
    if(WIN32)
        target_link_libraries(${TARGET} PRIVATE imagehlp dbghelp ole32 uuid winmm ws2_32)
    endif()
endfunction()
