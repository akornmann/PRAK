CXX=g++
CFLAGS=-Wall -std=c++0x -g -O0

EXEC=bin/PRAK

SRC = $(notdir $(wildcard src/*.cpp))
OBJS = $(SRC:.cpp=.o)

vpath %.cpp src

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^
	mv *.o bin/
	cp src/server.cfg bin
	cp -r bin /home/kalex/server1
	cp -r bin /home/kalex/server2
	cp -r bin /home/kalex/server3
%.o: %.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf bin/* rm -rf src/*~ rm -rf src/#* rm -rf *.o

