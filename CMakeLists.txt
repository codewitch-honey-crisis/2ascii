cmake_minimum_required(VERSION 3.24)

project(2ascii VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")


include(FetchContent)
FetchContent_Declare(htcw_uix
    GIT_REPOSITORY https://github.com/codewitch-honey-crisis/uix.git
    GIT_TAG 1.2.0
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 1.2.0
)
FetchContent_MakeAvailable(htcw_uix)

add_executable(2ascii 
    src/main.cpp
)
target_link_libraries(2ascii htcw_bits htcw_io htcw_ml htcw_data htcw_gfx)
target_include_directories(2ascii PUBLIC
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src"
  )
