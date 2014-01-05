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

string Record::formatFile(string file)
{
	file = file.substr(0,8);
	unsigned int lim = 8;
	if(file.length()<lim) for(unsigned int i=0;i<lim-file.length();++i) file = file + " ";
	return file;
}

ostream & operator<<(ostream &os, Record &r)
{
	string title = r._title.substr(0,70);
	string file = Record::formatFile(r._file);

	os << file << " " << title;

	return os;
}
