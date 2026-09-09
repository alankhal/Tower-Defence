#pragma once
#include <vector>
#include "Grid.h"
#include "Enemy.h"

/* Clears the terminal so each frame of the wave redraws in place. */
void clearScreen();

/*
    Draws the static board, then stamps the enemies on top of it.
    Pass an empty enemy vector to draw just the board (used during the build
    phase). The path argument is only the walking route, drawn as a hint for
    the player; each enemy follows its own stored path, which is why flyers can
    appear off this line.
*/
void render(const Grid& grid, const std::vector<Enemy>& enemies, const std::vector<int>& path);
