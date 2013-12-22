#include "Counter.hpp"

Counter::Counter(int max):_curr(0),_max(max)
{
}

Counter & Counter::operator++()
{
	if(_curr<_max) ++_curr;
	else throw (Exception("Counter::++ : Counter expired. (Too many retry)",__LINE__));
	return *this;
}

void Counter::restart(int max)
{
	_max = max;
	_curr = 0;
}
