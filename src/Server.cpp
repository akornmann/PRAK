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

	_timer = _timer/1000;

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
		vector<AddrStorage*> addr;
		Client::synchronize(addr);

		if(addr.size()<1) throw (Exception("There is no server active, unable ton find a library.",__LINE__));

		_lib = Client::get_library(*(addr[0]));

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
	Datagram cp = dg;
	cp.code = htons(dg.code);
	cp.seq = htons(dg.seq);

	int r = sendto(sock(addr),&cp,sizeof(Datagram),0,addr.sockaddr(),addr.len());
	
	if(r==-1) throw (Exception("Server::send_to : sendto failed.",__LINE__));

	//cout << getpid() << " : "<< dg << " to " << addr << endl;
	return;
}

void Server::receive(Datagram &dg, AddrStorage *addr, int s)
{
	struct sockaddr_storage *temp_addr = addr->storage();
	socklen_t temp_len;
	temp_len = sizeof(struct sockaddr_storage);

	int r = recvfrom(s,&dg,sizeof(Datagram),0,(struct sockaddr*) temp_addr, &temp_len);

	dg.code = ntohs(dg.code);
	dg.seq = ntohs(dg.seq);

	if(r!=-1) addr->build(s);
	else throw (Exception("Server::receive : recvfrom failed.", __LINE__));
	
	//cout << getpid() << " : " << dg << " from " << *addr << endl;
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
	_client_map[addr].refresh();
	_client_map[addr]._status = CONNECT;

	Datagram s(CONNECTRA, 1, "Hey pretty client !");
	send_to(s,addr);

	return;
}

