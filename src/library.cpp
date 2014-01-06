#include "library.hpp"

library & insert(library &lib, Record &r)
{
	bool found = false;

	library::iterator it;
	
	for(it=lib.begin();it!=lib.end();++it)
	{
		if((*it).file()==r.file())
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		lib.push_back(r);
	}

	return lib;
}

library & remove(library &lib, const string &file)
{
	library::iterator it;
	
	for(it=lib.begin();it!=lib.end();)
	{
		if(it->file() == file) it = lib.erase(it);
		else ++it;
	}

	return lib;
}

library & operator+(library &rhs, library &lhs)
{
	library::iterator it;
	
	for(it=lhs.begin();it!=lhs.end();++it)
	{
		rhs = insert(rhs,*it);
	}

	return rhs;
}

ostream & operator<<(ostream &os, library &lib)
{
	library::iterator it;
	os << left << setw(10) << "file" << setw(60) << " | title" << endl << endl;

	for(it=lib.begin();it!=lib.end();++it)
	{
		os << *it << endl;
	}

	return os;
}
