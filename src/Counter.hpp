#ifndef COUNTER
#define COUNTER

#include "Exception.hpp"

class Counter
{
public :
	Counter(int max);

	Counter & operator++();

	void restart(int max);
private :
	int _curr;
	int _max;
};

#endif
