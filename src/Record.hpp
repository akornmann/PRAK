#ifndef RECORD
#define RECORD

#include "socket.hpp"
#include "AddrStorage.hpp"
#include "Exception.hpp"
#include "addr_map.hpp"

class Record
{
public :
	Record(const string &f, const string &t, const AddrStorage &addr);
	void add(const AddrStorage &addr);
	void remove(const AddrStorage &addr);
	
	string file();
	vector<AddrStorage> saved();

	friend ostream & operator<<(ostream &os, Record &r);

private :
	string _file;
	string _title;
	vector<AddrStorage> _saved;
};

#endif
