#ifndef SOCKET
#define SOCKET

//STDL
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Entree/sortie
#include <iostream>
#include <sstream>

//Sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

//fork
#include <sys/times.h>
#include <inttypes.h>
#include <sys/wait.h>

#define MAXSOCK 5


struct Datagram
{
	int code;
	char data[512];
};


enum Code
{
	TOCTOC,
};

enum Status
{
	CONNECT,
	DISCONNECT,
	SEND_FILE,
	RECEIVE_FILE,
};

#endif
