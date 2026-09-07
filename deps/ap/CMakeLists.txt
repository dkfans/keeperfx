cmake_minimum_required(VERSION 3.24)

project(APCpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# APCpp itself

add_library(APCpp STATIC
    Archipelago.cpp
    Archipelago_DataStorageSim.cpp
    Archipelago_Gifting.cpp
)

target_include_directories(APCpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(APCpp PROPERTIES
    PUBLIC_HEADER Archipelago.h
)

# Dependency configuration

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)

# mbedTLS

set(USE_TLS ON CACHE BOOL "Enable TLS support" FORCE)

if (NOT MINGW)
    find_package(OpenSSL)

    if (NOT OPENSSL_FOUND)
        set(USE_MBED_TLS ON)
    endif()
endif()

if ((WIN32 AND USE_TLS) OR USE_MBED_TLS)

    set(APCPP_MBEDTLS_VER 3.6.4)

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E remove -f
                mbedtls-${APCPP_MBEDTLS_VER}
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf
                ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-${APCPP_MBEDTLS_VER}.tar.bz2
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(USE_STATIC_MBEDTLS_LIBRARY ON CACHE BOOL "" FORCE)

    set(MBEDTLS_INCLUDE_DIRS
        ${CMAKE_BINARY_DIR}/mbedtls-${APCPP_MBEDTLS_VER}/include
    )

    set(MBEDTLS_VERSION_GREATER_THAN_3
        ${MBEDTLS_INCLUDE_DIRS}
    )

    set(MBEDTLS_LIBRARY mbedtls)
    set(MBEDX509_LIBRARY mbedx509)
    set(MBEDCRYPTO_LIBRARY mbedcrypto)

    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/mbedtls-${APCPP_MBEDTLS_VER}
        mbedtls_bin
    )

    if (MINGW)
        target_compile_options(mbedtls PRIVATE -Wno-format)
    endif()

endif()

# zlib
# KeeperFX supplies its own zlib, so it is deliberately excluded
# from the final bundled APCpp archive.

if (NOT MINGW)

    find_package(ZLIB QUIET)

    if (ZLIB_FOUND)
        message(STATUS "Using system zlib installation.")
    endif()

endif()

if (MINGW OR NOT ZLIB_FOUND)

    set(ZLIB_BUILD_SHARED OFF)

    include(FetchContent)

    FetchContent_Declare(
        zlib
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_MakeAvailable(zlib)

    add_library(ZLIB::ZLIB ALIAS zlibstatic)

endif()

# IXWebSocket

set(IXWEBSOCKET_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(IXWebSocket)

target_include_directories(APCpp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/IXWebSocket
)

target_link_libraries(APCpp
    ixwebsocket
)

# JsonCpp

find_package(PkgConfig)

if (PKG_CONFIG_FOUND AND NOT MINGW AND NOT BUNDLED_JSONCPP)

    pkg_check_modules(JSONCPP jsoncpp)

    if (JSONCPP_FOUND)

        target_include_directories(APCpp PRIVATE
            ${JSONCPP_INCLUDE_DIRS}
        )

        target_link_libraries(APCpp
            ${JSONCPP_LDFLAGS}
        )

    endif()

endif()

if (NOT JSONCPP_FOUND OR BUNDLED_JSONCPP)

    set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
    set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)

    add_subdirectory(jsoncpp)

    set_property(
        TARGET jsoncpp_static
        PROPERTY POSITION_INDEPENDENT_CODE ON
    )

    target_include_directories(APCpp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/include
    )

    target_link_libraries(APCpp
        jsoncpp_static
    )

endif()

# Windows

if (WIN32)

    target_link_libraries(APCpp
        ws2_32
    )

endif()

# MinGW static runtime

if (MINGW)

    target_link_libraries(APCpp
        -static
        -static-libstdc++
        -static-libgcc
    )

endif()

# Build the single archive required by KeeperFX

if (MINGW)

    set(APCPP_BUNDLED_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/APCpp_all.o"
    )

    set(APCPP_BUNDLED_LIBRARY
        "${CMAKE_CURRENT_SOURCE_DIR}/libAPCpp.a"
    )

    set(APCPP_BUNDLE_LIBRARIES
        "${CMAKE_CURRENT_BINARY_DIR}/libAPCpp.a"
        "${CMAKE_CURRENT_BINARY_DIR}/IXWebSocket/libixwebsocket.a"
        "${CMAKE_CURRENT_BINARY_DIR}/jsoncpp/src/lib_json/libjsoncpp.a"
        "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/library/libmbedtls.a"
        "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/library/libmbedx509.a"
        "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/library/libmbedcrypto.a"
        "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/3rdparty/everest/libeverest.a"
        "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/3rdparty/p256-m/libp256m.a"
    )

    add_custom_command(
        OUTPUT
            "${APCPP_BUNDLED_LIBRARY}"

        COMMAND
            "${CMAKE_CXX_COMPILER}"
            -r
            -Wl,--whole-archive
            ${APCPP_BUNDLE_LIBRARIES}
            -Wl,--no-whole-archive
            -o
            "${APCPP_BUNDLED_OBJECT}"

        COMMAND
            "${CMAKE_AR}"
            rcs
            "${APCPP_BUNDLED_LIBRARY}"
            "${APCPP_BUNDLED_OBJECT}"

        COMMAND
            "${CMAKE_RANLIB}"
            "${APCPP_BUNDLED_LIBRARY}"

        DEPENDS
            APCpp
            ixwebsocket
            jsoncpp_static
            "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/library/libmbedtls.a"
            "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/library/libmbedx509.a"
            "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/library/libmbedcrypto.a"
            "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/3rdparty/everest/libeverest.a"
            "${CMAKE_CURRENT_BINARY_DIR}/mbedtls_bin/3rdparty/p256-m/libp256m.a"

        COMMENT "Building bundled KeeperFX APCpp library"

        VERBATIM
    )

    add_custom_target(APCpp_bundled ALL
        DEPENDS
            "${APCPP_BUNDLED_LIBRARY}"
    )

endif()

# Install

install(
    TARGETS APCpp
    PUBLIC_HEADER
)

if (MINGW)

    install(
        FILES
            "${APCPP_BUNDLED_LIBRARY}"
        DESTINATION .
    )

endif()
