cmake_minimum_required(VERSION 3.20)

## centijson
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
add_library(json STATIC
    centijson/src/json.c
    centijson/src/json.h
    centijson/src/json-dom.c
    centijson/src/json-dom.h
    centijson/src/json-ptr.c
    centijson/src/json-ptr.h
    centijson/src/value.c
    centijson/src/value.h
)
target_include_directories(json INTERFACE "centijson/src")
target_link_libraries(keeperfx PUBLIC json)

## astronomy.
add_library(astronomy OBJECT "astronomy/source/c/astronomy.c")
target_include_directories(astronomy INTERFACE "astronomy/source/c")
target_link_libraries(keeperfx PUBLIC astronomy)

## centitoml.
add_library(centitoml OBJECT "centitoml/toml_api.c")
target_link_libraries(centitoml PUBLIC json)
target_include_directories(centitoml INTERFACE "centitoml")
target_link_libraries(keeperfx PUBLIC centitoml)

## findfirst
add_library(findfirst OBJECT "findfirst/findfirst.c" "findfirst/spec.c")
target_include_directories(findfirst INTERFACE "findfirst")
target_link_libraries(keeperfx PUBLIC findfirst)

## libbcrypt
add_library(bcrypt OBJECT "libbcrypt/bcrypt.c" "libbcrypt/crypt_blowfish/crypt_gensalt.c" "libbcrypt/crypt_blowfish/crypt_blowfish.c" "libbcrypt/crypt_blowfish/wrapper.c")
target_include_directories(bcrypt INTERFACE "libbcrypt")
target_link_libraries(keeperfx PUBLIC bcrypt)

