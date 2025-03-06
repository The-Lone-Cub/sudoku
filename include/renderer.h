#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "sudoku.h"

class Renderer {
public:
    static const int CELL_SIZE = 60;
    static const int WINDOW_WIDTH = CELL_SIZE * Sudoku::GRID_SIZE;
    static const int WINDOW_HEIGHT = CELL_SIZE * Sudoku::GRID_SIZE + 100; // Increased height to accommodate score display
    
    Renderer();
    ~Renderer();
    
    bool init();
    void render(const Sudoku& sudoku, int selectedRow = -1, int selectedCol = -1);
    void renderMessage(const std::string& message);
    void getGridPosition(int mouseX, int mouseY, int& row, int& col);
    void close();
    void renderScore(int score);
    void renderTimer(int elapsedSeconds);
    void renderVictoryScreen(int score, int elapsedSeconds);
    bool handleVictoryScreenClick(int x, int y);
    void renderHighGammaEffect();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    static SDL_Texture *cachedBackground;

    void renderGrid();
    void renderNumbers(const Sudoku& sudoku);
    void renderSelectedCell(int row, int col);
    void renderNumber(int number, int row, int col, bool isFixed);
    void renderNumberCounts(const Sudoku& sudoku);
    std::array<int, 9> calculateNumberCounts(const Sudoku& sudoku) const;
    void renderText(const std::string& text, int x, int y, SDL_Color color);
};

#endif // RENDERER_H