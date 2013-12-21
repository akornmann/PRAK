#include "Server.hpp"

Server::Server(string port):_run(true)
{
	//We look every interfaces we can use
	struct addrinfo local, *iterator, *start;
	
	memset(&local,0,sizeof(struct addrinfo));
	local.ai_family = PF_UNSPEC;
	local.ai_socktype = SOCK_DGRAM;
	local.ai_flags = AI_PASSIVE;
	int r = getaddrinfo(NULL,port.c_str(),&local,&start);
	_n_socks = 0;

	for(iterator=start;iterator&&_n_socks<MAXSOCK;iterator=iterator->ai_next)
	{
		_sockets[_n_socks] = socket(iterator->ai_family,iterator->ai_socktype,iterator->ai_protocol);
		if(_sockets[_n_socks] != -1)
		{
			r = bind(_sockets[_n_socks],iterator->ai_addr,iterator->ai_addrlen);
			if(r != -1)
			{
				_n_socks++;
			}
			else _exc.push_back(Exception("Server::Server : Bind failed.", __LINE__));
		}
		else _exc.push_back(Exception("Server::Server : socket failed.", __LINE__));
	}
	
	freeaddrinfo(start);

	run();
}

Server::~Server()
{
	for(int i=0;i<_n_socks;i++)
	{
		close(_sockets[i]);
	}
}

int Server::sock(const AddrStorage &addr)
{
	return addr.sock();
}

void Server::run()
{
	//Initialization of socket descriptor
	fd_set readfds;
	int max = 0;
		
	FD_ZERO(&readfds);
	for(int i=0;i<_n_socks;i++)
	{
		FD_SET(_sockets[i], &readfds);
		if(_sockets[i]>max) max = _sockets[i];
	}

	Datagram dg;;

	cout << "Server started" << endl;

	//Start server (infinite loop)
	while(_run)
	{
		if(select(max+1, &readfds, NULL, NULL, NULL)>0)
		{
			for(int i = 0; i<_n_socks; i++)
			{
				if(FD_ISSET(_sockets[i], &readfds))
				{
					AddrStorage *addr = new AddrStorage();
					receive(dg,addr,_sockets[i]);
					cout << dg << " from " << *addr << endl;
					update_client_map(*addr);
					process(dg,*addr);
				}
			}
		}
		else _exc.push_back(Exception("Server::server : select failed.",__LINE__));
	}
}

bool Server::send_to(const Datagram &dg, const AddrStorage &addr)
{
	int r = sendto(sock(addr),&dg,sizeof(Datagram),0,addr.sockaddr(),addr.len());
	
	if(r!=-1) return true;
	else
	{
		_exc.push_back(Exception("Server::send_to : sendto failed.",__LINE__));
		return false;
	}
}

bool Server::receive(Datagram &dg, AddrStorage *addr, int s)
{
	struct sockaddr_storage *temp_addr = addr->storage();
	//struct sockaddr_storage temp_addr;
	socklen_t temp_len;
	temp_len = sizeof(struct sockaddr_storage);

	int r = recvfrom(s,&dg,sizeof(Datagram),0,(struct sockaddr*) temp_addr, &temp_len);
	
	if(r!=-1)
	{
		addr->build(s);
		return true;
	}
	else
	{
		_exc.push_back(Exception("Server::receive : recvfrom failed.", __LINE__));
		return false;
	}
}


/*
 *
 * Protocoles de base
 *
 *
 */

bool Server::toctoc(Datagram &dg, const AddrStorage &addr)
{
	dg.seq++;
	return send_to(dg,addr);
}

bool Server::send_file(const Datagram &dg, const AddrStorage &addr)
{
	string file = "";
	int end_size = 0, seq = 0, size = 0, packet_number = 0;

	Datagram s;
	char* buffer;
	
	switch(dg.seq)
	{
	case -1 :
		//trouver le fichier local ou à l'exterieur !
		file = dg.data; //hidden cast
		_curr.file(file);
		size = _curr.size();
		s.init(0,size);
		send_to(s,addr);
		break;
	case 0 :
		packet_number = ceil((float) _curr.size()/ (float) (DATASIZE-1));
		size = _curr.size();
		for(int i=1;i<=packet_number;i++)
		{
			if(i*(DATASIZE-1)<=size) //C'est un paquet intermediaire, data est complet.
			{
				seq = i*(DATASIZE-1);
				buffer = _curr.readChar(DATASIZE-1); //Buffer sera de taille DATASIZE (avec le \0 final)
				s.init(1,seq,Converter::cstos(buffer));
				send_to(s,addr);
			}
			else //C'est le paquet final
			{
				end_size = size-(i-1)*(DATASIZE-1);
				buffer = _curr.readChar(end_size);
				s.init(1,size,Converter::cstos(buffer));
				send_to(s,addr);
			}
		}
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

bool Server::process(Datagram &dg, const AddrStorage &addr)
{
	bool res = false;

	switch(dg.code)
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

bool Server::update_client_map(const AddrStorage &addr)
{
	addr_map::const_iterator it = _client_map.find(addr);
	if(it == _client_map.end())
	{
		State new_state(CONNECT);
		_client_map[addr] = new_state;
	}

	return true;
}
