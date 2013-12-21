#include <fstream>

#ifndef FILE
#define FILE

#include "socket.hpp"

class File
{
 public :
	File() {};
	File(std::string f);

	void open();
	void close();
	//Ecriture dans le fichier
	void write(const std::string &msg);
	
	//Lecture dans le fichier
	std::string read(int n);
	char* readChar(int n);

	//Infos sur le fichier
	int size();
	int line();

	//Setter/getter
	void file(std::string file);
	std::string file() const { return _file; };

 private :
	std::string _file;
	std::ofstream _out;
	std::ifstream _in;
};

#endif
