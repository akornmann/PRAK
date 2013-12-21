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
		State s(DISCONNECT);
		
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
		_exc.push_back(Exception("Client::is_connect : Server answer's wrong.", __LINE__));
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
		switch((it->second).status())
		{
		case ACTIVE :
			_exc.push_back(Exception("Client::synchronize : A connection with a server is still active.",__LINE__));
			//LA FAUT PANIQUER
			break;
		case CONNECT :
			(it->second).status(DISCONNECT);
			break;
		case DISCONNECT :
			break;
		default :
			_exc.push_back(Exception("Client::synchronize : Unknow status.", __LINE__));
			//LA FAUT CARREMENT PANIQUER
			break;
		}
	}



	Datagram dg;
	//Now, we send a "toctoc" packet to every server
	for(it=_server_map.begin();it!=_server_map.end();++it)
	{
		dg.init(0,0,"sync");
		send_to(dg,it->first);
	}

	//First receive -> Winner server !
	if(!receive(dg,addr))
	{
		_exc.push_back(Exception("Client::synchronize : No server answered.", __LINE__));
		res = false;
	}
	
	cout << "Winner is :" <<  dg << " from " << *addr << endl;
	return res;
}

bool Client::send_to(const Datagram &dg, const AddrStorage &addr)
{
	cout << "send_to : " << dg << " " << addr <<  endl;
	int r = sendto(sock(addr), &dg, sizeof(Datagram), 0, addr.sockaddr(), addr.len());
	
	if(r==-1)
	{
		_exc.push_back(Exception("Client::send_to : Failed", __LINE__));
		perror("errno : send_to ");
		return false;
	}
	else return true;
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
			cout << "receive : " << dg << " from " << *addr << endl;
			res = true;
		}
		else _exc.push_back(Exception("Client::received : Failed.", __LINE__));
	}
	else _exc.push_back(Exception("Client::receive : Timer expired.", __LINE__));

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
		int r = recvfrom(s, &dg, sizeof(Datagram), 0, (struct sockaddr*) &temp_addr, &temp_len);
		
		Equal e;
		incoming.build(s);
		if(e(addr,incoming))
		{
			if(r!=-1) res = true;
			else _exc.push_back(Exception("Client::receive_from : Failed.", __LINE__));
		}
		else _exc.push_back(Exception("Client::receive_from : Packet coming from wrong server.", __LINE__));
		
	}
	else _exc.push_back(Exception("Client::receive_from : Timer expired.", __LINE__));

	return res;
}

/*
 *
 * Protocoles de base
 *
 *
 */

bool Client::toctoc(Datagram &dg, const AddrStorage &addr)
{
	dg.seq++;
	return send_to(dg,addr);
}


bool Client::get_file(const string &file, const AddrStorage &addr)
{
	bool succes = false;
	Datagram s(1,-1,file);
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
			else _exc.push_back(Exception("Client::get_file : Server doesn't find this file, or this file's empty.", __LINE__));		
		}
		else _exc.push_back(Exception("Client::_get_file : Server doesn't find this file.", __LINE__));
	}
	else _exc.push_back(Exception("Client::get_file : Server didn't answer", __LINE__));

	return succes;
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

exc Client::error()
{
	return _exc;
}
