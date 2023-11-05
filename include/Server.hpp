#ifndef SERVER_HPP
# define SERVER_HPP

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

typedef struct s_client
{
	int fd;
} t_client;

class Server
{
  public:
	Server(std::string ip_address, int port);
	~Server();
	void runLoop();

  private:
	// t_client clients[MAX_CONNECTIONS];
	std::string _ipAddress;
	int _port;
	int _listeningSocket;
	struct sockaddr_in _socketInfo;
	unsigned int _socketInfoSize;

	std::string buildResponse(std::string str);
	void sendResponse(std::string str, int socket);
	void initAndListen();
	int addClient(int fd);
	int removeClient(int fd);
	std::string receiveMessage(int socket);
};

void	log(const std::string &message);
void	logError(const std::string &message);
void	logInfo(const std::string &message);
void	logSuccess(const std::string &message);
void	exitWithError(const std::string &errorMessage);

#endif
