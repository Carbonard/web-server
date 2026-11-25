#include <string>
#include <vector>
#include <arpa/inet.h>
#include <poll.h>
#include <iostream>

class Server
{
public:
	typedef std::vector<pollfd>::iterator poll_it;

	const uint16_t			port;

	Server(uint16_t port);
	~Server();

	int				accept_client();
	void			close_client(poll_it cli);
	void 			check_clients();

protected:
	std::vector<pollfd>		_poll_fds;
	nfds_t					_clients;
	sockaddr_in				_addr;
	int						_socket_fd;

	std::string		receive_str_line(poll_it cli);
	virtual void		manage_client(poll_it cli);

	typedef enum {SOCKET_EMPTY, CLI_DISCONECTED} e_issues;
	class client_issue : public std::exception
	{
	private:
		std::string	_msg;
		const e_issues	_error;

	public:
		client_issue(const std::string& msg, e_issues err)
			: _msg(msg), _error(err) {}
		virtual ~client_issue() noexcept {}
	
		virtual const char* what() const noexcept override
		{return _msg.c_str();}
		int error() const noexcept
		{return _error;}

		void print(std::ostream& out) const
		{out << "Error " << _error << ": " << _msg << '\n';}
	};
};


std::string error(const char* errHeader);
void terror(const char* errHeader);

void print_log(std::string msg, const char* color);
void print_log_no_endl(std::string msg, const char* color);
void debug(std::string msg);
void debug(std::string msg, int n);