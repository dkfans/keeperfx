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

