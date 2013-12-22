#ifndef EXCEPTION
#define EXCEPTION

#include <iostream>
#include <sstream>
#include <exception>

#include "AddrStorage.hpp"
#include "Converter.hpp"

using namespace std;

class Exception : public exception
{
 public :
	Exception(const string &str, int line);
	Exception(const string &str, const AddrStorage &addr, int line);
	virtual ~Exception() throw(){}
 
	virtual const char * what() const throw();
	AddrStorage & addr();
 
 private:
	string _exc;
	int _line;
	AddrStorage _addr;
};

#endif
