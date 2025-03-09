# Default compiler if none specified
CXX ?= g++

# Set flags based on compiler
ifeq ($(findstring cl,$(CXX)),cl)
    CXXFLAGS = /std:c++17 /W4 /EHsc /I$(CURDIR)/include
    SDL_FLAGS = SDL2main.lib SDL2.lib SDL2_ttf.lib SDL2_image.lib /subsystem:windows
else
    CXXFLAGS = -std=c++17 -Wall -Wextra -I$(CURDIR)/include
    # Detect OS and set appropriate flags
    ifeq ($(OS),Windows_NT)
        SDL_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -mwindows
    else
        SDL_FLAGS = -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image
    endif
endif

TARGET = sudoku
.PHONY: all clean

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:src/%.cpp=obj/%.o)

# Create necessary directories
$(shell mkdir -p obj)

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CXX) $(OBJS) -o $(TARGET) $(SDL_FLAGS)

obj/%.o: src/%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(TARGET).exe

run: $(TARGET)
	@./$(TARGET)