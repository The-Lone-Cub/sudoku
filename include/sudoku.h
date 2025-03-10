#ifndef SUDOKU_H
#define SUDOKU_H

#include <vector>
#include <random>
#include <algorithm>

class Sudoku {
public:
    static const int GRID_SIZE = 9;
    static const int SUBGRID_SIZE = 3;

    Sudoku();
    bool setNumber(int row, int col, int num);
    int getNumber(int row, int col) const;
    bool isCellEditable(int row, int col) const;
    bool isSolved() const;
    int getScore() const { return score; }
    void setHighlightedNumber(int num) {
        if (num == highlightedNumber && highlightedVisible) {
            highlightedVisible = false;
        } else {
            highlightedNumber = num;
            highlightedVisible = true;
        }
    }

    bool isValid(int row, int col, int num) const;
    bool isHighlightVisible() const { return highlightedVisible; }
    int getHighlightedNumber() const { return highlightedNumber; }

private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> solution;
    std::vector<std::vector<bool>> fixed;
    std::vector<std::vector<bool>> scored;
    std::vector<std::vector<int>> wrong_answers;
    int score;
    int correctInputs;
    int totalAttempts;
    int highlightedNumber;
    bool highlightedVisible;

    void generatePuzzle();
    bool solveGrid();
    bool findEmptyCell(int &row, int &col) const;
    void removeCells();
    int countSolutions(int count);
    bool isValidInRow(int row, int num) const;
    bool isValidInCol(int col, int num) const;
    bool isValidInBox(int startRow, int startCol, int num) const;
    void initializeScore();
    bool isRowComplete(int row) const;
    bool isColumnComplete(int col) const;
    bool isBoxComplete(int startRow, int startCol) const;
    int getPenaltyForDifficulty() const;
    float getAccuracyPercentage() const;
private:
    bool findEmptyCellInGrid(const std::vector<std::vector<int>>& checkGrid, int& row, int& col) const;
    bool isValidInGrid(const std::vector<std::vector<int>>& checkGrid, int row, int col, int num) const;
    int countSolutionsRecursive(std::vector<std::vector<int>>& checkGrid, int count);
};

#endif // SUDOKU_H