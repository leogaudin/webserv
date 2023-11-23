#ifndef WEBSERV_HPP
# define WEBSERV_HPP

enum	Method
{
	GET,
	POST,
	DELETE,
	UNKNOWN
};

# include <algorithm>
# include <arpa/inet.h>
# include <dirent.h>
# include <fcntl.h>
# include <fstream>
# include <ifaddrs.h>
# include <iostream>
# include <map>
# include <netinet/in.h>
# include <sstream>
# include <stdio.h>
# include <stdlib.h>
# include <string>
# include <sys/event.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
# include <vector>

# include "colors.h"
# include "Config.hpp"
# include "Request.hpp"
# include "CGI.hpp"
# include "Response.hpp"
# include "Server.hpp"
# include "ServerManager.hpp"

# define MAX_CONNECTIONS 255
# define BUFFER_SIZE 20
# define MAX_EVENTS 1
# define CRLF "\r\n"
# define LOOPBACK "127.0.0.1"
# define MAX_SERVERS 3

void		printHeaders(std::map< std::string, std::vector<std::string> > headers);
std::string toLowercase(std::string str);
void		exitWithError(const std::string &errorMessage);
Config*     parseConfig(std::string filename);

#endif
