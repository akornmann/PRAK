#include "Shell.hpp"

Shell::Shell()
{
	string file = "server.cfg";
	try
	{
		_c = new Client(file);
	}
	catch(Exception e)
	{
		cout << endl <<  "A fatal error occured : " << endl << e.what() << endl << endl;
		exit(false);
	}
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
	
	char delim = ' ';
	vector<string> v = Converter::split(cmd, delim);

	string ask = v[0];

	try
	{
		if(ask == "exit") close();
		else if(ask == "dl" || ask == "lire") download(v);
		else if(ask == "ul" || ask == "stocker") upload(v);
		else if(ask == "lib" || ask == "catalogue") library();
		else if(ask == "rm" || ask == "detruire") remove(v);
		else fail();
	}
	catch(Exception e)
	{
		cout << endl <<  "An error occured : " << endl << e.what() << endl << endl;
		AddrStorage addr = e.addr();
		if(addr.pport()!="0") _c->disconnect_req(addr);
	}

	wait_command();
}

void Shell::close()
{
	delete _c;
	cout << endl << "Goodbye !" << endl << endl;
	exit(true);
	return;
}

void Shell::fail()
{
	cout << endl << "Command fail" << endl << endl;
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

void Shell::library()
{
	_c->get_library();
	return;
}

void Shell::remove(vector<string> v)
{
	switch(v.size())
	{
	case 2 :
		_c->remove_file(v[1]);
		return;
	default :
		fail();
		return;
	}
	return;
}
