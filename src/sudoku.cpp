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
                if (!scored[row][col]) {
                    score += 5;  // Award 5 points for correct number
                    scored[row][col] = true;  // Mark cell as scored
                }
                wrong_answers[row][col] = 0;  // Reset wrong answers when correct
            } else {
                // Only avoid score reduction if it's the exact same number as the last attempt
                if (num != wrong_answers[row][col]) {
                    score -= 2;  // Deduct 2 points for wrong number
                }
                wrong_answers[row][col] = num;  // Track this wrong answer
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