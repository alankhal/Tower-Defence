#pragma once
#include <vector>
#include "Grid.h"
#include "Enemy.h"

// Clears the terminal so each frame of the wave redraws in place.
void clearScreen();

// Draws the static board, then stamps the enemies on top of it.
// Pass empty vectors to draw just the board (used during the build phase).
void render(const Grid& grid, const std::vector<Enemy>& enemies, const std::vector<int>& path);
