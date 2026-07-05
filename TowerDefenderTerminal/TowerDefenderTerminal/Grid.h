#pragma once
#include <vector>

// The game board: a flat, row-major array of cell codes plus the spawn and
// base coordinates. This class is pure data + coordinate math. It deliberately
// knows NOTHING about drawing itself — that's the renderer's job.
//
// Cell codes:  0 = empty   1 = tower   3 = wall   4 = spawn   5 = base   6 = path
class Grid {
public:
    int numColumns;
    int numRows;
    std::vector<int> cells;
    int spawnCol, spawnRow;
    int baseCol, baseRow;

    Grid(int columns, int rows);

    // Converts a (col, row) coordinate into the matching flat index in cells.
    int indexOf(int col, int row) const;
};