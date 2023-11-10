#ifndef SERVER_HPP
# define SERVER_HPP

# include "Request.hpp"
# include "Response.hpp"
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
# include "Config.hpp"

# define MAX_CONNECTIONS 255
# define BUFFER_SIZE 20
# define MAX_EVENTS 16
# define CRLF "\r\n"
# define LOOPBACK "127.0.0.1"

class Server
{
  public:
	Server(Config config);
	~Server();
	void runLoop();
	Request _request;
	Config _config;
	int getListeningSocket() const { return _listeningSocket; }
	bool isClientSocket(int socket) const;
	int acceptConnection();
	void receiveRequest(int socket);
	void sendResponse(std::string str, int socket);
	void	log(const std::string &message);
	void	logError(const std::string &message);
	void	logInfo(const std::string &message);
	void	logSuccess(const std::string &message);
	void	addClientSocket(int socket);
	void	closeConnection(int socket);
	void	printClients();

  private:
	int _listeningSocket;
	int _kq;
	std::vector<int> _clientSockets;

	void checkInputs();
	void initAndListen();
	void updateEvent(int ident, short filter, u_short flags, u_int fflags,
			int data, void *udata);
};

void	exitWithError(const std::string &errorMessage);

#endif
