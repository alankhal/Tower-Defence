#pragma once
#include <vector>
#include <random>
#include "Grid.h"    /* Game HAS a Grid as a member, so it needs the full type */
#include "Enemy.h"   /* createEnemy() returns one by value, so the same applies */

/*
    A running tally of the whole match, kept so the game can print a report at
    the end. The "= 0" on each line is a default member initialiser: it means
    every counter starts at zero without the constructor having to say so.
*/
struct GameStats {
    int enemiesSpawned = 0;
    int basicKilled = 0;
    int fastKilled = 0;
    int flyingKilled = 0;
    int reachedBase = 0;     /* enemies that got through and cost a life */
    int cannonsBuilt = 0;
    int antiAirBuilt = 0;
    int upgradesBought = 0;
    int goldEarned = 0;
    int goldSpent = 0;
};

/*
    Owns all game state and drives the whole match: it alternates build phases
    and enemy waves until the player wins (survives every wave) or loses (runs
    out of lives). main() just creates one of these and calls run().
*/
class Game {
public:
    Game(int columns, int rows);
    void run();                     /* top-level loop: waves until win or loss */

private:
    /* ---- Shared state (every method below can read/modify these) ---- */
    Grid grid;
    int  lives;
    int  currentWave;
    int  totalWaves;
    int  gold;                      /* spent on building and upgrading towers */
    GameStats stats;                /* everything the final report prints */
    std::mt19937 randomEngine;      /* the source of all the wave randomness */

    void runBuildPhase();           /* let the player build and upgrade (time frozen) */
    bool runWave(int enemyCount);   /* play one wave; returns false if lives hit 0 */

    /* ---- Small helpers used by the two phases above ---- */
    int  randomInt(int minimum, int maximum);   /* inclusive on both ends */
    int  rollEnemyType();                       /* picks basic, fast or flying */
    int  healthForType(int enemyType) const;    /* wave-scaled starting health */

    /* Fills in one enemy of the given type, ready to be added to a wave. */
    Enemy createEnemy(int enemyType, const std::vector<int>& route, int spawnDelay);

    /* True when this tower is allowed to shoot this enemy. This one rule is
       what makes anti-air towers necessary. */
    bool canShoot(const Tower& tower, const Enemy& enemy) const;

    void printStatus() const;       /* the wave / lives / gold readout */
    void printBuildMenu() const;    /* the list of build-phase commands */
    void printFinalReport() const;  /* the end-of-match summary */
};
