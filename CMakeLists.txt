# Build script for the terminal tower defence game.
#
# Visual Studio: File > Open > Folder, pick this folder, and Visual Studio
# configures the project by itself. Pick TowerDefence.exe as the startup item
# and press F5.
#
# Command line (any platform):
#     cmake -S . -B build
#     cmake --build build

cmake_minimum_required(VERSION 3.15)

project(TowerDefence LANGUAGES CXX)

# C++17, and required rather than optional so the build fails loudly on a
# compiler that cannot supply it instead of quietly falling back.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TowerDefence
    main.cpp
    Game.cpp
    Grid.cpp
    Pathfinder.cpp
    Renderer.cpp
    Game.h
    Grid.h
    Tower.h
    Enemy.h
    Pathfinder.h
    Renderer.h
)

# Warnings on. The project is expected to build with none of them firing.
if(MSVC)
    target_compile_options(TowerDefence PRIVATE /W4)
else()
    target_compile_options(TowerDefence PRIVATE -Wall -Wextra)
endif()
