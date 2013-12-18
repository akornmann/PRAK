#ifndef STATE
#define STATE

#include "socket.h"
#include "File.h"

class State
{
 private :
	Status _status;
	File* _file;
	
 public :
	Status status() { return _status; };
	File* file() { return _file; }; 
};


#endif
