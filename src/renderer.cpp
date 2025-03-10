#include "renderer.h"
#include "game.h"
#include <stdexcept>
#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>

SDL_Texture* Renderer::cachedBackground = nullptr;  // Define static member
SDL_Texture *Renderer::resetTexture = nullptr;

Renderer::Renderer() : window(nullptr), renderer(nullptr), font(nullptr) {}

Renderer::~Renderer() {
    if (cachedBackground)
    {
        SDL_DestroyTexture(cachedBackground);
        cachedBackground = nullptr;
    }
    if (resetTexture)
    {
        SDL_DestroyTexture(resetTexture);
        resetTexture = nullptr;
    }
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
                            WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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

    //Initialise SDL image
    if(!IMG_Init(IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
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
    IMG_Quit();
    SDL_Quit();
}

void Renderer::render(const Sudoku& sudoku, int selectedRow, int selectedCol) {

    // Set background color based on theme
    SDL_SetRenderDrawColor(renderer, 
        currentTheme == Theme::Light ? 255 : 0,
        currentTheme == Theme::Light ? 255 : 0,
        currentTheme == Theme::Light ? 255 : 0, 255);
    SDL_RenderClear(renderer);

    // Add padding at the top for score display
    SDL_SetRenderDrawColor(renderer,
        currentTheme == Theme::Light ? 255 : 0,
        currentTheme == Theme::Light ? 255 : 0,
        currentTheme == Theme::Light ? 255 : 0, 255);
    SDL_Rect topPadding = {0, 0, WINDOW_WIDTH, 50};
    SDL_RenderFillRect(renderer, &topPadding);

    // Render score in top-left corner
    renderScore(sudoku.getScore());

    // Get mouse state for reset button
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    // Render reset button before timer
    renderResetButton(mouseX, mouseY, mouseState);

    // Render timer in top-right corner
    renderTimer(Game::getElapsedSeconds());

    // Render highlighted numbers when no cell is selected
    if (selectedRow == -1 && selectedCol == -1) {
        renderHighlightedNumbers(sudoku, sudoku.getHighlightedNumber());
    } else if (selectedRow >= 0 && selectedCol >= 0) {
        renderSelectedCell(selectedRow, selectedCol);
    }
    renderGrid();
    renderNumbers(sudoku);
    renderNumberCounts(sudoku);

    SDL_RenderPresent(renderer);
}

void Renderer::renderScore(int score) {
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Color color = currentTheme == Theme::Light ? SDL_Color{0, 0, 0, 255} : SDL_Color{255, 255, 255, 255};
    renderText(scoreText, 10, 10, color);
}

void Renderer::renderTimer(int elapsedSeconds) {
    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;
    
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << minutes << ":" 
       << std::setfill('0') << std::setw(2) << seconds;
    
    SDL_Color color = currentTheme == Theme::Light ? SDL_Color{0, 0, 0, 255} : SDL_Color{255, 255, 255, 255};
    renderText(ss.str(), WINDOW_WIDTH - 100, 10, color); // Position in top-right corner
}

void Renderer::renderResetButton(int mouseX, int mouseY, Uint32 mouseState) {
    // Load the texture if it hasn't been loaded yet
    if (!resetTexture) {
        std::string imagePath = currentTheme == Theme::Light ? 
            "assets/light/reset.png" : "assets/dark/reset.png";

        SDL_Surface* resetSurface = IMG_Load(imagePath.c_str());
        if (!resetSurface) return;

        resetTexture = SDL_CreateTextureFromSurface(renderer, resetSurface);
        SDL_FreeSurface(resetSurface);
        
        if (!resetTexture) return;
    }
    
    // Position the reset button before the timer
    SDL_Rect resetButton = {WINDOW_WIDTH - 180, 5, 40, 40};
    
    // Check if mouse is over the button
    bool isHovered = mouseX >= resetButton.x && mouseX <= resetButton.x + resetButton.w &&
                    mouseY >= resetButton.y && mouseY <= resetButton.y + resetButton.h;
    
    // Apply hover effect
    if (isHovered) {
        SDL_SetTextureAlphaMod(resetTexture, 200);
        if (mouseState & SDL_BUTTON_LMASK) {
            // Click effect
            SDL_SetTextureAlphaMod(resetTexture, 150);
        }
    } else {
        SDL_SetTextureAlphaMod(resetTexture, 255);
    }

    // Render the reset button texture
    SDL_RenderCopy(renderer, resetTexture, NULL, &resetButton);
}

bool Renderer::handleResetButtonClick(int x, int y) {
    SDL_Rect resetButton = {WINDOW_WIDTH - 180, 5, 40, 40};
    return (x >= resetButton.x && x <= resetButton.x + resetButton.w &&
            y >= resetButton.y && y <= resetButton.y + resetButton.h);
}

void Renderer::renderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    SDL_Rect destRect;
    TTF_SizeText(font, text.c_str(), &destRect.w, &destRect.h);
    destRect.x = x;
    destRect.y = y;

    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
}

void Renderer::renderGrid() {
    SDL_SetRenderDrawColor(renderer,
        currentTheme == Theme::Light ? 0 : 255,
        currentTheme == Theme::Light ? 0 : 255,
        currentTheme == Theme::Light ? 0 : 255, 255);
    // Move grid down by 50 pixels to accommodate score display
    const int GRID_START_Y = 50;

    // Calculate exact grid size (9 cells)
    const int GRID_PIXELS = Sudoku::GRID_SIZE * CELL_SIZE;

    // Draw horizontal lines
    for (int i = 0; i <= Sudoku::GRID_SIZE; i++) {
        int lineWidth = (i % 3 == 0) ? 2 : 1;
        SDL_Rect rect = {0, GRID_START_Y + i * CELL_SIZE - lineWidth/2, GRID_PIXELS, lineWidth};
        SDL_RenderFillRect(renderer, &rect);
    }

    // Draw vertical lines
    for (int i = 0; i <= Sudoku::GRID_SIZE; i++) {
        int lineWidth = (i % 3 == 0) ? 2 : 1;
        SDL_Rect rect = {i * CELL_SIZE - lineWidth/2, GRID_START_Y, lineWidth, GRID_PIXELS};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void Renderer::renderHighlightedNumbers(const Sudoku& sudoku, int highlightedNumber) {
    if (highlightedNumber <= 0 || !sudoku.isHighlightVisible()) return;
    
    const int GRID_START_Y = 50;
    SDL_Color highlightColor = currentTheme == Theme::Light 
        ? SDL_Color{220, 230, 240, 255}  // Soft blue highlight for light theme
        : SDL_Color{101, 84, 43, 255};   // Dark golden brown for dark theme

    for (int row = 0; row < Sudoku::GRID_SIZE; row++) {
        for (int col = 0; col < Sudoku::GRID_SIZE; col++) {
            if (sudoku.getNumber(row, col) == highlightedNumber) {
                SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
                SDL_Rect cellRect = {
                    col * CELL_SIZE,
                    GRID_START_Y + row * CELL_SIZE,
                    CELL_SIZE,
                    CELL_SIZE
                };
                SDL_RenderFillRect(renderer, &cellRect);
            }
        }
    }
}

void Renderer::renderNumbers(const Sudoku& sudoku) {
    for (int row = 0; row < Sudoku::GRID_SIZE; row++) {
        for (int col = 0; col < Sudoku::GRID_SIZE; col++) {
            int number = sudoku.getNumber(row, col);
            if (number != 0) {
                renderNumber(number, row, col, !sudoku.isCellEditable(row, col), sudoku);
            }
        }
    }
}

void Renderer::renderSelectedCell(int row, int col) {
    // Calculate exact grid size
    const int GRID_PIXELS = Sudoku::GRID_SIZE * CELL_SIZE;
    const int GRID_START_Y = 50;

    // Define colors based on theme
    SDL_Color rowColor, colColor, subgridColor, selectedColor;
    if (currentTheme == Theme::Light) {
        rowColor = {230, 240, 250, 255};     // Light blue
        colColor = {230, 240, 250, 255};     // Light blue
        subgridColor = {230, 240, 250, 255}; // Light blue
        selectedColor = {180, 210, 240, 255}; // More saturated blue for better visibility
    } else {
        rowColor = {189, 139, 16, 255};       // Slightly lighter goldenrod
        colColor = {189, 139, 16, 255};       // Slightly lighter goldenrod
        subgridColor = {189, 139, 16, 255};   // Slightly lighter goldenrod
        selectedColor = {235, 185, 45, 255};  // Brighter golden highlight for better visibility
    }

    // Render row, column, and subgrid highlights first
    SDL_SetRenderDrawColor(renderer, rowColor.r, rowColor.g, rowColor.b, rowColor.a);
    SDL_Rect rowRect = {0, GRID_START_Y + row * CELL_SIZE, GRID_PIXELS, CELL_SIZE};
    SDL_RenderFillRect(renderer, &rowRect);

    SDL_SetRenderDrawColor(renderer, colColor.r, colColor.g, colColor.b, colColor.a);
    SDL_Rect colRect = {col * CELL_SIZE, GRID_START_Y, CELL_SIZE, GRID_PIXELS};
    SDL_RenderFillRect(renderer, &colRect);

    SDL_SetRenderDrawColor(renderer, subgridColor.r, subgridColor.g, subgridColor.b, subgridColor.a);
    int subgridStartRow = (row / 3) * 3;
    int subgridStartCol = (col / 3) * 3;
    SDL_Rect subgridRect = {subgridStartCol * CELL_SIZE, GRID_START_Y + subgridStartRow * CELL_SIZE, CELL_SIZE * 3, CELL_SIZE * 3};
    SDL_RenderFillRect(renderer, &subgridRect);

    // Render the selected cell on top with yellow highlight
    SDL_SetRenderDrawColor(renderer, selectedColor.r, selectedColor.g, selectedColor.b, selectedColor.a);
    SDL_Rect selectedRect = {col * CELL_SIZE, GRID_START_Y + row * CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &selectedRect);
}

std::array<int, 9> Renderer::calculateNumberCounts(const Sudoku& sudoku) const {
    std::array<int, 9> counts {0};  // Zero-initialize array
    for (int row = 0; row < Sudoku::GRID_SIZE; row++) {
        for (int col = 0; col < Sudoku::GRID_SIZE; col++) {
            int num = sudoku.getNumber(row, col);
            if (num > 0) {
                counts[num - 1]++;
            }
        }
    }
    return counts;
}

void Renderer::renderNumberCounts(const Sudoku& sudoku) {
    auto counts = calculateNumberCounts(sudoku);
    
    int numberWidth = CELL_SIZE / 2;
    // Calculate padding to span the entire width
    int totalWidth = WINDOW_WIDTH;
    int padding = (totalWidth - (9 * numberWidth)) / 10;  // 10 spaces (9 numbers + 1)
    int startX = padding;  // Start after first padding
    int startY = WINDOW_HEIGHT - 40;  // Position for number counts (moved down 4 pixels)
    
    for (int i = 0; i < 9; i++) {
        SDL_Color color;
        if (currentTheme == Theme::Light) {
            color = (counts[i] == 9) ? SDL_Color{0, 255, 0, 255} : SDL_Color{0, 0, 0, 255};
        } else {
            color = (counts[i] == 9) ? SDL_Color{255, 0, 0, 255} : SDL_Color{255, 255, 255, 255};
        }
        
        // Render the number
        TTF_SetFontStyle(font, TTF_STYLE_BOLD);
        std::string numStr = std::to_string(i + 1);
        SDL_Surface* numSurface = TTF_RenderText_Blended(font, numStr.c_str(), color);
        if (!numSurface) continue;
        
        SDL_Texture* numTexture = SDL_CreateTextureFromSurface(renderer, numSurface);
        if (!numTexture) {
            SDL_FreeSurface(numSurface);
            continue;
        }
        
        // Position each number with uniform padding
        SDL_Rect numRect = {startX + i * (numberWidth + padding), startY, numSurface->w, numSurface->h};
        SDL_RenderCopy(renderer, numTexture, nullptr, &numRect);
        
        // Render the count as superscript if not complete
        if (counts[i] < 9) {
            TTF_SetFontStyle(font, TTF_STYLE_NORMAL);  // Use normal style for count
            std::string countStr = std::to_string(counts[i]);
            SDL_Surface* countSurface = TTF_RenderText_Blended(font, countStr.c_str(), color);
            if (countSurface) {
                SDL_Texture* countTexture = SDL_CreateTextureFromSurface(renderer, countSurface);
                if (countTexture) {
                    SDL_Rect countRect = {
                        startX + i * (numberWidth + padding) + numSurface->w, // Match the number's padding
                        startY - 5,  // Keep the vertical offset
                        countSurface->w / 2, 
                        countSurface->h / 2
                    };
                    SDL_RenderCopy(renderer, countTexture, nullptr, &countRect);
                    SDL_DestroyTexture(countTexture);
                }
                SDL_FreeSurface(countSurface);
            }
        }
        
        SDL_FreeSurface(numSurface);
        SDL_DestroyTexture(numTexture);
    }
    
    // Reset font style to normal at the end
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
}

void Renderer::renderNumber(int number, int row, int col, bool isFixed, const Sudoku& sudoku) {
    SDL_Color color;
    if (number == 0) return;
    
    // Check if number is valid without modifying the grid
    bool isWrong = !isFixed && number != 0 && !sudoku.isValid(row, col, number);

    // Set color based on number state and theme
    if (isWrong)
    {
        color = SDL_Color{255, 0, 0, 255}; // Red color for wrong numbers
    }
    else if (currentTheme == Theme::Light)
    {
        color = isFixed ? SDL_Color{47, 79, 79, 255} : SDL_Color{70, 130, 180, 255};
    }
    else
    {
        color = isFixed ? SDL_Color{255, 223, 186, 255} : SDL_Color{218, 165, 32, 255};
    }

    std::string text = std::to_string(number);
    const int GRID_START_Y = 50;
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int textW, textH;
    SDL_QueryTexture(texture, nullptr, nullptr, &textW, &textH);

    SDL_Rect dstRect = {
        col * CELL_SIZE + (CELL_SIZE - textW) / 2,
        GRID_START_Y + row * CELL_SIZE + (CELL_SIZE - textH) / 2,
        textW,
        textH
    };

    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}

void Renderer::renderMessage(const std::string& message) {
    // Use theme-appropriate colors
    SDL_Color textColor;
    if (currentTheme == Theme::Light) {
        textColor = SDL_Color{47, 79, 79, 255};  // Dark slate gray for light theme
    } else {
        textColor = SDL_Color{255, 223, 186, 255};  // Light peach for dark theme
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, message.c_str(), textColor);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int textW, textH;
    SDL_QueryTexture(texture, nullptr, nullptr, &textW, &textH);

    SDL_Rect dstRect = {
        (WINDOW_WIDTH - textW) / 2,
        WINDOW_HEIGHT / 2 - textH / 2,
        textW,
        textH
    };

    // Draw semi-transparent background based on theme
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (currentTheme == Theme::Light) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    }
    SDL_Rect bgRect = {0, WINDOW_HEIGHT/2 - 30, WINDOW_WIDTH, 60};
    SDL_RenderFillRect(renderer, &bgRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
    SDL_RenderPresent(renderer);
}

void Renderer::renderHighGammaEffect() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
    SDL_Rect fullScreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &fullScreen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void Renderer::renderMenuScreen() {
    // Set background color based on theme
    if (currentTheme == Theme::Light) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
    SDL_RenderClear(renderer);

    // Calculate dynamic button dimensions based on window size
    const int buttonWidth = WINDOW_WIDTH * 0.4;  // 40% of window width
    const int buttonHeight = WINDOW_HEIGHT * 0.08;  // 8% of window height
    const int buttonSpacing = buttonHeight * 1.5;  // Space between buttons
    const int startY = WINDOW_HEIGHT / 2;  // Start buttons from middle of screen

    // Initialize difficulty slider if not already done
    static bool sliderInitialized = false;
    if (!sliderInitialized) {
        DifficultySettings::getDifficultySlider()->slider = {WINDOW_WIDTH/4, startY + buttonSpacing + buttonHeight + 80, WINDOW_WIDTH/2, 20};
        DifficultySettings::getDifficultySlider()->value = 0.5f; // Start at medium difficulty
        DifficultySettings::getDifficultySlider()->isDragging = false;
        sliderInitialized = true;
    }

    // Set font style and size for the title
    TTF_Font* titleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 72);
    TTF_SetFontStyle(titleFont, TTF_STYLE_BOLD);
    SDL_Color titleColor = currentTheme == Theme::Light ? SDL_Color{0, 0, 0, 255} : SDL_Color{255, 255, 255, 255};
    
    // Render SUDOKU title with larger font
    SDL_Surface* surface = TTF_RenderText_Blended(titleFont, "SUDOKU", titleColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect destRect;
            TTF_SizeText(titleFont, "SUDOKU", &destRect.w, &destRect.h);
            destRect.x = WINDOW_WIDTH / 2 - destRect.w / 2;
            destRect.y = WINDOW_HEIGHT / 4 - destRect.h / 2;  // Moved up to 1/4 of screen
            SDL_RenderCopy(renderer, texture, NULL, &destRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    TTF_CloseFont(titleFont);

    // Set font style and size for the subtitle
    TTF_Font* subtitleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 16);
    TTF_SetFontStyle(subtitleFont, TTF_STYLE_ITALIC);
    SDL_Color subtitleColor = currentTheme == Theme::Light ? SDL_Color{128, 128, 128, 255} : SDL_Color{200, 200, 200, 255};
    
    // Render subtitle with smaller font
    surface = TTF_RenderText_Blended(subtitleFont, "Made by Nsubuga Benard", subtitleColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect destRect;
            TTF_SizeText(subtitleFont, "Made by Nsubuga Benard", &destRect.w, &destRect.h);
            destRect.x = WINDOW_WIDTH / 2 - destRect.w / 2;
            destRect.y = WINDOW_HEIGHT / 4 + 60;
            SDL_RenderCopy(renderer, texture, NULL, &destRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    TTF_CloseFont(subtitleFont);

    // Reset font style for the buttons
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    // Get mouse position for slider interaction
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    // Render the difficulty slider
    renderDifficultySlider();
 
    // Theme button
    SDL_Rect themeBtn = {
        WINDOW_WIDTH / 2 - buttonWidth / 2,
        startY,
        buttonWidth,
        buttonHeight
    };
    
    // Start button
    SDL_Rect startBtn = {
        WINDOW_WIDTH / 2 - buttonWidth / 2,
        startY + buttonSpacing,
        buttonWidth,
        buttonHeight
    };

    // Get mouse state for hover effects
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    // Render theme button
    renderMenuButton(themeBtn, 
                    "Switch Theme",
                    mouseX, mouseY, mouseState);

    // Render start button
    renderMenuButton(startBtn, "Start Game", mouseX, mouseY, mouseState, true);

    SDL_RenderPresent(renderer);
}

void Renderer::renderDifficultySlider() {
    // Draw slider background
    SDL_Color bgColor = currentTheme == Theme::Light ? SDL_Color{200, 200, 200, 255} : SDL_Color{100, 100, 100, 255};
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &DifficultySettings::getDifficultySlider()->slider);

    // Draw slider handle
    int handleX = DifficultySettings::getDifficultySlider()->slider.x + 
                  (int)(DifficultySettings::getDifficultySlider()->value * 
                  DifficultySettings::getDifficultySlider()->slider.w);
    SDL_Rect handle = {handleX - 10, 
                      DifficultySettings::getDifficultySlider()->slider.y - 5, 
                      20, 30};
    SDL_Color handleColor = currentTheme == Theme::Light ? 
                           SDL_Color{0, 120, 215, 255} : 
                           SDL_Color{255, 200, 0, 255};
    SDL_SetRenderDrawColor(renderer, handleColor.r, handleColor.g, handleColor.b, handleColor.a);
    SDL_RenderFillRect(renderer, &handle);

    // Draw difficulty labels
    SDL_Color textColor = currentTheme == Theme::Light ? SDL_Color{0, 0, 0, 255} : SDL_Color{255, 255, 255, 255};
    renderText("Hard", DifficultySettings::getDifficultySlider()->slider.x, DifficultySettings::getDifficultySlider()->slider.y - 30, textColor);
    renderText("Medium", DifficultySettings::getDifficultySlider()->slider.x + DifficultySettings::getDifficultySlider()->slider.w/2 - 30, DifficultySettings::getDifficultySlider()->slider.y - 30, textColor);
    renderText("Easy", DifficultySettings::getDifficultySlider()->slider.x + DifficultySettings::getDifficultySlider()->slider.w - 30, DifficultySettings::getDifficultySlider()->slider.y - 30, textColor);
}

void Renderer::updateDifficultySlider(int mouseX) {
    if (DifficultySettings::getDifficultySlider()->isDragging) {
        float newValue = (float)(mouseX - DifficultySettings::getDifficultySlider()->slider.x) / DifficultySettings::getDifficultySlider()->slider.w;
        DifficultySettings::getDifficultySlider()->value = std::max(0.0f, std::min(1.0f, newValue));
    }
}

void Renderer::renderMenuButton(const SDL_Rect& btn, const std::string& text, int mouseX, int mouseY, Uint32 mouseState, bool isGreen) {
    bool isHovered = (mouseX >= btn.x && mouseX <= btn.x + btn.w &&
                     mouseY >= btn.y && mouseY <= btn.y + btn.h);
    bool isClicked = isHovered && (mouseState & SDL_BUTTON_LMASK);

    // Button shadow
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 60);
    SDL_Rect btnShadow = {btn.x + 2, btn.y + 2, btn.w, btn.h};
    SDL_RenderFillRect(renderer, &btnShadow);

    // Button body with interaction effects
    SDL_Rect buttonRect = btn;
    if (isHovered) {
        if (isClicked) {
            buttonRect.x += 2;
            buttonRect.y += 2;
        } else {
            buttonRect.x += 1;
            buttonRect.y += 1;
        }
    }

    // Button colors based on state and type
    if (isGreen) {
        SDL_SetRenderDrawColor(renderer, 0,
            isHovered ? (isClicked ? 80 : 100) : 128,
            0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer,
            isHovered ? (isClicked ? 80 : 100) : 100,
            isHovered ? (isClicked ? 80 : 100) : 100,
            isHovered ? (isClicked ? 80 : 100) : 100,
            255);
    }
    SDL_RenderFillRect(renderer, &buttonRect);

    // Button border
    SDL_SetRenderDrawColor(renderer,
        isGreen ? 0 : (currentTheme == Theme::Light ? 50 : 200),
        isGreen ? 200 : (currentTheme == Theme::Light ? 50 : 200),
        isGreen ? 0 : (currentTheme == Theme::Light ? 50 : 200),
        255);
    SDL_RenderDrawRect(renderer, &buttonRect);

    // Calculate text position to center it in the button
    int textWidth, textHeight;
    TTF_SizeText(font, text.c_str(), &textWidth, &textHeight);
    
    SDL_Color textColor = {255, 255, 255, 255};
    renderText(text,
        buttonRect.x + (buttonRect.w - textWidth) / 2 + (isHovered ? (isClicked ? 2 : 1) : 0),
        buttonRect.y + (buttonRect.h - textHeight) / 2 + (isHovered ? (isClicked ? 2 : 1) : 0),
        textColor);
}

bool Renderer::handleMenuClick(int x, int y) {
    // Use same calculations as renderMenuScreen
    const int buttonWidth = WINDOW_WIDTH * 0.4;
    const int buttonHeight = WINDOW_HEIGHT * 0.08;
    const int buttonSpacing = buttonHeight * 1.5;
    const int startY = WINDOW_HEIGHT / 2;

    // Theme button
    SDL_Rect themeBtn = {
        WINDOW_WIDTH / 2 - buttonWidth / 2,
        startY,
        buttonWidth,
        buttonHeight};

    // Start button
    SDL_Rect startBtn = {
        WINDOW_WIDTH / 2 - buttonWidth / 2,
        startY + buttonSpacing,
        buttonWidth,
        buttonHeight};

    // Check if theme button was clicked
    if (x >= themeBtn.x && x <= themeBtn.x + themeBtn.w &&
        y >= themeBtn.y && y <= themeBtn.y + themeBtn.h)
    {
        currentTheme = (currentTheme == Theme::Light) ? Theme::Dark : Theme::Light;
        return false;
    }

    // Check if start button was clicked
    return (x >= startBtn.x && x <= startBtn.x + startBtn.w &&
            y >= startBtn.y && y <= startBtn.y + startBtn.h);
}

void Renderer::renderVictoryScreen(int score, int elapsedSeconds) {
    // First, render the high gamma background based on theme
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,
                           currentTheme == Theme::Light ? 255 : 0, // White/Black background
                           currentTheme == Theme::Light ? 255 : 0,
                           currentTheme == Theme::Light ? 255 : 0,
                           230); // High alpha for gamma effect
    SDL_RenderClear(renderer);

    // Create the victory box with theme-appropriate colors
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,
                           currentTheme == Theme::Light ? 255 : 0, // White/Black fill
                           currentTheme == Theme::Light ? 255 : 0,
                           currentTheme == Theme::Light ? 255 : 0,
                           255); // Full opacity for box
    SDL_Rect victoryBox = {WINDOW_WIDTH / 2 - 150, 80, 300, 350};
    SDL_RenderFillRect(renderer, &victoryBox);

    // Draw outline with theme-appropriate color
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer,
                           currentTheme == Theme::Light ? 0 : 255, // Black/White outline
                           currentTheme == Theme::Light ? 0 : 255,
                           currentTheme == Theme::Light ? 0 : 255,
                           255);
    SDL_RenderDrawRect(renderer, &victoryBox);

    // Calculate accuracy
    float accuracy = (static_cast<float>(score) / 405.0f) * 100.0f;
    // Set text color based on theme
    SDL_Color textColor = currentTheme == Theme::Light ? 
        SDL_Color{0, 0, 0, 255} :     // Black text for light theme
        SDL_Color{255, 255, 255, 255}; // White text for dark theme
    // Render "SUCCESS" label
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    renderText("SUCCESS!", WINDOW_WIDTH / 2 - 60, 100, textColor);
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    // Render stats with aligned formatting
    const int labelX = WINDOW_WIDTH / 2 - 120;  // Left align position for labels
    const int valueX = WINDOW_WIDTH / 2 + 120;   // Increased space for right-aligned values
    int yPos = 150;
    // Score
    renderText("Score:", labelX, yPos, textColor);
    std::string scoreStr = std::to_string(score);
    int textW, textH;
    TTF_SizeText(font, scoreStr.c_str(), &textW, &textH);
    renderText(scoreStr, valueX - textW, yPos, textColor);
    yPos += 30;
    // Time
    renderText("Time:", labelX, yPos, textColor);
    std::stringstream timeStr;
    timeStr << elapsedSeconds / 60 << ":" << std::setfill('0') << std::setw(2) << elapsedSeconds % 60;
    TTF_SizeText(font, timeStr.str().c_str(), &textW, &textH);
    renderText(timeStr.str(), valueX - textW, yPos, textColor);
    yPos += 30;
    // Accuracy
    renderText("Accuracy:", labelX, yPos, textColor);
    std::stringstream accuracyStr;
    accuracyStr << std::fixed << std::setprecision(1) << accuracy << "%";
    TTF_SizeText(font, accuracyStr.str().c_str(), &textW, &textH);
    renderText(accuracyStr.str(), valueX - textW, yPos, textColor);
    yPos += 30;
    // Render buttons with enhanced visual effects
    SDL_Rect newGameBtn = {WINDOW_WIDTH / 2 - 100, yPos + 20, 200, 40};
    SDL_Rect mainMenuBtn = {WINDOW_WIDTH / 2 - 100, yPos + 70, 200, 40};
    SDL_Rect exitBtn = {WINDOW_WIDTH / 2 - 100, yPos + 120, 200, 40};

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // Calculate fresh hover states based on current mouse position
    bool newGameHover = (mouseX >= newGameBtn.x && mouseX <= newGameBtn.x + newGameBtn.w &&
                        mouseY >= newGameBtn.y && mouseY <= newGameBtn.y + newGameBtn.h);
    bool mainMenuHover = (mouseX >= mainMenuBtn.x && mouseX <= mainMenuBtn.x + mainMenuBtn.w &&
                         mouseY >= mainMenuBtn.y && mouseY <= mainMenuBtn.y + mainMenuBtn.h);
    bool exitHover = (mouseX >= exitBtn.x && mouseX <= exitBtn.x + exitBtn.w &&
                     mouseY >= exitBtn.y && mouseY <= exitBtn.y + exitBtn.h);
    
    int currentMouseState = SDL_GetMouseState(nullptr, nullptr);
    // Render buttons with current state
    for (int i = 0; i < 3; i++) {
        SDL_Rect* btn = (i == 0) ? &newGameBtn : (i == 1) ? &mainMenuBtn : &exitBtn;
        bool isHovered = (i == 0) ? newGameHover : (i == 1) ? mainMenuHover : exitHover;
        bool isCurrentlyClicked = isHovered && (currentMouseState & SDL_BUTTON_LMASK);
        const char* buttonText = (i == 0) ? "New Game" : (i == 1) ? "Main Menu" : "Exit";
    // Use theme-appropriate colors
    SDL_Color baseColor, hoverColor, clickColor;
    if (currentTheme == Theme::Light) {
    baseColor = {63, 81, 181, 255};    // Indigo 500
    hoverColor = {92, 107, 192, 255};  // Indigo 400
    clickColor = {48, 63, 159, 255};   // Indigo 700
    } else {
    baseColor = {130, 177, 255, 255};  // Light Blue
    hoverColor = {179, 229, 252, 255}; // Lighter Blue
    clickColor = {100, 149, 237, 255}; // Cornflower Blue
    }
    // Shadow effect with depth
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 60);
    SDL_Rect btnShadow = {btn->x + 2, btn->y + 2, btn->w, btn->h};
    SDL_RenderFillRect(renderer, &btnShadow);
    // Button body with position offset when interacting
    SDL_Rect buttonRect = *btn;
    if (isHovered) {
    if (isCurrentlyClicked) {
    buttonRect.x += 2;
    buttonRect.y += 2;
    } else {
    buttonRect.x += 1;
    buttonRect.y += 1;
    }
    }
    // Set button color based on state
    SDL_Color currentColor = isHovered ? 
    (isCurrentlyClicked ? clickColor : hoverColor) : 
    baseColor;
    SDL_SetRenderDrawColor(renderer, 
    currentColor.r, currentColor.g, currentColor.b, currentColor.a);
    // Render button body and effects
    SDL_SetRenderDrawColor(renderer, 0,
    isHovered ? (isCurrentlyClicked ? 80 : 100) : 128,
    0, 255);
    SDL_RenderFillRect(renderer, &buttonRect);
    // Button border
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderDrawRect(renderer, &buttonRect);
    // Button text
    SDL_Color white = {255, 255, 255, 255};
    // Calculate text dimensions for centering
    int textWidth, textHeight;
    TTF_SizeText(font, buttonText, &textWidth, &textHeight);
    // Center text horizontally and vertically within the button
    int textX = buttonRect.x + (buttonRect.w - textWidth) / 2 + (isHovered ? (isCurrentlyClicked ? 3 : 1) : 0);
    int textY = buttonRect.y + (buttonRect.h - textHeight) / 2 + (isHovered ? (isCurrentlyClicked ? 3 : 1) : 0);
    renderText(buttonText, textX, textY, white);
    }
    SDL_RenderPresent(renderer);
}
int Renderer::handleVictoryScreenClick(int x, int y) {
    int yPos = 150 + 90;  // Match the button positions from renderVictoryScreen
    SDL_Rect newGameBtn = {WINDOW_WIDTH / 2 - 100, yPos + 20, 200, 40};
    SDL_Rect mainMenuBtn = {WINDOW_WIDTH / 2 - 100, yPos + 70, 200, 40};
    SDL_Rect exitBtn = {WINDOW_WIDTH / 2 - 100, yPos + 120, 200, 40};
    
    if (x >= WINDOW_WIDTH / 2 - 100 && x <= WINDOW_WIDTH / 2 + 100) {
        if (y >= newGameBtn.y && y <= newGameBtn.y + newGameBtn.h) {
            return 1;  // New Game clicked
        } else if (y >= mainMenuBtn.y && y <= mainMenuBtn.y + mainMenuBtn.h) {
            return 2;  // Main Menu clicked
        } else if (y >= exitBtn.y && y <= exitBtn.y + exitBtn.h) {
            SDL_Quit();
            exit(0);
        }
    }
    return false;
}
void Renderer::getGridPosition(int mouseX, int mouseY, int& row, int& col) {
    const int GRID_START_Y = 50;
    row = (mouseY - GRID_START_Y) / CELL_SIZE;
    col = mouseX / CELL_SIZE;
    
    if (row < 0) row = 0;
    if (row >= Sudoku::GRID_SIZE) row = Sudoku::GRID_SIZE - 1;
    if (col < 0) col = 0;
    if (col >= Sudoku::GRID_SIZE) col = Sudoku::GRID_SIZE - 1;
}