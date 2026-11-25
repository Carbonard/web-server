#include <string>
#include <iostream>

void debug(std::string msg)
{
	std::cerr << "\001\e[33m\002" << msg << "\001\e[0m\002\n";
}

void debug(std::string msg, int n)
{
	std::cerr << "\001\e[33m\002" << msg << n << "\001\e[0m\002\n";
}

void print_log_no_endl(std::string msg, const char* color)
{
	std::cerr << color << msg << "\001\e[0m\002";
}

void print_log(std::string msg, const char* color)
{
	std::cerr << color << msg << "\001\e[0m\002\n";
}
