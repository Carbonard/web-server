#include <unistd.h>
#include <string.h>
#include <stdexcept>
#include "Server.hpp"

Server::Server(uint16_t port)
:  port(port),
  _poll_fds(1),
  _clients(0),
  _addr{}
{
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	_poll_fds.front().fd = _socket_fd;
	if (_socket_fd < 0) terror("socket");
debug("Open socket ", _socket_fd);
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &_addr.sin_addr);

	if (bind(_socket_fd, (struct sockaddr *)&_addr, sizeof(_addr)) == -1)
	{
		int err = errno;
		close(_socket_fd);
		throw std::runtime_error(std::string("socket: ") + strerror(err));
	}

	if (listen(_socket_fd, 10) == -1)
	{
		int err = errno;
		close(_socket_fd);
		throw std::runtime_error(std::string("listen: ") + strerror(err));
	}

}

Server::~Server()
{
	while (_poll_fds.size() > 0)
	{
		close(_poll_fds.back().fd);
debug("Close socket ", _poll_fds.back().fd);
		_poll_fds.pop_back();
	}
}
