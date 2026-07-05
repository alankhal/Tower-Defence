#include "Grid.h"

Grid::Grid(int columns, int rows)
    : numColumns(columns), numRows(rows), cells(columns* rows, 0)
{
    spawnCol = 0;
    spawnRow = numRows / 2;
    baseCol = numColumns - 1;
    baseRow = numRows / 2;

    cells[indexOf(spawnCol, spawnRow)] = 4;   // mark spawn (S)
    cells[indexOf(baseCol, baseRow)] = 5;   // mark base  (B)
}

int Grid::indexOf(int col, int row) const {
    return row * numColumns + col;
}