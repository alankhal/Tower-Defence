#pragma once
#include <vector>
#include "Grid.h"

/*
    BFS pathfinding. Both functions return the route from the grid's spawn to
    its base as a list of cell indices, or an EMPTY vector if the base is
    unreachable.

    They take the grid as a const reference: they read the board but promise
    not to change it. Keeping these free functions (not Grid methods) keeps the
    algorithm independent and testable on its own.
*/

/*
    The ground route: walks around towers and walls. This is the path basic and
    fast enemies follow, and it is also the route the build rules protect - a
    tower that would make this come back empty is not allowed to be built.
*/
std::vector<int> findPath(const Grid& grid);

/*
    The flying route: towers are ignored completely, so flyers cut straight
    across the maze the player has built. Walls still block them, because walls
    are terrain rather than something the player placed.
*/
std::vector<int> findFlyingPath(const Grid& grid);
