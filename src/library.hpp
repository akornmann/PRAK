#ifndef LIBRARY
#define LIBRARY

#include "Record.hpp"

typedef vector<Record> library;

library & insert(library &lib, Record &r);
library & operator+(library &rhs, library &lhs);
ostream & operator<<(ostream &os, library &lib);

#endif
