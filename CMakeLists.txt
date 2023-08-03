cmake_minimum_required(VERSION 3.16)
project(JAPIPlugin C CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp" "src/*.h" "src/*.hpp")

add_library(JAPIPlugin SHARED ${SOURCES})

add_subdirectory(external/japi)

target_include_directories(${PROJECT_NAME} PRIVATE "src" "external/japi/src/exports")
target_link_libraries(${PROJECT_NAME} PRIVATE d3dcompiler_47)