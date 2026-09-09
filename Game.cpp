/*
    Game.cpp - the match controller (DEFINITIONS)

    This file contains the TWO nested loops that make it a game:
      - run()      = the OUTER loop over waves (build -> fight -> repeat)
      - runWave()  = the INNER loop of ticks within a single wave
    The outer loop ends when the player wins (all waves cleared) or loses
    (lives reach 0).
*/
#include "Game.h"
#include "Pathfinder.h"   /* findPath(), findFlyingPath() */
#include "Renderer.h"     /* render(), clearScreen() */
#include <iostream>
#include <string>
#include <vector>
#include <thread>         /* sleep_for - paces the animation */
#include <chrono>         /* milliseconds */
#include <ctime>          /* time(), used to seed the random engine */
using namespace std;

/*
    ---- Tunable match values (change these to rebalance the game) ----
    static here means "private to this file".
*/
static const int STARTING_LIVES = 15;
static const int STARTING_GOLD = 12;
static const int TOTAL_WAVES = 5;

static const int CANNON_COST = 5;    /* gold for a tower that shoots the ground */
static const int ANTI_AIR_COST = 6;    /* gold for a tower that shoots the air */
static const int UPGRADE_COST = 4;    /* gold to raise one tower by a level */
static const int GOLD_PER_KILL = 1;    /* gold earned for destroying an enemy */
static const int GOLD_PER_WAVE = 4;    /* gold earned for surviving a wave */

static const int FRAME_DELAY_MS = 150;  /* how long each tick is drawn for */
static const int SPAWN_GAP_TICKS = 4;    /* ticks between enemies entering */

/*
    ---- Tunable enemy values ----
    START health is what the type has on wave 1; GROWTH is added for every wave
    after that, so later waves are steadily tougher. DELAY is how many ticks the
    enemy waits between steps, so a LOWER delay means a FASTER enemy.
*/
static const int BASIC_START_HEALTH = 30;
static const int BASIC_HEALTH_GROWTH = 15;
static const int BASIC_MOVE_DELAY = 3;

static const int FAST_START_HEALTH = 18;
static const int FAST_HEALTH_GROWTH = 9;
static const int FAST_MOVE_DELAY = 1;

static const int FLYING_START_HEALTH = 24;
static const int FLYING_HEALTH_GROWTH = 12;
static const int FLYING_MOVE_DELAY = 2;

/*
    ---- Constructor: set up starting state ----
    The ':' list builds `grid` (calling its constructor) and sets the counters.
    The random engine is seeded from the clock so every playthrough gets a
    different mix of enemies.
*/
Game::Game(int columns, int rows)
    : grid(columns, rows),
    lives(STARTING_LIVES),
    currentWave(1),
    totalWaves(TOTAL_WAVES),
    gold(STARTING_GOLD),
    randomEngine((unsigned int)time(nullptr))
{
}

/* ---- Returns a random whole number between minimum and maximum, inclusive ---- */
int Game::randomInt(int minimum, int maximum) {
    uniform_int_distribution<int> spread(minimum, maximum);
    return spread(randomEngine);
}

/*
    ---- Picks what the next enemy will be ----
    The mix is weighted rather than an even three-way split: ordinary enemies
    are the backbone of a wave, while the awkward ones show up often enough to
    matter without being the whole fight.
*/
int Game::rollEnemyType() {
    int roll = randomInt(1, 100);
    if (roll <= 50) return ENEMY_BASIC;    /* 50% of the time */
    if (roll <= 75) return ENEMY_FAST;     /* 25% of the time */
    return ENEMY_FLYING;                   /* the remaining 25% */
}

/*
    ---- Starting health for one enemy ----
    Each type has its own base value and its own growth rate, so the wave
    number alone decides how tough this enemy is.
*/
int Game::healthForType(int enemyType) const {
    int wavesPassed = currentWave - 1;

    if (enemyType == ENEMY_FAST) {
        return FAST_START_HEALTH + wavesPassed * FAST_HEALTH_GROWTH;
    }
    if (enemyType == ENEMY_FLYING) {
        return FLYING_START_HEALTH + wavesPassed * FLYING_HEALTH_GROWTH;
    }
    return BASIC_START_HEALTH + wavesPassed * BASIC_HEALTH_GROWTH;
}

