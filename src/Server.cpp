#include "Server.hpp"

Server::Server(string port, string config):Client(config),_run(true)
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
	
	if(r!=-1) 
	{
		cout << dg << " to " << addr << endl;
		return true;
	}
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
		cout << dg << " from " <<* addr << endl;
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

bool Server::connect_ack(const Datagram &dg, const AddrStorage &addr)
{
	Datagram s(CONNECTRA, dg.seq+1, "Hey pretty client !");
	_client_map[addr]._status = CONNECT;
	return send_to(s,addr);
}

bool Server::disconnect_ack(const Datagram &dg, const AddrStorage &addr)
{
	Datagram s(DISCONNECTRA, dg.seq-1, "Bye lovely client !");
	_client_map[addr]._status = DISCONNECT;
	return send_to(s,addr);
}

bool Server::send_file(const Datagram &dg, const AddrStorage &addr)
{
	string new_file = "", file = "";
	int end_size = 0, seq = 0, size = 0, packet_number = 0;

	Datagram s;

	char* buffer;
	State *current = &_client_map[addr];
	File *f = new File();

	switch(dg.seq)
	{
	case -1 :
		//trouver le fichier local ou à l'exterieur !
		new_file = dg.data; //hidden cast
		current->_file = new_file;
		f->file(new_file);
		size = f->size();

		s.init(1,size);
		send_to(s,addr);
		break;
	case 0 :
		f->file(current->_file);
		size = f->size();

		packet_number = ceil((float) size/ (float) (DATASIZE-1));
		for(int i=1;i<=packet_number;i++)
		{
			if(i*(DATASIZE-1)<=size)
			{
				seq = i*(DATASIZE-1);
				buffer = f->readChar(DATASIZE-1);
				s.init(1,seq,Converter::cstos(buffer));
				send_to(s,addr);
			}
			else
			{
				end_size = size-(i-1)*(DATASIZE-1);
				buffer = f->readChar(end_size);
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


	//PROTOCOLE A AMELIORER !!!
	//VERIFICATION PAS DE PERTE DE PAQUETS
}

bool Server::get_file(const Datagram &dg, const AddrStorage &addr)
{
	int i, init, size, packet_number, current_packet;
	Datagram asw;

	State *current = &_client_map[addr];
	File *f;

	switch(current->_status)
	{
	case CONNECT :
		asw.init(UPLOAD,dg.seq,"I'm ready !");
		send_to(asw,addr);
		current->_init_seq = dg.seq;
		current->_status = META;
		break;

	case META :
		if(dg.seq==-2)
		{
			current->_file = dg.data;
			remove_file(current->_file);
		}
		if(dg.seq==-1) current->_title = dg.data;
		if(dg.seq>0)
		{
			current->_size = dg.seq;
			current->_buffer = new char[dg.seq];
			packet_number = ceil((float) current->_size/(float) (DATASIZE-1));
			current->_received_packet.resize(packet_number,false);
		}

		if(current->is_meta())
		{
			asw.init(UPLOAD,0,"Metas are nice.");
			send_to(asw,addr);
			current->_status = DATA;
		}
		break;

	case DATA :
		size = current->_size;
		packet_number = ceil((float) size/(float) (DATASIZE-1));
		if(dg.seq%(DATASIZE-1)==0)
		{
			init = dg.seq-(DATASIZE-1);
			current_packet = dg.seq/(DATASIZE-1);
		}
		else
		{
			init = dg.seq-(size-(packet_number-1)*(DATASIZE-1));
			current_packet = packet_number-1;
		}

		for(i=init;i<=dg.seq;i++)
		{
			current->_buffer[i] = dg.data[i-init];
		}
		current->_received_packet[current_packet] = true;
		
		asw.init(UPLOAD,dg.seq,"Ack");
		send_to(asw,addr);

		if(current->is_data())
		{
			current->_received_packet.resize(0);
			current->_status = DISCONNECT;
			f = new File(current->_file);
			f->write(Converter::cstos(current->_buffer));
			current->_file = "";
			current->_title = "";
			current->_size = 0;
			delete[] current->_buffer;
			delete f;
		}
		break;

	default :
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

bool Server::process(const Datagram &dg, const AddrStorage &addr)
{
	bool res = false;

	switch(dg.code)
	{
	case CONNECT :
		res = connect_ack(dg,addr);
		break;
	case DISCONNECT :
		res = disconnect_ack(dg,addr);
		break;
	case DOWNLOAD :
		res = send_file(dg,addr);
		break;
	case UPLOAD :
		res = get_file(dg,addr);
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
		State new_state(DISCONNECT,CLIENT);
		_client_map[addr] = new_state;
	}

	return true;
}

bool Server::remove_file(const string &file)
{
	//penser a supprimer sur tout les serveurs
	return remove(Converter::stocs(file));
}
