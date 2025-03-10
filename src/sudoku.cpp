#include "sudoku.h"
#include "difficulty_settings.h"
#include <ctime>

Sudoku::Sudoku() : grid(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)),
           solution(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)),
           fixed(GRID_SIZE, std::vector<bool>(GRID_SIZE, false)),
           scored(GRID_SIZE, std::vector<bool>(GRID_SIZE, false)),
           wrong_answers(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)),
           score(0),
           highlightedNumber(0),
           highlightedVisible(false) {
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
    std::vector<std::pair<int, int>> cells;
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            cells.emplace_back(i, j);
        }
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(cells.begin(), cells.end(), gen);
    
    // Use difficulty slider to determine target clues
    // Update this line:
    float sliderValue = DifficultySettings::getDifficultySlider()->value;
    // Interpolate between hard (20-30) and easy (40-50)
    int minClues = static_cast<int>(20 + sliderValue * 20); // 20-40
    int maxClues = static_cast<int>(30 + sliderValue * 20); // 30-50
    int targetClues = minClues + (std::rand() % (maxClues - minClues + 1));
    
    int currentClues = GRID_SIZE * GRID_SIZE;
    
    // Initialize cluesPerRegion with current counts
    std::vector<int> cluesPerRegion(9, 9);
    std::vector<int> cluesPerRow(9, 9);    // Track clues in each row
    std::vector<int> cluesPerCol(9, 9);    // Track clues in each column
    
    // Adjust minimum clues per region/row/col based on difficulty
    int minRegionClues = static_cast<int>(2 + sliderValue * 3); // 2-5 clues per region
    int minRowColClues = static_cast<int>(2 + sliderValue * 2); // 2-4 clues per row/col
    
    for (const auto& cell : cells) {
        if (currentClues <= targetClues) break;
        
        int row = cell.first;
        int col = cell.second;
        int region = (row / 3) * 3 + (col / 3);
        int temp = grid[row][col];
        
        if (temp == 0) continue;
        
        // Ensure minimum clues in regions, rows, and columns based on difficulty
        if (cluesPerRegion[region] <= minRegionClues) continue;
        if (cluesPerRow[row] <= minRowColClues) continue;
        if (cluesPerCol[col] <= minRowColClues) continue;
        
        grid[row][col] = 0;
        
        // For harder difficulties (lower slider value), allow more complex solving techniques
        int maxSolutions = (sliderValue < 0.3) ? 2 : 1; // Allow multiple solutions for hard difficulty
        int solutions = countSolutions(0);
        
        if (solutions > maxSolutions || solutions == 0) {
            grid[row][col] = temp;
            fixed[row][col] = true;
        } else {
            fixed[row][col] = false;
            currentClues--;
            cluesPerRegion[region]--;
            cluesPerRow[row]--;
            cluesPerCol[col]--;
        }
    }
}

// Helper method to count solutions (add this to the class)
int Sudoku::countSolutions(int count) {
    if (count > 1) return 2;  // We only need to know if it's not unique
    
    // Create a temporary grid for solution checking
    std::vector<std::vector<int>> tempGrid = grid;
    
    int row, col;
    if (!findEmptyCellInGrid(tempGrid, row, col)) return count + 1;
    
    for (int num = 1; num <= GRID_SIZE; num++) {
        if (isValidInGrid(tempGrid, row, col, num)) {
            tempGrid[row][col] = num;
            count = countSolutionsRecursive(tempGrid, count);
            tempGrid[row][col] = 0;
        }
    }
    
    return count;
}

// Add these helper methods
bool Sudoku::findEmptyCellInGrid(const std::vector<std::vector<int>>& checkGrid, int& row, int& col) const {
    for (row = 0; row < GRID_SIZE; row++) {
        for (col = 0; col < GRID_SIZE; col++) {
            if (checkGrid[row][col] == 0) {
                return true;
            }
        }
    }
    return false;
}

bool Sudoku::isValidInGrid(const std::vector<std::vector<int>>& checkGrid, int row, int col, int num) const {
    // Check row
    for (int x = 0; x < GRID_SIZE; x++) {
        if (checkGrid[row][x] == num) return false;
    }
    
    // Check column
    for (int x = 0; x < GRID_SIZE; x++) {
        if (checkGrid[x][col] == num) return false;
    }
    
    // Check box
    int startRow = row - row % SUBGRID_SIZE;
    int startCol = col - col % SUBGRID_SIZE;
    for (int i = 0; i < SUBGRID_SIZE; i++) {
        for (int j = 0; j < SUBGRID_SIZE; j++) {
            if (checkGrid[i + startRow][j + startCol] == num) return false;
        }
    }
    
    return true;
}

