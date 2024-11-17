cmake_minimum_required(VERSION 3.20)


if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/enet )
	file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/enet)
	if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/enet-mingw32.tar.gz )
		file(DOWNLOAD https://github.com/dkfans/kfx-deps/releases/download/initial/enet-mingw32.tar.gz ${CMAKE_SOURCE_DIR}/deps/enet-mingw32.tar.gz SHOW_PROGRESS)
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_SOURCE_DIR}/deps/enet-mingw32.tar.gz WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/enet)
endif()

if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/zlib )
	file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/zlib)
	if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/zlib-mingw32.tar.gz )
		file(DOWNLOAD https://github.com/dkfans/kfx-deps/releases/download/initial/zlib-mingw32.tar.gz ${CMAKE_SOURCE_DIR}/deps/zlib-mingw32.tar.gz SHOW_PROGRESS)
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_SOURCE_DIR}/deps/zlib-mingw32.tar.gz WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/zlib)
endif()

if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/spng )
	file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/spng)
	if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/spng-mingw32.tar.gz )
		file(DOWNLOAD https://github.com/dkfans/kfx-deps/releases/download/initial/spng-mingw32.tar.gz ${CMAKE_SOURCE_DIR}/deps/spng-mingw32.tar.gz SHOW_PROGRESS)
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_SOURCE_DIR}/deps/spng-mingw32.tar.gz WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/spng)
endif()

if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/astronomy )
	file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/astronomy)
	if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/astronomy-mingw32.tar.gz )
		file(DOWNLOAD https://github.com/dkfans/kfx-deps/releases/download/initial/astronomy-mingw32.tar.gz ${CMAKE_SOURCE_DIR}/deps/astronomy-mingw32.tar.gz SHOW_PROGRESS)
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_SOURCE_DIR}/deps/astronomy-mingw32.tar.gz WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/astronomy)
endif()

if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/centijson )
	file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/centijson)
	if( NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/centijson-mingw32.tar.gz )
		file(DOWNLOAD https://github.com/dkfans/kfx-deps/releases/download/initial/centijson-mingw32.tar.gz ${CMAKE_SOURCE_DIR}/deps/centijson-mingw32.tar.gz SHOW_PROGRESS)
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_SOURCE_DIR}/deps/centijson-mingw32.tar.gz WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/centijson)
endif()

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/ffmpeg)
	file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/ffmpeg)
	if(NOT EXISTS ${CMAKE_SOURCE_DIR}/deps/ffmpeg-mingw32.tar.gz)
		file(DOWNLOAD https://github.com/dkfans/kfx-deps/releases/download/initial/ffmpeg-mingw32.tar.gz ${CMAKE_SOURCE_DIR}/deps/ffmpeg-mingw32.tar.gz SHOW_PROGRESS)
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_SOURCE_DIR}/deps/ffmpeg-mingw32.tar.gz WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/ffmpeg)
endif()

## enet
add_library(enet_static STATIC IMPORTED)
set_target_properties(enet_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/enet/libenet.a)
set_target_properties(enet_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/enet/include)
target_link_libraries(enet_static INTERFACE ws2_32 winmm)
target_link_libraries(keeperfx PUBLIC enet_static)
target_link_libraries(keeperfx_hvlog PUBLIC enet_static)

## spng
add_library(spng_static STATIC IMPORTED)
set_target_properties(spng_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/spng/libspng.a)
set_target_properties(spng_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/spng/include)
target_link_libraries(keeperfx PUBLIC spng_static)
target_link_libraries(keeperfx_hvlog PUBLIC spng_static)

## centijson
add_library(centijson_static STATIC IMPORTED)
set_target_properties(centijson_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/centijson/libjson.a)
set_target_properties(centijson_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/centijson/include)
target_link_libraries(keeperfx PUBLIC centijson_static)
target_link_libraries(keeperfx_hvlog PUBLIC centijson_static)

## astronomy.
add_library(astronomy_static STATIC IMPORTED)
set_target_properties(astronomy_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/astronomy/libastronomy.a)
set_target_properties(astronomy_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/astronomy/include)
target_link_libraries(keeperfx PUBLIC astronomy_static)
target_link_libraries(keeperfx_hvlog PUBLIC astronomy_static)

## zlib
add_library(zlib_static STATIC IMPORTED)
set_target_properties(zlib_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/zlib/libz.a)
set_target_properties(zlib_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/zlib/include)
target_link_libraries(keeperfx PUBLIC zlib_static)
target_link_libraries(keeperfx_hvlog PUBLIC zlib_static)

# Add minizip
add_library(minizip_static STATIC IMPORTED)
set_target_properties(minizip_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/zlib/libminizip.a)
set_target_properties(minizip_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/zlib/include)
target_link_libraries(minizip_static INTERFACE zlib_static)
target_link_libraries(keeperfx PUBLIC minizip_static)
target_link_libraries(keeperfx_hvlog PUBLIC minizip_static)

## centitoml.
add_library(centitoml OBJECT "centitoml/toml_api.c")
target_link_libraries(centitoml PUBLIC centijson_static)
target_include_directories(centitoml INTERFACE "centitoml")
target_link_libraries(keeperfx PUBLIC centitoml)
target_link_libraries(keeperfx_hvlog PUBLIC centitoml)

## ffmpeg
add_library(libavcodec_static STATIC IMPORTED)
set_target_properties(libavcodec_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/libavcodec/libavcodec.a)
set_target_properties(libavcodec_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg)

add_library(libavformat_static STATIC IMPORTED)
set_target_properties(libavformat_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/libavformat/libavformat.a)
set_target_properties(libavformat_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg)

add_library(libavutil_static STATIC IMPORTED)
set_target_properties(libavutil_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/libavutil/libavutil.a)
set_target_properties(libavutil_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg)

add_library(libswresample_static STATIC IMPORTED)
set_target_properties(libswresample_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/libswresample/libswresample.a)
set_target_properties(libswresample_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg)

target_link_libraries(keeperfx PUBLIC bcrypt libavcodec_static libavformat_static libavutil_static libswresample_static)
target_link_libraries(keeperfx_hvlog PUBLIC bcrypt libavcodec_static libavformat_static libavutil_static libswresample_static)
