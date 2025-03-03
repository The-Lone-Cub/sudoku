CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I$(CURDIR)/include
SDL_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

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