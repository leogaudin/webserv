#include "../include/Server.hpp"

/**
 * @brief Constructor for the Server class
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
* @brief Closes the listening socket and exits the program
*/
Server::~Server() {
	close(_listeningSocket);
	exit(EXIT_SUCCESS);
}

/**
 * @brief Checks the validity of the port
 */
void Server::checkInputs() {
	if (_config._listen < 1024)
		exitWithError("Port numbers 0 - 1023 are used for common ports");
	if (_config._listen > 49151)
		exitWithError("Port numbers 49152 - 65535 are reserved for clients");
}

/**
 * @brief Initialises the server and starts listening for connections
 */
void Server::initAndListen() {
	sockaddr_in socketInfo;
	socklen_t socketInfoSize = sizeof(socketInfo);

	memset(&socketInfo, 0, sizeof(socketInfo));
	socketInfo.sin_family = AF_INET;
	socketInfo.sin_port = htons(_config._listen);
	socketInfo.sin_addr.s_addr = inet_addr(LOOPBACK);

	_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listeningSocket < 0)
		exitWithError("Creating socket");
	if (bind(_listeningSocket, (sockaddr *)&socketInfo, socketInfoSize) < 0 && errno == EADDRINUSE)
		exitWithError("Address already in use");
	if (listen(_listeningSocket, MAX_CONNECTIONS) < 0)
		exitWithError("Socket listening");
	logSuccess("Listening at http://" + std::string(inet_ntoa(socketInfo.sin_addr)) + ":" + std::to_string(ntohs(socketInfo.sin_port)) + "\n");
}

// /**
//  * @brief The main event loop. Waits for new events and handles them
//  * in a non-blocking way.
//  */
// void Server::runLoop() {
//     struct kevent evList[MAX_EVENTS];
// 	_kq = kqueue();
// 	if (_kq == -1)
// 		exitWithError("Failed to create kqueue");

//     updateEvent(_listeningSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);

// 	while (true) {
// 		// logInfo("Waiting for new events...");
// 		int newEvents = kevent(_kq, NULL, 0, evList, MAX_EVENTS, NULL);
// 		if (newEvents <= 0)
// 			continue;
// 		for (int i = 0; i < newEvents; i++) {
// 			int eventSocket = evList[i].ident;
// 			if (eventSocket == _listeningSocket) {
// 				int clientSocket = acceptConnection();
// 				fcntl(clientSocket, F_SETFL, O_NONBLOCK);
// 				updateEvent(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
// 				updateEvent(clientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
// 			}
// 			else if (evList[i].flags & EV_EOF) {
// 				logInfo("Client has disconnected");
// 				updateEvent(eventSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
// 				updateEvent(eventSocket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
// 			}
// 			else if (evList[i].filter == EVFILT_READ) {
// 				receiveRequest(eventSocket);
// 				updateEvent(eventSocket, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
// 				updateEvent(eventSocket, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
// 			}
// 			else if (evList[i].filter == EVFILT_WRITE) {
// 				Response response = Response(_request, _config);
// 				sendResponse(response.build(), eventSocket);
// 				updateEvent(eventSocket, EVFILT_READ, EV_ENABLE, 0, 0, NULL);
// 				updateEvent(eventSocket, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
// 				close(eventSocket);
// 			}
// 		}
// 	}
// }

/**
 * @brief Accepts a new connection and adds it to the kqueue
 */
int Server::acceptConnection() {
	sockaddr_in socketInfo;
	socklen_t socketInfoSize = sizeof(socketInfo);

	int clientSocket = accept(_listeningSocket, (sockaddr *)&socketInfo, &socketInfoSize);
	if (clientSocket == -1)
		exitWithError("Error accepting new connection");
	addClientSocket(clientSocket);
	logInfo("New client connected: " + std::string(inet_ntoa(socketInfo.sin_addr)) + ":" + std::to_string(ntohs(socketInfo.sin_port)));
	return clientSocket;
}

/**
 * @brief Receives a message from a client socket
 *
 * @param socket	The client socket to receive from
 * @return 			The received message
 */
void Server::receiveRequest(int socket) {
	std::string stash = "";
	char buffer[BUFFER_SIZE];
	long bytesReceived = 0;
	while (true) {
		bytesReceived = recv(socket, buffer, BUFFER_SIZE, 0);
		if (bytesReceived < 0) {
			logError("Failed to read request from client");
			close(socket);
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
 * @brief Sends a response to a client socket
 *
 * @param str		The response to send
 * @param socket	The client socket to send to
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
			close(socket);
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
