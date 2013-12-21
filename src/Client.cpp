#include "Client.hpp"


Client::Client(string &config)
{
	File conf(config);
	
	int i = 1;
	string line = "end";
	string a,p;
	
	while((line = conf.read(i)) != "end")
	{
		string delim = " ";
		vector<string> v = Converter::split(line,delim);
		string a = v[0];
		string p = v[1];
		
		AddrStorage addr(a,p);
		State s(DISCONNECT,SERVER);
		
		_server_map[addr] = s;

		i++;
	}

	// Create socket listening IPv4
	_sock_4 = socket(AF_INET, SOCK_DGRAM, 0);
	//int o = 1 ;
	//setsockopt(_sock_4, SOL_SOCKET, SO_BROADCAST, &o, sizeof o);
	
	// Create socket listening IPv6
	_sock_6 = socket(AF_INET6, SOCK_DGRAM, 0);
}

Client::~Client()
{	
	close(_sock_4);
	close(_sock_6);
}

int Client::sock(const AddrStorage &addr)
{
	int res = -1;
	switch(addr.family())
	{
	case AF_INET :
		res = _sock_4;
		break;
	case AF_INET6 :
		res = _sock_6;
		break;
	default :
		_exc.push_back(Exception("Client::socket : Bad family definition.", __LINE__));
		break;
	}
	return res;
}

bool Client::connect(const AddrStorage &addr)
{
	Datagram dg(0,0,"connect using toctoc");
	send_to(dg,addr);
	if(dg.code==0&&dg.seq==1) return true;
	else
	{
		throw (Exception("Client::is_connect : Server answer's wrong.", __LINE__));
		return false;
	}
}

bool Client::synchronize(AddrStorage *addr)
{
	bool res = true;

	//First, we disconnect client from every server
	addr_map::iterator it;
	for(it=_server_map.begin();it!=_server_map.end();++it)
	{
		switch((it->second)._status)
		{
		case ACTIVE :
			throw (Exception("Client::synchronize : A connection with a server is still active.",__LINE__));
			//LA FAUT PANIQUER
			break;
		case CONNECT :
			(it->second)._status = DISCONNECT;
			break;
		case DISCONNECT :
			break;
		default :
			throw (Exception("Client::synchronize : Unknow status.", __LINE__));
			//LA FAUT CARREMENT PANIQUER
			break;
		}
	}



	Datagram dg;
	for(it=_server_map.begin();it!=_server_map.end();++it)
	{
		connect_req(it->first);
	}

	//First receive -> Winner server !
	if(!receive(dg,addr))
	{
		throw (Exception("Client::synchronize : No server answered.", __LINE__));
		res = false;
	}
	else
	{
		_server_map[*addr]._status = CONNECT;

		//Disconnect from other server
		for(it=_server_map.begin();it!=_server_map.end();++it)
		{
			Equal e;
			if(!e(it->first,*addr)) disconnect_req(it->first);
		}
	}
	
	return res;
}

bool Client::send_to(const Datagram &dg, const AddrStorage &addr)
{
	int r = sendto(sock(addr), &dg, sizeof(Datagram), 0, addr.sockaddr(), addr.len());
	
	if(r==-1)
	{
		throw (Exception("Client::send_to : Failed", __LINE__));
		return false;
	}
	else
	{
		cout << dg << " to " << addr << endl;
		return true;
	}
}

bool Client::receive(Datagram &dg, AddrStorage *addr)
{
	bool res = false;

	//struct sockaddr_storage temp_addr;
	struct sockaddr_storage *temp_addr = addr->storage();
	socklen_t temp_len;
	struct timeval time_val;
	fd_set readfds;

	temp_len = sizeof(struct sockaddr_storage);

	time_val.tv_sec = 1;
	time_val.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(_sock_4,&readfds);
	FD_SET(_sock_6,&readfds);
	
	int max = (_sock_4<_sock_6) ? _sock_6 : _sock_4;
	if(select(max+1, &readfds, NULL, NULL, &time_val))
	{
		int r,s;
		if(FD_ISSET(_sock_6,&readfds))
		{
			r = recvfrom(_sock_6, &dg, sizeof(Datagram), 0, (struct sockaddr*) temp_addr, &temp_len);
			s = _sock_6;
		}
		else if(FD_ISSET(_sock_4,&readfds))
		{
			r = recvfrom(_sock_4, &dg, sizeof(Datagram), 0, (struct sockaddr*) temp_addr, &temp_len);
			s = _sock_4;
		}
		else r = -1;
			
		if(r!=-1) 
		{
			addr->build(s);
			cout << dg << " from (new) " << *addr << endl;
			res = true;
		}
		else throw (Exception("Client::received : Failed.", __LINE__));
	}
	else
	{
		res = false;
		_exc.push_back(Exception("Client::receive_from : Timer expired.", __LINE__));
	}
	return res;
}

