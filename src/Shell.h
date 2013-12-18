#ifndef SHELL
#define SHELL

#include "socket.h"
#include "Client.h"

class Shell
{
 public :
	Shell();
	~Shell();

 private :
	void wait_command();

	void close();
	void disconnect();
	void status();
	void connect(vector<string> cmd);
	void error();
	void fail();
	void file(vector<string> cmd);

	Client c;
};


#endif
