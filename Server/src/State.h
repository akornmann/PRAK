#ifndef STATE
#define STATE

#include "socket.h"
#include "File.h"


#define CLIENT false;
#define SERVER true;

typedef bool CS;

using namespace std;

class State
{
 private :
	Status _status;
	File* _file;
	CS _cs;
	
	friend ostream& operator<<(ostream& os, const State& s);
 public :
	Status status() { return _status; };
	void status(Status s) { _status = s; };
	File* file() { return _file; };
	void file(File* f) { _file = f; };
};

#endif
