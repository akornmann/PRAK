#include "Exception.hpp"

Exception::Exception(const string &str, int line)
{
	_exc = str;
	_line = line;
}

const char * Exception::what() const throw()
{
	string str = "Ligne "+Converter::itos(_line)+" : "+_exc;
	return str.c_str();
}
