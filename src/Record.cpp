#include "Record.hpp"

Record::Record(const string &f, const string &t, const AddrStorage &addr):_file(f),_title(t)
{
	_saved.push_back(addr);
}

void Record::add(const AddrStorage &addr)
{
	bool found = false;
	Equal e;
	vector<AddrStorage>::iterator it;

	for(it=_saved.begin();it!=_saved.end();++it)
	{
		if(e(*it,addr))
		{
			found = true;
			break;
		}
	}
	if(!found) _saved.push_back(addr);

	return;
}

void Record::remove(const AddrStorage &addr)
{
	Equal e;
	vector<AddrStorage>::iterator it;

	for(it=_saved.begin();it!=_saved.end();)
	{
		if(e(*it,addr)) it = _saved.erase(it);
		else ++it;
	}

	return;
}

string Record::file()
{
	return _file;
}

vector<AddrStorage> Record::saved()
{
	return _saved;
}


ostream & operator<<(ostream &os, Record &r)
{
	os << r._file << " " << r._title << " ";

	vector<AddrStorage>::iterator it;

	for(it=r._saved.begin();it!=r._saved.end();++it)
	{
		os << *it << " ";
	}
	
	return os;
}
