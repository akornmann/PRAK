#include "Server.h"

Server::Server(string port)
{
	_log = new File("/home/kalex/.log/PRAK/Server.log");

	_port = port;
	memset(&_local,0,sizeof _local);
	_local.ai_family = PF_UNSPEC;
	_local.ai_socktype = SOCK_DGRAM;
	_local.ai_flags = AI_PASSIVE;
	_r = getaddrinfo(NULL,_port.c_str(),&_local,&_start);
	_n_socks = 0;

	_client_len = sizeof(struct sockaddr_storage);

	_log->write("Server::Server","Server started");
}

Server::~Server()
{
	_log->write("Server::Server","Server stoped");
}

bool Server::connect()
{
	for(_iterator = _start; _iterator && _n_socks<MAXSOCK; _iterator = _iterator->ai_next)
	{
		_sockets[_n_socks] = socket(_iterator->ai_family,_iterator->ai_socktype,_iterator->ai_protocol);
		if(_sockets[_n_socks] != -1)
		{
			_r = bind(_sockets[_n_socks],_iterator->ai_addr, _iterator->ai_addrlen);
			if(_r != -1) _n_socks++;
		}
	}
	
	freeaddrinfo(_start);

	while(1)
	{
		fd_set readfds;
		int max = 0;
		
		FD_ZERO(&readfds);
		for(int i = 0; i<_n_socks; i++)
		{
			FD_SET(_sockets[i], &readfds);
			if(_sockets[i]>max) max = _sockets[i];
		}
		
		select(max+1, &readfds, NULL, NULL, NULL);

		for(int i = 0; i<_n_socks; i++)
		{
			if(FD_ISSET(_sockets[i], &readfds))
			{
				receive(_sockets[i]);
			}
		}

	}

	return true;
}

bool Server::receive(int s)
{
	_log->write("Server::receive","socket used : "+Converter::itos(s));
	Datagram buffer;
	memset(&buffer,0,DGSIZE);
	
	_r = recvfrom(s,&buffer,DGSIZE,0,(struct sockaddr*) &_client_addr, &_client_len);
	_addr = new AddrStorage((struct sockaddr*) &_client_addr, _log);
       
	toctoc(_addr);
	return true;
}

bool Server::send_to(Datagram* dg, AddrStorage* addr)
{
	_r = sendto(3,dg, DGSIZE, 0, addr->sockaddr(), addr->len());
	return true;
}

void Server::toctoc(AddrStorage* addr)
{
	Datagram dg;
	memset(&dg,0,DGSIZE);
	dg.code = 0;
	dg.seq = 0;
	string s = "Qui est là ?";
	strcpy(dg.data,Converter::stocs(s));
	
	send_to(&dg, addr);
}
