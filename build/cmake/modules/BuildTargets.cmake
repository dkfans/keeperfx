# BuildTargets.cmake - source collection, executables, per-target flags + linking.

file(GLOB_RECURSE KEEPERFX_SOURCES_C   CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE KEEPERFX_SOURCES_CXX CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Functional-test harness is off by default (FTEST_DEBUG defaults to 0).
list(FILTER KEEPERFX_SOURCES_C   EXCLUDE REGEX "/src/ftests/")
list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX "/src/ftests/")

# Desktop platform filtering (matches the hand Makefiles).
if(WIN32)
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX "/src/linux\\.cpp$")
elseif(UNIX AND NOT APPLE)
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX "/src/(cdrom|steam_api|windows)\\.cpp$")
endif()

add_executable(keeperfx       ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})
add_executable(keeperfx_hvlog ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})
target_compile_definitions(keeperfx       PUBLIC BFDEBUG_LEVEL=0)
target_compile_definitions(keeperfx_hvlog PUBLIC BFDEBUG_LEVEL=10)

set(KFX_TARGETS keeperfx keeperfx_hvlog)

if(WIN32)
    foreach(_t IN LISTS KFX_TARGETS)
        target_sources(${_t} PRIVATE "${CMAKE_SOURCE_DIR}/res/keeperfx_stdres.rc")
        # bfd is slow; prefer LLD when available (LINKER_TYPE needs CMake >= 3.29,
        # harmlessly ignored on older CMake, which uses the default linker).
        set_property(TARGET ${_t} PROPERTY LINKER_TYPE LLD)
    endforeach()
endif()

foreach(_t IN LISTS KFX_TARGETS)
    apply_keeperfx_warnings(${_t})
    apply_keeperfx_link_flags(${_t})
    kfx_link_dependencies(${_t})
    apply_windows_system_libs(${_t})
endforeach()

kfx_status("BUILD" "${CMAKE_CXX_COMPILER_ID} -> keeperfx, keeperfx_hvlog")
