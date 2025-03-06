#include "sudoku.h"
#include <ctime>

Sudoku::Sudoku() : grid(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)),
                   solution(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)),
                   fixed(GRID_SIZE, std::vector<bool>(GRID_SIZE, false)),
                   score(0) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    generatePuzzle();
    initializeScore();
}

void Sudoku::generatePuzzle() {
    // Start with an empty grid
    for (auto& row : grid) {
        std::fill(row.begin(), row.end(), 0);
    }
    for (auto& row : fixed) {
        std::fill(row.begin(), row.end(), false);
    }

    // Fill diagonal boxes first (they are independent)
    for (int box = 0; box < GRID_SIZE; box += SUBGRID_SIZE) {
        std::vector<int> nums(GRID_SIZE);
        std::iota(nums.begin(), nums.end(), 1);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(nums.begin(), nums.end(), gen);
        
        for (int i = 0; i < SUBGRID_SIZE; i++) {
            for (int j = 0; j < SUBGRID_SIZE; j++) {
                grid[box + i][box + j] = nums[i * SUBGRID_SIZE + j];
                solution[box + i][box + j] = nums[i * SUBGRID_SIZE + j];
            }
        }
    }

    // Solve the rest of the grid
    solveGrid();

    // Mark all cells as fixed
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            fixed[i][j] = true;
        }
    }

    // Remove some numbers to create the puzzle
    removeCells();
}

bool Sudoku::solveGrid() {
    int row, col;
    
    if (!findEmptyCell(row, col)) {
        return true; // Puzzle is solved
    }

    std::vector<int> nums(GRID_SIZE);
    std::iota(nums.begin(), nums.end(), 1);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(nums.begin(), nums.end(), gen);

    for (int num : nums) {
        if (isValid(row, col, num)) {
            grid[row][col] = num;
            solution[row][col] = num;

            if (solveGrid()) {
                return true;
            }

            grid[row][col] = 0; // Backtrack
            solution[row][col] = 0;
        }
    }

    return false;
}

bool Sudoku::findEmptyCell(int& row, int& col) const {
    for (row = 0; row < GRID_SIZE; row++) {
        for (col = 0; col < GRID_SIZE; col++) {
            if (grid[row][col] == 0) {
                return true;
            }
        }
    }
    return false;
}

void Sudoku::removeCells() {
    // Remove about 50-60 numbers
    int cellsToRemove = 45 + (std::rand() % 11);
    while (cellsToRemove > 0) {
        int row = std::rand() % GRID_SIZE;
        int col = std::rand() % GRID_SIZE;
        
        if (grid[row][col] != 0) {
            grid[row][col] = 0;
            fixed[row][col] = false;
            cellsToRemove--;
        }
    }
}

bool Sudoku::isValid(int row, int col, int num) const {
    return isValidInRow(row, num) && 
           isValidInCol(col, num) && 
           isValidInBox(row - row % SUBGRID_SIZE, col - col % SUBGRID_SIZE, num);
}

bool Sudoku::isValidInRow(int row, int num) const {
    for (int col = 0; col < GRID_SIZE; col++) {
        if (grid[row][col] == num) {
            return false;
        }
    }
    return true;
}

bool Sudoku::isValidInCol(int col, int num) const {
    for (int row = 0; row < GRID_SIZE; row++) {
        if (grid[row][col] == num) {
            return false;
        }
    }
    return true;
}

bool Sudoku::isValidInBox(int startRow, int startCol, int num) const {
    for (int row = 0; row < SUBGRID_SIZE; row++) {
        for (int col = 0; col < SUBGRID_SIZE; col++) {
            if (grid[row + startRow][col + startCol] == num) {
                return false;
            }
        }
    }
    return true;
}

bool Sudoku::isCellEditable(int row, int col) const {
    return !fixed[row][col];
}

bool Sudoku::setNumber(int row, int col, int num) {
    if (!isCellEditable(row, col)) {
        return false;
    }

    // Temporarily remove the current number to check validity
    int temp = grid[row][col];
    grid[row][col] = 0;

    if (num == 0 || isValid(row, col, num)) {
        grid[row][col] = num;
        
        // Update score based on correctness
        if (num != 0) {
            if (isCorrectNumber(row, col, num)) {
                score += 5;  // Award 5 points for correct number
            } else {
                score -= 2;  // Deduct 2 points for wrong number
            }
        }
        
        return true;
    }

    grid[row][col] = temp;
    return false;
}

int Sudoku::getNumber(int row, int col) const {
    return grid[row][col];
}

void Sudoku::initializeScore() {
    score = 0;
    // Add 5 points for each initial clue
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (fixed[i][j]) {
                score += 5;
            }
        }
    }
}

bool Sudoku::isCorrectNumber(int row, int col, int num) const {
    return solution[row][col] == num;
}

bool Sudoku::isSolved() const {
    // Check if all cells are filled and match the solution
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0 || grid[i][j] != solution[i][j]) {
                return false;
            }
        }
    }
    return true;
}