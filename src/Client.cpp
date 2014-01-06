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

		//Convert hostname->ip
		const char *hostname = a.c_str();
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
	int r = sendto(sock(addr), &dg, sizeof(Datagram), 0, addr.sockaddr(), addr.len());
			
	if(r==-1)
	{
		throw (Exception("send_to : Failed", __LINE__));
	}
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
			res = true;
		}
		else throw (Exception("received : Failed.", __LINE__));
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

	time_val.tv_sec = TIMER;
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
			if(r!=-1) res = true;
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
	
	return;
}

void Client::disconnect_req(const AddrStorage &addr)
{
	Datagram dg(DISCONNECTRA,0,"It's time to leave.");
	send_to(dg,addr);

	return;
}

string Client::get_file(const string &file, vector<AddrStorage *> addr)
{
	if(file == "") throw (Exception("File name can't be empty.", __LINE__));

	Datagram rcv(0,-2);

	//META
	Datagram start(DOWNLOAD,META,file);

	unsigned int k=0;
	Counter c(5,"This file doesn't exist !");
	do
	{
		send_to(start,*addr[k]);	
		receive_from(rcv,*addr[k]);

		if(rcv.seq==-1)
		{
			//This server doesn't have the requested file
			rcv.init(0,-2);
			k++;
			if(k>=addr.size()) throw (Exception("This file doesn't exist",__LINE__));
		}
		++c;
	}
	while(rcv.seq<0 || rcv.code != DOWNLOAD);

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
		
		Datagram ask(DOWNLOAD,seq);
		
		string tmp;
		c.restart(5,"Too many packets lost, download abort.");
		do
		{
			send_to(ask,*addr[k]);	
			receive_from(rcv,*addr[k]);
			++c;
			tmp =Converter::cstos(rcv.data);
		}
		while(rcv.seq!=seq || rcv.code != DOWNLOAD || tmp.length() != to_read);

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
	if(file == "") throw (Exception("File name can't be empty",addr,__LINE__));
	File f(file);
	int size = f.size();
	if(size<=MINSIZE)
	{
		remove(Converter::stocs(file));
		throw (Exception("This file doesn't exist !",addr, __LINE__));
	}
	if(title == "") throw (Exception("Title can't be empty.",addr, __LINE__));
	Datagram rcv;
	char* buffer;

	//INITIATE TRANSFERT
	int init_seq = 1; //never use 0 !!!
	Datagram start(UPLOAD,init_seq,"Wake up lazzy server !");

	Counter c(5, "Connection to server failed.");
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

	c.restart(5,"Too many packets lost, upload abort");
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

		c.restart(5,"Too many packets lost, upload abort.");
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


void Client::add_file(const string& file, string& title, bool rec, const AddrStorage &addr)
{
	Datagram t(ADD,-1,Converter::stocs(title));
	Datagram f(ADD,-2,Converter::stocs(file));

	Datagram rcv;

	Counter c(5,"Adding file to library abort.");
	do
	{
		send_to(f,addr);
		
		receive_from(rcv,addr);
		++c;
	}
	while(rcv.seq != -2 || rcv.code != ADD);
	
	c.restart(5,"Too many packets lost, adding file to library abort.");
	do
	{
		send_to(t,addr);
		
		receive_from(rcv,addr);
		++c;
	}
	while(rcv.seq != -1 || rcv.code != ADD);

	if(rec)
	{
		c.restart(5,"Too many packets lost, adding file to others library abort.");
		Datagram r(ADD,1,"I'm a client, start recursive mode");
		do
		{
			send_to(r,addr);
		
			receive_from(rcv,addr);
			++c;
		}
		while(rcv.seq != 1 || rcv.code != ADD);
	}

	return;
}

void Client::remove_file(const string& file, bool rec, const AddrStorage &addr)
{
	Datagram dg(REMOVE,rec,Converter::stocs(file));

	send_to(dg,addr);

	return;	
}

library & Client::get_library(const AddrStorage& addr)
{
	library &lib = *new library;
	Datagram rcv(0,-1);
	Datagram rcv2(0,-1);

	//META
	Datagram start(GET,-1,"Wake up and send me some metas !");

	Counter c(5,"Getting library abort.");
	do
	{
		send_to(start,addr);	
		receive_from(rcv,addr);
		++c;
	}
	while(rcv.seq<0 || rcv.code != GET);
	
	int size = rcv.seq;

	Datagram s;
	for(int i=0;i<size;i++)
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


vector<AddrStorage *> * Client::synchronize()
{

	addr_map::const_iterator it_map;
	//Sending a connection request to every server
	for(it_map=_server_map.begin();it_map!=_server_map.end();++it_map)
	{
		connect_req(it_map->first);
	}

	vector<AddrStorage *> *addr = new vector<AddrStorage *>;

	Datagram dg(DISCONNECTRA);

	vector<AddrStorage *>::iterator it;

	bool res = false;
	do
	{
		AddrStorage *temp_addr = new AddrStorage();
		res = receive(dg,temp_addr);
		if(res&&dg.code==CONNECTRA)
		{
			it = addr->begin();
			it = addr->insert(it,temp_addr);
		}
	}
	while(res);


	return addr;
}

void Client::disconnect(vector<AddrStorage *> *addr)
{
	vector<AddrStorage *>::iterator it;
	Datagram dg(CONNECTRA);
	for(it=addr->begin();it!=addr->end();++it)
	{
		disconnect_req(**it);
		receive_from(dg,**it);
		delete *it;
	}
}

void Client::get_file(string file)
{
	vector<AddrStorage*> *addr = synchronize();

	if(addr->size()<1) throw (Exception("There is no server active, unable to get any file.",__LINE__));

	string res = get_file(file,*addr);
	disconnect(addr);

	
	file = Record::formatFile(file);

	cout << endl << "---- " << file << " ----" << endl << endl;
	cout << res << endl;
	cout << endl << "---------------" << endl << endl;
	return;
}

void Client::send_file(string file, string title)
{
	vector<AddrStorage*> *addr = synchronize();

	if(addr->size()<2) throw (Exception("There is less than one server active, unable to send any file.",__LINE__));

	remove_file(file,true,*((*addr)[0]));
	
	send_file(file,title,*((*addr)[0]));
	send_file(file,title,*((*addr)[1]));

	add_file(file,title,true,*((*addr)[0]));

	disconnect(addr);

	cout << endl << file << " sent with succes." << endl << endl;

	return;
}

void Client::remove_file(string file)
{
	vector<AddrStorage*> *addr = synchronize();

	if(addr->size()<1) throw (Exception("There is less than one server active, unable to remove any file.",__LINE__));

	remove_file(file,true,*((*addr)[0]));
	disconnect(addr);

	cout << endl << file << " removed with succes." << endl << endl;

	return;
}

void Client::get_library()
{
	vector<AddrStorage*> *addr = synchronize();

	if(addr->size()<1) throw (Exception("There is no server active, unable to find a library.",__LINE__));

	library lib = get_library(*((*addr)[0]));

	disconnect(addr);

	cout << endl << lib << endl;

	return;
}
