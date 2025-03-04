#include "game.h"
#include <SDL2/SDL.h>

int Game::currentElapsedSeconds = 0;  // Initialize static member

Game::Game() : running(false), selectedRow(-1), selectedCol(-1), startTime(0), elapsedSeconds(0) {
}

Game::~Game() {}

bool Game::init() {
    if (!renderer.init()) {
        return false;
    }
    running = true;
    startTime = SDL_GetTicks();
    return true;
}

void Game::run() {
    while (running) {
        handleEvents();
        updateTimer();
        renderer.render(sudoku, selectedRow, selectedCol);
        SDL_Delay(16); // Cap at ~60 FPS
    }
}

void Game::updateTimer() {
    if (running) {
        Uint32 currentTime = SDL_GetTicks();
        elapsedSeconds = (currentTime - startTime) / 1000;
        currentElapsedSeconds = elapsedSeconds;  // Update static member
        renderer.renderTimer(elapsedSeconds);
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    handleMouseClick(event.button.x, event.button.y);
                }
                break;
            case SDL_KEYDOWN:
                handleKeyPress(event.key.keysym.sym);
                break;
        }
    }
}

void Game::handleMouseClick(int x, int y) {
    int newRow, newCol;
    renderer.getGridPosition(x, y, newRow, newCol);
    
    // If clicking the same cell that's already selected, unselect it
    if (newRow == selectedRow && newCol == selectedCol) {
        selectedRow = selectedCol = -1;
        return;
    }
    
    // Otherwise, try to select the new cell if it's editable
    if (sudoku.isCellEditable(newRow, newCol)) {
        selectedRow = newRow;
        selectedCol = newCol;
    } else {
        selectedRow = selectedCol = -1;
    }
}

void Game::handleKeyPress(SDL_Keycode key) {
    if (selectedRow == -1 || selectedCol == -1) return;

    if (key >= SDLK_1 && key <= SDLK_9) {
        int number = key - SDLK_0;
        if (sudoku.setNumber(selectedRow, selectedCol, number)) {
            checkWinCondition();
        }
    } else if (key == SDLK_BACKSPACE || key == SDLK_DELETE) {
        sudoku.setNumber(selectedRow, selectedCol, 0);
    }
}

void Game::checkWinCondition() {
    if (sudoku.isSolved()) {
        renderer.renderMessage("Congratulations! You've solved the Sudoku!");
        SDL_Delay(2000); // Show message for 2 seconds
        running = false;
    }
}

void Game::initializeScore() {
    // Score initialization is now handled by the Sudoku class
}