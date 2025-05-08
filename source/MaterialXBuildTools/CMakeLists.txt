# This subproject is necessary to support building the build tools on the host machine
# where they may be used when cross compiling for another target

cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_MACOSX_RPATH ON)

project(MaterialXBuildTools)

list(APPEND CMAKE_MODULE_PATH
        ${PROJECT_SOURCE_DIR}/../../cmake/macros)

include(Public)

add_subdirectory(../MaterialXCore MaterialXCore)
add_subdirectory(../MaterialXFormat MaterialXFormat)

add_subdirectory(buildLibrary)
