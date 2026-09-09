#pragma once

/*
    Tower.h - the data for one tower.

    Like Enemy, this is a pure data type (no functions), so it lives ENTIRELY
    in a header with no matching .cpp file. Nothing to compile separately means
    it can never cause a linker error.

    A tower keeps its own level, range and damage rather than looking them up
    from a table. That way an upgraded tower carries its improved stats with
    it, and the combat loop only has to read tower.range and tower.damage.
*/

/*
    The two kinds of tower, and the rule that makes them worth choosing between:

    CANNON   - shoots ground enemies only. It cannot touch anything in the air.
    ANTI_AIR - shoots flying enemies only. It ignores everything on the ground.

    Neither kind covers the whole game on its own, so the player has to split
    their gold between the two and decide where each one is worth putting.
*/
const int TOWER_CANNON = 0;
const int TOWER_ANTI_AIR = 1;

/*
    Tower balance values, all in one place so the game is easy to re-tune.
    A freshly built tower starts at level 1 with the START values; every
    upgrade adds the PER_LEVEL values on top. Both kinds share this table.

    Level 1: range 2, damage 3
    Level 2: range 3, damage 6
    Level 3: range 4, damage 9
*/
const int TOWER_MAX_LEVEL = 3;
const int TOWER_START_RANGE = 2;
const int TOWER_START_DAMAGE = 3;
const int TOWER_RANGE_PER_LEVEL = 1;
const int TOWER_DAMAGE_PER_LEVEL = 3;

struct Tower {
    int kind;     /* TOWER_CANNON or TOWER_ANTI_AIR */
    int col;      /* column this tower stands in */
    int row;      /* row this tower stands in */
    int level;    /* 1 up to TOWER_MAX_LEVEL */
    int range;    /* how far it can shoot, measured in cells */
    int damage;   /* health removed from every valid target in range, each tick */
};
