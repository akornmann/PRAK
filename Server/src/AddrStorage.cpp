#include "AddrStorage.h"

AddrStorage::AddrStorage(struct sockaddr* addr, int s, File* log)
{
	_log = log;
	_socket = s;
	_sockaddr = addr;
	_family = addr->sa_family;
	_len = sizeof *addr;

	struct in_addr* n_addr;
	struct in6_addr* n_addr6;

	char paddr[INET6_ADDRSTRLEN];
	unsigned short port;

	switch(_family)
	{
	case AF_INET :
	        n_addr = &((struct sockaddr_in *) _sockaddr)->sin_addr;
		port = ((struct sockaddr_in *) _sockaddr)->sin_port;
		inet_ntop(_family, n_addr, paddr, INET6_ADDRSTRLEN);
		break ;
	case AF_INET6 :
		n_addr6 = &((struct sockaddr_in6 *) _sockaddr)->sin6_addr;
		port = ((struct sockaddr_in6 *) _sockaddr)->sin6_port;
		inet_ntop(_family, n_addr6, paddr, INET6_ADDRSTRLEN);
		break ;
	}
	port = ntohs(port);

	_p_port = Converter::itos(port);
	_p_addr = Converter::cstos(paddr);
}

AddrStorage::AddrStorage(string addr, string port, int s, File* log)
{
	_log = log;
	_socket = s;

	_p_port = port;
	_p_addr = addr;

	_addr_ipv4 = (struct sockaddr_in*) &_addr;
	_addr_ipv6 = (struct sockaddr_in6*) &_addr;
	
	memset(&_addr,0,sizeof _addr);

	_n_port = htons(Converter::stoi(_p_port));

	const char* c_addr = _p_addr.c_str();
	if(inet_pton(AF_INET6, c_addr, &_addr_ipv6->sin6_addr) == 1)
	{
		_family = PF_INET6;
		_addr_ipv6->sin6_family = AF_INET6;
		_addr_ipv6->sin6_port = _n_port;
		_len = sizeof *_addr_ipv6;
	}
	else if(inet_pton(AF_INET,c_addr, &_addr_ipv4->sin_addr) == 1)
	{
		_family = PF_INET;
		_addr_ipv4->sin_family = AF_INET;
		_addr_ipv4->sin_port = _n_port;
		_len = sizeof *_addr_ipv4;
	}
	else _log->write("AddrStorage","Format d'adresse inconnue ("+_p_port+":"+_p_addr+")");

	_sockaddr = (struct sockaddr*) &_addr;
}

int AddrStorage::family()
{
	return _family;
}

struct sockaddr* AddrStorage::sockaddr()
{
	return _sockaddr;
}

socklen_t AddrStorage::len()
{
	return  _len;
}

string AddrStorage::paddr() const
{
	return _p_addr;
}

string AddrStorage::pport() const
{
	return _p_port;
}

int AddrStorage::socket()
{
	return _socket;
}

void AddrStorage::socket(int s)
{
	_socket = s;
}


void AddrStorage::show()
{
	cout << "Adresse IP : " << paddr() << endl
	     << "  Port UDP : " << pport() << endl;
}
