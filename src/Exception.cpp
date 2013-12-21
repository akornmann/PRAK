#include "Exception.hpp"

Exception::Exception(const string &str, int line)
{
	_exc = str;
	_line = line;
	_addr = AddrStorage();
}

Exception::Exception(const string &str, const AddrStorage &addr, int line)
{
	_exc = str;
	_line = line;
	_addr = addr;
}

const char * Exception::what() const throw()
{
	string str = "Ligne "+Converter::itos(_line)+" : "+_exc;
	cout << str << endl;
	return str.c_str();
}
