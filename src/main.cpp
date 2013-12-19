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
	string port;
	bool server;
	switch(argc)
	{
	case 2 :
		port = argv[1];
		server = false;
	case 3 :
		port = argv[2];
		server = true;
		break;
	default :
		server = false;
		port = "4242";
		break;
	}

	if(server)
	{
		try
		{
			Server s(port);
		}
		catch(const Exception &e)
		{
			cout << e.what() << endl;
		}
	}
	else
	{
		Shell s;
	}

	return true;
}
