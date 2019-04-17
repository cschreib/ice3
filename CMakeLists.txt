cmake_minimum_required(VERSION 2.6)
project(ice3)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release (-DCMAKE_BUILD_TYPE=...)")
    set(CMAKE_BUILD_TYPE "Release")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions(-Weverything)
    add_definitions(-Wno-c++98-compat-pedantic)
    add_definitions(-Wno-c++98-compat)
    add_definitions(-Wno-unused-parameter)
    add_definitions(-Wno-sign-conversion)
    add_definitions(-Wno-conversion)
    add_definitions(-Wno-missing-prototypes)
    add_definitions(-Wno-missing-variable-declarations)
    add_definitions(-Wno-padded)
    add_definitions(-Wno-float-equal)
    add_definitions(-Wno-unused-variable)
    add_definitions(-Wno-global-constructors)
    add_definitions(-Wno-exit-time-destructors)
    add_definitions(-Wno-shadow)
    add_definitions(-Wno-weak-vtables)
    add_definitions(-Wno-covered-switch-default)
    add_definitions(-Wno-documentation-unknown-command)
    add_definitions(-Wno-unneeded-internal-declaration)
    add_definitions(-Wno-unused-function)
    add_definitions(-Wno-unused-macros)
    add_definitions(-Wno-switch-enum)
    add_definitions(-Wno-disabled-macro-expansion)
    add_definitions(-Wno-missing-noreturn)
    add_definitions(-std=c++11)
    add_definitions(-ftemplate-backtrace-limit=0)
    add_definitions(-ferror-limit=5)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_definitions(-Wall)
    add_definitions(-std=c++11)
    add_definitions(-fmax-errors=5)
    add_definitions(-Wno-unused-variable)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

if (UNIX)
    add_definitions(-DPOSIX)
endif()
if (WINDOWS)
    add_definitions(-DWIN32)
endif()

file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
set(BINARY_DIR "${PROJECT_SOURCE_DIR}/bin")

find_package(Threads REQUIRED)
find_package(SFML 2 COMPONENTS system network window graphics)
find_package(lxgui)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${SFML_INCLUDE_DIR} SYSTEM)
include_directories(${LXGUI_INCLUDE_DIRS})

message(STATUS ${LXGUI_INCLUDE_DIRS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}")

add_executable(ice3
    src/application.cpp
    src/axisalignedbox.cpp
    src/blockchunk.cpp
    src/block.cpp
    src/camera.cpp
    src/color.cpp
    src/compress.cpp
    src/cylinder.cpp
    src/glew.cpp
    src/light.cpp
    src/main.cpp
    src/matrix.cpp
    src/movable.cpp
    src/node.cpp
    src/perlin.cpp
    src/quaternion.cpp
    src/shader.cpp
    src/state.cpp
    src/texture.cpp
    src/texture_manager.cpp
    src/threading_policies.cpp
    src/unit.cpp
    src/vector3.cpp
    src/vertexbufferobject.cpp
    src/world.cpp
    src/world_glues.cpp
    src/world_load.cpp
    src/world_render.cpp
)

target_link_libraries(ice3 ${SFML_GRAPHICS_LIBRARY})
target_link_libraries(ice3 ${SFML_WINDOW_LIBRARY})
target_link_libraries(ice3 ${SFML_NETWORK_LIBRARY})
target_link_libraries(ice3 ${SFML_SYSTEM_LIBRARY})
target_link_libraries(ice3 ${LXGUI_LIBRARIES})
target_link_libraries(ice3 ${CMAKE_THREAD_LIBS_INIT})

if(WINDOWS AND MSVC)
    target_link_libraries(ice3 user32)
endif()
if(UNIX)
    target_link_libraries(ice3 X11)
endif()
