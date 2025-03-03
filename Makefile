CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
SDL_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

# Use wildcard to get all .cpp files in the current directory
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = sudoku

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(SDL_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET).exe

run: $(TARGET)
	./$(TARGET)