#include "renderer.h"
#include <stdexcept>

Renderer::Renderer() : window(nullptr), renderer(nullptr), font(nullptr) {}

Renderer::~Renderer() {
    close();
}

bool Renderer::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    if (TTF_Init() < 0) {
        SDL_Quit();
        return false;
    }

    window = SDL_CreateWindow("Sudoku", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            WINDOW_SIZE, WINDOW_SIZE, SDL_WINDOW_SHOWN);
    if (!window) {
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24);
    if (!font) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

void Renderer::close() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
}

void Renderer::render(const Sudoku& sudoku, int selectedRow, int selectedCol) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    if (selectedRow >= 0 && selectedCol >= 0) {
        renderSelectedCell(selectedRow, selectedCol);
    }
    renderGrid();
    renderNumbers(sudoku);

    SDL_RenderPresent(renderer);
}

void Renderer::renderGrid() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Draw horizontal lines
    for (int i = 0; i <= Sudoku::GRID_SIZE; i++) {
        int lineWidth = (i % 3 == 0) ? 2 : 1;
        SDL_Rect rect = {0, i * CELL_SIZE - lineWidth/2, WINDOW_SIZE, lineWidth};
        SDL_RenderFillRect(renderer, &rect);
    }

    // Draw vertical lines
    for (int i = 0; i <= Sudoku::GRID_SIZE; i++) {
        int lineWidth = (i % 3 == 0) ? 2 : 1;
        SDL_Rect rect = {i * CELL_SIZE - lineWidth/2, 0, lineWidth, WINDOW_SIZE};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void Renderer::renderNumbers(const Sudoku& sudoku) {
    for (int row = 0; row < Sudoku::GRID_SIZE; row++) {
        for (int col = 0; col < Sudoku::GRID_SIZE; col++) {
            int number = sudoku.getNumber(row, col);
            if (number != 0) {
                renderNumber(number, row, col, !sudoku.isCellEditable(row, col));
            }
        }
    }
}

void Renderer::renderSelectedCell(int row, int col) {
    // Highlight the entire row with a very faint blue
    SDL_SetRenderDrawColor(renderer, 230, 240, 255, 255); // Very light blue
    SDL_Rect rowRect = {0, row * CELL_SIZE, WINDOW_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &rowRect);

    // Highlight the entire column with a slightly different faint blue
    SDL_SetRenderDrawColor(renderer, 220, 235, 255, 255); // Another very light blue
    SDL_Rect colRect = {col * CELL_SIZE, 0, CELL_SIZE, WINDOW_SIZE};
    SDL_RenderFillRect(renderer, &colRect);

    // Highlight the 3x3 subgrid with yet another faint blue
    SDL_SetRenderDrawColor(renderer, 225, 238, 255, 255); // Third very light blue
    int subgridStartRow = (row / 3) * 3;
    int subgridStartCol = (col / 3) * 3;
    SDL_Rect subgridRect = {subgridStartCol * CELL_SIZE, subgridStartRow * CELL_SIZE, CELL_SIZE * 3, CELL_SIZE * 3};
    SDL_RenderFillRect(renderer, &subgridRect);

    // Highlight the selected cell with the original light blue color
    SDL_SetRenderDrawColor(renderer, 173, 216, 230, 255); // Original light blue
    SDL_Rect selectedRect = {col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &selectedRect);
}

void Renderer::renderNumber(int number, int row, int col, bool isFixed) {
    SDL_Color color = isFixed ? SDL_Color{0, 0, 0, 255} : SDL_Color{0, 0, 255, 255};
    std::string text = std::to_string(number);
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int textW, textH;
    SDL_QueryTexture(texture, nullptr, nullptr, &textW, &textH);

    SDL_Rect dstRect = {
        col * CELL_SIZE + (CELL_SIZE - textW) / 2,
        row * CELL_SIZE + (CELL_SIZE - textH) / 2,
        textW,
        textH
    };

    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}

void Renderer::renderMessage(const std::string& message) {
    SDL_Color color = {0, 128, 0, 255}; // Green color for success message
    SDL_Surface* surface = TTF_RenderText_Blended(font, message.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int textW, textH;
    SDL_QueryTexture(texture, nullptr, nullptr, &textW, &textH);

    SDL_Rect dstRect = {
        (WINDOW_SIZE - textW) / 2,
        WINDOW_SIZE / 2 - textH / 2,
        textW,
        textH
    };

    // Draw semi-transparent background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    SDL_Rect bgRect = {0, WINDOW_SIZE/2 - 30, WINDOW_SIZE, 60};
    SDL_RenderFillRect(renderer, &bgRect);

    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
    SDL_RenderPresent(renderer);
}

void Renderer::getGridPosition(int mouseX, int mouseY, int& row, int& col) {
    row = mouseY / CELL_SIZE;
    col = mouseX / CELL_SIZE;
    
    if (row < 0) row = 0;
    if (row >= Sudoku::GRID_SIZE) row = Sudoku::GRID_SIZE - 1;
    if (col < 0) col = 0;
    if (col >= Sudoku::GRID_SIZE) col = Sudoku::GRID_SIZE - 1;
}