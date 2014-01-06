#include "Record.hpp"

Record::Record(const string &f, const string &t):_file(f),_title(t)
{
}


string Record::file()
{
	return _file;
}

string Record::title()
{
	return _title;
}

ostream & operator<<(ostream &os, Record &r)
{	
	os << left << setw(10) << r._file << setw(60) << " | " + r._title;

	return os;
}
