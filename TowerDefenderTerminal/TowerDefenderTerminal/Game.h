#pragma once
#include "Grid.h" // Game HAS a Grid as a member, so it needs the full type

// Owns all game state and drives the whole match: it alternates build phases
// and enemy waves until the player wins (survives every wave) or loses (runs
// out of lives). main() just creates one of these and calls run().
class Game {
public:
    Game(int columns, int rows);
    void run();                     // top-level loop: waves until win or loss

private:
    // ---- Shared state (every method below can read/modify these) ----
    Grid grid;
    int  lives;
    int  currentWave;
    int  totalWaves;

    void runBuildPhase();           // let the player place towers (time frozen)
    bool runWave(int enemyCount);   // play one wave; returns false if lives hit 0
};