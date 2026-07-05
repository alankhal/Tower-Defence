// ============================================================================
//  Game.cpp  —  the match controller (DEFINITIONS)
//
//  This file contains the TWO nested loops that make it a game:
//    - run()      = the OUTER loop over waves (build -> fight -> repeat)
//    - runWave()  = the INNER loop of ticks within a single wave
//  The outer loop ends when the player wins (all waves cleared) or loses
//  (lives reach 0).
// ============================================================================
#include "Game.h"
#include "Pathfinder.h"   // findPath()
#include "Renderer.h"     // render(), clearScreen()
#include "Enemy.h"        // Enemy
#include <iostream>
#include <vector>
#include <thread>         // sleep_for — paces the animation
#include <chrono>         // milliseconds
using namespace std;

// ---- Tunable combat values (change these to rebalance the game) ----
static const int ENEMY_HEALTH = 30;   // starting health of each enemy
static const int TOWER_DAMAGE = 10;   // health removed per tower, per tick, in range
static const int TOWER_RANGE = 2;    // tower reach, in cells (Euclidean radius)

// ---- Constructor: set up starting state ----
// The ':' list builds `grid` (calling its constructor) and sets the counters.
Game::Game(int columns, int rows)
    : grid(columns, rows), lives(20), currentWave(1), totalWaves(5)
{
}

// ---- run(): the OUTER game loop ----
void Game::run() {
    // Keep playing waves while the player is alive and waves remain.
    while (lives > 0 && currentWave <= totalWaves) {
        runBuildPhase();                          // let them build

        int enemyCount = 4 + currentWave * 2;     // each wave is a bit bigger
        bool survived = runWave(enemyCount);      // fight the wave
        if (!survived) break;                     // died mid-wave -> stop

        currentWave++;                            // on to the next wave
    }

    // Decide the outcome from why the loop ended.
    clearScreen();
    if (lives > 0)
        cout << "You survived all " << totalWaves << " waves. You win!\n";
    else
        cout << "The base has fallen. Game over on wave " << currentWave << ".\n";
}

// ---- runBuildPhase(): interactive tower placement ----
void Game::runBuildPhase() {
    while (true) {
        render(grid, {}, {});   // {} , {} = no enemies, no path -> just the board
        cout << "\nWave " << currentWave << "/" << totalWaves << " | Lives: " << lives
            << "\nPlace a tower: enter  col row  (or  -1  to start the wave): ";

        int col;
        if (!(cin >> col)) return;   // input ended
        if (col == -1) return;       // player chooses to start the wave
        int row;
        cin >> row;

        // Reject coordinates that are off the board.
        if (col < 0 || col >= grid.numColumns || row < 0 || row >= grid.numRows) {
            cout << "That cell is off the grid.\n";
            continue;
        }
        int idx = grid.indexOf(col, row);

        // Reject cells that already hold something.
        if (grid.cells[idx] != 0) {
            cout << "That cell is already occupied.\n";
            continue;
        }

        // GOLDEN RULE: tentatively place the tower, then ask the pathfinder
        // whether a route still exists. If not, undo it. (This is the same
        // findPath() used for navigation, reused here as a placement rule.)
        grid.cells[idx] = 1;
        if (findPath(grid).empty()) {
            grid.cells[idx] = 0;   // revert: it would wall off the base
            cout << "Can't build there: it would block the path to the base!\n";
        }
        else {
            cout << "Tower placed at (" << col << ", " << row << ").\n";
        }
    }
}

// ---- runWave(): the INNER tick loop for one wave ----
// Returns true if the player survived the wave, false if lives ran out.
bool Game::runWave(int enemyCount) {
    // Compute the route these enemies will follow (towers are already placed).
    vector<int> path = findPath(grid);
    int lastIndex = (int)path.size() - 1;   // index of the base within the path

    // Collect the tower cells ONCE up front. Towers don't move during a wave,
    // so there's no reason to rescan the whole board every tick.
    vector<int> towerCells;
    for (int i = 0; i < (int)grid.cells.size(); i++)
        if (grid.cells[i] == 1) towerCells.push_back(i);

    // Enemy health DOUBLES each wave: wave 1 = base, wave 2 = 2x, wave 3 = 4x...
    // (1 << n) is a compact way to write "2 to the power n", so
    // 1 << (currentWave - 1) gives 1, 2, 4, 8, ... for waves 1, 2, 3, 4, ...
    int waveHealth = ENEMY_HEALTH * (1 << (currentWave - 1));

    // Create the wave. Each enemy starts at this wave's health; the negative,
    // spaced pathIndex values stagger them so they enter one after another.
    vector<Enemy> enemies;
    for (int i = 0; i < enemyCount; i++) {
        Enemy e;
        e.pathIndex = -2 * i;         // enemy i enters 2 ticks after enemy i-1
        e.health = waveHealth;     // scaled up by the wave number
        e.reachedBase = false;
        enemies.push_back(e);
    }

    int killed = 0;   // enemies destroyed this wave (shown in the readout)

    // Tick until no enemy is still active (all either dead or arrived).
    bool waveOver = false;
    while (!waveOver) {
        // --- Draw the current frame ---
        clearScreen();
        render(grid, enemies, path);
        cout << "Wave " << currentWave << "/" << totalWaves
            << " | Lives: " << lives << " | Killed: " << killed << "\n";

        // --- COMBAT: every tower damages every enemy within its range ---
        for (int towerCell : towerCells) {
            int tcol = towerCell % grid.numColumns;
            int trow = towerCell / grid.numColumns;

            for (Enemy& e : enemies) {
                // Only target enemies that are alive and actually on the board.
                if (e.health <= 0 || e.reachedBase || e.pathIndex < 0) continue;

                // Work out the enemy's current (col, row) from its path cell.
                int cell = path[e.pathIndex];
                int ecol = cell % grid.numColumns;
                int erow = cell / grid.numColumns;

                int dc = tcol - ecol;
                int dr = trow - erow;
                // (dc*dc + dr*dr) is the SQUARED distance. Comparing squared
                // distances avoids a slow square root and does the same job.
                if (dc * dc + dr * dr <= TOWER_RANGE * TOWER_RANGE) {
                    e.health -= TOWER_DAMAGE;
                    if (e.health <= 0) killed++;   // this shot destroyed it
                }
            }
        }

        // --- MOVEMENT: advance every surviving enemy one step ---
        for (Enemy& e : enemies) {
            if (e.health <= 0 || e.reachedBase) continue;   // dead or already home
            e.pathIndex++;
            if (e.pathIndex >= lastIndex) {                 // arrived at the base
                e.pathIndex = lastIndex;
                e.reachedBase = true;
                lives--;                                    // it leaked through
                if (lives <= 0) return false;               // base fell -> loss
            }
        }

        // --- Wave ends once every enemy is dead or has reached the base ---
        waveOver = true;
        for (const Enemy& e : enemies)
            if (e.health > 0 && !e.reachedBase) waveOver = false;

        // --- Pace the animation so a human can watch it ---
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    return true;   // survived the wave
}