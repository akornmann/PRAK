#include "State.hpp"

State::State():_status(DISCONNECT),_cs(CLIENT)
{
}

State::State(CS cs):_status(DISCONNECT),_cs(cs)
{
}


State::State(Status s, CS cs):_status(s),_cs(cs)
{
}

State::State(const State &s)
{
	_status = s._status;
	_cs = s._cs;
	_file = s._file;
}

State & State::operator=(const State &s)
{
	if(this!=&s) //Prevent auto-copy
	{
		_status = s._status;
		_cs = s._cs;
		_file = s._file;	
	}

	return *this;
}

State::~State()
{
}

Status State::status() const
{
	return _status;
}

void State::status(Status s)
{
	_status = s;
}

string State::file() const
{
	return _file;
}

void State::file(string f)
{
	_status = ACTIVE;
	_file = f;
}


ostream& operator<<(ostream& os, const State &s)
{
	string status;
	switch(s._status)
	{
	case CONNECT :
		status = "Connect";
		break;
	case DISCONNECT :
		status = "Disconnect";
		break;
	case ACTIVE :
		status = "Active";
		break;
	default :
		status = "Unknow status";
		break;
	}
	return os << status;
}