int Sudoku::countSolutionsRecursive(std::vector<std::vector<int>>& checkGrid, int count) {
    if (count > 1) return 2;
    
    int row, col;
    if (!findEmptyCellInGrid(checkGrid, row, col)) return count + 1;
    
    for (int num = 1; num <= GRID_SIZE; num++) {
        if (isValidInGrid(checkGrid, row, col, num)) {
            checkGrid[row][col] = num;
            count = countSolutionsRecursive(checkGrid, count);
            checkGrid[row][col] = 0;
        }
    }
    
    return count;
}
bool Sudoku::isValid(int row, int col, int num) const {
    // Temporarily store and remove the current number
    int currentNum = grid[row][col];
    const_cast<Sudoku*>(this)->grid[row][col] = 0;

    bool valid = isValidInRow(row, num) &&
                isValidInCol(col, num) &&
                isValidInBox(row - row % SUBGRID_SIZE, col - col % SUBGRID_SIZE, num);

    // Restore the original number
    const_cast<Sudoku*>(this)->grid[row][col] = currentNum;

    return valid;
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

    totalAttempts++;
    // Set the new value
    grid[row][col] = num;
    
    if (num != 0) {
        if (num == solution[row][col]) {
            correctInputs++;
            score += 5;
            
            // Check for completed sections and award bonuses
            if (!scored[row][col]) {
                scored[row][col] = true;
                
                // Check row completion
                if (isRowComplete(row)) {
                    score += 10;
                }
                
                // Check column completion
                if (isColumnComplete(col)) {
                    score += 10;
                }
                
                // Check box completion
                int startRow = row - (row % SUBGRID_SIZE);
                int startCol = col - (col % SUBGRID_SIZE);
                if (isBoxComplete(startRow, startCol)) {
                    score += 10;
                }
            }
        } else {
            // Apply difficulty-based penalty for incorrect answers
            score -= getPenaltyForDifficulty();
            wrong_answers[row][col]++;
        }
    }

    return true;
}

bool Sudoku::isRowComplete(int row) const {
    for (int col = 0; col < GRID_SIZE; col++) {
        if (grid[row][col] != solution[row][col]) {
            return false;
        }
    }
    return true;
}

bool Sudoku::isColumnComplete(int col) const {
    for (int row = 0; row < GRID_SIZE; row++) {
        if (grid[row][col] != solution[row][col]) {
            return false;
        }
    }
    return true;
}

bool Sudoku::isBoxComplete(int startRow, int startCol) const {
    for (int i = 0; i < SUBGRID_SIZE; i++) {
        for (int j = 0; j < SUBGRID_SIZE; j++) {
            if (grid[startRow + i][startCol + j] != solution[startRow + i][startCol + j]) {
                return false;
            }
        }
    }
    return true;
}

int Sudoku::getPenaltyForDifficulty() const {
    float difficulty = DifficultySettings::getDifficultySlider()->value;
    if (difficulty >= 0.7) { // Easy
        return 1;
    } else if (difficulty >= 0.3) { // Medium
        return 2;
    } else { // Hard
        return 3;
    }
}

float Sudoku::getAccuracyPercentage() const {
    if (totalAttempts == 0) return 0.0f;
    return (static_cast<float>(correctInputs) / totalAttempts) * 100.0f;
}

void Sudoku::initializeScore() {
    score = 0;
    correctInputs = 0;
    totalAttempts = 0;
    
    // Initialize scored array
    for (auto& row : scored) {
        std::fill(row.begin(), row.end(), false);
    }
    
    // Initialize wrong_answers array
    for (auto& row : wrong_answers) {
        std::fill(row.begin(), row.end(), 0);
    }
}

int Sudoku::getNumber(int row, int col) const {
    return grid[row][col];
}

bool Sudoku::isSolved() const {
    // Check if all cells are filled
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0) {
                return false;
            }
        }
    }

    // Check each row for uniqueness
    for (int row = 0; row < GRID_SIZE; row++) {
        std::vector<bool> used(GRID_SIZE + 1, false);
        for (int col = 0; col < GRID_SIZE; col++) {
            int num = grid[row][col];
            if (used[num]) return false;
            used[num] = true;
        }
    }

    // Check each column for uniqueness
    for (int col = 0; col < GRID_SIZE; col++) {
        std::vector<bool> used(GRID_SIZE + 1, false);
        for (int row = 0; row < GRID_SIZE; row++) {
            int num = grid[row][col];
            if (used[num]) return false;
            used[num] = true;
        }
    }

    // Check each 3x3 box for uniqueness
    for (int boxRow = 0; boxRow < GRID_SIZE; boxRow += SUBGRID_SIZE) {
        for (int boxCol = 0; boxCol < GRID_SIZE; boxCol += SUBGRID_SIZE) {
            std::vector<bool> used(GRID_SIZE + 1, false);
            for (int i = 0; i < SUBGRID_SIZE; i++) {
                for (int j = 0; j < SUBGRID_SIZE; j++) {
                    int num = grid[boxRow + i][boxCol + j];
                    if (used[num]) return false;
                    used[num] = true;
                }
            }
        }
    }

    return true;
}