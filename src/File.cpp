#include "File.hpp"

File::File(std::string f)
{
	file(f);
}

void File::file(std::string file)
{
	_file = file;
	open();
	return;
}

void File::open()
{
	close();

	_out.open(_file.c_str(), std::ios_base::app);
	_in.open(_file.c_str());
	return;
}

void File::close()
{
	_out.close();
	_in.close();
}

void File::write(const std::string &msg)
{
	if(_out) _out << msg;
}


std::string File::read(int n)
{
	int lines = line();
	if(lines<n)
	{
		return "end";
	}
	else
	{
		close();
		open();

		char buffer[1024];
		int i = 0;
		if(_in)
		{
			while(i<n && _in.getline(buffer,1024)) i++;
			std::string res(buffer);	
			return res;
		}
		else return "end";	
	}
}

char* File::readChar(int n)
{
	char* res = new char[n+1];
	if(_in)
	{
		for(int i=0;i<n;i++) _in.get(res[i]);
		res[n] = '\0';
	}

	return res;
}

int File::line()
{
	close();
	open();
	int lines = 0;
	if(_in) while(_in.ignore(100, '\n')) ++lines;

	return lines;
}

int File::size()
{
    int pos = _in.tellg();
    _in.seekg(0, std::ios_base::end);
    int size = _in.tellg();
    _in.seekg(pos, std::ios_base::beg);
    return size;
}
