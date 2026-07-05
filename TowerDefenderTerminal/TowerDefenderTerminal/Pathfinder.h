#pragma once
#include <vector>
#include "Grid.h"

// BFS pathfinding. Returns the route from the grid's spawn to its base as a
// list of cell indices, or an EMPTY vector if the base is unreachable.

// It takes the grid as a const reference: it reads the board but promises not
// to change it. Keeping this a free function (not a Grid method) keeps the
// algorithm independent and testable on its own.
std::vector<int> findPath(const Grid& grid);