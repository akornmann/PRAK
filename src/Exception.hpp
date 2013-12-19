#ifndef EXCEPTION
#define EXCEPTION

#include "Converter.hpp"
#include <iostream>
#include <sstream>
#include <exception>
 
class Exception : public std::exception
{
 public :
	Exception(const string &str, int line);
	virtual ~Exception() throw(){}
 
	virtual const char * what() const throw();
 
 private:
	std::string _exc;
	int _line;
};

#endif