bool Client::receive_from(Datagram &dg, const AddrStorage &addr)
{
	bool res = false;

	//struct sockaddr_storage temp_addr;
	socklen_t temp_len;
	struct timeval time_val;
	fd_set readfds;

	int s = sock(addr);

	temp_len = sizeof(struct sockaddr_storage);

	time_val.tv_sec = 1;
	time_val.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(sock(addr), &readfds);
	
	if(select(s+1, &readfds, NULL, NULL, &time_val))
	{
		AddrStorage incoming;
		struct sockaddr_storage *temp_addr = incoming.storage();
		int r = recvfrom(s, &dg, sizeof(Datagram), 0, (struct sockaddr*) temp_addr, &temp_len);
		incoming.build(s);
		Equal e;
		if(e(addr,incoming))
		{
			if(r!=-1)
			{
				cout << dg << " from " << addr << endl;
				res = true;
			}
			else throw (Exception("Client::receive_from : Failed.", __LINE__));
		}
		else throw (Exception("Client::receive_from : Packet coming from wrong server.", __LINE__));
		
	}
	else
	{
		res = false;
		_exc.push_back(Exception("Client::receive_from : Timer expired.", __LINE__));
	}

	return res;
}

/*
 *
 * Protocoles de base
 *
 *
 */

bool Client::connect_req(const AddrStorage &addr)
{
	Datagram dg(CONNECTRA,0,"Is there any server here ?");
	return send_to(dg,addr);
}

bool Client::disconnect_req(const AddrStorage &addr)
{
	Datagram dg(DISCONNECTRA,0,"It's time to leave.");
	return send_to(dg,addr);
}

bool Client::get_file(const string &file, const AddrStorage &addr)
{
	bool succes = false;
	Datagram s(DOWNLOAD,-1,file);
	send_to(s,addr);

	Datagram r(0,0);

	if(receive_from(r,addr))
	{
		if(r.code == 1)
		{
			int size = r.seq;
			if(size>0)
			{
				char* res = new char[size+1];
				res[size] = '\0';
				res[0] = 'x';
				res[size-1] = 'x';
			
				int packet_number = ceil((float) size / (float) (DATASIZE-1));
				Datagram p(1,0,"Ready to receive");
				send_to(p, addr);
				
				
				
				while(receive_from(r,addr))
				{
					if(r.seq%(DATASIZE-1)==0)
					{
						int init = r.seq-(DATASIZE-1); 
						for(int i=init;i<=r.seq;i++)
						{
							res[i] = r.data[i-init];
						}
					}
					else
					{
						int end_size = size-(packet_number-1)*(DATASIZE-1);
						int init = r.seq-end_size;
						for(int i=init;i<=r.seq;i++)
						{
							res[i] = r.data[i-init];
						}
					}
				}

				cout  << res << endl;

				succes = true;
			}
			else throw (Exception("Client::get_file : Server doesn't find this file, or this file's empty.", __LINE__));		
		}
		else throw (Exception("Client::get_file : Server doesn't find this file.", __LINE__));
	}
	else throw (Exception("Client::get_file : Server didn't answer", __LINE__));

	return succes;
}


bool Client::send_file(const string &file, const string &title,const AddrStorage &addr)
{	
	if(file == "") throw Exception("Client::send_file : File name can't be empty",__LINE__);
	File f(file);
	int size = f.size();
	if(size<=0) throw Exception("Client::send_file : This file doesn't exist !", __LINE__);
	string trunc_title = title.substr(0,80); //truncate title to 80
	   if(trunc_title == "") throw Exception("Client::send_file : Title can't be empty.", __LINE__);
	Datagram rcv;
	char* buffer;

	//INITIATE TRANSFERT
	int init_seq = 1; //never use 0 !!!
	Datagram start(UPLOAD,init_seq,"Wake up lazzy server !");
	
	do
	{
		send_to(start,addr);
		
		receive_from(rcv,addr);
	}
	while(rcv.seq != init_seq);

	//SENDING METADATA
	Datagram meta_file(UPLOAD,-2,file);
	Datagram meta_title(UPLOAD,-1,trunc_title);
	Datagram meta_size(UPLOAD,size);

	do
	{
		send_to(meta_file,addr);
		send_to(meta_title,addr);
		send_to(meta_size,addr);

		receive_from(rcv,addr);
	}
	while(rcv.seq != 0);

	//SENDING DATA
	int packet_number = ceil((float) size/(float) (DATASIZE-1));
	cout << "n packets : " << packet_number << endl;
	for(int i=1;i<=packet_number;i++)
	{
		int seq, to_read;
		if(i*(DATASIZE-1)<=size)
		{
			seq = i*(DATASIZE-1);
			to_read = DATASIZE-1;
		}
		else
		{
			seq = size;
			to_read = size-(i-1)*(DATASIZE-1);
		}
		
		buffer = f.readChar(to_read);
		Datagram up(UPLOAD,seq,Converter::cstos(buffer));

		do
		{
			send_to(up,addr);
			receive_from(rcv,addr);
		}
		while(rcv.seq!=seq);

		delete[] buffer;
	}

	_server_map[addr]._status = DISCONNECT;
	return true;
}

/*
 *
 * Surcouche client
 *
 *
 */

bool Client::get_file(string file)
{
	AddrStorage *addr = new AddrStorage(); 
	synchronize(addr);
	bool res = get_file(file,*addr);
	delete addr;

	return res;
}

bool Client::send_file(string file, string title)
{
	AddrStorage *addr = new AddrStorage();
	synchronize(addr);
	bool res = send_file(file,title,*addr);
	delete addr;

	return res;
}

exc Client::error()
{
	return _exc;
}
