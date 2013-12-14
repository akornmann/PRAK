#include "File.h"
#include "socket.h"
File::File(const std::string file):_file(file)
{
}
void File::set_file(std::string file)
{
	_file = file;
	return;
}


void File::write(const std::string &msg)
{
	std::ofstream out(_file.c_str(), std::ios_base::out | std::ios_base::app);
	out << msg;
}

void File::write(const std::string &prefix, const std::string &msg)
{
	std::ofstream out(_file.c_str(), std::ios_base::out | std::ios_base::app);
	
	out << currentTime() << " " << prefix << " : " << msg << std::endl;	
}

void File::write(const std::string &name, const std::string &prefix, const std::string &msg)
{
	std::ofstream out(_file.c_str(), std::ios_base::out | std::ios_base::app);
	
	out << currentTime() << " (" << name << ") " << prefix << " : " << msg << std::endl;	
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
		std::ifstream in(_file.c_str(), std::ifstream::in);
		char buffer[1024];
		int i = 0;
		
		while(i<n && in.getline(buffer,1024)) i++;
		
		std::string res(buffer);	
		return res;
	}
}

int File::count()
{
	std::ifstream in(_file.c_str(), std::ifstream::in);
	int lines = 0;
	if(in) while(in.ignore(100, '\n')) ++lines;

	return lines;
}

std::string File::currentTime()
{
	time_t t = time(NULL);
 
	std::string time = ctime(&t);
	
	return time;
}
