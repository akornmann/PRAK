#include "File.h"

File::File(const std::string file):_file(file)
{
}

void File::write(const std::string &msg)
{
	std::ofstream out(_file.c_str(), std::ios_base::out | std::ios_base::app);
	out << msg << std::endl;
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


std::string File::read()
{
	std::ifstream in(_file.c_str(), std::ifstream::in);
	char buffer[1024];
	in.getline(buffer,1024);
	
	std::string res(buffer);
	return res;
}

std::string File::currentTime()
{
  time_t t = time(NULL);
 
  std::string time = ctime(&t);
 
  return time;
}
