/*
    Pathfinder.cpp - breadth-first search from the spawn to the base.

    Ground enemies and flying enemies need the same search with one difference:
    whether towers count as solid. So the search is written once here, with an
    ignoreTowers switch, and the two public functions just call it.
*/
#include "Pathfinder.h"
#include <queue>
#include <algorithm>   /* std::reverse */
using namespace std;

/*
    The shared search. BFS visits cells in rings outwards from the spawn, so
    the first time it reaches the base it has done so in the fewest steps.
    ignoreTowers = true makes towers walkable, which is what flying enemies do.

    static here means "private to this file": nothing outside Pathfinder.cpp
    can call it, which keeps the header clean.
*/
static vector<int> searchRoute(const Grid& grid, bool ignoreTowers) {
    int start = grid.indexOf(grid.spawnCol, grid.spawnRow);
    int goal = grid.indexOf(grid.baseCol, grid.baseRow);

    int totalCells = grid.numColumns * grid.numRows;
    vector<bool> visited(totalCells, false);
    vector<int>  parent(totalCells, -1);   /* breadcrumb: which cell we arrived from */
    queue<int>   frontier;

    visited[start] = true;
    frontier.push(start);

    /* Four moves: up, down, left, right - as (dcol, drow) pairs. */
    int dcol[4] = { 0,  0, -1, 1 };
    int drow[4] = { -1, 1,  0, 0 };

    while (!frontier.empty()) {
        int current = frontier.front();
        frontier.pop();
        if (current == goal) break;   /* reached base, stop searching */

        int col = grid.colOf(current);
        int row = grid.rowOf(current);

        for (int i = 0; i < 4; i++) {
            int nextCol = col + dcol[i];
            int nextRow = row + drow[i];

            /* Off the edge of the grid? */
            if (!grid.inBounds(nextCol, nextRow)) continue;

            int next = grid.indexOf(nextCol, nextRow);

            /* Walls always block. Towers block only for ground enemies. */
            if (grid.cells[next] == CELL_WALL) continue;
            if (!ignoreTowers && grid.cells[next] == CELL_TOWER) continue;

            /* Already reached this cell earlier? */
            if (visited[next]) continue;

            /* First time here: remember where we came from, then queue it. */
            visited[next] = true;
            parent[next] = current;
            frontier.push(next);
        }
    }

    /* If the base was never reached, there is no path. */
    if (goal != start && parent[goal] == -1) {
        return vector<int>();
    }

    /* Reconstruct the route by walking breadcrumbs backwards from goal to start. */
    vector<int> path;
    int step = goal;
    while (step != -1) {          /* parent of start is -1, so this stops after start */
        path.push_back(step);
        step = parent[step];
    }
    reverse(path.begin(), path.end());   /* flip goal->start into start->goal */
    return path;
}

/* The walking route: towers are solid and have to be walked around. */
vector<int> findPath(const Grid& grid) {
    return searchRoute(grid, false);
}

/* The flying route: towers are ignored, so flyers take the direct line. */
vector<int> findFlyingPath(const Grid& grid) {
    return searchRoute(grid, true);
}
