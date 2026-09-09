#pragma once
#include <vector>

/*
    Enemy.h - the data for one moving enemy.

    This is a pure data type (no functions), so it lives ENTIRELY in a header
    with no matching .cpp file. There is nothing to compile separately, so it
    can never cause a linker error. Rule of thumb: data-only structs are
    header-only.

    An enemy does not store its own (col, row). Instead it stores its route and
    how far along that route it is, so its actual cell is always path[pathIndex].
    Each enemy carries its OWN copy of the route, which is what lets flying
    enemies take a different line across the board from ground enemies.
*/

/*
    The three kinds of enemy a wave can be made of.

    BASIC  - ordinary speed, most health, walks the maze around the towers.
    FAST   - moves every tick, but has less health. Rushes past slow towers.
    FLYING - ignores towers completely and flies the direct route to the base,
             so towers can only hit it if they happen to cover its flight line.
*/
const int ENEMY_BASIC = 0;
const int ENEMY_FAST = 1;
const int ENEMY_FLYING = 2;

struct Enemy {
    int  type;         /* ENEMY_BASIC, ENEMY_FAST or ENEMY_FLYING */
    int  health;       /* hit points. Towers reduce this; at 0 the enemy dies
                          and stops moving, being drawn, and being targeted. */
    int  spawnDelay;   /* ticks left before it walks onto the board. This
                          staggers the wave so enemies do not all stack up. */
    int  moveDelay;    /* ticks it waits between steps. LOWER = FASTER. */
    int  moveTimer;    /* counts up to moveDelay, then the enemy takes a step. */
    int  pathIndex;    /* how far along its own path it currently is. */
    bool reachedBase;  /* set true once it arrives at the base, so each arrival
                          is only counted against the player's lives one time. */

    std::vector<int> path;   /* this enemy's own route, as a list of cell
                                indices from spawn through to the base. */
};
