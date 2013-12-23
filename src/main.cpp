/*
 * Alexandre Kornmann
 * Projet reseau
 * Master 2 CSMI
 */

#include "Server.hpp"
#include "Shell.hpp"
#include "Exception.hpp"

int main(int argc, const char* argv[])
{
	string type,port,addr;
	switch(argc)
	{
	case 2 :
		type = argv[1];
		break;
	case 4 :
		type = argv[1];
		addr = argv[2];
		port = argv[3];
		break;
	default :
		cout << "Invalid arguments" << endl;
		cout << argv[0] << " server|client [port]" << endl;
		break;
	}

	if(type == "server")
	{
		string config = "server.cfg";
		Server s(addr,port,config);
	}
	else if(type == "client")
	{
		Shell s;
	}
	else
	{
		cout << "Invalid arguments" << endl;
		cout << argv[0] << " server|client [ip port]" << endl;
	}

	return true;
}
