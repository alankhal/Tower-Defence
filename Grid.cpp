/*
    Grid.cpp - the board's data and the rules about what may be built on it.
*/
#include "Grid.h"
#include "Pathfinder.h"   /* findPath(), used to protect the route to the base */

/*
    Builds an empty board and marks the two fixed cells: the spawn on the left
    edge and the base on the right edge, both in the middle row.
*/
Grid::Grid(int columns, int rows)
    : numColumns(columns), numRows(rows), cells(columns* rows, CELL_EMPTY)
{
    spawnCol = 0;
    spawnRow = numRows / 2;
    baseCol = numColumns - 1;
    baseRow = numRows / 2;

    cells[indexOf(spawnCol, spawnRow)] = CELL_SPAWN;   /* mark spawn (S) */
    cells[indexOf(baseCol, baseRow)] = CELL_BASE;      /* mark base  (B) */
}

/* Turns a coordinate into the matching flat index in cells. */
int Grid::indexOf(int col, int row) const {
    return row * numColumns + col;
}

/* Turns a flat index back into its column. */
int Grid::colOf(int index) const {
    return index % numColumns;
}

/* Turns a flat index back into its row. */
int Grid::rowOf(int index) const {
    return index / numColumns;
}

/* True only when the coordinate sits inside the board. */
bool Grid::inBounds(int col, int row) const {
    if (col < 0 || col >= numColumns) return false;
    if (row < 0 || row >= numRows)    return false;
    return true;
}

/*
    Searches the towers vector for one standing on this cell.
    Returns its position in the vector, or -1 when the cell holds no tower.
*/
int Grid::towerIndexAt(int col, int row) const {
    for (int i = 0; i < (int)towers.size(); i++) {
        if (towers[i].col == col && towers[i].row == row) return i;
    }
    return -1;
}

/*
    THE ANTI-BLOCKING RULE.

    Copy the whole board, drop the tower onto the COPY, then ask the pathfinder
    whether the base can still be reached. An empty path means this placement
    would wall the base off, so it must be refused. The real board is never
    touched, so a rejected placement leaves no trace behind.
*/
bool Grid::wouldBlockBase(int col, int row) const {
    Grid testBoard = *this;
    testBoard.cells[testBoard.indexOf(col, row)] = CELL_TOWER;
    return findPath(testBoard).empty();
}

/* Every build rule collected in one place, so the game loop just asks once. */
bool Grid::canBuildAt(int col, int row) const {
    if (!inBounds(col, row)) return false;

    /* Anything that is not empty is taken: towers, walls, the spawn, the base. */
    if (cells[indexOf(col, row)] != CELL_EMPTY) return false;

    if (wouldBlockBase(col, row)) return false;

    return true;
}

/*
    Places a new level 1 tower, recording it both in the cells array (so the
    pathfinder treats it as solid) and in the towers vector (so combat and
    upgrades can find its stats).
*/
bool Grid::buildTower(int col, int row, int kind) {
    if (!canBuildAt(col, row)) return false;

    Tower tower;
    tower.kind = kind;
    tower.col = col;
    tower.row = row;
    tower.level = 1;
    tower.range = TOWER_START_RANGE;
    tower.damage = TOWER_START_DAMAGE;

    towers.push_back(tower);
    cells[indexOf(col, row)] = CELL_TOWER;
    return true;
}

/*
    Upgrades an existing tower: one level up, with more range and more damage.
    The tower does not move, so the board's cell codes do not change and the
    path is unaffected.
*/
bool Grid::upgradeTower(int col, int row) {
    int index = towerIndexAt(col, row);
    if (index < 0) return false;                              /* nothing here */
    if (towers[index].level >= TOWER_MAX_LEVEL) return false; /* already maxed */

    towers[index].level += 1;
    towers[index].range += TOWER_RANGE_PER_LEVEL;
    towers[index].damage += TOWER_DAMAGE_PER_LEVEL;
    return true;
}
