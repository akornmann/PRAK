#ifndef SOCKET
#define SOCKET

//STDL
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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

#define MAXSOCK 5


struct Datagram
{
	int code;
	int seq;
	char data[512];
};

#define DGSIZE 520
#define DATASIZE 4

enum Status
{
	CONNECT,
	DISCONNECT,
	ACTIVE,
};

#endif
