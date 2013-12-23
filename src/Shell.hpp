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
	void download(vector<string> cmd);
	void upload(vector<string> cmd);
	void library();

	Client *_c;
};


#endif
