# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# Platforms.cmake — Platform detection and build options
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

message(STATUS "Building for desktop platform")

# Build options
option(KEEPERFX_NETWORKING "Build with multiplayer networking support" ON)
option(KEEPERFX_RENDERER_OPENGL "Enable the OpenGL renderer backend (desktop only)" ON)
option(KFX_DEBUG_MEMORY "Enable KfxAlloc guard zones and per-site tracking" OFF)

# Apply compile definitions
if(KEEPERFX_NETWORKING)
    add_compile_definitions("KEEPERFX_NETWORKING=1")
endif()

if(KFX_DEBUG_MEMORY)
    add_compile_definitions("KFX_DEBUG_MEMORY=1")
endif()

add_compile_definitions("KEEPERFX_LUA_AVAILABLE=1")
add_compile_definitions("SDL_MIXER_AVAILABLE=1")

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# Sanitizers (AddressSanitizer + UndefinedBehaviorSanitizer)
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
option(KEEPERFX_SANITIZERS "Enable AddressSanitizer (+ UBSan on GCC/Clang)" OFF)
if(KEEPERFX_SANITIZERS)
    if(MSVC)
        add_compile_options(/fsanitize=address)
        # MSVC ASan needs the debug info for useful stack traces
        add_compile_options(/Zi)
        add_link_options(/DEBUG)
        # Disable STL container annotations — pre-built vcpkg libs don't have them,
        # causing LNK2038 mismatch errors (annotate_string / annotate_vector)
        add_compile_definitions(_DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
        kfx_status("SANITIZERS" "MSVC AddressSanitizer enabled (/fsanitize=address)")
    else()
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
        kfx_status("SANITIZERS" "GCC/Clang ASan + UBSan enabled")
    endif()
endif()