/*
    ---- Fills in one ready-to-use enemy ----
    The route is handed in rather than looked up here, because flying enemies
    are given the flying route and everyone else is given the walking route.
*/
Enemy Game::createEnemy(int enemyType, const vector<int>& route, int spawnDelay) {
    Enemy enemy;
    enemy.type = enemyType;
    enemy.health = healthForType(enemyType);
    enemy.spawnDelay = spawnDelay;
    enemy.moveTimer = 0;
    enemy.pathIndex = 0;
    enemy.reachedBase = false;
    enemy.path = route;

    /* Speed is the whole point of the fast type, so it is set from the type. */
    if (enemyType == ENEMY_FAST)        enemy.moveDelay = FAST_MOVE_DELAY;
    else if (enemyType == ENEMY_FLYING) enemy.moveDelay = FLYING_MOVE_DELAY;
    else                                enemy.moveDelay = BASIC_MOVE_DELAY;

    return enemy;
}

/*
    ---- THE TARGETING RULE ----
    Flying enemies are untouchable by cannons, and anti-air towers waste their
    time on anything walking. Splitting targets this way is what stops one long
    line of cannons from answering every threat in the game.
*/
bool Game::canShoot(const Tower& tower, const Enemy& enemy) const {
    bool enemyIsFlying = (enemy.type == ENEMY_FLYING);

    if (tower.kind == TOWER_ANTI_AIR) return enemyIsFlying;
    return !enemyIsFlying;   /* a cannon: ground targets only */
}

/* ---- The one-line readout shown above the board in both phases ---- */
void Game::printStatus() const {
    cout << "Wave " << currentWave << "/" << totalWaves
        << "   Lives: " << lives
        << "   Gold: " << gold << "\n";
}

/* ---- The build-phase command list ---- */
void Game::printBuildMenu() const {
    cout << "\n  Tower levels: ";
    for (int level = 1; level <= TOWER_MAX_LEVEL; level++) {
        cout << level
            << ") range " << TOWER_START_RANGE + (level - 1) * TOWER_RANGE_PER_LEVEL
            << " damage " << TOWER_START_DAMAGE + (level - 1) * TOWER_DAMAGE_PER_LEVEL
            << "   ";
    }
    cout << "\n\n  Commands:\n";
    cout << "    b <col> <row>   build a cannon   (" << CANNON_COST << " gold, ground only)\n";
    cout << "    a <col> <row>   build anti-air   (" << ANTI_AIR_COST << " gold, flying only)\n";
    cout << "    u <col> <row>   upgrade a tower  (" << UPGRADE_COST << " gold, either kind)\n";
    cout << "    s               start the wave\n";
}

/* ---- The end-of-match summary ---- */
void Game::printFinalReport() const {
    int totalKilled = stats.basicKilled + stats.fastKilled + stats.flyingKilled;

    /* Guard the division: a match can end before anything has spawned. */
    int stoppedPercent = 0;
    if (stats.enemiesSpawned > 0) {
        stoppedPercent = (totalKilled * 100) / stats.enemiesSpawned;
    }

    cout << "\n===== FINAL REPORT =====\n";
    cout << "  Waves survived    : " << (currentWave - 1) << " of " << totalWaves << "\n";
    cout << "  Lives remaining   : " << (lives > 0 ? lives : 0) << " of " << STARTING_LIVES << "\n";
    cout << "\n";
    cout << "  Enemies spawned   : " << stats.enemiesSpawned << "\n";
    cout << "  Destroyed         : " << totalKilled
        << "  (basic " << stats.basicKilled
        << ", fast " << stats.fastKilled
        << ", flying " << stats.flyingKilled << ")\n";
    cout << "  Reached the base  : " << stats.reachedBase << "\n";
    cout << "  Stopped           : " << stoppedPercent << "% of everything that spawned\n";
    cout << "\n";
    cout << "  Cannons built     : " << stats.cannonsBuilt << "\n";
    cout << "  Anti-air built    : " << stats.antiAirBuilt << "\n";
    cout << "  Upgrades bought   : " << stats.upgradesBought << "\n";
    cout << "  Gold earned/spent : " << stats.goldEarned << " / " << stats.goldSpent << "\n";
    cout << "========================\n";
}

