#ifndef DATAGRAM
#define DATAGRAM

#include "socket.hpp"
#include "Converter.hpp"

using namespace std;

#define DATASIZE 512

class Datagram
{
 public :
	Datagram(int code, int seq, string str);
	Datagram(int code, int seq);
	Datagram(int code);
	Datagram();
	
	void init(int c, int s, string str);
	void init(int code, int seq);
	void init(int code);

	int size();
	int length();

	friend ostream& operator<<(ostream &os, const Datagram &dg);
	
	int code;
	int seq;
	char data[DATASIZE];	
};

#endif