void Server::disconnect_ack(const Datagram &dg, const AddrStorage &addr)
{
	_client_map[addr].refresh();
	_client_map[addr]._status = DISCONNECT;

	Datagram s(DISCONNECTRA, 0, "Bye lovely client !");
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

	switch(current->_status)
	{
	case CONNECT :
		file = Converter::cstos(dg.data);
		current->_file = file;
			
		switch(find_file(file))
		{
		case 2:
			asw.init(DOWNLOAD,2,"File is here.");
			current->_status = META;
			break;
		case 1:
			if(dg.seq==1) //if a client ask
			{
				asw.init(DOWNLOAD,1,"File is in library, wait until I download it.");
				import(file);
				current->_status = DL;
			}
			else //if a server ask
			{
				asw.init(DOWNLOAD,0,"File does'nt exist.");
				current->_status = DISCONNECT;
			}
			break;
		case 0:
			asw.init(DOWNLOAD,0,"File does'nt exist.");
			current->_status = DISCONNECT;
			break;
		default:
			break;
		}
		send_to(asw,addr);
		break;

	case DL:
		file = Converter::cstos(dg.data);
		current->_file = file;
			
		switch(find_file(file))
		{
		case 2:
			asw.init(DOWNLOAD,2,"File is here.");
			current->_status = META;
			break;
		case 1:
			asw.init(DOWNLOAD,1,"Wait please.");
			break;
		case 0:
			asw.init(DOWNLOAD,0,"File does'nt exist.");
			current->_status = DISCONNECT;
			break;
		default:
			break;
		}
		send_to(asw,addr);
		break;

	case META :
		file = current->_file;
		switch(find_file(file))
		{
		case 2: //local file
			f = new File(file);
			size = f->size();
			asw.init(DOWNLOAD,size,"Here is meta !");
			send_to(asw,addr);
			current->_status = DATA;
			current->_buffer = f->readChar(size);
			current->_size = size;
			delete f;

			break;

		case 1:
		case 0:
			asw.init(DOWNLOAD,0,"File doesn't exist.");
			send_to(asw,addr);
		default:
			break;
		}
		
		break;

	case DATA :
		if(dg.seq >= current->_size) init = dg.seq-(dg.seq%(DATASIZE-1));
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
		current->_status = META;
		break;

	case META :
		//file name
		if(dg.seq==0) current->_file = dg.data;
		//title
		if(dg.seq==1) current->_title = dg.data;
		//size
		if(dg.seq>1)
		{
			current->_size = dg.seq;
			current->_buffer = new char[dg.seq+1];
			current->_buffer[dg.seq] = '\0';
			packet_number = ceil((float) current->_size/(float) (DATASIZE-1));
			current->_received_packet.resize(packet_number,false);
		}

		if(current->is_meta())
		{
			asw.init(UPLOAD,current->_size,"Metas are nice.");
			send_to(asw,addr);
			current->_status = DATA;
			remove(Converter::stocs(current->_file));
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
	case 0:
		current->_file = dg.data;
		asw.init(ADD,dg.seq,"Ack");
		send_to(asw,addr);
		break;
	case 1:
		current->_title = dg.data;
		asw.init(ADD,dg.seq,"Ack");
		send_to(asw,addr);

		r = new Record(current->_file,current->_title);
		_lib = insert(_lib,*r);

		break;

	case 2:
		//request from a client : sending info to other server
		asw.init(ADD,dg.seq,"Ack");
		try
		{
			addr_map::iterator it;
			switch(fork())
			{
			case -1:
				throw (Exception("Fork operation failed.",__LINE__));
				break;
			case 0:
				_run = false; //shut down listening server side
				for(it=_server_map.begin();it!=_server_map.end();++it)
				{
					Client::add_file(current->_file,current->_title,false,it->first);	
				}
				exit(0);

				break;
			default:
				break;
			}
			
		}
		catch(Exception e)
		{
		}
		
		send_to(asw,addr);
		break;
	}

	return;
}

void Server::remove_file(const Datagram &dg, const AddrStorage &addr)
{
	string file = dg.data;

	Datagram asw(REMOVE,0);

	remove(dg.data); //rm file system
	remove(_lib,file); //rm library

	if(dg.seq>0) //recursive mode
	{
		try
		{
			addr_map::iterator it;
			switch(fork())
			{
			case -1:
				throw (Exception("Fork operation failed.",__LINE__));
				break;
			case 0:
				_run = false; //shut down listening server side
				for(it=_server_map.begin();it!=_server_map.end();++it)
				{
					Client::remove_file(file,false,it->first);
				}
				exit(0);

				break;
			default:
				break;
			}
			
		}
		catch(Exception e)
		{
		}
	}

	send_to(asw,addr);
	return;
}

void Server::send_library(const Datagram &dg, const AddrStorage &addr)
{
	Datagram asw;
	unsigned int size;
	switch(dg.seq)
	{
	case 0:
		asw.init(GET,_lib.size(),"Here is meta !");
		send_to(asw,addr);
		break;
	default:
		size = dg.seq;
		if(size>0&&size<=_lib.size())
		{
			asw.init(GET,dg.seq,_lib[dg.seq-1].file());
			send_to(asw,addr);
			asw.init(GET,dg.seq+_lib.size(),_lib[dg.seq-1].title());
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
		case CONNECTRA :
			connect_ack(dg,addr);
			break;
		case DISCONNECTRA :
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

int Server::find_file(const string &file)
{
	library::iterator it;

	for(it=_lib.begin();it!=_lib.end();++it)
	{
		if(it->file()==file) //if file is in library
		{
			if(File::exist(file)) //file exist
			{
				return 2;
			}
			else //file doesn't exist DL it !
			{				
				return 1;
			}
		}
	}

	return 0;
}

void Server::import(const string &file)
{
	addr_map::iterator it;
	bool found = false;
	Datagram rcv(DEFAULT);
	Datagram ask(DEFAULT);


	Counter c(RETRY,"");

	string res;
	File *f;

	try
	{
		switch(fork())
		{
		case -1:
			throw (Exception("Fork operation failed.",__LINE__));
			break;
		case 0:
			for(it=_server_map.begin();it!=_server_map.end()&&!found;++it)
			{
				try
				{
					c.restart(RETRY,"");
					rcv.init(DEFAULT);
					do
					{
						Client::connect_req(it->first);
						Client::receive_from(rcv,it->first);
						++c;
					}
					while(rcv.seq!=1 || rcv.code!=CONNECTRA);

					while(Client::receive_from(rcv,it->first)); //flush

					c.restart(RETRY,"");
					rcv.init(DEFAULT,3);
					ask.init(DOWNLOAD,0,file);
					do
					{
						Client::send_to(ask,it->first);
						Client::receive_from(rcv,it->first);
						++c;
					}
					while(rcv.seq!=2 || rcv.code!=DOWNLOAD);
			
					found = true;

					while(Client::receive_from(rcv,it->first)); //flush
						
				}
				catch(Exception e)
				{
				}
			}
			try
			{
				if(it==_server_map.end())
				{
					//this file does'nt exist anymore
					Client::remove_file(file);
				}
				else
				{
					c.restart(RETRY,"");
					rcv.init(DEFAULT);
					do
					{
						Client::disconnect_req(it->first);
						Client::receive_from(rcv,it->first);
						++c;
					}
					while(rcv.seq!=0 || rcv.code!=DISCONNECTRA);
				
					//reconnect and start a standard dl
					c.restart(RETRY,"");
					do
					{
						Client::connect_req(it->first);
						Client::receive_from(rcv,it->first);
						++c;
					}
					while(rcv.seq!=1 || rcv.code!=CONNECTRA);
				
					while(Client::receive_from(rcv,it->first)); //flush

					res = Client::get_file(file,false,it->first);

					f = new File(file);
					f->write(res);
					delete f;

					c.restart(RETRY,"");
					rcv.init(DEFAULT);
					do
					{
						Client::disconnect_req(it->first);
						Client::receive_from(rcv,it->first);
						++c;
					}
					while(rcv.seq!=0 || rcv.code!=DISCONNECTRA);	
				}
			}
			catch(Exception e)
			{
				exit(0);
			}
			exit(0);
			break;
		default:
			break;
		}
	}
	catch(Exception e)
	{
	}

	return;
}
