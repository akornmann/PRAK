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
			
			vector<AddrStorage>::iterator it2;
			vector<AddrStorage> saved = r.saved();
			for(it2=saved.begin();it2!=saved.end();++it2)
			{
				it->add(*it2);
			}
			
			break;
		}
	}

	if(!found)
	{
		lib.push_back(r);
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
	
	os << "File Title" << endl;
	for(it=lib.begin();it!=lib.end();++it)
	{
		os << *it << endl;
	}

	return os;
}
