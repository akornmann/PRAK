#include "Client.hpp"


Client::Client(string &config):_timer(100000)
{
	File conf(config);
	
	int i = 1;
	string line = "end";
	string a,p;
	
	while((line = conf.read(i)) != "end")
	{
		char delim = ' ';
		vector<string> v = Converter::split(line,delim);
		
		if(v.size()==2)
		{	
			string a = v[0];
			string p = v[1];

			//Convert hostname->ip
			const char *hostname = Converter::stocs(a);
			struct addrinfo hints, *res;
			struct in_addr tmp;

			memset(&hints, 0, sizeof(hints));
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_family = AF_INET;

			if(getaddrinfo(hostname, NULL, &hints, &res)!=0)
			{
				throw (Exception("Unable to find this server ("+a+")",__LINE__));
			}

			tmp.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;

			freeaddrinfo(res);


			a = inet_ntoa(tmp);
			//end convert

			AddrStorage addr(a,p);
			State s(DISCONNECT);
		
			_server_map[addr] = s;
		}
		else throw (Exception("There is a mistake in server.cfg",__LINE__));

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
		throw (Exception("Bad family definition.", addr, __LINE__));
		break;
	}
	return res;
}

void Client::send_to(const Datagram &dg, const AddrStorage &addr)
{
	Datagram cp = dg;
	cp.code = htons(dg.code);
	cp.seq = htons(dg.seq);

	int r = sendto(sock(addr), &cp, sizeof(Datagram), 0, addr.sockaddr(), addr.len());
			
	if(r==-1)
	{
		throw (Exception("send_to : Failed", __LINE__));
	}

	//cout << getpid() << " : " << dg << " to " << addr << endl;
	return;
}

bool Client::receive(Datagram &dg, AddrStorage *addr)
{
	bool res = false;

	struct sockaddr_storage *temp_addr = addr->storage();
	socklen_t temp_len;
	struct timeval time_val;
	fd_set readfds;

	temp_len = sizeof(struct sockaddr_storage);

	time_val.tv_sec = 0;
	time_val.tv_usec = _timer;

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
			res = true;

			dg.code = ntohs(dg.code);
			dg.seq = ntohs(dg.seq);

			//cout << getpid() << " : " << dg << " from " << *addr << endl;
		}
		else throw (Exception("received : Failed.", __LINE__));
	}
	else res = false; //Timer expired


	return res;
}

bool Client::receive_from(Datagram &dg, const AddrStorage &addr)
{
	bool res = false;

	socklen_t temp_len;
	struct timeval time_val;
	fd_set readfds;

	int s = sock(addr);

	temp_len = sizeof(struct sockaddr_storage);

	time_val.tv_sec = 0;
	time_val.tv_usec = _timer;

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
				dg.code = ntohs(dg.code);
				dg.seq = ntohs(dg.seq);

				res = true;
				//cout << getpid() << " : " << dg << " from " << addr << endl;
			}
			else throw (Exception("receive_from : Failed.", __LINE__));
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

	_server_map[addr].refresh();
	_server_map[addr]._status = CONNECT;
	
	return;
}

void Client::disconnect_req(const AddrStorage &addr)
{
	Datagram dg(DISCONNECTRA,1,"It's time to leave.");
	send_to(dg,addr);

	_server_map[addr].refresh();
	_server_map[addr]._status = DISCONNECT;

	return;
}

