#ifndef CONVERTER
#define CONVERTER

#include "socket.h"
#include <vector>
using namespace std;

class Converter
{
 public :
	Converter(){};
	static string itos(int i);
	static int stoi(string s);
	static string cstos(char* cstr);
	static const char* stocs(string s);
	static vector<string> split(string str, string delim);
};

#endif
