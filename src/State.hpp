#ifndef STATE
#define STATE

#include "socket.hpp"
#include "File.hpp"
#include <vector>

using namespace std;

enum Status
{
	CONNECT,
	DISCONNECT,
	ACTIVE,
	META,
	DATA,
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

	~State();

	bool is_meta();
	bool is_data();

	friend ostream& operator<<(ostream& os, const State &s);

	Status _status;

	CS _cs;

	//PACKET RECEIVED
	int _init_seq;
	vector<bool> _received_packet;

	//METADATA
	int _size;
	string _file;
	string _title;

	//DATA
	char *_buffer;



};

#endif
