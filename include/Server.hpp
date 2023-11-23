#ifndef SERVER_HPP
# define SERVER_HPP

# include "Webserv.hpp"

class Server
{
  public:
	Server(Config config);
	~Server();
	Request _request;
	Config _config;
	int getListeningSocket() const { return _listeningSocket; }
	bool isClientSocket(int socket) const;
	int acceptConnection();
	void receiveRequest(int socket);
	void sendResponse(std::string str, int socket);
	void log(const std::string &message);
	void logError(const std::string &message);
	void logInfo(const std::string &message);
	void logSuccess(const std::string &message);
	void addClientSocket(int socket);
	void closeConnection(int socket);
	void printClients();

  private:
	int _listeningSocket;
	std::vector<int> _clientSockets;

	void checkInputs();
	void initAndListen();
};

std::string getLocalIPAddress();

#endif
