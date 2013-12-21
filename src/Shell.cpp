#include "Shell.hpp"

Shell::Shell()
{
	system("clear");
	string file = "server.cfg";
	_c = new Client(file);
	wait_command();
}

Shell::~Shell()
{
}

void Shell::wait_command()
{
	cout << "PRAKClient> ";
	
	string cmd;
	getline(cin,cmd);
	
	string delim = " ";
	vector<string> v = Converter::split(cmd, delim);

	string ask = v[0];

	try
	{
		if(ask == "exit") close();
		else if(ask == "connect") connect(v);
		else if(ask == "dl" || ask == "lire") download(v);
		else if(ask == "ul" || ask == "stocker") upload(v);
		else if(ask == "error") error();
		else fail();
	}
	catch(Exception e)
	{
		//DECONNEXION !!!
		e.what();
	}

	wait_command();
}

void Shell::close()
{
	delete _c;
	cout << "Goodbye !" << endl;
	exit(true);
	return;
}

void Shell::fail()
{
	cout << "Command fail" << endl;
	return;
}

void Shell::connect(vector<string> cmd)
{
	string file;
	AddrStorage *addr = new AddrStorage();
	switch(cmd.size())
	{
	case 1 :
		cout << "Connecting with default configuration file" << endl;
		delete _c;
		
		file = "server.cfg";
		_c = new Client(file);
		_c->synchronize(addr);
		break;
	case 2 :
		cout << "Connecting with custom configuration file" << endl;
		delete _c;
		
		file = cmd[1];
		_c = new Client(file);
		break;
	default :
		fail();
		break;
	}

	return;
}

void Shell::download(vector<string> v)
{
	switch(v.size())
	{
	case 2 :
		_c->get_file(v[1]);
		return;
	default :
		fail();
		return;
	}
	
	return;
}

void Shell::upload(vector<string> v)
{
	switch(v.size())
	{
	case 3 :
		_c->send_file(v[1],v[2]);
		return;
	default :
		fail();
		return;
	}
	
	return;
}

void Shell::error()
{
	exc error = _c->error();
	exc::const_iterator it;
	for(it=error.begin();it!=error.end();++it)
	{
		cout << it->what() << endl;
	}
	return;
}
