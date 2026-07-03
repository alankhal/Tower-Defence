#include <iostream>
#include <vector>
#include <queue>
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

            }
            cout << '\n';   // row finished -> drop to the next line
        }
    }
};


//Using BFS algorithm to find end point of grid 
vector<int> findPath(const Grid& grid) {




}



int main() {
    Grid grid(16, 16);
    grid.print();
    return 0;
}