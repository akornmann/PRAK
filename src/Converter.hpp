#ifndef CONVERTER
#define CONVERTER

#include "socket.hpp"
#include <vector>
using namespace std;

class Converter
{
 public :
	Converter(){};
	static string itos(int i);
	static int stoi(string s);
	static string cstos(char* cstr);
	static const char* stocs(string str);
	static vector<string> split(string str, string delim);
};

#endif
