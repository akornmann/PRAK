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

	if(ask == "exit") close();
	else if(ask == "connect") connect(v);
	else if(ask == "dl") file(v);
	else if(ask == "error") error();
	else fail();

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
	switch(cmd.size())
	{
	case 1 :
		cout << "Connecting with default configuration file" << endl;
		delete _c;
		
		file = "server.cfg";
		_c = new Client(file);
		AddrStorage *addr;
		_c->synchronize(addr);
		//cout << "Server : " << *addr << endl;
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

void Shell::file(vector<string> v)
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
