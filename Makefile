CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -g

TARGET = tictactoe
SOURCES = main.cpp functions.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.cpp functions.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(OBJECTS) $(TARGET)

