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
	
	for(it=lib.begin();it!=lib.end();++it)
	{
		os << *it << endl << endl;
	}

	return os;
}

void to_file(library &lib, const string &file)
{
	remove(Converter::stocs(file));
	File f(file);
	
	stringstream sstr;

	library::iterator it;
	for(it=lib.begin();it!=lib.end();++it)
	{
		sstr << it->file() << " " << it->title();
			
		vector<AddrStorage>::iterator it2;
		vector<AddrStorage> r = it->saved();
		
		for(it2=r.begin();it2!=r.end();++it2)
		{
			sstr << " " << *it2;
		}
		sstr << endl;
	
	}
	
	f.write(sstr.str());

	return;
}

library from_file(const string &file)
{
	library lib;	
	File f(file);
	
	for(int i=0;i<f.line();++i)
	{
		string l = f.read(i);
		cout << "form_file : " << l << endl;
		vector<string> v = Converter::split(l," ");
		int size = v.size();
		string file = v[0];
		string title = v[1];
		
		vector<string> w = Converter::split(v[2],":");
		AddrStorage addr(w[0],w[1]);
		
		Record r(file,title,addr);
		for(int i=3;i<size;++i)
		{
			vector<string> w = Converter::split(v[i],":");
			AddrStorage addr(w[0],w[1]);
			r.add(addr);
		}
		lib = insert(lib,r);
	}

	cout << "builed library : "<<endl<<lib<<endl;

	return lib;
}
