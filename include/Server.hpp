#ifndef SERVER_HPP
# define SERVER_HPP

# include "Request.hpp"
# include "colors.h"
# include <arpa/inet.h>
# include <fcntl.h>
# include <iostream>
# include <sstream>
# include <stdio.h>
# include <stdlib.h>
# include <string>
# include <sys/event.h>
# include <sys/socket.h>
# include <unistd.h>

# define MAX_CONNECTIONS 64
# define BUFFER_SIZE 1024
# define MAX_EVENTS 16

class Server
{
  public:
	Server(std::string ip_address, int port);
	~Server();
	void runLoop();

  private:
	std::string _ipAddress;
	int _port;
	int _listeningSocket;
	int _kq;

	void checkInputs();
	void initAndListen();
	std::string receiveMessage(int socket);
	std::string buildResponse(std::string str);
	void sendResponse(std::string str, int socket);
	void updateEvent(int ident, short filter, u_short flags, u_int fflags,
			int data, void *udata);
	void acceptConnection();
};

void	log(const std::string &message);
void	logError(const std::string &message);
void	logInfo(const std::string &message);
void	logSuccess(const std::string &message);
void	exitWithError(const std::string &errorMessage);

#endif
