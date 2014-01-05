#ifndef LIBRARY
#define LIBRARY

#include "Record.hpp"
#include "File.hpp"
#include "Converter.hpp"
#include <sstream>

typedef vector<Record> library;

library & insert(library &lib, Record &r);
library & remove(library &lib, const string &file);
library & operator+(library &rhs, library &lhs);
ostream & operator<<(ostream &os, library &lib);

#endif
