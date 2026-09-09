/*
    Renderer.cpp - drawing the game to the terminal (DEFINITIONS)

    Key idea: enemies are NEVER written into the grid's data. The board stays a
    clean, permanent record; enemies are drawn as a temporary overlay on top of
    a fresh copy each frame. That "static world + entities drawn over it" split
    is exactly how real game rendering works.

    Every cell is drawn as a TWO character label rather than a single symbol.
    That extra character is what lets a tower show both its kind and its level
    at once, for example T2 for an upgraded cannon or A1 for a new anti-air.
*/
#include "Renderer.h"
#include <iostream>
#include <iomanip>    /* std::setw, used to line the coordinate headers up */
#include <string>
#include <cstdlib>    /* std::system, for the screen-clear command */
using namespace std;

/*
    Picks the label for an enemy, so the player can tell the three kinds apart
    at a glance. Each label is two characters wide to match the board cells.
*/
static string labelForEnemy(int enemyType) {
    if (enemyType == ENEMY_FAST)   return " >";
    if (enemyType == ENEMY_FLYING) return " ^";
    return " o";   /* ENEMY_BASIC */
}

/*
    Picks the label for a tower: its kind as a letter, then its level as a
    digit. T1/T2/T3 are cannons and A1/A2/A3 are anti-air.
*/
static string labelForTower(const Tower& tower) {
    string label;
    label += (tower.kind == TOWER_ANTI_AIR) ? 'A' : 'T';
    label += (char)('0' + tower.level);
    return label;
}

void clearScreen() {
    /*
        "cls" is the Windows clear-screen command, which is what Visual Studio
        will use. The #ifdef keeps the same code building and running on
        macOS/Linux, where the command is "clear".
    */
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void render(const Grid& grid, const vector<Enemy>& enemies, const vector<int>& path) {
    /* ---- 1) Build this frame's picture from the static board ---- */
    /* Translate each numeric cell code into the label we display for it. */
    vector<string> display(grid.cells.size());
    for (int i = 0; i < (int)grid.cells.size(); i++) {
        int value = grid.cells[i];
        string label = " .";                    /* 0 = empty */
        if (value == CELL_WALL)       label = " H";
        else if (value == CELL_SPAWN) label = " S";
        else if (value == CELL_BASE)  label = " B";
        display[i] = label;
    }

    /* ---- 2) Draw the walking route as a hint, without covering anything ---- */
    /* Only empty cells are marked, so the S and B stay readable. */
    for (int i = 0; i < (int)path.size(); i++) {
        if (display[path[i]] == " .") display[path[i]] = " *";
    }

    /* ---- 3) Draw the towers as kind + level ---- */
    /* Showing the level means the player can see at a glance which towers are
       already upgraded and which are still worth spending gold on. */
    for (int i = 0; i < (int)grid.towers.size(); i++) {
        int index = grid.indexOf(grid.towers[i].col, grid.towers[i].row);
        display[index] = labelForTower(grid.towers[i]);
    }

    /* ---- 4) Stamp the enemies on top ---- */
    /* Each enemy's cell comes from its OWN path, so flyers draw on their direct
       line while ground enemies draw on the route around the towers. We only
       draw ones that are alive, have entered the board, and have not arrived. */
    for (int i = 0; i < (int)enemies.size(); i++) {
        const Enemy& enemy = enemies[i];
        if (enemy.health <= 0) continue;
        if (enemy.reachedBase) continue;
        if (enemy.spawnDelay > 0) continue;
        if (enemy.pathIndex < 0 || enemy.pathIndex >= (int)enemy.path.size()) continue;

        display[enemy.path[enemy.pathIndex]] = labelForEnemy(enemy.type);
    }

    /* ---- 5) Print the column numbers along the top ---- */
    /* The player types coordinates during the build phase, so the board is
       printed with its column and row numbers to read them off. */
    cout << "\n    ";
    for (int col = 0; col < grid.numColumns; col++) {
        cout << setw(2) << col << " ";
    }
    cout << "\n";

    /* ---- 6) Print the finished picture, one grid row per line ---- */
    for (int row = 0; row < grid.numRows; row++) {
        cout << setw(3) << row << " ";
        for (int col = 0; col < grid.numColumns; col++) {
            cout << display[grid.indexOf(col, row)] << " ";
        }
        cout << "\n";
    }

    /* ---- 7) Legend, so none of the symbols need explaining ---- */
    cout << "\n  S spawn    B base    * walking route\n";
    cout << "  T1/T2/T3 cannon (ground only)    A1/A2/A3 anti-air (flying only)\n";
    cout << "  o basic    > fast    ^ flying (ignores towers, immune to cannons)\n";
}
