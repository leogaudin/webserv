#include "../include/Webserv.hpp"

/**
 * @brief   Constructor for the Server class
 *
 * @param ip_address	The IP address to listen on
 * @param port			The port to listen on
 */
Server::Server(Config config) {
	_config = config;
	checkInputs();
	initAndListen();
}

/**
* @brief   Closes the listening socket and exits the program
*/
Server::~Server() {
	close(_listeningSocket);
	exit(EXIT_SUCCESS);
}

/**
 * @brief   Checks the validity of the port.
 */
void Server::checkInputs() {
	// if (_config._listen < 1024)
	// 	exitWithError("Port numbers 0 - 1023 are used for common ports");
	// if (_config._listen > 49151)
	// 	exitWithError("Port numbers 49152 - 65535 are reserved for clients");
}

/**
 * @brief   Initialises the server and starts listening for connections.
 *
 * This function initialises the server and starts listening for connections.
 * It first sets up the socket information, then creates a socket, binds it to the specified host and port, and starts listening for connections.
 * If any of these operations fail, it calls the exitWithError function with an appropriate error message.
 */
void Server::initAndListen() {
	sockaddr_in socketInfo;
	socklen_t socketInfoSize = sizeof(socketInfo);

	memset(&socketInfo, 0, sizeof(socketInfo));
	socketInfo.sin_family = AF_INET;
	socketInfo.sin_port = htons(_config._listen);
	socketInfo.sin_addr.s_addr = inet_addr(_config._host.c_str());

	_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listeningSocket < 0)
		exitWithError("Error creating socket: " + std::string(strerror(errno)));
	if (bind(_listeningSocket, (sockaddr *)&socketInfo, socketInfoSize) < 0)
		exitWithError("Error binding socket: " + std::string(strerror(errno)));
	if (listen(_listeningSocket, MAX_CONNECTIONS) < 0)
		exitWithError("Socket listening failed: " + std::string(strerror(errno)));
	logSuccess("Listening on http://" + std::string(inet_ntoa(socketInfo.sin_addr)) + ":" + std::to_string(ntohs(socketInfo.sin_port)));
}

/**
 * @brief	Accepts a new connection.
 *
 * If the accept operation fails, it calls the exitWithError function with an appropriate error message.
 * Otherwise, it adds the client socket to the clients list and logs the connection.
 *
 * @return	int	The client socket.
 */
int Server::acceptConnection() {
	sockaddr_in socketInfo;
	socklen_t socketInfoSize = sizeof(socketInfo);

	int clientSocket = accept(_listeningSocket, (sockaddr *)&socketInfo, &socketInfoSize);
	if (clientSocket == -1)
		exitWithError("Error accepting new connection");
	addClientSocket(clientSocket);
	logInfo(std::string(inet_ntoa(socketInfo.sin_addr)) + ":" + std::to_string(ntohs(socketInfo.sin_port)) + " connected");
	return clientSocket;
}

/**
 * @brief	Receives a message from a client socket.
 *
 * It reads data from the socket into a buffer until no more data is available, then appends the data to a string.
 * If the read operation fails, it logs an error, closes the connection, and breaks the loop.
 * If the string is not empty after the loop, it creates a Request object from the string and the configuration.
 */
void Server::receiveRequest(int socket) {
	std::string stash = "";
	char buffer[BUFFER_SIZE];
	long bytesReceived = 0;
	while (true) {
		bytesReceived = recv(socket, buffer, BUFFER_SIZE, 0);
		if (bytesReceived < 0) {
			logError("Failed to read request from client");
			closeConnection(socket);
			break;
		}
		stash.append(buffer, bytesReceived);
		if (bytesReceived < BUFFER_SIZE) {
			break;
		}
	}
	if (stash != "")
		_request = Request(stash, _config);
}

/**
 * @brief	Sends a response to a client socket.
 *
 * It sends data from the response string to the socket until all data has been sent.
 * If the send operation fails, it logs an error, closes the connection, and returns.
 * After the loop, it logs a success message.
 */
void Server::sendResponse(std::string str, int socket) {
	long bytesSent = 0;
	size_t totalBytesSent = 0;
	size_t bytesToSend = str.size();
	const char* buffer = str.c_str();

	while (totalBytesSent < bytesToSend) {
		bytesSent = send(socket, buffer + totalBytesSent, bytesToSend - totalBytesSent, 0);
		if (bytesSent == -1) {
			logError("Error sending response to client: " + std::string(strerror(errno)));
			closeConnection(socket);
			return;
		}
		totalBytesSent += bytesSent;
	}

	logSuccess("Response sent to client");
}

void	Server::log(const std::string &message)
{
	std::cout << _config._serverName << "\t" << message << std::endl;
}

void	Server::logError(const std::string &message)
{
	std::cerr << BOLD_RED << _config._serverName << "\t" << "⚠ " << message << RESET << std::endl;
}

void	Server::logInfo(const std::string &message)
{
	std::cout << BOLD_CYAN << _config._serverName << "\t" << "ℹ " << message << RESET << std::endl;
}

void	Server::logSuccess(const std::string &message)
{
	std::cout << BOLD_GREEN << _config._serverName << "\t" << "✔ " << message << RESET << std::endl;
}

void	exitWithError(const std::string &errorMessage)
{
	std::cerr << BOLD_RED << "⚠ " << errorMessage << RESET << std::endl;
	exit(EXIT_FAILURE);
}

bool Server::isClientSocket(int socket) const {
	return (std::find(_clientSockets.begin(), _clientSockets.end(), socket) != _clientSockets.end());
}

void Server::addClientSocket(int socket) {
	_clientSockets.push_back(socket);
}

void Server::closeConnection(int socket) {
	_clientSockets.erase(std::remove(_clientSockets.begin(), _clientSockets.end(), socket), _clientSockets.end());
	close(socket);
}

void Server::printClients() {
	logInfo("Clients:");
	std::vector<int>::iterator it;
	for (it = _clientSockets.begin(); it != _clientSockets.end(); ++it) {
		logInfo(std::to_string(*it));
	}
}
