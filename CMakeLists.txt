cmake_minimum_required(VERSION 3.0)

project(cxxbellhoptest)

set(CMAKE_CXX_STANDARD 17)

find_package(ZLIB REQUIRED)
include(FetchContent)
FetchContent_Declare(
        cnpy
        GIT_REPOSITORY https://github.com/rogersce/cnpy.git
        GIT_TAG master
)
FetchContent_MakeAvailable(cnpy)

include_directories(${PROJECT_SOURCE_DIR}/bhc)
include_directories(${PROJECT_SOURCE_DIR}/glm)
include_directories(${CMAKE_BINARY_DIR}/_deps/cnpy-src)

add_executable(underwater_acoustic_computing_core 
underwater_acoustic_computing_core.cpp
bellhop_params.cpp
utils.cpp
read_npz.cpp
read_output.cpp
write2file.cpp
config_param.cpp
)

target_include_directories(underwater_acoustic_computing_core PUBLIC "${CMAKE_SOURCE_DIR}/bhc")

target_link_libraries(underwater_acoustic_computing_core PUBLIC 
        "${CMAKE_SOURCE_DIR}/bin/bellhopcxxlib.lib" 
        cnpy 
        ZLIB::ZLIB)