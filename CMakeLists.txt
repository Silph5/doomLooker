cmake_minimum_required(VERSION 4.0)
project(doomLooker C)

set(CMAKE_C_STANDARD 11)

set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/glfw/include")
set(GLFW_LIBRARY "${CMAKE_SOURCE_DIR}/libs/glfw/libglfw3.a")

set(LTC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/ltc")
set(LTC_LIBRARY "${CMAKE_SOURCE_DIR}/libs/ltc/libtrychain.a")

add_executable(doomLooker
        src/main.c
        src/glad.c
        include/mapStruct.h
        include/mapComponentStructs.h
        src/game.c
        include/game.h
        src/sceneBuilding/buildModel.c
        include/buildModel.h
        include/vertex.h
        include/model.h
        include/ut_hash/uthash.h
        include/texture.h
        src/sceneBuilding/atlas.c
        include/atlas.h
        src/wadReading/DFread.c
        src/wadReading/wadComposite.c
        include/wad.h
        include/directoryEntry.h
        src/wadReading/directoryEntry.c
        include/wadComposite.h
        src/wadReading/textureBuilding.c
        include/textureBuilding.h
        include/DFread.h
)

target_include_directories(doomLooker PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${GLFW_INCLUDE_DIR}
        ${LTC_INCLUDE_DIR}
)

find_package(OpenGL REQUIRED)

target_link_libraries(doomLooker PRIVATE
        ${LTC_LIBRARY}
        ${GLFW_LIBRARY}
        OpenGL::GL
)

if (WIN32)
    target_link_libraries(doomLooker PRIVATE opengl32)
else ()
    target_link_libraries(doomLooker PRIVATE dl pthread)
endif ()