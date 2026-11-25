#include "WebServer.hpp"
#include <iostream>

int main()
{
	try
	{
		WebServer serv(8080);
		while (1)
		{
			std::cerr << "--------------------------------\n";
			serv.check_clients();
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
	return 0;
}
