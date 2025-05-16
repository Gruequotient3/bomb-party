PROG = main

CC = g++
CXXFLAGS = -Wall -Wextra -O3

DEBUG= -fsanitize=address

SOURCES = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)

OBJ = $(SOURCES:.cpp=.o)

RM = rm -rf

all: $(PROG)

debug: $(OBJ)
	$(CC) $(DEBUG) $(OBJ) $(LDFLAGS) -o $(PROG)

$(PROG): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(PROG)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(PROG)
