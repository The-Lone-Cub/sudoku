#include "game.h"
#include "renderer.h"
#include <SDL2/SDL.h>

int Game::currentElapsedSeconds = 0;  // Initialize static member

Game::Game() : running(false), state(GameState::MENU), selectedRow(-1), selectedCol(-1), startTime(0), elapsedSeconds(0) {
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
        // First: Handle all input events
        handleEvents();
        
        // Second: Update game state
        if (state == GameState::PLAYING) {
            updateTimer();
        }
        
        // Third: Render the current state
        if (running) {  // Only render if we're still running
            if (state == GameState::MENU) {
                renderer.renderMenuScreen();
            } else if (state == GameState::PLAYING) {
                renderer.render(sudoku, selectedRow, selectedCol);
            }
            SDL_Delay(16); // Cap at ~60 FPS
        }
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
                SDL_Quit();
                exit(0);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (state == GameState::MENU) {
                        // Check if click is on slider handle
                        int handleX = DifficultySettings::getDifficultySlider()->slider.x + (int)(DifficultySettings::getDifficultySlider()->value * DifficultySettings::getDifficultySlider()->slider.w);
                        SDL_Rect handle = {handleX - 10, DifficultySettings::getDifficultySlider()->slider.y - 5, 20, 30};
                        if (event.button.x >= handle.x && event.button.x <= handle.x + handle.w &&
                            event.button.y >= handle.y && event.button.y <= handle.y + handle.h) {
                            DifficultySettings::getDifficultySlider()->isDragging = true;
                        } else {
                            handleMouseClick(event.button.x, event.button.y);
                        }
                    } else {
                        handleMouseClick(event.button.x, event.button.y);
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    DifficultySettings::getDifficultySlider()->isDragging = false;
                }
                break;
            case SDL_MOUSEMOTION:
                if (state == GameState::MENU && DifficultySettings::getDifficultySlider()->isDragging) {
                    renderer.updateDifficultySlider(event.motion.x);
                }
                break;
            case SDL_KEYDOWN:
                handleKeyPress(event.key.keysym.sym);
                break;
        }
    }
}

bool Game::handleMenuClick(int x, int y) {
    if (renderer.handleMenuClick(x, y)) {
        state = GameState::PLAYING;
        startTime = SDL_GetTicks();
        return true;
    }
    return false;
}

void Game::handleMouseClick(int x, int y) {
    if (state == GameState::PLAYING && renderer.handleResetButtonClick(x, y)) {
        // Reset the game with current settings
        sudoku = Sudoku();
        startTime = SDL_GetTicks();
        selectedRow = -1;
        selectedCol = -1;
        return;
    }
    if (state == GameState::MENU) {
        handleMenuClick(x, y);
        return;
    }
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
        // When selecting a cell, turn off number highlighting
        sudoku.setHighlightedNumber(0);
    } else {
        selectedRow = selectedCol = -1;
    }
}

void Game::handleKeyPress(SDL_Keycode key) {
    if (key >= SDLK_1 && key <= SDLK_9) {
        int number = key - SDLK_0;
        if (selectedRow == -1 || selectedCol == -1) {
            sudoku.setHighlightedNumber(number);
            return;
        }
        if (sudoku.setNumber(selectedRow, selectedCol, number)) {
            checkWinCondition();
        }
    } else if (key == SDLK_BACKSPACE || key == SDLK_DELETE) {
        sudoku.setNumber(selectedRow, selectedCol, 0);
    }
}

void Game::checkWinCondition() {
    if (sudoku.isSolved()) {
        bool shouldClose = false;
        SDL_Event event;
        int clickResult = 0;
        
        while (clickResult == 0 && !shouldClose) {
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        shouldClose = true;
                        running = false;
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            clickResult = renderer.handleVictoryScreenClick(event.button.x, event.button.y);
                        }
                        break;
                }
            }
            
            if (!shouldClose) {
                renderer.renderVictoryScreen(sudoku.getScore(), elapsedSeconds);
                SDL_Delay(16);
            }
        }
        
        if (clickResult == 1) {  // New Game
            sudoku = Sudoku();
            selectedRow = selectedCol = -1;
            startTime = SDL_GetTicks();
            elapsedSeconds = 0;
            currentElapsedSeconds = 0;
        } else if (clickResult == 2) {  // Main Menu
            state = GameState::MENU;
            sudoku = Sudoku();
            selectedRow = selectedCol = -1;
            startTime = SDL_GetTicks();
            elapsedSeconds = 0;
            currentElapsedSeconds = 0;
        } else if(shouldClose) {
            running = false;
        }
    }
}