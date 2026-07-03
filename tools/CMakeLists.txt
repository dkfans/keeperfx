cmake_minimum_required(VERSION 3.20)

if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
    set( PNGTOICO_DOWNLOAD https://github.com/dkfans/png2ico/releases/download/2024-10-28/png2ico-win-2024-10-28.zip )
	set( ZIP_FILE_NAME png2ico-win-2024-10-28.zip )
	set( EXE_FILE_NAME png2ico.exe)
elseif( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux" )
    set( PNGTOICO_DOWNLOAD https://github.com/dkfans/png2ico/releases/download/2024-10-28/png2ico-lin-2024-10-28.tar.gz )
	set( ZIP_FILE_NAME png2ico-lin-2024-10-28.tar.gz )
	set( EXE_FILE_NAME png2ico)
endif()

set(DESTINATION_DIR ${CMAKE_SOURCE_DIR}/tools/png2ico/${CMAKE_HOST_SYSTEM_NAME})

if( NOT EXISTS ${DESTINATION_DIR} )
	file( MAKE_DIRECTORY ${DESTINATION_DIR} )
	if( NOT EXISTS ${DESTINATION_DIR}/ZIP_FILE_NAME )
		file( DOWNLOAD ${PNGTOICO_DOWNLOAD} ${DESTINATION_DIR}/${ZIP_FILE_NAME} SHOW_PROGRESS )
	endif()

	execute_process( COMMAND ${CMAKE_COMMAND} -E tar xzf ${DESTINATION_DIR}/${ZIP_FILE_NAME} WORKING_DIRECTORY ${DESTINATION_DIR} )
endif()

set( PNG_FILE_PREFIX "keeperfx_icon")

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/res/keeperfx_icon.ico
  COMMAND ${DESTINATION_DIR}/${EXE_FILE_NAME} keeperfx_icon.ico ${PNG_FILE_PREFIX}512-24bpp.png ${PNG_FILE_PREFIX}256-24bpp.png ${PNG_FILE_PREFIX}128-24bpp.png
                                                   --colors 256 ${PNG_FILE_PREFIX}128-08bpp.png ${PNG_FILE_PREFIX}064-08bpp.png ${PNG_FILE_PREFIX}048-08bpp.png
												   --colors 16 ${PNG_FILE_PREFIX}032-08bpp.png ${PNG_FILE_PREFIX}016-08bpp.png
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/res
  VERBATIM
)
add_custom_target(generate_ico DEPENDS ${CMAKE_SOURCE_DIR}/res/keeperfx_icon.ico)

add_dependencies( keeperfx generate_ico )
add_dependencies( keeperfx_hvlog generate_ico )
