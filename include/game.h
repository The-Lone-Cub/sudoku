#ifndef GAME_H
#define GAME_H

#include "renderer.h"
#include "sudoku.h"

class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();

private:
    Renderer renderer;
    Sudoku sudoku;
    bool running;
    int selectedRow;
    int selectedCol;

    void handleEvents();
    void handleMouseClick(int x, int y);
    void handleKeyPress(SDL_Keycode key);
    void checkWinCondition();
    void initializeScore();
};

#endif // GAME_H