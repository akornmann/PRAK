#ifndef ADDRSTORAGE
#define ADDRSTORAGE

#include "socket.hpp"
#include "File.hpp"
#include "Converter.hpp"
#include "Exception.hpp"

using namespace std;

class AddrStorage
{
 public :
	AddrStorage(struct sockaddr *addr, int s); //From Server side : client address
	AddrStorage(string addr, string port); //From Client side : server address
	//AddrStorage(const AddrStorage &as); //copy constructor
	AddrStorage & operator=(const AddrStorage &as); //assignement operator
	~AddrStorage(); //destructor

	//Setter
	void sock(int s);
	//Getter
	int sock() const;
	string paddr() const;
	string pport() const;
	socklen_t len() const;
	int family() const;
	struct sockaddr* sockaddr() const;
	
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