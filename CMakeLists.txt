cmake_minimum_required(VERSION 3.23)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_C_COMPILER ${CMAKE_SOURCE_DIR}/gbdk/bin/lcc)
project(OofProject
    VERSION 1.0
    DESCRIPTION "A brief CMake experiment"
    LANGUAGES C)
set(CMAKE_C_FLAGS "-Wl-m -Wl-j -Wm-yS -Wm-yn\"OOF\" -debug")
add_executable(oof.gb
    src/oof.c
)
target_include_directories(oof.gb
    PRIVATE
    gbdk/include
)
add_custom_target(run
    gbmulator oof.gb
    DEPENDS oof.gb
)
