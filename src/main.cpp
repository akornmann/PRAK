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
	case 3 :
		type = argv[1];
		port = argv[2];
		break;
	default :
		cout << "Invalid arguments" << endl;
		cout << argv[0] << " server|client [port]" << endl;
		break;
	}

	if(type == "server")
	{
		string config = "server.cfg";
		Server s(port,config);
	}
	else if(type == "client")
	{
		Shell s;
	}
	else
	{
		cout << "Invalid arguments" << endl;
		cout << argv[0] << " server|client [port]" << endl;
	}

	return true;
}
