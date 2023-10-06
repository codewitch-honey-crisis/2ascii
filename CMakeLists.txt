cmake_minimum_required(VERSION 3.24)

project(2ascii VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_subdirectory(lib/htcw_bits)
add_subdirectory(lib/htcw_io)
add_subdirectory(lib/htcw_ml)
add_subdirectory(lib/htcw_data)
add_subdirectory(lib/htcw_gfx)

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
