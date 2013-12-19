#ifndef SHELL
#define SHELL

#include "socket.hpp"
#include "Client.hpp"

class Shell
{
 public :
	Shell();
	~Shell();

 private :
	void wait_command();

	void fail();
	void close();
	void connect(vector<string> cmd);
	void file(vector<string> cmd);
	void error();
	Client *_c;
};


#endif
