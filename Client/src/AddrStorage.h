#ifndef ADDRSTORAGE
#define ADDRSTORAGE

#include "socket.h"
#include "File.h"
#include "Converter.h"

using namespace std;

class AddrStorage
{
 private :
	struct sockaddr_storage _addr;
	struct sockaddr_in* _addr_ipv4;
	struct sockaddr_in6* _addr_ipv6;
	struct sockaddr* _sockaddr;
	int _n_port;
	
	int _socket;

	socklen_t _len;
	int _family;

	string _p_addr;
	string _p_port;

	File* _log;

 public :
	AddrStorage(struct sockaddr* addr, int s, File* log); //From Server side : client address
	AddrStorage(string addr, string port, int s, File* log); //From Client side : server address

	int family();
	struct sockaddr* sockaddr();
	socklen_t len();
	string paddr();
	string pport();
	int socket();
	void socket(int s);

	void show();
};

#endif
