#ifndef ADDRSTORAGE
#define ADDRSTORAGE

#include "socket.hpp"
#include "File.hpp"
#include "Converter.hpp"

using namespace std;

class AddrStorage
{
 public :
	AddrStorage(); //From Server side : client adress (part 1)
	void build(int s); //From Server side : client address (part 2)

	AddrStorage(string addr, string port); //From Client side : server address
	AddrStorage(const AddrStorage &as); //copy constructor
	AddrStorage & operator=(const AddrStorage &as); //assignement operator
	~AddrStorage(); //destructor

	//Getter
	struct sockaddr_storage * storage();
	int sock() const;
	string paddr() const;
	string pport() const;
	socklen_t len() const;
	int family() const;
	struct sockaddr * sockaddr() const;
	
	friend ostream & operator<<(ostream &os, const AddrStorage &addr);

 private :
	struct sockaddr_storage _addr;
	struct sockaddr *_sockaddr;
	int _n_port;
	
	socklen_t _len;
	int _family;

	string _p_addr;
	string _p_port;

	int _socket;
};

#endif
