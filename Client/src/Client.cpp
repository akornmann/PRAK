#include "Client.h"
string Client::status()
{
	string s = "";
	switch(_status)
	{
	case CONNECT :
		s = "You are connect to "+_server->paddr()+":"+_server->pport();
		break;
	case DISCONNECT :
		s = "You are not connect to any server !";
		break;
	default  :
		s = "You are in an unknow status !";
		break;
	}
	return s;
}

Client::Client()
{
	_conf = new File("server.cfg");
	_log = new File("/home/kalex/.log/PRAK/Client.log");

	_server_len = sizeof(struct sockaddr_storage);

	_time_val.tv_sec = 1;
	_time_val.tv_usec = 0;
	Status s = DISCONNECT;
	set_status(s);
}

Client::~Client()
{
	disconnect();
}

bool Client::connect(string addr, string port)
{
	_server = new AddrStorage(addr,port,_log);
	// Creation socket UDP
	_socket = socket(_server->family(), SOCK_DGRAM, 0);
	_log->write("Client::connect","Socket "+Converter::itos(_socket)+" open");

	int o = 1 ;
	setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, &o, sizeof o);
	
	return do_toctoc();
}

bool Client::disconnect()
{
	_log->write("Client::disconnect","Socket "+Converter::itos(_socket)+" closed\n\n\n");
	close(_socket);
	free(_server);
	Status s = DISCONNECT;
	set_status(s);
	return true;
}

bool Client::send_to(Datagram* dg, AddrStorage* addr)
{
	_r = sendto(_socket, dg, 516, 0, addr->sockaddr(), addr->len());
	_log->write("Client::send_to", dg->data);
	return true;
}

bool Client::receive_from(Datagram* dg, AddrStorage* addr)
{
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(_socket, &readfds);

	if(select(_socket+1, &readfds, NULL, NULL, &_time_val))
	{
		memset(dg,0,sizeof(Datagram));

		if((_r = recvfrom(_socket, dg, 516, 0, (struct sockaddr*) &_server_addr, &_server_len)) !=-1)
		{
			addr = new AddrStorage((struct sockaddr*) &_server_addr, _log);
			_log->write("Client::receive_from",dg->data);
			return true;
		}
		else
		{
			_log->write("Client::receive_from","recvfrom failed");
			return false;
		}
	}
	else
	{
		_log->write("Client::receive_from","Timer expired");

		_time_val.tv_sec = 1;
		_time_val.tv_usec = 0;

		return false;
	}
}

void Client::toctoc(AddrStorage* addr)
{
	Datagram dg;
	dg.code = 0;
	memset(&dg.data,0, 512);
	char answer[] = "Qui est là ?";
	strcpy (dg.data,answer);
	
	send_to(&dg, addr);
}

bool Client::do_toctoc()
{
	Datagram dg;
	dg.code = 0;
	memset(&dg.data,0, 512);
	char ask[] = "Toc toc";
	strcpy(dg.data,ask);
	
	send_to(&dg, _server);
	
	if(receive_from(&dg, _server))
	{
		if(dg.code==0)
		{
			_log->write("do_toctoc", "Server answer successfull");
			Status s = CONNECT;
			set_status(s);
			return true;
		}
		else
		{
			_log->write("do_toctoc", "Bad server answer");
			return false;
		}
	}
	else
	{
		_log->write("do_toctoc", "No server answer");
		return false;
	}
}



bool Client::server_select()
{
	int i = 1;
	bool found = false;
	string line = "end";
	string a,p;

	while(!found && (line = _conf->read(i)) != "end")
	{
		string delim = " ";
		vector<string> v = Converter::split(line,delim);
		string a = v[0];
		string p = v[1];
		
		_log->write("Client::select_server","Try to connect to server "+a+":"+p);
		
		found = connect(a,p);

		if(!found) disconnect();
		i++;
	}
	if(found) _log->write("Client::select_server","Server "+_server->paddr()+":"+_server->pport()+" selected");
	else _log->write("Client::select_server","No server available");

	return found;
}


bool Client::do_file(string file)
{
	return true;
}
