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
	MINSIZE,
};

class State
{
public :
	State();
	State(Status s);

	~State();

	void refresh();

	bool is_meta();
	bool is_data();

	friend ostream& operator<<(ostream& os, const State &s);

	Status _status;

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
