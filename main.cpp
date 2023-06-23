#include <iostream>
#include <vector>
#include <fstream>  // file streaming classes
#include <string>
#include <sstream> // In C++ strings can be streamed into temporary variables , Streaming a string allows us to work with each character individually.
#include <cmath>
#include <algorithm>


using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream; //(i - in)
using std::istringstream; // process and store data (\can make it execute until stream fails)
using std::sort;

// node states on board
enum class State {kEmpty, kObstacle, kClosed, kPath, kStart, kFinish};
// directional deltas
const int delta[4][2]{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}; // will iterate over these delta values to check the neighbors in each direction

vector<State> ParseLine(string s);
vector<vector<State>> ReadBoardFile(string file_path);
void PrintBoard(vector<vector<State>> &board);
string CellString(State cell_state);

bool Compare(vector<int>, vector<int>);
void CellSort(vector<vector<int>> *v);
vector<vector<State>> Search(vector<vector<State>> &grid, int init[2], int goal[2]);
int Heuristic(int, int, int, int);
void AddToOpen(int, int, int, int, vector<vector<int>> &, vector<vector<State>> &);
bool CheckValidCell(int x, int y, vector<vector<State>> &);
void ExpandNeighbors(vector<int> &current,  int goal[2],vector<vector<int>> &open, vector<vector<State>> &grid);

int main() {
    int init[2] = {0,0};
    int goal[2] = {4,5};
    // a new input stream object can be declared and initialized using a file path
    vector<vector<State>> board = ReadBoardFile("../files/1.board");
    PrintBoard(board);
    vector<vector<State>> solution = Search(board, init, goal);
    PrintBoard(solution);
    return 0;
}

void PrintBoard(vector<vector<State>> &board){
    for ( vector<State> r: board){
        for ( State c : r){
            cout << CellString(c) << " ";
        }
        cout << endl;
    }
}

vector<vector<State>> ReadBoardFile(string file_path){
    /* read a file and prints each line */
    ifstream myfile(file_path);
    vector<vector<State>> full_board;

    // use object as bool to check if stream has been created successfully
    if (myfile){
        string line;
        while (getline(myfile, line)){
            full_board.push_back(ParseLine(line));
        }
    } else cout << "file stream not created successfully" << endl;
    return full_board;
}

vector<State> ParseLine(string s){
    /* accepts a string as an arg
     * parse the line by comma chars and store the ints ina vector<int>
     * return the vector after parsing
    */
    vector<State> board_row;
    int n;
    char c;
    istringstream my_stream(s); // stream string into temp vars n and c
    while (my_stream >> n >> c && c == ','){  // single quote for single character
        switch (n) {
            case 0:
                board_row.push_back(State::kEmpty);
                break;
            default:
                board_row.push_back(State::kObstacle);
                break;
        }
    }
    return board_row;
}

string CellString(State cell_state){
    // converts each state to an appropriate string
    switch (cell_state){
        case State::kObstacle: return "⛰️";
        case State::kPath: return "C   ";
        case State::kStart: return "S   ";
        case State::kFinish: return "G   ";
        default: return "0   ";
    }
}

int Heuristic(int x1, int y1, int x2, int y2){
    // Manhattan Distance
    return abs(x2-x1) + abs(y2 - y1);
}

/**
 * Add nodes to open vector and mark them as visited on the grid
 */
void AddToOpen(int x, int y, int g, int h, vector<vector<int>> &open, vector<vector<State>> &board){
    open.push_back({x, y, g, h});
    board[x][y] = State::kClosed;
}

vector<vector<State>> Search(vector<vector<State>> &grid, int init[2], int goal[2]) {
    // Create the vector of open nodes.
    vector<vector<int>> open {};

    // Initialize the starting node.
    int x = init[0];
    int y = init[1];
    int g = 0;                                  // increases with each EXPANSION
    int h = Heuristic(x, y, goal[0],goal[1]);  // Manhattan distance
    AddToOpen(x, y, g, h, open, grid);


    while (open.size() > 0){
        // Sort the open list using CellSort, and get the current node.
        CellSort(&open);
        auto current = open.back();         // the cell with lowest fscore
        open.pop_back();                   // take current cell off open list

        // Get the x and y values from the current node,
        int x = current[0];
        int y = current[1];
        grid[x][y] = State::kPath;

        //Check if you've reached the goal. If so, return grid.
        if (x == goal[0] && y == goal[1]) {
            cout << "goal reached" << endl;
            //Set the grid cell to kStart for the initial coordinates and kFinish for the goal coordinates
            grid[init[0]][init[1]] = State::kStart;
            grid[goal[0]][goal[1]] = State::kFinish;
            return grid;
        }

        // If we're not done, expand search to current node's neighbors.
        ExpandNeighbors( current, goal, open, grid);
    }

    // We've run out of new nodes to explore and haven't found a path.
    cout << "No path found!" << "\n";
    return std::vector<vector<State>>{};
}

/**
 * Compare the F values of two cells.
 */
bool Compare(vector<int> node1, vector<int> node2){
    int f1 = node1[2] + node1[3];
    int f2 = node2[2] + node2[3];
    return f1 > f2;
}

/**
 * Sort the two-dimensional vector of ints in descending order.
 */
void CellSort(vector<vector<int>> *v) {
    sort(v->begin(), v->end(), Compare); // the sort() function sorts the elements in ascending order.
}

/**
 * Check that neighboring grid cells are not closed, that they are not an obstacle, and are on the grid
 */
bool CheckValidCell(int x, int y, vector<vector<State>> &grid){
    // check that the (x,y) coordinate pair is on the grid
    int rows = grid.size(); // x
    int columns = grid[0].size(); // y
    if (!((x >= 0 && x < rows)&&(y >= 0 && y < columns))) return false;

    // Check that the grid at (x, y) is kEmpty (not close or an obstacle)
    return grid[x][y] == State::kEmpty;
}

/**
 * Expand current nodes's neighbors and add them to the open list.
 */
void ExpandNeighbors(vector<int> &current,  int goal[2],vector<vector<int>> &open, vector<vector<State>> &grid){
    // get current node's data.
    int x = current[0];
    int y = current[1];
    int g = current[2]; // g increases with each expansion

    // Loop through current node's potential neighbors (expand node).
    for (auto neighbor : delta){
        // Check that the potential neighbor's x2 and y2 values are on the grid and not closed.
        int xn = x + neighbor[0];
        int yn = y + neighbor[1];
        if (CheckValidCell( xn, yn, grid)){
            //Increment g value, compute h value, and add neighbor to open list.
            int hn = Heuristic( xn, yn, goal[0],goal[1]);
            AddToOpen(xn, yn, g+1, hn, open, grid);
        }
    }
}
