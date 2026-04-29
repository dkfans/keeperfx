# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# BuildTargets.cmake — Target definitions (keeperfx, keeperfx_hvlog, tests)
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

# ━━━ Source File Collection ━━━
file(GLOB_RECURSE KEEPERFX_SOURCES_C "src/*.c")
file(GLOB_RECURSE KEEPERFX_SOURCES_CXX "src/*.cpp")

# Exclude stub files — added back explicitly when needed
list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/bflib_cpu_stub\\.c$")
list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/bflib_network_stub\\.c$")

# OpenGL renderer exclusion
if(NOT KEEPERFX_RENDERER_OPENGL)
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/renderer/RendererOpenGL\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/platform_gl_sdl2\\.cpp$")
endif()

# ━━━ Networking Exclusions ━━━
if(NOT KEEPERFX_NETWORKING)
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/api\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/bflib_tcpsp\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/bflib_base_tcp\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/bflib_client_tcp\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/bflib_server_tcp\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/bflib_enet\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/net_portforward\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/bflib_netsession\\.c$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/bflib_netsp\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/bflib_network\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/bflib_network_exchange\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/net_resync\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/net_input_lag\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/net_received_packets\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/net_redundant_packets\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/net_checksums\\.c$")
    list(FILTER KEEPERFX_SOURCES_C EXCLUDE REGEX ".*/net_game\\.c$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/bflib_enet\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/bflib_base_tcp\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/bflib_client_tcp\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/bflib_server_tcp\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/net_portforward\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/net_resync\\.cpp$")
    list(APPEND KEEPERFX_SOURCES_C "src/bflib_network_stub.c")
endif()

# ━━━ Desktop platform filtering (Windows vs Linux) ━━━
if(WIN32 OR MINGW)
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/platform/PlatformLinux\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/src/linux\\.cpp$")
elseif(UNIX AND NOT APPLE)
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/platform/PlatformWindows\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/src/windows\\.cpp$")
elseif(APPLE)
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/platform/PlatformLinux\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/platform/PlatformWindows\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/src/linux\\.cpp$")
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX ".*/src/windows\\.cpp$")
endif()

# ━━━ Main Targets: keeperfx & keeperfx_hvlog ━━━
add_executable(keeperfx ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})
if(MSVC)
    set_target_properties(keeperfx PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/.deploy"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/.deploy"
    )
endif()
target_include_directories(keeperfx PRIVATE src deps/centitoml deps/centijson/include)
target_compile_definitions(keeperfx PUBLIC BFDEBUG_LEVEL=0)
if(WIN32)
    target_sources(keeperfx PRIVATE "res/keeperfx_stdres.rc")
endif()
apply_keeperfx_warnings(keeperfx)
apply_keeperfx_link_flags(keeperfx)
apply_windows_system_libs(keeperfx)

# Link SDL2 (vcpkg static targets have -static suffix; mingw/Linux use plain names)
macro(kfx_link_sdl2_target TARGET_NAME)
    if(TARGET SDL2::SDL2-static)
        target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2-static)
    elseif(TARGET SDL2::SDL2)
        target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
    elseif(TARGET SDL2)
        target_link_libraries(${TARGET_NAME} PRIVATE SDL2)
    endif()
    if(TARGET SDL2_image::SDL2_image-static)
        target_link_libraries(${TARGET_NAME} PRIVATE SDL2_image::SDL2_image-static)
    elseif(TARGET SDL2_image::SDL2_image)
        target_link_libraries(${TARGET_NAME} PRIVATE SDL2_image::SDL2_image)
    endif()
    if(TARGET SDL2_mixer::SDL2_mixer-static)
        target_link_libraries(${TARGET_NAME} PRIVATE SDL2_mixer::SDL2_mixer-static)
    elseif(TARGET SDL2_mixer::SDL2_mixer)
        target_link_libraries(${TARGET_NAME} PRIVATE SDL2_mixer::SDL2_mixer)
    endif()
    if(KEEPERFX_NETWORKING)
        if(TARGET SDL2_net::SDL2_net-static)
            target_link_libraries(${TARGET_NAME} PRIVATE SDL2_net::SDL2_net-static)
        elseif(TARGET SDL2_net::SDL2_net)
            target_link_libraries(${TARGET_NAME} PRIVATE SDL2_net::SDL2_net)
        endif()
    endif()
endmacro()
kfx_link_sdl2_target(keeperfx)

add_executable(keeperfx_hvlog ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})
target_include_directories(keeperfx_hvlog PRIVATE src deps/centitoml deps/centijson/include)
target_compile_definitions(keeperfx_hvlog PUBLIC BFDEBUG_LEVEL=10)
if(WIN32)
    target_sources(keeperfx_hvlog PRIVATE "res/keeperfx_stdres.rc")
endif()
apply_keeperfx_warnings(keeperfx_hvlog)
apply_keeperfx_link_flags(keeperfx_hvlog)
apply_windows_system_libs(keeperfx_hvlog)

kfx_link_sdl2_target(keeperfx_hvlog)

# ━━━ Dependent Platform Libraries ━━━
# Linked AFTER targets are created (see `add_subdirectory(deps)` in root CMakeLists.txt)

# ━━━ Test Target ━━━
file(GLOB TEST_SOURCES "tests/*.cpp")
add_library(cunit_static STATIC
    "deps/CUnit-2.1-3/CUnit/Sources/Basic/Basic.c"
    "deps/CUnit-2.1-3/CUnit/Sources/Framework/TestDB.c"
    "deps/CUnit-2.1-3/CUnit/Sources/Framework/CUError.c"
    "deps/CUnit-2.1-3/CUnit/Sources/Framework/TestRun.c"
    "deps/CUnit-2.1-3/CUnit/Sources/Framework/Util.c"
)
target_include_directories(cunit_static PUBLIC "deps/CUnit-2.1-3/CUnit/Headers")

add_executable(tests ${TEST_SOURCES} ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})
target_compile_definitions(tests PUBLIC BFDEBUG_LEVEL=0)
target_link_libraries(tests PRIVATE cunit_static)
apply_keeperfx_warnings(tests)
apply_keeperfx_link_flags(tests)
apply_windows_system_libs(tests)

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
