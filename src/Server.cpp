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

	Datagram dg;

	
	AddrStorage *addr = new AddrStorage();
	//Start server (infinite loop)
	while(_run)
	{
		if(select(max+1, &readfds, NULL, NULL, NULL)>0)
		{
			for(int i = 0; i<_n_socks; i++)
			{
				if(FD_ISSET(_sockets[i], &readfds))
				{
					receive(dg,addr,_sockets[i]);
					update_client_map(*addr);
					process(dg,*addr);
				}
			}
		}
		else _exc.push_back(Exception("Server::server : select failed.",__LINE__));
	}
	delete addr;
}

void Server::send_to(const Datagram &dg, const AddrStorage &addr)
{
	int r = sendto(sock(addr),&dg,sizeof(Datagram),0,addr.sockaddr(),addr.len());
	
	if(r==-1) _exc.push_back(Exception("Server::send_to : sendto failed.",__LINE__));
	else cout << dg << " to " << addr << endl;
}

void Server::receive(Datagram &dg, AddrStorage *addr, int s)
{
	struct sockaddr_storage *temp_addr = addr->storage();
	//struct sockaddr_storage temp_addr;
	socklen_t temp_len;
	temp_len = sizeof(struct sockaddr_storage);

	int r = recvfrom(s,&dg,sizeof(Datagram),0,(struct sockaddr*) temp_addr, &temp_len);
	
	if(r!=-1)
	{
		addr->build(s);
		cout << dg << " from " << *addr << endl;
	}
	else _exc.push_back(Exception("Server::receive : recvfrom failed.", __LINE__));
	
	return;
}


/*
 *
 * Protocoles de base
 *
 *
 */

void Server::connect_ack(const Datagram &dg, const AddrStorage &addr)
{
	Datagram s(CONNECTRA, dg.seq+1, "Hey pretty client !");
	_client_map[addr].refresh();
	_client_map[addr]._status = CONNECT;
	send_to(s,addr);
	return;
}

void Server::disconnect_ack(const Datagram &dg, const AddrStorage &addr)
{
	Datagram s(DISCONNECTRA, dg.seq-1, "Bye lovely client !");
	_client_map[addr].refresh();
	send_to(s,addr);
	return;
}

void Server::send_file(const Datagram &dg, const AddrStorage &addr)
{
	int init, size;
	string file;

	Datagram asw;

	State *current = &_client_map[addr];
	File *f;

	char *buffer;
	
	if(dg.seq==META) current->_status = META;

	switch(current->_status)
	{
	case META :
		file = Converter::cstos(dg.data);
		if(find_file(file))
		{
			f = new File(file);
			size = f->size();
			asw.init(DOWNLOAD,size,"Here is meta !");
			send_to(asw,addr);
			current->_status = DATA;
			current->_file = file;
			current->_buffer = f->readChar(size);
			current->_size = size;
			delete f;
		}
		else
		{
			asw.init(DOWNLOAD,-1,"File doesn't exist !");
			send_to(asw,addr);
			current->refresh();
		}
		break;

	case DATA :
		if(dg.seq == current->_size) init = dg.seq-(dg.seq%(DATASIZE-1));
		else init = dg.seq-(DATASIZE-1);
		
		buffer = new char[dg.seq-init+1];
		buffer[dg.seq-init] = '\0';

		for(int j=init;j<dg.seq;j++)
		{
			buffer[j-init] = current->_buffer[j];
		}
		
		asw.init(DOWNLOAD,dg.seq,buffer);
		send_to(asw,addr);
		break;
		
	default :
		break;
	}

	return;
}

void Server::get_file(const Datagram &dg, const AddrStorage &addr)
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
		if(dg.seq>MINSIZE)
		{
			current->_size = dg.seq;
			current->_buffer = new char[dg.seq+1];
			current->_buffer[dg.seq] = '\0';
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
			current_packet = packet_number;
		}

		for(i=init;i<dg.seq;i++)
		{
			current->_buffer[i] = dg.data[i-init];
		}

		current->_received_packet[current_packet-1] = true;
		
		asw.init(UPLOAD,dg.seq,"Ack");
		send_to(asw,addr);

		if(current->is_data())
		{
			f = new File(current->_file);
			f->write(Converter::cstos(current->_buffer));
			delete f;
			current->refresh();
		}
		break;

	default :
		break;
	}

	return;
}

/*
 *
 * Surcouche serveur
 *
 *
 */

void Server::process(const Datagram &dg, const AddrStorage &addr)
{
	//gestionnaire d'exception
	switch(dg.code)
	{
	case CONNECT :
		connect_ack(dg,addr);
		break;
	case DISCONNECT :
		disconnect_ack(dg,addr);
		break;
	case DOWNLOAD :
		send_file(dg,addr);
		break;
	case UPLOAD :
		get_file(dg,addr);
		break;
	default :
		break;
	}

	return;
}

void Server::update_client_map(const AddrStorage &addr)
{
	addr_map::const_iterator it = _client_map.find(addr);
	if(it == _client_map.end())
	{
		State new_state(DISCONNECT,CLIENT);
		_client_map[addr] = new_state;
	}

	return;
}

bool Server::find_file(const string &file)
{
	return true;
}

void Server::remove_file(const string &file)
{
	//penser a supprimer sur tout les serveurs
	remove(Converter::stocs(file));
}
