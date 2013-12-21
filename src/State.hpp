#ifndef STATE
#define STATE

#include "socket.hpp"
#include "File.hpp"


using namespace std;

enum Status
{
	CONNECT,
	DISCONNECT,
	ACTIVE,
};

enum CS
{
	CLIENT,
	SERVER,
};

class State
{
public :
	State();
	State(CS cs);
	State(Status s);
	State(Status s, CS cs);
	State(const State &s);
	State & operator=(const State &s);
	~State();

	Status status() const;
	void status(Status s);
	string file() const;
	void file(string f);

	friend ostream& operator<<(ostream& os, const State &s);
 private :
	Status _status;
	CS _cs;
	string _file;
};

#endif
