#include "AddrStorage.hpp"

AddrStorage::AddrStorage():_p_addr(""),_p_port("")
{
}

void AddrStorage::build(int s)
{
	_socket = s;

	_sockaddr = (struct sockaddr*) &_addr;

	_family = _sockaddr->sa_family;
	_len = sizeof(struct sockaddr);

	struct in_addr* n_addr;
	struct in6_addr* n_addr6;

	char paddr[INET6_ADDRSTRLEN];
	unsigned short port;

	switch(_family)
	{
	case AF_INET :
	        n_addr = &((struct sockaddr_in *) _sockaddr)->sin_addr;
		port = ((struct sockaddr_in *) _sockaddr)->sin_port;
		inet_ntop(_family, n_addr, paddr, INET_ADDRSTRLEN);
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



AddrStorage::AddrStorage(string addr, string port):_p_addr(addr),_p_port(port),_socket(-1)
{
	struct sockaddr_in *addr_ipv4 = (struct sockaddr_in*) &_addr;
	struct sockaddr_in6 *addr_ipv6 = (struct sockaddr_in6*) &_addr;
	
	memset(&_addr,0,sizeof(struct sockaddr_storage));

	_n_port = htons(Converter::stoi(_p_port));

	const char* c_addr = _p_addr.c_str();
	if(inet_pton(AF_INET6, c_addr, &addr_ipv6->sin6_addr) == 1)
	{
		_family = PF_INET6;
		addr_ipv6->sin6_family = AF_INET6;
		addr_ipv6->sin6_port = _n_port;
		_len = sizeof *addr_ipv6;
	}
	else if(inet_pton(AF_INET,c_addr, &addr_ipv4->sin_addr) == 1)
	{
		_family = PF_INET;
		addr_ipv4->sin_family = AF_INET;
		addr_ipv4->sin_port = _n_port;
		_len = sizeof *addr_ipv4;
	}

	_sockaddr = (struct sockaddr*) &_addr;
}

AddrStorage::AddrStorage(const AddrStorage &as)
{	
	_addr = as._addr;
	_n_port = as._n_port;
	
	_len = as._len;
	_family = as._family;

	_p_addr = as._p_addr;
	_p_port = as._p_port;

	_socket = as._socket;

	_sockaddr = (struct sockaddr*) &_addr;
}

AddrStorage & AddrStorage::operator=(const AddrStorage &as)
{
	if(this!=&as) //Prevent auto-copy
	{
		_addr = as._addr;
		_n_port = as._n_port;
	
		_len = as._len;
		_family = as._family;

		_p_addr = as._p_addr;
		_p_port = as._p_port;

		_socket = as._socket;

		_sockaddr = (struct sockaddr*) &_addr;
	}

	return *this;
}

AddrStorage::~AddrStorage()
{
}


int AddrStorage::family() const
{
	return _family;
}

struct sockaddr* AddrStorage::sockaddr() const
{
	return _sockaddr;
}

socklen_t AddrStorage::len() const
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

int AddrStorage::sock() const
{
	return _socket;
}

ostream & operator<<(ostream &os, const AddrStorage &addr)
{
	return os << addr._p_addr << ":" << addr._p_port;
}


struct sockaddr_storage * AddrStorage::storage()
{
	return &_addr;
}
