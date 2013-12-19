#include "State.hpp"

State::State():_status(DISCONNECT)
{
}

State::State(Status s):_status(s)
{
}

State::~State()
{
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
