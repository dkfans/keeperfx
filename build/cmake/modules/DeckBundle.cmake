# ---------------------------------------------------------------------------
# DeckBundle.cmake — self-contained Linux bundle (Steam Deck / portable Linux)
# ---------------------------------------------------------------------------
#
#
# It deliberately does NOT bundle glibc, the C++ runtime, or the GPU / display /
# audio / system-integration libraries — those must come from the Deck so they
# match its drivers and compositor. See docs/steam-deck.md.
#
# Usage:  cmake -S . -B out -DKFX_DECK_BUNDLE=ON && cmake --build out --target keeperfx
#         cmake --install out --prefix keeperfx-deck
# ---------------------------------------------------------------------------

option(KFX_DECK_BUNDLE
    "Bundle runtime shared libraries next to keeperfx for portable Linux/Steam Deck deployment" OFF)

if(KFX_DECK_BUNDLE)
    if(WIN32 OR MINGW OR CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "KFX_DECK_BUNDLE is for native Linux builds only")
    endif()
    find_program(KFX_PATCHELF patchelf)
    if(NOT KFX_PATCHELF)
        message(FATAL_ERROR "KFX_DECK_BUNDLE requires 'patchelf' (apt install patchelf)")
    endif()

    # The installed binary finds its bundled libraries in ./lib next to itself.
    set_target_properties(keeperfx PROPERTIES INSTALL_RPATH "$ORIGIN/lib")

    # The heavy lifting (dependency resolution + copy + rpath) runs at install
    # time, when the built binary exists, via a configured script.
    set(_deck_install_script "${CMAKE_BINARY_DIR}/DeckBundleInstall.cmake")
    configure_file(
        "${CMAKE_SOURCE_DIR}/build/cmake/DeckBundleInstall.cmake.in"
        "${_deck_install_script}"
        @ONLY)
    install(SCRIPT "${_deck_install_script}")

    message(STATUS "KFX_DECK_BUNDLE=ON: keeperfx will be installed as a self-contained Deck bundle")
endif()
