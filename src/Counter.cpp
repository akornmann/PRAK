#include "Counter.hpp"

Counter::Counter(int max, string to_throw):_curr(0),_max(max),_error(to_throw)
{
}

Counter & Counter::operator++()
{
	if(_curr<_max) ++_curr;
	else throw (Exception(_error,__LINE__));
	return *this;
}

void Counter::restart(int max, string to_throw)
{
	_max = max;
	_curr = 0;
	_error = to_throw;
}
