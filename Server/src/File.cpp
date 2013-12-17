#include "File.h"
#include "socket.h"

File::File(const std::string file):_file(file)
{
	_out.open(_file.c_str(), std::ios_base::app);
	_in.open(_file.c_str());
}

void File::set_file(std::string file)
{
	_file = file;
	return;
}

void File::write(const std::string &msg)
{
	if(_out) _out << msg;
}

void File::write(const std::string &prefix, const std::string &msg)
{
	_out << currentTime() << " " << prefix << " : " << msg << std::endl;	
}

void File::write(const std::string &name, const std::string &prefix, const std::string &msg)
{
	_out << currentTime() << " (" << name << ") " << prefix << " : " << msg << std::endl;	
}


std::string File::read(int n)
{
	int lines = count();
	if(lines<n)
	{
		return "end";
	}
	else
	{
		_in.close();
		_in.open(_file.c_str());
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

int File::count()
{
	_in.close();
	_in.open(_file.c_str());
	int lines = 0;
	if(_in) while(_in.ignore(100, '\n')) ++lines;

	return lines;
}

std::string File::currentTime()
{
	time_t t = time(NULL);
 
	std::string time = ctime(&t);
	
	return time;
}


int File::size()
{
    int pos = _in.tellg();
    _in.seekg(0, std::ios_base::end);
    int size = _in.tellg();
    _in.seekg(pos, std::ios_base::beg);
    return size;
}
