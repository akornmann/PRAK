#include "State.hpp"

State::State():_status(DISCONNECT),_cs(CLIENT),_init_seq(0),_size(0),_file(""),_title("")
{
}

State::State(CS cs):_status(DISCONNECT),_cs(cs),_init_seq(0),_size(0),_file(""),_title("")
{
}


State::State(Status s, CS cs):_status(s),_cs(cs),_init_seq(0),_size(0),_file(""),_title("")
{
}

State::~State()
{
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
	cout << "is data : ";
	bool res = true;
	vector<bool>::const_iterator it;
	for(it=_received_packet.begin();it!=_received_packet.end();++it)
	{
		res = res && *it;
		if(*it) cout << "1";
		else cout << "0";
	}
	cout << endl;
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
