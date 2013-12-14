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

vector<string> Converter::split(string str, string delim)
{ 
      unsigned start = 0;
      unsigned end; 
      vector<string> v; 

      while((end = str.find(delim, start)) != string::npos)
      { 
            v.push_back(str.substr(start, end-start)); 
            start = end + delim.length(); 
      }

      v.push_back(str.substr(start)); 
      return v; 
}
