#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <string>
#include <map>
#include "Server.hpp"


#define EMPTY_REQUEST ""
#define BAD_REQUEST "400"
#define FORBIDDEN "403"
#define NOT_FOUND "404"


typedef std::map<std::string,std:: string>  str_map;

typedef struct httpRequest
{
	std::string	method;
	std::string	path;
	std::string	version;
	str_map		headers;
	std::string	body;
}	httpRequest;

typedef struct httpResponse
{
	std::string	version;
	int			code;
	std::string	msg;
	str_map		headers;
	std::string	file_name;
	int			body_fd;
	size_t		body_size;
}	httpResponse;

class WebServer: public Server
{
private:
	class bad_request : public std::exception
	{
	public:
		const char*	message;
		int		code;
		bad_request() : message("Bad Request"), code(400) {}
		bad_request(const char* msg, int err)
			: message(msg), code(err) {}
		virtual ~bad_request() noexcept {}
	
		virtual const char* what() const noexcept
		{return message;}

		std::string str() const
		{return "Error " + std::to_string(code) + ": " + message;}
	};

public:
	using	Server::Server;

	void	manage_client(poll_it cli) override;

private:
	void	parse_request(poll_it cli, httpRequest& request);
	void	generate_response(httpRequest& request, httpResponse& response);
	void	send_response(poll_it cli, httpResponse& response);
	void	get_requested_file(httpRequest& request, httpResponse& response);
	// void	manage_request(poll_it cli, httpRequest& request);
};

#endif