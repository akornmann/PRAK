#ifndef COUNTER
#define COUNTER

#include "Exception.hpp"

class Counter
{
public :
	Counter(int max, string to_throw);

	Counter & operator++();

	void restart(int max, string to_throw);
private :
	int _curr;
	int _max;
	string _error;
};

#endif
