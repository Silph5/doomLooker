cmake_minimum_required(VERSION 4.0)
project(doomLooker C)

set(CMAKE_C_STANDARD 11)

add_executable(doomLooker
        src/main.c
        src/readWad.c
        include/mapStruct.h
        include/mapComponentStructs.h
)

target_include_directories(doomLooker PRIVATE ${CMAKE_SOURCE_DIR}/Include)