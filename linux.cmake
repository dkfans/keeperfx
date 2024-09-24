# These are various overrides for dependencies built using CMake.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-m32 -mno-sse")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-m32 -mno-sse")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
