PROGCLIENT = client
PROGSERVER = server

CC = g++
CXXFLAGS = -Wall -Wextra -O3

DEBUG= -fsanitize=address

SOURCES = $(wildcard src/network/*.cpp)
SOURCESSERVER = $(wildcard src/server/*.cpp)
SOURCESCLIENT = $(wildcard src/client/*.cpp)

OBJSERVER = $(SOURCESSERVER:.cpp=.o) $(SOURCES:.cpp=.o)
OBJCLIENT = $(SOURCESCLIENT:.cpp=.o) $(SOURCES:.cpp=.o)

RM = rm -rf

all: serv clt

serv: $(PROGSERVER)
clt: $(PROGCLIENT)

$(PROGSERVER): $(OBJSERVER)
	$(CC) -fsanitize=address $(OBJSERVER) $(LDFLAGS) -o $(PROGSERVER)

$(PROGCLIENT): $(OBJCLIENT)
	$(CC) -fsanitize=address $(OBJCLIENT) $(LDFLAGS) -o $(PROGCLIENT)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJSERVER) $(OBJCLIENT) $(PROGSERVER) $(PROGCLIENT)