string Client::get_file(const string &file, bool rec, const AddrStorage &addr)
{
	//INIT
	Datagram ask(DOWNLOAD,rec,file);
	Datagram rcv(DEFAULT,3);
	
	Counter c(RETRY,"Too many packets lost, download abort.");
	do
	{
		send_to(ask,addr);
		receive_from(rcv,addr);

		switch(rcv.seq)
		{
		case 2:
			break;
		case 1:
			ask.init(DOWNLOAD,rec,file);
			rcv.init(DEFAULT,3);
			usleep(1000000); //1sec
			c.restart(RETRY,"Too many packets lost, download abort.");
			break;
		case 0:
			throw (Exception("File doesn't exist.",__LINE__));
			break;
		default:
			break;
		}
		
		++c;
	}
	while(rcv.seq!=2 || rcv.code!=DOWNLOAD);

	//META
	ask.init(DOWNLOAD,rec,file);
	rcv.init(DEFAULT);

	c.restart(RETRY,"Too many packets lost, download abort.");
	do
	{
		send_to(ask,addr);
		receive_from(rcv,addr);
		++c;
	}
	while(rcv.seq==0 || rcv.code!=DOWNLOAD);

	int size = rcv.seq;

	//DATA
	char *buffer = new char[size+1];
	buffer[size] = '\0';
	int packet_number = ceil((float) size / (float) (DATASIZE-1));
	string tmp;

	for(int i=1;i<=packet_number;i++)
	{
		int seq;
		unsigned int to_read;
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
		
		ask.init(DOWNLOAD,seq);
		rcv.init(DEFAULT);
		
		c.restart(RETRY,"Too many packets lost, download abort.");
		do
		{
			send_to(ask,addr);	
			receive_from(rcv,addr);
			tmp=Converter::cstos(rcv.data);
			++c;
		}
		while(rcv.seq!=seq || rcv.code!=DOWNLOAD || tmp.length()!=to_read);

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
	File f(file);
	int size = f.size();

	if(size<=1)
	{
		remove(Converter::stocs(file));
		throw (Exception("This file doesn't exist.",addr, __LINE__));
	}

	Datagram rcv(DEFAULT);
	int init_seq = 17; //maybe use rand
	Datagram ask(UPLOAD,init_seq,"Wake up lazzy server !");
	
	char* buffer;

	//INITIATE TRANSFERT
	Counter c(RETRY, "Connection to server failed.");
	do
	{
		send_to(ask,addr);
		receive_from(rcv,addr);

		++c;
	}
	while(rcv.seq!=init_seq || rcv.code!=UPLOAD);
	
	//SENDING METADATA
	Datagram meta_file(UPLOAD,0,file);
	Datagram meta_title(UPLOAD,1,title);
	Datagram meta_size(UPLOAD,size);

	c.restart(RETRY,"Too many packets lost, upload abort.");
	do
	{
		send_to(meta_file,addr);
		send_to(meta_title,addr);
		send_to(meta_size,addr);

		receive_from(rcv,addr);
		++c;
	}
	while(rcv.seq!=size || rcv.code!=UPLOAD);

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

		c.restart(RETRY,"Too many packets lost, upload abort.");
		do
		{
			send_to(up,addr);
			receive_from(rcv,addr);
			++c;
		}
		while(rcv.seq!=seq || rcv.code!=UPLOAD);

		delete[] buffer;
	}
	
	return;
}


void Client::add_file(const string& file, string& title, bool rec, const AddrStorage &addr)
{
	Datagram f(ADD,0,Converter::stocs(file));
	Datagram t(ADD,1,Converter::stocs(title));

	Datagram rcv(DEFAULT);

	Counter c(RETRY,"Too many packets lost, adding file to library abort.");
	do
	{
		send_to(f,addr);
		receive_from(rcv,addr);

		++c;
	}
	while(rcv.seq!=f.seq || rcv.code!=ADD);
	
	c.restart(RETRY,"Too many packets lost, adding file to library abort.");
	do
	{
		send_to(t,addr);	
		receive_from(rcv,addr);

		++c;
	}
	while(rcv.seq!=t.seq || rcv.code!=ADD);

	if(rec)
	{
		c.restart(RETRY,"Too many packets lost, adding file to others library abort.");
		Datagram r(ADD,2,"I'm a client, start recursive mode");
		do
		{
			send_to(r,addr);
		
			receive_from(rcv,addr);
			++c;
		}
		while(rcv.seq!=2 || rcv.code!=ADD);
	}

	return;
}

void Client::remove_file(const string& file, bool rec, const AddrStorage &addr)
{
	Datagram dg(REMOVE,rec,Converter::stocs(file));

	Datagram rcv(DEFAULT);

	Counter c(RETRY,"Remove file from library abort.");
	do
	{
		send_to(dg,addr);
		receive_from(rcv,addr);

		++c;
	}
	while(rcv.seq!=0 || rcv.code!=REMOVE);
	
	return;	
}

