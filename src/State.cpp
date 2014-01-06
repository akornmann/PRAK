#include "State.hpp"

State::State():_status(DISCONNECT),_init_seq(0),_size(0),_file(""),_title(""),_buffer(NULL)
{
}


State::State(Status s):_status(s),_init_seq(0),_size(0),_file(""),_title(""),_buffer(NULL)
{
}

State::~State()
{
}

void State::refresh()
{
	if(_buffer!=NULL)
	{
		delete[] _buffer;
		_buffer = NULL;
	}
	_file = "";
	_title = "";
	_size = 0;
	_init_seq = 0;
	_received_packet.resize(0);

	_status = DISCONNECT;

	return;
}

bool State::is_meta()
{
	bool f = (_file != "");
	bool t = (_title != "");
	bool s = (_size>0);

	return f&&t&&s;
}

bool State::is_data()
{
	bool res = true;
	vector<bool>::const_iterator it;
	for(it=_received_packet.begin();it!=_received_packet.end();++it)
	{
		res = res && *it;
	}
	return res;
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
