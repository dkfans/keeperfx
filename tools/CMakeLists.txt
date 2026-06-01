cmake_minimum_required(VERSION 3.20)

# Docker-first: tools are pre-installed at /usr/local/bin in KFX build images.
# find_program sets these cache variables; download fallback only runs if not found.
find_program(PNGTOICO_PATH   png2ico)
find_program(PNGTORAW_PATH   pngpal2raw)
find_program(PNGTOBSPAL_PATH png2bestpal)
find_program(POTONGDAT_PATH  po2ngdat)
find_program(SNDBANKER_PATH  sndbanker)
find_program(RNC_PATH        rnc)
find_program(DERNC_PATH      dernc)

if( NOT PNGTOICO_PATH OR NOT PNGTORAW_PATH OR NOT PNGTOBSPAL_PATH OR
    NOT POTONGDAT_PATH OR NOT SNDBANKER_PATH OR NOT RNC_PATH OR NOT DERNC_PATH )

# Determine platform-specific download URLs
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
    set( PNGTOICO_DOWNLOAD https://github.com/dkfans/png2ico/releases/download/2024-10-28/png2ico-win-2024-10-28.zip )
	set( PNGTOICO_FILE png2ico-win-2024-10-28.zip )
	set( PNGTOICO_EXE png2ico.exe)
	
	set( PNGTORAW_DOWNLOAD https://github.com/dkfans/pngpal2raw/releases/download/v1.0.2/pngpal2raw-1_0_2_35-devel-win.zip )
	set( PNGTORAW_FILE pngpal2raw-1_0_2_35-devel-win.zip )
	set( PNGTORAW_EXE pngpal2raw.exe )
	
	set( PNGTOBSPAL_DOWNLOAD https://github.com/dkfans/png2bestpal/releases/download/v1.0.3/png2bestpal-1_0_3_21-devel-win.zip )
	set( PNGTOBSPAL_FILE png2bestpal-1_0_3_21-devel-win.zip )
	set( PNGTOBSPAL_EXE png2bestpal.exe )
	
	set( POTONGDAT_DOWNLOAD https://github.com/dkfans/po2ngdat/releases/download/v1.0.2.31-c/po2ngdat-1_0_2_31-devel-win.zip )
	set( POTONGDAT_FILE po2ngdat-1_0_2_31-devel-win.zip )
	set( POTONGDAT_EXE po2ngdat.exe )
	
	set( SNDBANKER_DOWNLOAD https://github.com/dkfans/sndbanker/releases/download/v1.0.1/sndbanker-1_0_1_13-devel-win.zip )
	set( SNDBANKER_FILE sndbanker-1_0_1_13-devel-win.zip )
	set( SNDBANKER_EXE sndbanker.exe )
	
	set( RNCTOOLS_DOWNLOAD https://github.com/dkfans/rnctools/releases/download/v1.0.2/rnctools-1_0_2_5-devel-win.zip )
	set( RNCTOOLS_FILE rnctools-1_0_2_5-devel-win.zip )
	set( RNC_EXE rnc.exe )
	set( DERNC_EXE dernc.exe )
	
elseif( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux" )
    set( PNGTOICO_DOWNLOAD https://github.com/dkfans/png2ico/releases/download/2024-10-28/png2ico-lin-2024-10-28.tar.gz )
	set( PNGTOICO_FILE png2ico-lin-2024-10-28.tar.gz )
	set( PNGTOICO_EXE png2ico)
	
	set( PNGTORAW_DOWNLOAD https://github.com/dkfans/pngpal2raw/releases/download/v1.0.2/pngpal2raw-1_0_2_35-devel-lin.tar.gz )
	set( PNGTORAW_FILE pngpal2raw-1_0_2_35-devel-lin.tar.gz )
	set( PNGTORAW_EXE pngpal2raw )
	
	set( PNGTOBSPAL_DOWNLOAD https://github.com/dkfans/png2bestpal/releases/download/v1.0.3/png2bestpal-1_0_3_21-devel-lin.tar.gz )
	set( PNGTOBSPAL_FILE png2bestpal-1_0_3_21-devel-lin.tar.gz )
	set( PNGTOBSPAL_EXE png2bestpal )
	
	set( POTONGDAT_DOWNLOAD https://github.com/dkfans/po2ngdat/releases/download/v1.0.2.31-c/po2ngdat-1_0_2_31-devel-lin.tar.gz )
	set( POTONGDAT_FILE po2ngdat-1_0_2_31-devel-lin.tar.gz )
	set( POTONGDAT_EXE po2ngdat )
	
	set( SNDBANKER_DOWNLOAD https://github.com/dkfans/sndbanker/releases/download/v1.0.1/sndbanker-1_0_1_13-devel-lin.tar.gz )
	set( SNDBANKER_FILE sndbanker-1_0_1_13-devel-lin.tar.gz )
	set( SNDBANKER_EXE sndbanker )
	
	set( RNCTOOLS_DOWNLOAD https://github.com/dkfans/rnctools/releases/download/v1.0.2/rnctools-1_0_2_5-devel-lin.tar.gz )
	set( RNCTOOLS_FILE rnctools-1_0_2_5-devel-lin.tar.gz )
	set( RNC_EXE rnc )
	set( DERNC_EXE dernc )
endif()

    # Helper: download and extract a tool into tools/<dir>/bin/<exe>
    macro(download_tool TOOL_NAME TOOL_DOWNLOAD TOOL_FILE TOOL_DIR TOOL_EXE)
        if( NOT EXISTS ${CMAKE_SOURCE_DIR}/tools/${TOOL_DIR}/bin/${TOOL_EXE} )
            file( MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/${TOOL_DIR} )
            file( MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/${TOOL_DIR}/bin )
            if( NOT EXISTS ${CMAKE_SOURCE_DIR}/tools/${TOOL_DIR}/${TOOL_FILE} )
                message(STATUS "Downloading ${TOOL_NAME}...")
                file( DOWNLOAD ${TOOL_DOWNLOAD} ${CMAKE_SOURCE_DIR}/tools/${TOOL_DIR}/${TOOL_FILE} SHOW_PROGRESS )
            endif()
            message(STATUS "Extracting ${TOOL_NAME}...")
            execute_process( COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_SOURCE_DIR}/tools/${TOOL_DIR}/${TOOL_FILE} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/${TOOL_DIR}/bin )
        endif()
    endmacro()

    if( NOT PNGTOICO_PATH )
        download_tool("png2ico" "${PNGTOICO_DOWNLOAD}" "${PNGTOICO_FILE}" "png2ico" "${PNGTOICO_EXE}")
        set( PNGTOICO_PATH ${CMAKE_SOURCE_DIR}/tools/png2ico/bin/${PNGTOICO_EXE} )
    endif()
    if( NOT PNGTORAW_PATH )
        download_tool("pngpal2raw" "${PNGTORAW_DOWNLOAD}" "${PNGTORAW_FILE}" "pngpal2raw" "${PNGTORAW_EXE}")
        set( PNGTORAW_PATH ${CMAKE_SOURCE_DIR}/tools/pngpal2raw/bin/${PNGTORAW_EXE} )
    endif()
    if( NOT PNGTOBSPAL_PATH )
        download_tool("png2bestpal" "${PNGTOBSPAL_DOWNLOAD}" "${PNGTOBSPAL_FILE}" "png2bestpal" "${PNGTOBSPAL_EXE}")
        set( PNGTOBSPAL_PATH ${CMAKE_SOURCE_DIR}/tools/png2bestpal/bin/${PNGTOBSPAL_EXE} )
    endif()
    if( NOT POTONGDAT_PATH )
        download_tool("po2ngdat" "${POTONGDAT_DOWNLOAD}" "${POTONGDAT_FILE}" "po2ngdat" "${POTONGDAT_EXE}")
        set( POTONGDAT_PATH ${CMAKE_SOURCE_DIR}/tools/po2ngdat/bin/${POTONGDAT_EXE} )
    endif()
    if( NOT SNDBANKER_PATH )
        download_tool("sndbanker" "${SNDBANKER_DOWNLOAD}" "${SNDBANKER_FILE}" "sndbanker" "${SNDBANKER_EXE}")
        set( SNDBANKER_PATH ${CMAKE_SOURCE_DIR}/tools/sndbanker/bin/${SNDBANKER_EXE} )
    endif()
    if( NOT RNC_PATH )
        download_tool("rnctools (rnc)" "${RNCTOOLS_DOWNLOAD}" "${RNCTOOLS_FILE}" "rnctools" "${RNC_EXE}")
        set( RNC_PATH ${CMAKE_SOURCE_DIR}/tools/rnctools/bin/${RNC_EXE} )
    endif()
    if( NOT DERNC_PATH )
        download_tool("rnctools (dernc)" "${RNCTOOLS_DOWNLOAD}" "${RNCTOOLS_FILE}" "rnctools" "${DERNC_EXE}")
        set( DERNC_PATH ${CMAKE_SOURCE_DIR}/tools/rnctools/bin/${DERNC_EXE} )
    endif()

endif() # NOT all tools found on PATH

# Propagate resolved paths to parent scope
set(PNGTOICO_PATH   ${PNGTOICO_PATH}   PARENT_SCOPE)
set(PNGTORAW_PATH   ${PNGTORAW_PATH}   PARENT_SCOPE)
set(PNGTOBSPAL_PATH ${PNGTOBSPAL_PATH} PARENT_SCOPE)
set(POTONGDAT_PATH  ${POTONGDAT_PATH}  PARENT_SCOPE)
set(SNDBANKER_PATH  ${SNDBANKER_PATH}  PARENT_SCOPE)
set(RNC_PATH        ${RNC_PATH}        PARENT_SCOPE)
set(DERNC_PATH      ${DERNC_PATH}      PARENT_SCOPE)

# Generate keeperfx icon from PNGs
set( PNG_FILE_PREFIX "keeperfx_icon")

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/res/keeperfx_icon.ico
  COMMAND ${PNGTOICO_PATH} keeperfx_icon.ico ${PNG_FILE_PREFIX}512-24bpp.png ${PNG_FILE_PREFIX}256-24bpp.png ${PNG_FILE_PREFIX}128-24bpp.png
                                                   --colors 256 ${PNG_FILE_PREFIX}128-08bpp.png ${PNG_FILE_PREFIX}064-08bpp.png ${PNG_FILE_PREFIX}048-08bpp.png
											   --colors 16 ${PNG_FILE_PREFIX}032-08bpp.png ${PNG_FILE_PREFIX}016-08bpp.png
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/res
  VERBATIM
)
add_custom_target(generate_ico DEPENDS ${CMAKE_SOURCE_DIR}/res/keeperfx_icon.ico)

add_dependencies( keeperfx generate_ico )
add_dependencies( keeperfx_hvlog generate_ico )