library & Client::get_library(const AddrStorage& addr)
{
	library &lib = *new library;
	Datagram rcv(DEFAULT);
	Datagram rcv2(DEFAULT);

	//META
	Datagram start(GET,0,"Wake up and send me some metas !");

	Counter c(5,"Getting library abort.");
	do
	{
		send_to(start,addr);	
		receive_from(rcv,addr);
		++c;
	}
	while(rcv.seq==0 && rcv.code != GET);
	
	int size = rcv.seq;

	Datagram s;
	for(int i=1;i<=size;i++)
	{
		s.init(GET,i,"Send me this line please");
		c.restart(5,"Too many packets lost, getting library abort.");
		do
		{
			send_to(s,addr);	
			receive_from(rcv,addr);
			receive_from(rcv2,addr);
			++c;
		}
		while(rcv.seq!=i || rcv.code != GET || rcv.data == rcv2.data);
		Record r(Converter::cstos(rcv.data),Converter::cstos(rcv2.data));
		insert(lib,r);
	}

	return lib;
}

/*
 *
 * Surcouche client
 *
 *
 */


void Client::synchronize(vector<AddrStorage *> &addr)
{

	addr_map::const_iterator it_map;
	//Sending a connection request to every server
	for(it_map=_server_map.begin();it_map!=_server_map.end();++it_map)
	{
		connect_req(it_map->first);
	}

	Datagram dg(DEFAULT);

	vector<AddrStorage *>::iterator it;

	bool res = false;
	do
	{
		AddrStorage *temp_addr = new AddrStorage();
		res = receive(dg,temp_addr);
		if(res&&dg.code==CONNECTRA)
		{
			it = addr.begin();
			it = addr.insert(it,temp_addr);
		}
	}
	while(res);

	return;
}

void Client::disconnect(vector<AddrStorage *> &addr)
{
	vector<AddrStorage *>::iterator it;
	Datagram dg(DEFAULT);
	for(it=addr.begin();it!=addr.end();++it)
	{
		disconnect_req(**it);
		receive_from(dg,**it);
		delete *it;
	}
}

void Client::get_file(string file)
{
	if(file == "") throw (Exception("File name can't be empty.", __LINE__));
	
	vector<AddrStorage*> addr;
	synchronize(addr);

	if(addr.size()<1) throw (Exception("There is no server active, unable to get any file.",__LINE__));

	string res;
	try
	{
		res = get_file(file,true,*(addr[0]));
	}
	catch(Exception e)
	{
		disconnect(addr);
		throw e;
	}

	disconnect(addr);

       	cout << endl << "---- " << file << " ----" << endl << endl;
	cout << res << endl;
	cout << endl << "---------------" << endl << endl;

	return;
}

void Client::send_file(string file, string title)
{
	if(file == "") throw (Exception("File name can't be empty.",__LINE__));
	if(title == "") throw (Exception("Title can't be empty.", __LINE__));

	vector<AddrStorage*> addr;
	synchronize(addr);

	if(addr.size()<2) throw (Exception("There is less than one server active, unable to send any file.",__LINE__));

	try
	{
		remove_file(file,true,*(addr[0]));
	
		send_file(file,title,*(addr[0]));
		send_file(file,title,*(addr[1]));

		add_file(file,title,true,*(addr[1]));
	}
	catch(Exception e)
	{
		disconnect(addr);
		throw e;
	}

	disconnect(addr);

	cout << endl << file << " sent with succes." << endl << endl;

	return;
}

void Client::remove_file(string file)
{
	vector<AddrStorage*> addr;
	synchronize(addr);

	if(addr.size()<1) throw (Exception("There is less than one server active, unable to remove any file.",__LINE__));

	try
	{
		remove_file(file,true,*(addr[0]));
	}
	catch(Exception e)
	{
		disconnect(addr);
		throw e;
	}

	disconnect(addr);

	cout << endl << file << " removed with succes." << endl << endl;

	return;
}

void Client::get_library()
{
	vector<AddrStorage*> addr;
	synchronize(addr);

	if(addr.size()<1) throw (Exception("There is no server active, unable to find a library.",__LINE__));

	library lib;
	try
	{
		lib = get_library(*(addr[0]));
	}
	catch(Exception e)
	{
		disconnect(addr);
		throw e;
	}

	disconnect(addr);

	cout << endl << lib << endl;

	return;
}
