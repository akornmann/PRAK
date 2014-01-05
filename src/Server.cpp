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
		}
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
	//Get a updated library
	try
	{
		vector<AddrStorage*> *addr = Client::synchronize();

		if(addr->size()<1) throw (Exception("There is no server active, unable ton find a library.",__LINE__));

		_lib = Client::get_library(*((*addr)[0]));

		Client::disconnect(addr);
	}
	catch(Exception e)
	{
		//No other server is running, lib stay empty
	}



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
		else throw (Exception("Server::server : select failed.",__LINE__));
	}
}

void Server::send_to(const Datagram &dg, const AddrStorage &addr)
{
	int r = sendto(sock(addr),&dg,sizeof(Datagram),0,addr.sockaddr(),addr.len());
	
	if(r==-1) throw (Exception("Server::send_to : sendto failed.",__LINE__));
}

void Server::receive(Datagram &dg, AddrStorage *addr, int s)
{
	struct sockaddr_storage *temp_addr = addr->storage();
	//struct sockaddr_storage temp_addr;
	socklen_t temp_len;
	temp_len = sizeof(struct sockaddr_storage);

	int r = recvfrom(s,&dg,sizeof(Datagram),0,(struct sockaddr*) temp_addr, &temp_len);
	
	if(r!=-1) addr->build(s);
	else throw (Exception("Server::receive : recvfrom failed.", __LINE__));

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
			remove(dg.data);
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

void Server::add_file(const Datagram &dg, const AddrStorage &addr)
{
	State *current = &_client_map[addr];
	Datagram asw;
	Record *r;

	addr_map::iterator it;

	switch(dg.seq)
	{
	case -2:
		current->_file = dg.data;
		asw.init(ADD,dg.seq,"Ack");
		send_to(asw,addr);
		break;
	case -1:
		current->_title = dg.data;
		asw.init(ADD,dg.seq,"Ack");
		send_to(asw,addr);

		r = new Record(current->_file,current->_title);
		_lib = insert(_lib,*r);
		break;

	case 1:
		//request from a client : sending info to other server
		asw.init(ADD,dg.seq,"Ack");
		send_to(asw,addr);

		for(it=_server_map.begin();it!=_server_map.end();++it)
		{
		
			try
			{
				Client::add_file(current->_file,current->_title,false,it->first);
			}
			catch(Exception e)
			{
			}
		}
		
		break;
	}

	return;
}

void Server::remove_file(const Datagram &dg, const AddrStorage &addr)
{
	string file = dg.data;

	remove(dg.data); //rm file system
	remove(_lib,file); //rm library

	if(dg.seq>0) //recursive mode
	{
		addr_map::iterator it_map;
		for(it_map=_server_map.begin();it_map!=_server_map.end();++it_map)
		{
			try
			{
				Client::remove_file(file,false,it_map->first);
			}
			catch(Exception e)
			{
			}
		}
	}
	return;
}

void Server::send_library(const Datagram &dg, const AddrStorage &addr)
{
	Datagram asw;
	unsigned int size;
	switch(dg.seq)
	{
	case -1:
		asw.init(GET,_lib.size(),"Here is meta !");
		send_to(asw,addr);
		break;
	default:
		size = dg.seq;
		if(size>=0&&size<_lib.size())
		{
			asw.init(GET,dg.seq,_lib[dg.seq].file());
			send_to(asw,addr);
			asw.init(GET,dg.seq,_lib[dg.seq].title());
			send_to(asw,addr);
		}
		break;
	}
}

/*
 *
 * Surcouche serveur
 *
 *
 */

void Server::process(const Datagram &dg, const AddrStorage &addr)
{
	try
	{
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
		case ADD :
			add_file(dg,addr);
			break;
		case REMOVE :
			remove_file(dg,addr);
			break;
		case GET :
			send_library(dg,addr);
			break;
		default :
			break;
		}
	}
	catch(Exception e)
	{
	}

	return;
}

void Server::update_client_map(const AddrStorage &addr)
{
	addr_map::const_iterator it = _client_map.find(addr);
	if(it == _client_map.end())
	{
		State new_state(DISCONNECT);
		_client_map[addr] = new_state;
	}

	return;
}

bool Server::find_file(const string &file)
{
	library::iterator it;

	for(it=_lib.begin();it!=_lib.end();++it)
	{
		if(it->file()==file) //if file is in library
		{
			File f(file);
			if(f.size()>0) //file exist
			{
				return true;
			}
			else //file doesnt exist DL it !
			{
				remove(Converter::stocs(file));
				
				try
				{
					vector<AddrStorage*> *addr = Client::synchronize();

					if(addr->size()<1) throw (Exception("There is no server active, unable to get any file.",__LINE__));

					string res = Client::get_file(file,*addr);
					Client::disconnect(addr);
					
					File f(file);
					f.write(res);
				}
				catch(Exception e)
				{
				}
				return false;
			}
		}
	}

	return false;
}
