#include "Shell.h"

Shell::Shell()
{
	system("clear");
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
	else if(ask == "status") status();
	else if(ask == "connect") connect(v);
	else if(ask == "disconnect") disconnect();
	else if(ask == "file") file(v);
	else error();

	wait_command();
}

void Shell::close()
{
	cout << "Goodbye !" << endl;
	exit(true);
	return;
}

void Shell::status()
{
	string s = c.status();
	cout << s << endl;
	return;
}

void Shell::error()
{
	cout << "Command unknow" << endl;
	return;
}

void Shell::fail()
{
	cout << "Command fail" << endl;
	return;
}

void Shell::connect(vector<string> cmd)
{
	switch(cmd.size())
	{
	case 1 :
		cout << "Connecting with default configuration file" << endl;
		c.server_select();
		break;
	case 2 :
		cout << "Connecting with custom configuration file" << endl;
		c.set_conf(cmd[1]);
		c.server_select();
		break;
	case 3 :
		cout << "Connecting to a specified server" << endl;
		break;
	default :
		error();
		break;
	}
	status();
	return;
}

void Shell::disconnect()
{
	c.disconnect();
	status();
	return;
}

void Shell::file(vector<string> v)
{
	switch(v.size())
	{
	case 2 :
		c.get_file(v[1]);
		return;
	default :
		error();
		return;
	}
	
	return;
}
