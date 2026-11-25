#include <string.h>
#include <string>
#include <stdexcept>

std::string error(const char* errHeader)
{
	return std::string(errHeader) + ": " + strerror(errno);
}

// terror stands for throw error
void terror(const char* errHeader)
{
	throw std::runtime_error(error(errHeader));
}