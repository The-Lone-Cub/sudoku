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
    void generatePuzzle();
    bool isValid(int row, int col, int num) const;
    bool isCellEditable(int row, int col) const;
    bool setNumber(int row, int col, int num);
    int getNumber(int row, int col) const;
    bool isSolved() const;

private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<bool>> fixed;
    
    bool solveGrid();
    bool findEmptyCell(int& row, int& col) const;
    void removeCells();
    bool isValidInRow(int row, int num) const;
    bool isValidInCol(int col, int num) const;
    bool isValidInBox(int startRow, int startCol, int num) const;
};

#endif // SUDOKU_H