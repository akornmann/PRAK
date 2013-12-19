#include "Datagram.hpp"

Datagram::Datagram(int code, int seq, string s)
{
	init(code,seq,s);
}

Datagram::Datagram(int code, int seq)
{
	init(code,seq);
}

Datagram::Datagram(int code)
{
	init(code);
}

Datagram::Datagram()
{
	init(0);
}

void Datagram::init(int code, int seq, string s)
{
	s = s.substr(0,DATASIZE);
	
	memset(this,0,sizeof(*this));
	
	code = code;
	seq = seq;
	strcpy(data,Converter::stocs(s));
}

void Datagram::init(int code, int seq)
{
	init(code,seq,"");
}

void Datagram::init(int code)
{
	init(code,0);
}

ostream & operator<<(ostream &os, const Datagram &dg)
{
	return os << "Datagram(" << dg.code << "," << dg.seq << "," << dg.data << ")";
}
