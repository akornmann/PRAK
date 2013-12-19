#ifndef STATE
#define STATE

#include "socket.hpp"
#include "File.hpp"


#define CLIENT false;
#define SERVER true;

typedef bool CS;

using namespace std;

enum Status
{
	CONNECT,
	DISCONNECT,
	ACTIVE,
};

class State
{
public :
	State();
	State(Status s);
	~State();
	Status status() const { return _status; };
	void status(Status s) { _status = s; };

	friend ostream& operator<<(ostream& os, const State &s);
 private :
	Status _status;
	CS _cs;
};

#endif
