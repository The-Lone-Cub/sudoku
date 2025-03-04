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
    
    static int getElapsedSeconds() { return currentElapsedSeconds; }
private:
    Renderer renderer;
    Sudoku sudoku;
    bool running;
    int selectedRow;
    int selectedCol;
    Uint32 startTime;
    int elapsedSeconds;

    void handleEvents();
    void handleMouseClick(int x, int y);
    void handleKeyPress(SDL_Keycode key);
    void checkWinCondition();
    void initializeScore();
    void updateTimer();
    
private:
    static int currentElapsedSeconds;  // Static member to store current elapsed seconds
};

#endif // GAME_H