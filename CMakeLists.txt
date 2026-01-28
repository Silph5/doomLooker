cmake_minimum_required(VERSION 4.0)
project(doomLooker C)

set(CMAKE_C_STANDARD 11)

set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/glfw/include")
set(GLFW_LIBRARY "${CMAKE_SOURCE_DIR}/libs/glfw/libglfw3.a")

add_executable(doomLooker
        src/main.c
        src/glad.c
        include/mapStruct.h
        include/mapComponentStructs.h
        src/game.c
        include/game.h
        src/buildModel.c
        include/buildModel.h
        include/vertex.h
        include/model.h
        include/ut_hash/uthash.h
        include/texture.h
        src/atlas.c
        include/atlas.h
        src/wadReading/readWad.c
        src/wadReading/handleWadComposite.c
        include/wad.h
)

target_include_directories(doomLooker PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${GLFW_INCLUDE_DIR}
)

find_package(OpenGL REQUIRED)

#platform-specific
if (WIN32)
    target_link_libraries(doomLooker ${GLFW_LIBRARY} OpenGL::GL opengl32)
else ()
    target_link_libraries(doomLooker ${GLFW_LIBRARY} OpenGL::GL dl pthread)
endif ()