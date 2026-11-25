#include <unistd.h>
#include <fcntl.h>
#include "Server.hpp"

static void set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL);
	if (flags < 0) terror("fcntl F_GETFL");

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		terror("fcntl F_SETFL");
}

int Server::accept_client()
{
	int client_fd = accept(_socket_fd, NULL, NULL);
	if (client_fd < 0) terror("client");

	// set_nonblocking(client_fd);

debug("Open client ", client_fd);

	pollfd aux = {};
	aux.fd = client_fd;
	_poll_fds.push_back(aux);

	return client_fd;
}

void Server::close_client(std::vector<pollfd>::iterator cli)
{
debug("Close client ", cli->fd);
	close(cli->fd);
	_poll_fds.erase(cli);
}

void Server::check_clients()
{
	for (size_t i = 0; i < _poll_fds.size(); i++)
		_poll_fds[i].events = POLLIN;

	poll(_poll_fds.data(), static_cast<nfds_t>(_poll_fds.size()), -1);

	if (_poll_fds.front().revents == POLLIN)
		accept_client();

	for (size_t i = 1; i < _poll_fds.size(); i++)
	{
		if (_poll_fds[i].revents & POLLIN)
		try
		{
			manage_client(_poll_fds.begin() + i);
		} catch (client_issue& e)
		{
			if (e.error() == CLI_DISCONECTED)
				i--;
		}
	}
}

std::string Server::receive_str_line(poll_it cli)
{
	std::string ret;

	ssize_t bytes_read = 0;
	char	c;
	do
	{
		bytes_read = recv(cli->fd, &c, 1, 0);
		if (bytes_read < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				throw client_issue("Nothing to read", SOCKET_EMPTY);
			terror("recv");
		}
		if (bytes_read == 0)
		{
			close_client(cli);
			if (ret.length() == 0)
				throw client_issue("Client disconected", CLI_DISCONECTED);
			return ret;
		}
		if (bytes_read > 0 && c != '\n')
			ret += c;
	} while (bytes_read > 0 && c != '\n');
	return ret;
}

#include <iostream>

void	Server::manage_client(poll_it cli)
{
	std::string msg = receive_str_line(cli);
	std::cout << "Received from " << cli->fd << " : '" << msg << "'\n";
}
