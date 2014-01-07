#include <fstream>

#ifndef FILE
#define FILE

#include "socket.hpp"

using namespace std;

class File
{
 public :
	File() {};
	File(string f);
	~File();

	static bool exist(const string &file);
	void open();
	void close();
	//Ecriture dans le fichier
	void write(const string &msg);
	
	//Lecture dans le fichier
	string read(int n);
	char* readChar(int n);

	//Infos sur le fichier
	int size();
	int line();

	//Setter/getter
	void file(string file);
	string file() const { return _file; };

 private :
	string _file;
	ofstream _out;
	ifstream _in;
};

#endif
