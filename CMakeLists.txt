cmake_minimum_required(VERSION 3.15...3.30)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    SudokuSolver
  VERSION 0.1
  LANGUAGES C)

set(SOURCES
    src/main.c
    src/tui.c
    src/graphics.c
)

add_executable(tui ${SOURCES})

target_include_directories(tui
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)

