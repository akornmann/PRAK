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

bool Client::init(Datagram* dg, int code, int seq, string s)
{
	if(s.length()>=512)
	{
		_log->write("Client::init","Datagram overflow");
		return false;
	}
	else
	{
		memset(dg,0,DGSIZE);
		
		dg->code = code;
		dg->seq = seq;
		strcpy(dg->data,Converter::stocs(s));
		return true;
	}
}

bool Client::init(Datagram* dg, int code, int seq)
{
	string s = "";
	return init(dg, code, seq, s);
}

Client::Client()
{
	_conf = new File("server.cfg");
	_log = new File("/home/kalex/.log/PRAK/Client.log");

	_status=DISCONNECT;
}

Client::~Client()
{
	disconnect();
}

bool Client::connect(string addr, string port)
{

	_server = new AddrStorage(addr,port,-1,_log);

	// Creation socket UDP
	int s = socket(_server->family(), SOCK_DGRAM, 0);
	
	int o = 1 ;
	setsockopt(s, SOL_SOCKET, SO_BROADCAST, &o, sizeof o);

	_server->socket(s);
	

	if(is_connect())
	{
		_status = CONNECT;
		return true;
	}
	else return false;
}

bool Client::disconnect()
{
	_log->write("Client::disconnect","Socket "+Converter::itos(_server->socket())+" closed\n\n\n");
	close(_server->socket());
	free(_server);
	_status = DISCONNECT;
	return true;
}

bool Client::send_to(Datagram* dg, AddrStorage* addr)
{
	_r = sendto(addr->socket(), dg, DGSIZE, 0, addr->sockaddr(), addr->len());
	return (_r!=-1);
}

bool Client::receive_from(Datagram* dg, AddrStorage* addr)
{
	struct sockaddr_storage temp_addr;
	socklen_t temp_len;
	struct timeval time_val;
	fd_set readfds;

	int s = addr->socket();

	temp_len = sizeof(struct sockaddr_storage);

	time_val.tv_sec = 1;
	time_val.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(addr->socket(), &readfds);

	if(select(s+1, &readfds, NULL, NULL, &time_val))
	{
		init(dg,0,0);
		_r = recvfrom(s, dg, DGSIZE, 0, (struct sockaddr*) &temp_addr, &temp_len);

		addr = new AddrStorage((struct sockaddr*) &temp_addr,s, _log);

		return (_r!=-1);		
	}
	else
	{
		_log->write("Client::receive_from","Timer expired");
		return false;
	}
}


/*
 *
 * Protocoles de base
 *
 *
 */

void Client::toctoc(Datagram* dg, AddrStorage* addr)
{
	dg->seq++;
	send_to(dg, addr);
}



/*
 *
 * Surcouche client
 *
 *
 */

bool Client::is_connect()
{
	Datagram dg;
	init(&dg,0,0,"Je test");

	send_to(&dg, _server);

	if(receive_from(&dg, _server))
	{
		if(dg.code==0&&dg.seq==1)
		{
			_log->write("Client::is_connect", "Server answer successfull");
			return true;
		}
		else
		{
			_log->write("Client::is_connect", "Bad server answer");
			return false;
		}
	}
	else
	{
		_log->write("Client::is_connect", "No server answer");
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


bool Client::get_file(string file)
{
	return true;
}
