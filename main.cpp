#include "game.h"
#include <iostream>

int main(int, char**) {
    Game game;
    
    if (!game.init()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }

    game.run();
    return 0;
}