#include <iostream>
#include <filesystem>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "WebServer.hpp"

#define BUFF_SIZE 5
#define WEB_DIR "web"

#define BLUE "\001\033[34m\002"
#define GREEN "\001\033[32m\002"
#define RED "\001\033[31m\002"


void	WebServer::manage_client(poll_it cli)
{
	httpRequest		request{};
	httpResponse	response{};

	try
	{
		try
		{
			parse_request(cli, request);
			generate_response(request, response);
		}
		catch(const bad_request& br)
		{
			response.version = "HTTP/1.1";
			response.code = br.code;
			response.msg = br.message;
			debug("Error: ", br.code);
		}
		
		send_response(cli, response);
	}
	catch(const std::exception& e)
	{
		print_log(e.what(), RED);
	}
}

void	WebServer::parse_request(poll_it cli, httpRequest& request)
{
	/***** Parse request line *****/

	std::string line = this->receive_str_line(cli);

	if (line.back() != '\r' || line.size() < 3)
		throw bad_request();
	line.pop_back();

	size_t first_space = line.find(' ');
	size_t last_space = line.rfind(' ');

	if (line.find(' ', first_space + 1) != last_space)
		throw bad_request();
	request.method  = line.substr(0, first_space);
	request.path = line.substr(first_space + 1, last_space - first_space - 1);
	request.version = line.substr(last_space + 1);
	
	print_log("method: " + request.method, BLUE);
	print_log("path: " + request.path, BLUE);
	print_log("version: " + request.version, BLUE);

	/***** Parse headers *****/

	line = this->receive_str_line(cli);
	while (line != "\r")
	{
		size_t sep = line.find(": ");
		if (sep == std::string::npos)
			throw bad_request();
		
		request.headers.insert({line.substr(0, sep),
								line.substr(sep + 2)});
		line = this->receive_str_line(cli);
	}

	for (str_map::iterator it = request.headers.begin(); it != request.headers.end(); it++)
		print_log(it->first + ": " + it->second, BLUE);

	/***** Read body *****/

	if (request.headers.find("Transfer-Encoding") != request.headers.end()
	 || request.headers.find("Content-Length") != request.headers.end())
	{
		try
		{
			while (1)
				request.body += receive_str_line(cli);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}

	print_log(request.body, BLUE);
}

void WebServer::generate_response(httpRequest& request, httpResponse& response)
{
	response.version = "HTTP/1.1";
	response.headers.insert({"Content-Length", "0"});
	if (request.method == "GET")
	{
		get_requested_file(request, response);
		if ((response.body_fd = open(response.file_name.c_str(), O_RDONLY)) < 0)
			throw bad_request("Not Found", 404);

		struct stat fs;
		if (fstat(response.body_fd, &fs) < 0)
		{
			close(response.body_fd);
			throw bad_request("Internal Server Error", 500);
		}
		response.body_size = fs.st_size;
	}
	response.headers["Content-Length"] = std::to_string(response.body_size);
	response.code = 200;
	response.msg = "OK";
}

static bool forbidden_path(std::string& requested)
{
	debug(requested);
    if (requested.empty() || requested[0] == '/')
        return true;
    std::string s = std::filesystem::path(requested).lexically_normal().generic_string();
	debug(s);
    if (s == "/" || s == "/.." || s.rfind("/../", 0) == 0 || s.find('/') == std::string::npos)
        return true;

    return false;
}

void WebServer::get_requested_file(httpRequest& request, httpResponse& response)
{
	if (request.path == "/")
	{
		response.file_name = WEB_DIR "/index.html";
		response.headers.insert({
						"Content-Type",
						"text/html"});
	}
	else if (request.path[0] == '/')
	{
		response.file_name = WEB_DIR + request.path;
		response.headers.insert({
						"Content-Type",
						"text/html"});
	}
	else throw bad_request("Not Found", 404);
	if (forbidden_path(response.file_name))
		throw bad_request("Forbidden", 403);
}

static void log_send(int fd, std::string str)
{
	print_log_no_endl(str, GREEN);
	send(fd, str.c_str(), str.size(), 0);
}

static void log_send(int fd, char* str, size_t size)
{
	str[size] = 0;
	print_log_no_endl(str, GREEN);
	send(fd, str, size, 0);
}

void WebServer::send_response(poll_it cli, httpResponse& response)
{
	/***** First line *****/
	log_send(cli->fd,
		response.version + ' '
		+ std::to_string(response.code) + ' '
		+ response.msg + "\r\n");

	/***** Headers *****/
	for (str_map::iterator it = response.headers.begin(); it != response.headers.end(); it++)
		log_send(cli->fd, it->first + ": " + it->second + "\r\n");

	log_send(cli->fd, "\r\n");

	/***** Body *****/
	if (response.body_fd <= 0)
		return;
	char	buff[BUFF_SIZE + 1];
	ssize_t	bytes;
	while ((bytes = read(response.body_fd, buff, BUFF_SIZE)) > 0)
		log_send(cli->fd, buff, static_cast<size_t>(bytes));
	close (response.body_fd);
}
