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

private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> solution;
    std::vector<std::vector<bool>> fixed;
    int score;

    void generatePuzzle();
    bool solveGrid();
    bool findEmptyCell(int &row, int &col) const;
    void removeCells();
    bool isValid(int row, int col, int num) const;
    bool isValidInRow(int row, int num) const;
    bool isValidInCol(int col, int num) const;
    bool isValidInBox(int startRow, int startCol, int num) const;
    bool isCorrectNumber(int row, int col, int num) const;
    void initializeScore();
};

#endif // SUDOKU_H