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
	rm -r a1
	rm -r a2
	rm -r a3
	cp -r bin/ a1
	cp -r bin/ a2
	cp -r bin/ a3
%.o: %.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf bin/* rm -rf src/*~ rm -rf src/#* rm -rf *.o

