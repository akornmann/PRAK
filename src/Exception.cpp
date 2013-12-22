#include "Exception.hpp"

Exception::Exception(const string &str, int line):_addr("127.0.0.1","0")
{
	_exc = str;
	_line = line;
}

Exception::Exception(const string &str, const AddrStorage &addr, int line)
{
	_exc = str;
	_line = line;
	_addr = addr;
}

const char * Exception::what() const throw()
{
	if(_addr.pport()!= "22")
		return Converter::stocs(_exc+" "+_addr.paddr()+":"+_addr.pport());
	else
		return Converter::stocs(_exc);		
}

AddrStorage & Exception::addr()
{
	return _addr;
}
