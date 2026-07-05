#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>   // std::reverse
#include <thread>      // std::this_thread::sleep_for
#include <chrono>      // std::chrono::milliseconds
using namespace std;

class Grid {
public:
    int numColumns;
    int numRows;
    std::vector<int> cells;   // flat 1D array, row-major — the plain-C++ version of Unreal's TArray
    int spawnCol, spawnRow; //Enemy Position
    int baseCol, baseRow; //Base Position



    //Initialize list in the beggining to get the proper count of the columns and rows 
    Grid(int columns, int rows) : numColumns(columns), numRows(rows), cells(columns * rows, 0) {
        spawnCol = 0; 
        spawnRow = numRows / 2; 

        baseCol = numColumns - 1; 
        baseRow = numRows / 2; 

        cells[indexOf(spawnCol, spawnRow)] = 4;
        cells[indexOf(baseCol, baseRow)] = 5;
    };

    // Converts a (col, row) coordinate into the matching flat index in cells.
    int indexOf(int col, int row) const {
        return row * numColumns + col; 
    }

    //Prints the grid out 
    void print() const {
        for (int row = 0; row < numRows; row++) {         // outer: which row (line)
            for (int col = 0; col < numColumns; col++) {  // inner: walk across the row
                if (cells[indexOf(col, row)] == 0) {
                    cout << '.';
                } 
                else if (cells[indexOf(col, row)] == 1) { //1 For Towers
                    cout << '#';
                }
                else if (cells[indexOf(col, row)] == 2) { //2 For Enemies 
                    cout << '<'; 
                }
                else if (cells[indexOf(col, row)] == 3) { //3 For Walls 
                    cout << 'H';
                }
                else if (cells[indexOf(col, row)] == 4) { //4 For Spawn
                    cout << 'S';
                }
                else if (cells[indexOf(col, row)] == 5) { //5 For Home Base 
                    cout << 'B';
                }
                else if (cells[indexOf(col, row)] == 6) {
                    cout << '*'; 
                }

            }
            cout << '\n';   // row finished -> drop to the next line
        }
    }
};


//Using BFS algorithm to find end point of grid 
vector<int> findPath(const Grid& grid) {
    //Set Up starting point for the search 
    int start = grid.indexOf(grid.spawnCol, grid.spawnRow);
    int goal = grid.indexOf(grid.baseCol, grid.baseRow);

    //Establish the total cells contained in our grid, as well as our storing devices 
    int totalCells = grid.numColumns * grid.numRows;
    vector<bool> visited(totalCells, false);
    vector<int> parent(totalCells, -1); //Parent needed in order to reverse the path for the enemies later on  
    queue<int> frontier;

    //Begin the search at the spawn cell 
    visited[start] = true;
    frontier.push(start);

    //Directions in up,down,left,right in respective row terms 
    int dcol[4] = { 0, 0, -1, 1 };
    int drow[4] = { -1, 1, 0, 0 };

    while (!frontier.empty()) {
        int current = frontier.front();
        frontier.pop();

        if (current == goal) break; //Reached Base, no need to keep searching  

        int col = current % grid.numColumns;
        int row = current / grid.numColumns;



        // Off the edge of the grid?
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
        return vector<int>();   // empty vector = no path
    }

    vector<int> path;
    int step = goal;
    while (step != -1) {        // parent of start is -1, so this stops after start
        path.push_back(step);
        step = parent[step];
    }
    reverse(path.begin(), path.end());  // flip goal->start into start->goal
    return path;
}

// A moving enemy is just a position ALONG the path.
struct Enemy {
    int  pathIndex;    // how far along the path; negative = still waiting to spawn
    bool reachedBase;  // true once it arrives at B (counted once)
};

// Clears the terminal so each frame redraws in place.
void clearScreen() {
    system("cls");     // Windows. Use system("clear"); on macOS/Linux.
}


// Draws the static board with the enemies overlaid on top.
void render(const Grid& grid, const vector<Enemy>& enemies, const vector<int>& path) {

    // 1) Build the base picture from the grid's static cells.
    vector<char> display(grid.cells.size());
    for (int i = 0; i < (int)grid.cells.size(); i++) {
        int v = grid.cells[i];
        char c = '.';
        if (v == 1) c = '#';
        else if (v == 3) c = 'H';
        else if (v == 4) c = 'S';
        else if (v == 5) c = 'B';
        else if (v == 6) c = '*';
        display[i] = c;
    }

    // 2) Stamp each on-grid enemy on top as 'o'.
    for (const Enemy& e : enemies) {
        if (!e.reachedBase && e.pathIndex >= 0 && e.pathIndex < (int)path.size()) {
            display[path[e.pathIndex]] = 'o';
        }
    }
    // 3) Print the finished picture.
    for (int row = 0; row < grid.numRows; row++) {
        for (int col = 0; col < grid.numColumns; col++) {
            cout << display[grid.indexOf(col, row)];
        }
        cout << '\n';
    }
}



int main() {
    Grid grid(16, 16);

    // ---- BUILD PHASE: let the user place towers ----
    while (true) {
        grid.print();
        cout << "\nPlace a tower: enter  col row  (or  -1  to start the wave): ";

        int col;
        if (!(cin >> col)) break;   // stop if input ends
        if (col == -1) break;

        int row;
        cin >> row;

        // Validate the coordinate.
        if (col < 0 || col >= grid.numColumns || row < 0 || row >= grid.numRows) {
            cout << "That cell is off the grid.\n";
            continue;
        }
        int idx = grid.indexOf(col, row);
        if (grid.cells[idx] != 0) {
            cout << "That cell is already occupied.\n";
            continue;
        }

        // GOLDEN RULE: tentatively place, then check a path still exists.
        grid.cells[idx] = 1;
        if (findPath(grid).empty()) {
            grid.cells[idx] = 0;   // revert — it would wall off the base
            cout << "Can't build there: it would block the path to the base!\n";
        }
        else {
            cout << "Tower placed at (" << col << ", " << row << ").\n";
        }
    }

    // ---- Compute the route the enemies will follow ----
    vector<int> path = findPath(grid);
    if (path.empty()) {
        cout << "No path to the base — cannot start the wave.\n";
        return 0;
    }

    // ---- Spawn a wave of enemies, staggered so they don't overlap ----
    vector<Enemy> enemies;
    int numEnemies = 6;
    for (int i = 0; i < numEnemies; i++) {
        Enemy e;
        e.pathIndex = -2 * i;   // each enters 2 ticks after the previous
        e.reachedBase = false;
        enemies.push_back(e);
    }

    // ---- WAVE PHASE: the game loop ----
    int lives = 20;
    int lastIndex = (int)path.size() - 1;
    bool waveOver = false;

    while (!waveOver) {
        clearScreen();
        render(grid, enemies, path);
        cout << "Lives: " << lives << "\n";

        // Advance every enemy one step along the path.
        for (Enemy& e : enemies) {
            if (e.reachedBase) continue;
            e.pathIndex++;
            if (e.pathIndex >= lastIndex) {
                e.pathIndex = lastIndex;
                e.reachedBase = true;
                lives--;               // it leaked through to the base
            }
        }

        // The wave is over once every enemy has reached the base.
        waveOver = true;
        for (const Enemy& e : enemies) {
            if (!e.reachedBase) waveOver = false;
        }

        this_thread::sleep_for(chrono::milliseconds(200));
    }

    clearScreen();
    render(grid, enemies, path);
    cout << "Wave complete. Lives remaining: " << lives << "\n";
    return 0;
}