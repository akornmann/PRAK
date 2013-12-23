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

string Record::title()
{
	return _title;
}

vector<AddrStorage> Record::saved()
{
	return _saved;
}


ostream & operator<<(ostream &os, Record &r)
{
	string title = r._title.substr(0,70);
	string file = r._file.substr(0,8);
	unsigned int lim = 8;
	if(file.length()<lim) for(unsigned int i=0;i<lim-file.length();++i) file = file + " ";
	os << file << " " << title;

	/*
	vector<AddrStorage>::iterator it;

	for(it=r._saved.begin();it!=r._saved.end();++it)
	{
		os << "stored at " <<*it<<endl;
	}
	*/
	
	return os;
}
