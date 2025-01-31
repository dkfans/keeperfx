cmake_minimum_required(VERSION 3.20)

add_executable(png2ico ${CMAKE_SOURCE_DIR}/tools/png2ico/png2ico.cpp)
target_include_directories(png2ico PUBLIC PkgConfig::SPNG)
target_link_libraries(png2ico PUBLIC PkgConfig::SPNG)

if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
	set( EXE_EXTENSION exe)
endif()
set(EXE_FILE_NAME png2ico${EXE_EXTENSION})

set( PNG_FILE_PREFIX "keeperfx_icon")

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/res/keeperfx_icon.ico
  COMMAND ${CMAKE_BINARY_DIR}/tools/${EXE_FILE_NAME} keeperfx_icon.ico ${PNG_FILE_PREFIX}512-24bpp.png ${PNG_FILE_PREFIX}256-24bpp.png ${PNG_FILE_PREFIX}128-24bpp.png
                                                   --colors 256 ${PNG_FILE_PREFIX}128-08bpp.png ${PNG_FILE_PREFIX}064-08bpp.png ${PNG_FILE_PREFIX}048-08bpp.png
												   --colors 16 ${PNG_FILE_PREFIX}032-08bpp.png ${PNG_FILE_PREFIX}016-08bpp.png
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/res
  VERBATIM
  DEPENDS png2ico
)
add_custom_target(generate_ico DEPENDS png2ico ${CMAKE_SOURCE_DIR}/res/keeperfx_icon.ico)

add_dependencies( keeperfx generate_ico )
