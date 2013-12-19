#ifndef ADDR_MAP
#define ADDR_MAP

//Attention à compiler avec le flag -std=c++0x
#include <unordered_map>
#include "AddrStorage.hpp"
#include "State.hpp"

class Equal
{
 public :
	bool operator() (const AddrStorage &lhs, const AddrStorage &rhs) const
	{
		bool addr = (lhs.paddr().compare(rhs.paddr()) == 0);
		bool port = (lhs.pport().compare(rhs.pport()) == 0);
		return addr && port;
	};
};

class Hash
{
 public :
	unsigned long operator()(const AddrStorage &key) const 
        {
		unsigned long h1 = hash<string>()(key.paddr());
		unsigned long h2 = hash<string>()(key.pport());
		return h1 ^ (h2 << 1); 
        };
};


typedef unordered_map<AddrStorage,State,Hash,Equal> addr_map;

#endif
