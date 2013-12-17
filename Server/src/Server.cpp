#include "Server.h"

bool Server::init(Datagram* dg, int code, int seq, string s)
{
	if(s.length()>=512)
	{
		_log->write("Client::init","Datagram overflow");
		return false;
	}
	else
	{
		memset(dg,0,DGSIZE);
		
		dg->code = code;
		dg->seq = seq;
		strcpy(dg->data,Converter::stocs(s));
		return true;
	}
}

bool Server::init(Datagram* dg, int code, int seq)
{
	string s = "";
	return init(dg, code, seq, s);
}

void Server::show(string prefix, Datagram* dg)
{
	cout << prefix << endl
	     << "Code : " << dg->code << endl
	     << " Seq : " << dg->seq << endl
	     << "Data : " << dg->data << endl;

		return;
}

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

	_log->write("Server::Server","Server started");
}

Server::~Server()
{
	disconnect();
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

bool Server::disconnect()
{
	for(int i = 0; i<_n_socks; i++)
	{
		close(_sockets[i]);
	}
	return true;
}

bool Server::receive(int s)
{
	struct sockaddr_storage temp_addr;
	socklen_t temp_len;
	temp_len = sizeof(struct sockaddr_storage);

	Datagram buffer;
	memset(&buffer,0,DGSIZE);
	
	_r = recvfrom(s,&buffer,DGSIZE,0,(struct sockaddr*) &temp_addr, &temp_len);
	AddrStorage* addr = new AddrStorage((struct sockaddr*) &temp_addr, s, _log);

	return process(&buffer,addr);
}

bool Server::send_to(Datagram* dg, AddrStorage* addr)
{
	_r = sendto(addr->socket(),dg, DGSIZE, 0, addr->sockaddr(), addr->len());
	return true;
}


/*
 *
 * Protocoles de base
 *
 *
 */

bool Server::toctoc(Datagram* dg, AddrStorage* addr)
{
	dg->seq++;
	return send_to(dg, addr);
}

bool Server::send_file(Datagram* dg, AddrStorage* addr)
{
	string file = "";
	switch(dg->seq)
	{
	case -1 :
		//trouver le fichier local ou à l'exterieur !
		file = dg->data;
		dg->seq = 2532;
		send_to(dg, addr);
		break;
	case 0:
		break;
		
	default :
		cout << "End of transfer" << endl;
		break;
	}

	return true;
}

/*
 *
 * Surcouche serveur
 *
 *
 */

bool Server::process(Datagram* dg, AddrStorage* addr)
{
	bool res = false;

	cout << "process " << dg->code << endl;

	switch(dg->code)
	{
	case 0 :
		res = toctoc(dg,addr);
		break;
	case 1 :
		res = send_file(dg,addr);
		break;
	default :
		break;
	}

	return res;
}
