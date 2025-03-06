#include "renderer.h"
#include "game.h"
#include <stdexcept>
#include <array>
#include <sstream>
#include <iomanip>

SDL_Texture* Renderer::cachedBackground = nullptr;  // Define static member

Renderer::Renderer() : window(nullptr), renderer(nullptr), font(nullptr) {}

Renderer::~Renderer() {
    if (cachedBackground)
    {
        SDL_DestroyTexture(cachedBackground);
        cachedBackground = nullptr;
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

    // Add padding at the top for score display
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect topPadding = {0, 0, WINDOW_WIDTH, 50};
    SDL_RenderFillRect(renderer, &topPadding);

    // Render score in top-left corner
    renderScore(sudoku.getScore());

    // Render timer in top-right corner
    renderTimer(Game::getElapsedSeconds());

    if (selectedRow >= 0 && selectedCol >= 0) {
        renderSelectedCell(selectedRow, selectedCol);
    }
    renderGrid();
    renderNumbers(sudoku);
    renderNumberCounts(sudoku);

    SDL_RenderPresent(renderer);
}

void Renderer::renderScore(int score) {
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Color color = {0, 0, 0, 255}; // Black color for score
    renderText(scoreText, 10, 10, color);
}

void Renderer::renderTimer(int elapsedSeconds) {
    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;
    
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << minutes << ":" 
       << std::setfill('0') << std::setw(2) << seconds;
    
    SDL_Color color = {0, 0, 0, 255}; // Black color for timer
    renderText(ss.str(), WINDOW_WIDTH - 100, 10, color); // Position in top-right corner
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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
    // Calculate exact grid size
    const int GRID_PIXELS = Sudoku::GRID_SIZE * CELL_SIZE;
    const int GRID_START_Y = 50;

    // Highlight the entire row with a very faint blue
    SDL_SetRenderDrawColor(renderer, 230, 240, 255, 255); // Very light blue
    SDL_Rect rowRect = {0, GRID_START_Y + row * CELL_SIZE, GRID_PIXELS, CELL_SIZE};

    // Highlight the entire column with a slightly different faint blue
    SDL_SetRenderDrawColor(renderer, 220, 235, 255, 255); // Another very light blue
    SDL_Rect colRect = {col * CELL_SIZE, GRID_START_Y, CELL_SIZE, GRID_PIXELS};

    // Highlight the 3x3 subgrid with yet another faint blue
    SDL_SetRenderDrawColor(renderer, 225, 238, 255, 255); // Third very light blue
    int subgridStartRow = (row / 3) * 3;
    int subgridStartCol = (col / 3) * 3;
    SDL_Rect subgridRect = {subgridStartCol * CELL_SIZE, GRID_START_Y + subgridStartRow * CELL_SIZE, CELL_SIZE * 3, CELL_SIZE * 3};

    SDL_RenderFillRect(renderer, &rowRect);
    SDL_RenderFillRect(renderer, &colRect);
    SDL_RenderFillRect(renderer, &subgridRect);

    // Highlight the selected cell with the original light blue color
    SDL_SetRenderDrawColor(renderer, 173, 216, 230, 255); // Original light blue
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
        SDL_Color color = (counts[i] == 9) ? SDL_Color{0, 255, 0, 255} : SDL_Color{0, 0, 0, 255};
        
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

void Renderer::renderNumber(int number, int row, int col, bool isFixed) {
    SDL_Color color = isFixed ? SDL_Color{0, 0, 0, 255} : SDL_Color{0, 0, 255, 255};
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
    SDL_Color color = {0, 128, 0, 255}; // Green color for success message
    SDL_Surface* surface = TTF_RenderText_Blended(font, message.c_str(), color);
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

    // Draw semi-transparent background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    SDL_Rect bgRect = {0, WINDOW_HEIGHT/2 - 30, WINDOW_WIDTH, 60};
    SDL_RenderFillRect(renderer, &bgRect);

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
    // Clear the screen with white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Set font style and size for the title
    TTF_Font* titleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 72);
    TTF_SetFontStyle(titleFont, TTF_STYLE_BOLD);
    SDL_Color titleColor = {0, 0, 0, 255}; // Black color for title
    
    // Render SUDOKU title with larger font
    SDL_Surface* surface = TTF_RenderText_Blended(titleFont, "SUDOKU", titleColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect destRect;
            TTF_SizeText(titleFont, "SUDOKU", &destRect.w, &destRect.h);
            destRect.x = WINDOW_WIDTH / 2 - destRect.w / 2;
            destRect.y = WINDOW_HEIGHT / 3 - destRect.h / 2;
            SDL_RenderCopy(renderer, texture, NULL, &destRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    TTF_CloseFont(titleFont);

    // Set font style and size for the subtitle
    TTF_Font* subtitleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 16);
    TTF_SetFontStyle(subtitleFont, TTF_STYLE_ITALIC);
    SDL_Color subtitleColor = {128, 128, 128, 255}; // Gray color for subtitle
    
    // Render subtitle with smaller font
    surface = TTF_RenderText_Blended(subtitleFont, "Made by Nsubuga Benard", subtitleColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect destRect;
            TTF_SizeText(subtitleFont, "Made by Nsubuga Benard", &destRect.w, &destRect.h);
            destRect.x = WINDOW_WIDTH / 2 - destRect.w / 2;
            destRect.y = WINDOW_HEIGHT / 3 + 60;
            SDL_RenderCopy(renderer, texture, NULL, &destRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    TTF_CloseFont(subtitleFont);

    // Reset font style for the button
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    // Render start button with the same style as victory screen buttons
    SDL_Rect startBtn = {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 50, 200, 40};

    // Get mouse state for hover effect
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    bool isHovered = (mouseX >= startBtn.x && mouseX <= startBtn.x + startBtn.w &&
                     mouseY >= startBtn.y && mouseY <= startBtn.y + startBtn.h);
    bool isClicked = isHovered && (mouseState & SDL_BUTTON_LMASK);

    // Button shadow
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 60);
    SDL_Rect btnShadow = {startBtn.x + 2, startBtn.y + 2, startBtn.w, startBtn.h};
    SDL_RenderFillRect(renderer, &btnShadow);

    // Button body with interaction effects
    SDL_Rect buttonRect = startBtn;
    if (isHovered) {
        if (isClicked) {
            buttonRect.x += 2;
            buttonRect.y += 2;
        } else {
            buttonRect.x += 1;
            buttonRect.y += 1;
        }
    }

    // Button colors based on state
    SDL_SetRenderDrawColor(renderer, 0,
        isHovered ? (isClicked ? 80 : 100) : 128,
        0, 255);
    SDL_RenderFillRect(renderer, &buttonRect);

    // Button border
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderDrawRect(renderer, &buttonRect);

    // Button text
    SDL_Color white = {255, 255, 255, 255};
    renderText("Start Game",
              WINDOW_WIDTH / 2 - 50 + (isHovered ? (isClicked ? 3 : 1) : 0),
              WINDOW_HEIGHT / 2 + 60 + (isHovered ? (isClicked ? 3 : 1) : 0),
              white);

    SDL_RenderPresent(renderer);
}

bool Renderer::handleMenuClick(int x, int y) {
    SDL_Rect startBtn = {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 50, 200, 40};
    return (x >= startBtn.x && x <= startBtn.x + startBtn.w &&
            y >= startBtn.y && y <= startBtn.y + startBtn.h);
}

void Renderer::renderVictoryScreen(int score, int elapsedSeconds) {
    cachedBackground = nullptr;
    
    // Create cached background texture on first call
    if (!cachedBackground) {
        // Create a target texture for the background
        cachedBackground = SDL_CreateTexture(renderer, 
            SDL_PIXELFORMAT_RGBA8888, 
            SDL_TEXTUREACCESS_TARGET, 
            WINDOW_WIDTH, 
            WINDOW_HEIGHT);
            
        // Set the texture as the render target
        SDL_SetRenderTarget(renderer, cachedBackground);
        
        // Apply the high gamma effect
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
        SDL_Rect fullScreenRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &fullScreenRect);
        
        // Reset render target
        SDL_SetRenderTarget(renderer, nullptr);
    }
    
    // Copy the cached background
    SDL_RenderCopy(renderer, cachedBackground, nullptr, nullptr);

    // Rest of the victory screen rendering (stats, buttons, etc.)
    // Get fresh mouse state at the start of each render
    int mouseX, mouseY;
    Uint32 currentMouseState = SDL_GetMouseState(&mouseX, &mouseY);

    // Apply semi-transparent white overlay to create "high gamma" effect
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
    SDL_Rect fullScreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &fullScreen);

    // Create a semi-transparent white box for victory content
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 230);  // More opaque than the overlay
    SDL_Rect victoryBox = {WINDOW_WIDTH / 2 - 150, 80, 300, 300};
    SDL_RenderFillRect(renderer, &victoryBox);
    
    // Draw red outline with blend mode normal
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &victoryBox);

    // Calculate accuracy
    float accuracy = (static_cast<float>(score) / 405.0f) * 100.0f;

    // Render "SUCCESS" label
    SDL_Color color = {0, 128, 0, 255};
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    renderText("SUCCESS!", WINDOW_WIDTH / 2 - 60, 100, color);
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    // Render stats with aligned formatting
    const int labelX = WINDOW_WIDTH / 2 - 120;  // Left align position for labels
    const int valueX = WINDOW_WIDTH / 2 + 120;   // Increased space for right-aligned values
    int yPos = 150;

    // Score
    renderText("Score:", labelX, yPos, color);
    std::string scoreStr = std::to_string(score);
    int textW, textH;
    TTF_SizeText(font, scoreStr.c_str(), &textW, &textH);
    renderText(scoreStr, valueX - textW, yPos, color);
    yPos += 30;

    // Time
    renderText("Time:", labelX, yPos, color);
    std::stringstream timeStr;
    timeStr << elapsedSeconds / 60 << ":" << std::setfill('0') << std::setw(2) << elapsedSeconds % 60;
    TTF_SizeText(font, timeStr.str().c_str(), &textW, &textH);
    renderText(timeStr.str(), valueX - textW, yPos, color);
    yPos += 30;

    // Accuracy
    renderText("Accuracy:", labelX, yPos, color);
    std::stringstream accuracyStr;
    accuracyStr << std::fixed << std::setprecision(1) << accuracy << "%";
    TTF_SizeText(font, accuracyStr.str().c_str(), &textW, &textH);
    renderText(accuracyStr.str(), valueX - textW, yPos, color);
    yPos += 30;

    // Render buttons with enhanced visual effects
    SDL_Rect newGameBtn = {WINDOW_WIDTH / 2 - 100, yPos + 20, 200, 40};
    SDL_Rect exitBtn = {WINDOW_WIDTH / 2 - 100, yPos + 70, 200, 40};

    // Calculate fresh hover states based on current mouse position
    bool newGameHover = (mouseX >= newGameBtn.x && mouseX <= newGameBtn.x + newGameBtn.w &&
                        mouseY >= newGameBtn.y && mouseY <= newGameBtn.y + newGameBtn.h);
    bool exitHover = (mouseX >= exitBtn.x && mouseX <= exitBtn.x + exitBtn.w &&
                     mouseY >= exitBtn.y && mouseY <= exitBtn.y + exitBtn.h);

    // Render buttons with current state
    for (int i = 0; i < 2; i++) {
        SDL_Rect* btn = (i == 0) ? &newGameBtn : &exitBtn;
        bool isHovered = (i == 0) ? newGameHover : exitHover;
        bool isCurrentlyClicked = isHovered && (currentMouseState & SDL_BUTTON_LMASK);

        // Modern color scheme using Material Design-inspired colors
        SDL_Color baseColor = {63, 81, 181, 255};  // Indigo 500
        SDL_Color hoverColor = {92, 107, 192, 255}; // Indigo 400
        SDL_Color clickColor = {48, 63, 159, 255};  // Indigo 700

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
        if (i == 0) {
            renderText("New Game", 
                      WINDOW_WIDTH / 2 - 60 + (isHovered ? (isCurrentlyClicked ? 3 : 1) : 0),
                      yPos + 30 + (isHovered ? (isCurrentlyClicked ? 3 : 1) : 0),
                      white);
        } else {
            renderText("Exit",
                      WINDOW_WIDTH / 2 - 25 + (isHovered ? (isCurrentlyClicked ? 3 : 1) : 0),
                      yPos + 80 + (isHovered ? (isCurrentlyClicked ? 3 : 1) : 0),
                      white);
        }
    }
    
    SDL_RenderPresent(renderer);
}

bool Renderer::handleVictoryScreenClick(int x, int y) {
    int yPos = 150 + 90;  // Match the button positions from renderVictoryScreen
    SDL_Rect newGameBtn = {WINDOW_WIDTH / 2 - 100, yPos + 20, 200, 40};
    SDL_Rect exitBtn = {WINDOW_WIDTH / 2 - 100, yPos + 70, 200, 40};

    if (x >= newGameBtn.x && x <= newGameBtn.x + newGameBtn.w) {
        if (y >= newGameBtn.y && y <= newGameBtn.y + newGameBtn.h) {
            return true;  // New Game clicked
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