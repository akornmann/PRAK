#include "Client.hpp"


Client::Client(string &config)
{
	File conf(config);
	
	int i = 1;
	string line = "end";
	string a,p;
	
	while((line = conf.read(i)) != "end")
	{
		vector<string> v = Converter::split(line," ");
		string a = v[0];
		string p = v[1];
		
		AddrStorage addr(a,p);
		State s(DISCONNECT,SERVER);
		
		_server_map[addr] = s;

		i++;
	}

	// Create socket listening IPv4
	_sock_4 = socket(AF_INET, SOCK_DGRAM, 0);

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
		throw (Exception("Client::socket : Bad family definition.", addr, __LINE__));
		break;
	}
	return res;
}

void Client::send_to(const Datagram &dg, const AddrStorage &addr)
{
	int r = sendto(sock(addr), &dg, sizeof(Datagram), 0, addr.sockaddr(), addr.len());
			
	if(r==-1) throw (Exception("Client::send_to : Failed", __LINE__));
	else cout << dg << " to " << addr << endl;
	
	return;
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
	else res = false; //Timer expired

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
		else res = false; //Wrong server packet incoming
		
	}
	else res = false; //Timer expired

	return res;
}

/*
 *
 * Protocoles de base
 *
 *
 */

void Client::connect_req(const AddrStorage &addr)
{
	Datagram dg(CONNECTRA,0,"Is there any server here ?");
	send_to(dg,addr);
	
	return;
}

void Client::disconnect_req(const AddrStorage &addr)
{
	Datagram dg(DISCONNECTRA,0,"It's time to leave.");
	send_to(dg,addr);

	return;
}

string Client::get_file(const string &file, const AddrStorage &addr)
{
	if(file == "") throw (Exception("Client::get_file : File name can't be empty.", addr, __LINE__));

	Datagram rcv(0,-1);

	//META
	Datagram start(DOWNLOAD,META,file);

	Counter c(5);
	do
	{
		send_to(start,addr);	
		receive_from(rcv,addr);
		++c;
	}
	while(rcv.seq<0 || rcv.code != DOWNLOAD);

	if(rcv.seq<=MINSIZE) throw (Exception("Client::get_file : This file doesn't exist",addr,__LINE__));
	
	int size = rcv.seq;
	char *buffer = new char[size+1];
	buffer[size] = '\0';
	int packet_number = ceil((float) size / (float) (DATASIZE-1));

	//DATA
	Datagram ask;
	rcv.init(0,0);

	for(int i=1;i<=packet_number;i++)
	{
		int seq;
		int to_read;
		int init;

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

		Datagram ask(DOWNLOAD,seq);

		c.restart(5);
		do
		{
			send_to(ask,addr);	
			receive_from(rcv,addr);
			++c;
		}
		while(rcv.seq!=seq || rcv.code != DOWNLOAD);

		init = seq-to_read;

		for(int j=init;j<=seq;j++)
		{
			buffer[j] = rcv.data[j-init];
		}
	}
	
	return Converter::cstos(buffer);
}


void Client::send_file(const string &file, const string &title,const AddrStorage &addr)
{		
	if(file == "") throw (Exception("Client::send_file : File name can't be empty",addr,__LINE__));
	File f(file);
	int size = f.size();
	if(size<=MINSIZE)
	{
		remove(Converter::stocs(file));
		throw (Exception("Client::send_file : This file doesn't exist !",addr, __LINE__));
	}
	if(title == "") throw (Exception("Client::send_file : Title can't be empty.",addr, __LINE__));
	Datagram rcv;
	char* buffer;

	//INITIATE TRANSFERT
	int init_seq = 1; //never use 0 !!!
	Datagram start(UPLOAD,init_seq,"Wake up lazzy server !");

	Counter c(5);
	do
	{
		send_to(start,addr);
		
		receive_from(rcv,addr);
		++c;
	}
	while(rcv.seq != init_seq || rcv.code != UPLOAD);

	//SENDING METADATA
	Datagram meta_file(UPLOAD,-2,file);
	Datagram meta_title(UPLOAD,-1,title);
	Datagram meta_size(UPLOAD,size);

	c.restart(5);
	do
	{
		send_to(meta_file,addr);
		send_to(meta_title,addr);
		send_to(meta_size,addr);

		receive_from(rcv,addr);
		++c;
	}
	while(rcv.seq != 0 || rcv.code != UPLOAD);

	//SENDING DATA
	int packet_number = ceil((float) size/(float) (DATASIZE-1));
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

		c.restart(5);
		do
		{
			send_to(up,addr);
			receive_from(rcv,addr);
			++c;
		}
		while(rcv.seq!=seq || rcv.code != UPLOAD);

		delete[] buffer;
	}
	
	return;
}

/*
 *
 * Surcouche client
 *
 *
 */
void Client::flush()
{
	Datagram dg;
	AddrStorage *addr = new AddrStorage();

	while(receive(dg,addr)) cout << "flushed"<<endl;;
	
	delete addr;
}

void Client::synchronize(AddrStorage *addr)
{
	flush();
	addr_map::const_iterator it;

	//Sending a connection request to every server
	Datagram dg;
	for(it=_server_map.begin();it!=_server_map.end();++it)
	{
		connect_req(it->first);
	}

	//First receive -> Winner server !
	Counter c(5);
	do
	{
		receive(dg,addr);
		++c;
	}
	while(dg.code != CONNECTRA);

	//Set status connect to first answer
	_server_map[*addr]._status = CONNECT;

	//Disconnect from other server
	Equal e;
	for(it=_server_map.begin();it!=_server_map.end();++it)
	{
		if(!e(it->first,*addr)) disconnect_req(it->first);
	}

	flush();
	
	return;
}

void Client::get_file(string file)
{
	AddrStorage *addr = new AddrStorage(); 

	synchronize(addr);
	string res = get_file(file,*addr);
	
	disconnect_req(*addr);
	_server_map[*addr]._status = DISCONNECT;

	delete addr;
	
	cout << res << endl;

	return;
}

void Client::send_file(string file, string title)
{
	AddrStorage *addr = new AddrStorage();

	synchronize(addr);
	send_file(file,title,*addr);

	disconnect_req(*addr);
	_server_map[*addr]._status = DISCONNECT;

	delete addr;

	return;
}

void Client::get_library()
{
	get_file(".library");
	return;
}
