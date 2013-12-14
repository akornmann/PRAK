CXX=g++
CFLAGS=-Wall

EXEC=bin/Server

SRC = $(notdir $(wildcard src/*.cpp))
OBJS = $(SRC:.cpp=.o)

vpath %.cpp src

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^
	mv *.o bin
%.o: %.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf bin/*.o rm -rf bin/*~ rm -rf src/*~ rm bin/log*
cleaner : clean
	rm -rf $(EXEC)

