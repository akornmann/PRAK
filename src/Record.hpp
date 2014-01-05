#ifndef RECORD
#define RECORD

#include "socket.hpp"
#include "AddrStorage.hpp"
#include "Exception.hpp"
#include "addr_map.hpp"

class Record
{
public :
	Record(const string &f, const string &t);
	
	string file();
	string title();
	static string formatFile(string file);
	friend ostream & operator<<(ostream &os, Record &r);

private :
	string _file;
	string _title;
};

#endif
