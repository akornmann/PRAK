#include "Converter.hpp"

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

string Converter::cstos(const char* cstr)
{
	string s = string(cstr);
	return s;
}

const char* Converter::stocs(string str)
{
	return str.c_str();
}

vector<string> Converter::split(const string &str, char delim)
{
    stringstream ss(str);
    string item;
    vector<string> elems;

    while (std::getline(ss, item, delim))
    {
	    elems.push_back(item);
    }
    return elems;
}
