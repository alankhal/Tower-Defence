// ============================================================================
//  Renderer.cpp  —  drawing the game to the terminal (DEFINITIONS)
//
//  Key idea: enemies are NEVER written into the grid's data. The board stays a
//  clean, permanent record; enemies are drawn as a temporary overlay on top of
//  a fresh copy each frame. That "static world + entities drawn over it" split
//  is exactly how real game rendering works.
// ============================================================================
#include "Renderer.h"
#include <iostream>
#include <cstdlib>    // std::system, for the screen-clear command
using namespace std;

void clearScreen() {
    // "cls" is the Windows clear-screen command. On macOS/Linux use "clear".
    system("cls");
}

void render(const Grid& grid, const vector<Enemy>& enemies, const vector<int>& path) {
    // ---- 1) Build this frame's picture from the static board ----
    // Translate each numeric cell code into the character we display for it.
    vector<char> display(grid.cells.size());
    for (int i = 0; i < (int)grid.cells.size(); i++) {
        int v = grid.cells[i];
        char c = '.';               // 0 = empty
        if (v == 1)      c = '#';    // tower
        else if (v == 3) c = 'H';    // wall
        else if (v == 4) c = 'S';    // spawn
        else if (v == 5) c = 'B';    // base
        else if (v == 6) c = '*';    // path
        display[i] = c;
    }

    // ---- 2) Stamp the enemies on top ----
    // Each enemy's cell is path[pathIndex]. We only draw ones that have
    // spawned (pathIndex >= 0) and haven't yet reached the base.
    for (const Enemy& e : enemies) {
        if (e.health > 0 && !e.reachedBase &&
            e.pathIndex >= 0 && e.pathIndex < (int)path.size()) {
            display[path[e.pathIndex]] = 'o';
        }
    }

    // ---- 3) Print the finished picture, one grid row per line ----
    for (int row = 0; row < grid.numRows; row++) {
        for (int col = 0; col < grid.numColumns; col++) {
            cout << display[grid.indexOf(col, row)];
        }
        cout << '\n';
    }
}