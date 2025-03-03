#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "sudoku.h"

class Renderer {
public:
    static const int CELL_SIZE = 60;
    static const int WINDOW_SIZE = CELL_SIZE * Sudoku::GRID_SIZE;
    
    Renderer();
    ~Renderer();
    
    bool init();
    void render(const Sudoku& sudoku, int selectedRow = -1, int selectedCol = -1);
    void renderMessage(const std::string& message);
    void getGridPosition(int mouseX, int mouseY, int& row, int& col);
    void close();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    
    void renderGrid();
    void renderNumbers(const Sudoku& sudoku);
    void renderSelectedCell(int row, int col);
    void renderNumber(int number, int row, int col, bool isFixed);
};

#endif // RENDERER_H