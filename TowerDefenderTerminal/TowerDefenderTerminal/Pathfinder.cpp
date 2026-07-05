#include "Pathfinder.h"
#include <queue>
#include <algorithm>   // std::reverse
using namespace std;

vector<int> findPath(const Grid& grid) {
    int start = grid.indexOf(grid.spawnCol, grid.spawnRow);
    int goal = grid.indexOf(grid.baseCol, grid.baseRow);

    int totalCells = grid.numColumns * grid.numRows;
    vector<bool> visited(totalCells, false);
    vector<int>  parent(totalCells, -1);   // breadcrumb: which cell we arrived from
    queue<int>   frontier;

    visited[start] = true;
    frontier.push(start);

    // Four moves: up, down, left, right — as (dcol, drow) pairs.
    int dcol[4] = { 0,  0, -1, 1 };
    int drow[4] = { -1, 1,  0, 0 };

    while (!frontier.empty()) {
        int current = frontier.front();
        frontier.pop();
        if (current == goal) break;   // reached base, stop searching

        int col = current % grid.numColumns;
        int row = current / grid.numColumns;

        for (int i = 0; i < 4; i++) {
            int nextCol = col + dcol[i];
            int nextRow = row + drow[i];

            // Off the edge of the grid?
            if (nextCol < 0 || nextCol >= grid.numColumns) continue;
            if (nextRow < 0 || nextRow >= grid.numRows)    continue;

            int next = grid.indexOf(nextCol, nextRow);

            // Blocked by a tower (1) or a wall (3)?
            if (grid.cells[next] == 1 || grid.cells[next] == 3) continue;

            // Already reached this cell earlier?
            if (visited[next]) continue;

            // First time here: remember where we came from, then queue it.
            visited[next] = true;
            parent[next] = current;
            frontier.push(next);
        }
    }

    // If the base was never reached, there is no path.
    if (goal != start && parent[goal] == -1) {
        return vector<int>();
    }

    // Reconstruct the route by walking breadcrumbs backwards from goal to start.
    vector<int> path;
    int step = goal;
    while (step != -1) {          // parent of start is -1, so this stops after start
        path.push_back(step);
        step = parent[step];
    }
    reverse(path.begin(), path.end());   // flip goal->start into start->goal
    return path;
}