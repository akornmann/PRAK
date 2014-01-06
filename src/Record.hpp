#ifndef RECORD
#define RECORD

#include "socket.hpp"
#include "AddrStorage.hpp"
#include "Exception.hpp"
#include "addr_map.hpp"
#include <iomanip>

class Record
{
public :
	Record(const string &f, const string &t);
	
	string file();
	string title();
	friend ostream & operator<<(ostream &os, Record &r);

private :
	string _file;
	string _title;
};

#endif
