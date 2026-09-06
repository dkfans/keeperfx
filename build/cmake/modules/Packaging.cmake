# ---------------------------------------------------------------------------
# Packaging.cmake — CPack configuration for KeeperFX distribution packages
# ---------------------------------------------------------------------------
#
# Produces the same kind of release archive as `make package`, but takes the
# keeperfx binary from the CMake build and the game data from the make data
# pipeline. Usage:
#
#   1. Configure (pass the build number / suffix used for the filename):
#        cmake -S . -B out -G Ninja -DCMAKE_TOOLCHAIN_FILE=build/cmake/toolchains/mingw32.cmake \
#              -DCMAKE_BUILD_TYPE=RelWithDebInfo \
#              -DBUILD_NUMBER=1234 -DPACKAGE_SUFFIX=Alpha
#   2. Build the binary:
#        cmake --build build --target keeperfx
#   3. Assemble the game data (gfx/lang/sfx pipeline) into pkg/:
#        make BUILD_NUMBER=1234 PACKAGE_SUFFIX=Alpha pkg-assemble
#   4. Create the archive:
#        cmake --build build --target package
#
#
# Output: pkg/keeperfx-<maj>_<min>_<rel>_<build>[-<suffix>]-patch.7z
# ---------------------------------------------------------------------------

set(CPACK_PACKAGE_NAME      "keeperfx")
set(CPACK_PACKAGE_VENDOR    "KeeperFX Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "KeeperFX - Free implementation of Dungeon Keeper")
set(CPACK_PACKAGE_VERSION   "${VER_MAJOR}.${VER_MINOR}.${VER_RELEASE}.${BUILD_NUMBER}")
set(CPACK_PACKAGE_VERSION_MAJOR "${VER_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VER_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VER_RELEASE}")

# Archive filename keeperfx-<maj>_<min>_<rel>_<build>[-<suffix>]-patch
if(PACKAGE_SUFFIX AND NOT "${PACKAGE_SUFFIX}" STREQUAL "")
    set(CPACK_PACKAGE_FILE_NAME
        "keeperfx-${VER_MAJOR}_${VER_MINOR}_${VER_RELEASE}_${BUILD_NUMBER}-${PACKAGE_SUFFIX}-patch")
else()
    set(CPACK_PACKAGE_FILE_NAME
        "keeperfx-${VER_MAJOR}_${VER_MINOR}_${VER_RELEASE}_${BUILD_NUMBER}-patch")
endif()

# Place generated archives in the source-tree pkg/ directory
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_SOURCE_DIR}/pkg")

# Flat archive layout (no top-level directory prefix)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF)

# 7Z for the Windows/MinGW target; TGZ otherwise.
if(WIN32 OR MINGW OR CMAKE_CROSSCOMPILING OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CPACK_GENERATOR "7Z")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

# --- Install rules ---------------------------------------------------------

install(TARGETS keeperfx RUNTIME DESTINATION .)
install(TARGETS keeperfx_hvlog RUNTIME DESTINATION . OPTIONAL)

# The game data assembled by "make pkg-assemble" (configs, campaigns, levels,
# language/sound .dat files, SDL3 runtime DLLs, docs). Evaluated at pack time so
# pkg/ is read then, not at configure time. Skips any archive left in pkg/.
install(CODE "
    set(_pkg_src \"${CMAKE_SOURCE_DIR}/pkg\")
    if(EXISTS \"\${_pkg_src}\")
        file(GLOB_RECURSE _pkg_files
            LIST_DIRECTORIES false
            RELATIVE \"\${_pkg_src}\"
            \"\${_pkg_src}/*\")
        foreach(_f IN LISTS _pkg_files)
            if(NOT _f MATCHES \"keeperfx.*\\\\.(7z|tar\\\\.gz|tgz)\$\")
                get_filename_component(_dir \"\${_f}\" DIRECTORY)
                file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/\${_dir}\")
                file(COPY \"\${_pkg_src}/\${_f}\"
                    DESTINATION \"\${CMAKE_INSTALL_PREFIX}/\${_dir}\")
            endif()
        endforeach()
    endif()
")

include(CPack)