/* ---- run(): the OUTER game loop ---- */
void Game::run() {
    /* Keep playing waves while the player is alive and waves remain. */
    while (lives > 0 && currentWave <= totalWaves) {
        runBuildPhase();                          /* let them build and upgrade */

        int enemyCount = 4 + currentWave * 2;     /* each wave is a bit bigger */
        bool survived = runWave(enemyCount);      /* fight the wave */
        if (!survived) break;                     /* died mid-wave -> stop */

        /* Surviving pays, and later waves pay more, so upgrades stay affordable. */
        int reward = GOLD_PER_WAVE + currentWave;
        gold += reward;
        stats.goldEarned += reward;

        cout << "\n  Wave " << currentWave << " cleared. Gold is now " << gold << ".\n";
        this_thread::sleep_for(chrono::milliseconds(1500));

        currentWave++;                            /* on to the next wave */
    }

    /* Decide the outcome from why the loop ended, then show the summary. */
    clearScreen();
    if (lives > 0)
        cout << "You survived all " << totalWaves << " waves. You win!\n";
    else
        cout << "The base has fallen. Game over on wave " << currentWave << ".\n";

    printFinalReport();
}

/* ---- runBuildPhase(): interactive building and upgrading ---- */
void Game::runBuildPhase() {
    /*
        The screen is cleared every time round the loop, so the result of the
        last command is kept in this string and reprinted with the new frame.
        Otherwise the player would never see what went wrong.
    */
    string message = "Build your defences, then start the wave.";

    while (true) {
        clearScreen();
        cout << "===== BUILD PHASE =====\n";
        printStatus();
        render(grid, vector<Enemy>(), findPath(grid));   /* board only, no enemies */
        printBuildMenu();
        cout << "\n  " << message << "\n> ";

        string command;
        if (!(cin >> command)) return;      /* input ended (end of file) */

        /* Accept the commands in upper or lower case. */
        char action = command[0];
        if (action >= 'A' && action <= 'Z') action = action - 'A' + 'a';

        if (action == 's') return;          /* player starts the wave */

        if (action != 'b' && action != 'a' && action != 'u') {
            message = "Unknown command. Use b, a, u or s.";
            continue;
        }

        /* All three remaining commands take a coordinate, so read it once here. */
        int col = 0;
        int row = 0;
        if (!(cin >> col >> row)) {
            cin.clear();                 /* clear the failed-read flag */
            cin.ignore(10000, '\n');     /* throw away the rest of the bad line */
            message = "Enter a command and a coordinate, for example: b 3 4";
            continue;
        }

        /* ---- BUILD (either kind) ---- */
        if (action == 'b' || action == 'a') {
            bool wantsAntiAir = (action == 'a');
            int cost = wantsAntiAir ? ANTI_AIR_COST : CANNON_COST;
            int kind = wantsAntiAir ? TOWER_ANTI_AIR : TOWER_CANNON;
            string name = wantsAntiAir ? "Anti-air" : "Cannon";

            if (gold < cost) {
                message = "Not enough gold for that tower.";
            }
            else if (!grid.inBounds(col, row)) {
                message = "That cell is off the grid.";
            }
            else if (grid.cells[grid.indexOf(col, row)] != CELL_EMPTY) {
                message = "That cell is already occupied.";
            }
            else if (grid.wouldBlockBase(col, row)) {
                /*
                    THE GOLDEN RULE. The grid checks the placement against the
                    pathfinder before accepting it, so the player can never
                    seal the base off from the spawn.
                */
                message = "Can't build there: it would block the path to the base!";
            }
            else {
                grid.buildTower(col, row, kind);
                gold -= cost;
                stats.goldSpent += cost;
                if (wantsAntiAir) stats.antiAirBuilt++;
                else              stats.cannonsBuilt++;

                message = name + " built at (" + to_string(col) + ", " + to_string(row) + ").";
            }
        }
        /* ---- UPGRADE (works on either kind) ---- */
        else {
            int towerIndex = grid.towerIndexAt(col, row);
            if (towerIndex < 0) {
                message = "There is no tower on that cell.";
            }
            else if (grid.towers[towerIndex].level >= TOWER_MAX_LEVEL) {
                message = "That tower is already at the maximum level.";
            }
            else if (gold < UPGRADE_COST) {
                message = "Not enough gold for an upgrade.";
            }
            else {
                grid.upgradeTower(col, row);
                gold -= UPGRADE_COST;
                stats.goldSpent += UPGRADE_COST;
                stats.upgradesBought++;

                message = "Tower at (" + to_string(col) + ", " + to_string(row) +
                    ") is now level " + to_string(grid.towers[towerIndex].level) + ".";
            }
        }
    }
}

