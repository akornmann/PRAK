CXX=g++
CFLAGS=-Wall -std=c++0x

EXEC=bin/PRAK

SRC = $(notdir $(wildcard src/*.cpp))
OBJS = $(SRC:.cpp=.o)

vpath %.cpp src

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^
	mv *.o bin/
	cp src/server.cfg bin/

%.o: %.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf bin/* rm -rf src/*~ rm -rf src/#* rm -rf *.o

