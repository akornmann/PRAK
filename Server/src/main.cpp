/*
 * Alexandre Kornmann
 * Projet reseau
 * Master 2 CSMI
 * Serveur
 */

#include "Server.h"

int main(int argc, const char* argv[])
{
	string port;
	switch(argc)
	{
	case 2 :
		port = argv[1];
		break;
	default :
		port = "4242";
		break;
	}

	Server s(port);
	s.connect();

	return 0;
}