/* ---- runWave(): the INNER tick loop for one wave ---- */
/* Returns true if the player survived the wave, false if lives ran out. */
bool Game::runWave(int enemyCount) {
    /*
        Both routes are worked out ONCE, before the fighting starts. Towers
        cannot be built during a wave, so neither route can change while it
        runs, and every enemy just copies whichever one suits it.
    */
    vector<int> groundRoute = findPath(grid);
    vector<int> flyingRoute = findFlyingPath(grid);

    /* Build the whole wave up front: a random type each, staggered entries. */
    vector<Enemy> enemies;
    for (int i = 0; i < enemyCount; i++) {
        int type = rollEnemyType();

        /* This is the line that makes flyers ignore towers: they are handed a
           route that was searched with the towers treated as thin air. */
        const vector<int>& route = (type == ENEMY_FLYING) ? flyingRoute : groundRoute;

        enemies.push_back(createEnemy(type, route, i * SPAWN_GAP_TICKS));
        stats.enemiesSpawned++;
    }

    int killed = 0;   /* enemies destroyed this wave (shown in the readout) */

    /* Tick until no enemy is still active (all either dead or arrived). */
    bool waveOver = false;
    while (!waveOver) {
        /* --- Draw the current frame --- */
        clearScreen();
        cout << "===== WAVE " << currentWave << " =====\n";
        printStatus();
        render(grid, enemies, groundRoute);
        cout << "\n  Destroyed this wave: " << killed << "\n";

        /* --- COMBAT: every tower damages every valid target within range --- */
        for (int t = 0; t < (int)grid.towers.size(); t++) {
            const Tower& tower = grid.towers[t];

            for (int e = 0; e < (int)enemies.size(); e++) {
                Enemy& enemy = enemies[e];

                /* Only target enemies that are alive and actually on the board. */
                if (enemy.health <= 0) continue;
                if (enemy.reachedBase) continue;
                if (enemy.spawnDelay > 0) continue;
                if (enemy.pathIndex >= (int)enemy.path.size()) continue;

                /* Cannons cannot reach the air, anti-air cannot reach the ground. */
                if (!canShoot(tower, enemy)) continue;

                /* Work out the enemy's current (col, row) from its path cell. */
                int cell = enemy.path[enemy.pathIndex];
                int columnGap = tower.col - grid.colOf(cell);
                int rowGap = tower.row - grid.rowOf(cell);

                /*
                    (columnGap*columnGap + rowGap*rowGap) is the SQUARED distance.
                    Comparing squared distances avoids a slow square root and
                    does exactly the same job. An upgraded tower has a bigger
                    range and a bigger damage, so both sides of this improve.
                */
                if (columnGap * columnGap + rowGap * rowGap <= tower.range * tower.range) {
                    enemy.health -= tower.damage;

                    if (enemy.health <= 0) {
                        killed++;                /* this shot destroyed it */
                        gold += GOLD_PER_KILL;   /* and paid for the next tower */
                        stats.goldEarned += GOLD_PER_KILL;

                        if (enemy.type == ENEMY_FAST)        stats.fastKilled++;
                        else if (enemy.type == ENEMY_FLYING) stats.flyingKilled++;
                        else                                 stats.basicKilled++;
                    }
                }
            }
        }

        /* --- MOVEMENT: advance every surviving enemy that is due a step --- */
        for (int e = 0; e < (int)enemies.size(); e++) {
            Enemy& enemy = enemies[e];
            if (enemy.health <= 0 || enemy.reachedBase) continue;   /* dead or home */

            /* Still waiting to walk on? Count down and do nothing else. */
            if (enemy.spawnDelay > 0) {
                enemy.spawnDelay--;
                continue;
            }

            /*
                Speed control. Every enemy is ticked at the same rate, but a
                fast enemy has a moveDelay of 1 so it steps every single tick,
                while a basic enemy waits 3 ticks between steps.
            */
            enemy.moveTimer++;
            if (enemy.moveTimer < enemy.moveDelay) continue;
            enemy.moveTimer = 0;

            int lastIndex = (int)enemy.path.size() - 1;   /* the base, on this route */
            enemy.pathIndex++;

            if (enemy.pathIndex >= lastIndex) {           /* arrived at the base */
                enemy.pathIndex = lastIndex;
                enemy.reachedBase = true;
                lives--;                                  /* it leaked through */
                stats.reachedBase++;
                if (lives <= 0) return false;             /* base fell -> loss */
            }
        }

        /* --- Wave ends once every enemy is dead or has reached the base --- */
        waveOver = true;
        for (int e = 0; e < (int)enemies.size(); e++) {
            if (enemies[e].health > 0 && !enemies[e].reachedBase) waveOver = false;
        }

        /* --- Pace the animation so a human can watch it --- */
        this_thread::sleep_for(chrono::milliseconds(FRAME_DELAY_MS));
    }

    return true;   /* survived the wave */
}
