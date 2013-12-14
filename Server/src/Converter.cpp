#include "Converter.h"

string Converter::itos(int i)
{
    string str;
    ostringstream temp;
    temp << i;
    return temp.str();
}

int Converter::stoi(string s)
{
	int n;
	istringstream (s) >> n;
	return n;
}

string Converter::cstos(char* cstr)
{
	string s = string(cstr);
	return s;
}

const char* Converter::stocs(string s)
{
	return s.c_str();
}

