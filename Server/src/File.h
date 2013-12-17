#include <fstream>
#include <ctime>

#ifndef FILE
#define FILE

class File
{
 public :
	File(const std::string file);
	void write(const std::string &msg);
	void write(const std::string &prefix, const std::string &msg);
	void write(const std::string &name, const std::string &prefix, const std::string &msg);
	std::string read(int n);
	char* readChar(int n);
	int count();
	std::string currentTime();

	void set_file(std::string file);

	int size();
 private :
	std::string _file;
	std::ofstream _out;
	std::ifstream _in;
};

#endif
