#pragma once
#include <vector>
#include "Tower.h"

/*
    The game board: a flat, row-major array of cell codes plus the spawn and
    base coordinates. The grid also owns the list of towers standing on it, so
    that anything drawing or reading the board has one place to look.

    This class is data plus coordinate maths plus the building rules. It
    deliberately knows NOTHING about drawing itself - that is the renderer's job.
*/

/*
    Cell codes. These are named constants instead of bare numbers so that the
    rest of the code never has to guess what a value like 4 means.
*/
const int CELL_EMPTY = 0;
const int CELL_TOWER = 1;
const int CELL_WALL = 3;
const int CELL_SPAWN = 4;
const int CELL_BASE = 5;

class Grid {
public:
    int numColumns;
    int numRows;
    std::vector<int> cells;
    int spawnCol, spawnRow;
    int baseCol, baseRow;

    /* Every tower built so far. The matching cell also holds CELL_TOWER, so
       the pathfinder can stay simple and only ever look at the cell codes. */
    std::vector<Tower> towers;

    Grid(int columns, int rows);

    /* Converts a (col, row) coordinate into the matching flat index in cells. */
    int indexOf(int col, int row) const;

    /* The two conversions back the other way, from a flat index to a coordinate. */
    int colOf(int index) const;
    int rowOf(int index) const;

    /* True when the coordinate is actually on the board. */
    bool inBounds(int col, int row) const;

    /* Position of a tower inside the towers vector, or -1 if that cell has none. */
    int towerIndexAt(int col, int row) const;

    /* Would putting a tower here seal the base off from the spawn? */
    bool wouldBlockBase(int col, int row) const;

    /* All the placement rules in one place: on the board, empty, and not a
       placement that would trap the enemies away from the base. */
    bool canBuildAt(int col, int row) const;

    /* Places a level 1 tower of the given kind (TOWER_CANNON or TOWER_ANTI_AIR).
       Returns false and changes nothing if the rules above say the cell is not
       a legal build spot. Both kinds are solid, so both are walls that ground
       enemies must walk around. */
    bool buildTower(int col, int row, int kind);

    /* Raises the tower at this cell by one level, improving its range and its
       damage. Returns false if there is no tower here or it is already maxed. */
    bool upgradeTower(int col, int row);
};
