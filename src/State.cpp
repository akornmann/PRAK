#include "State.h"

ostream& operator<<(ostream& os, const State& s)
{
	string status;
	switch(s._status)
	{
	case CONNECT :
		status = "Connect";
	case DISCONNECT :
		status = "Disconnect";
	case ACTIVE :
		status = "Active : " + s._file->file();
	}
	return os << status << endl;
}
