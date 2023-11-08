#include "../include/Server.hpp"

/**
 * @brief Constructor for the Server class
 *
 * @param ip_address	The IP address to listen on
 * @param port			The port to listen on
 */
Server::Server(std::string ip_address, int port): _ipAddress(ip_address), _port(port), _listeningSocket(), _request() {
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
 * @brief Checks the validity of the IP address and port
 */
void Server::checkInputs() {
	if (_ipAddress.empty())
		exitWithError("IP address cannot be empty");
	if (_port < 1024)
		exitWithError("Port numbers 0 - 1023 are used for well-known ports");
	if (_port > 49151)
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
	socketInfo.sin_port = htons(_port);
	socketInfo.sin_addr.s_addr = inet_addr(_ipAddress.c_str());

	_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listeningSocket < 0)
		exitWithError("Creating socket");
	if (bind(_listeningSocket, (sockaddr *)&socketInfo, socketInfoSize) < 0 && errno == EADDRINUSE)
		exitWithError("Address already in use");
	if (listen(_listeningSocket, MAX_CONNECTIONS) < 0)
		exitWithError("Socket listening");
	logSuccess("Listening at http://" + std::string(inet_ntoa(socketInfo.sin_addr)) + ":" + std::to_string(ntohs(socketInfo.sin_port)));
}

/**
 * @brief Utility function to update a kevent
 */
void Server::updateEvent(int ident, short filter, u_short flags, u_int fflags, int data, void *udata) {
	struct kevent kev;
	EV_SET(&kev, ident, filter, flags, fflags, data, udata);
	if (kevent(_kq, &kev, 1, NULL, 0, NULL) == -1)
		exitWithError("Failed to update kevent");
}

/**
 * @brief The main event loop. Waits for new events and handles them
 * in a non-blocking way.
 */
void Server::runLoop() {
    struct kevent evList[MAX_EVENTS];
	_kq = kqueue();

    updateEvent(_listeningSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);

	while (true) {
		// logInfo("Waiting for new events...");
		int newEvents = kevent(_kq, NULL, 0, evList, MAX_EVENTS, NULL);
		if (newEvents <= 0)
			continue;
		for (int i = 0; i < newEvents; i++) {
			int eventSocket = evList[i].ident;
			if (eventSocket == _listeningSocket) {
				acceptConnection();
			}
			else if (evList[i].flags & EV_EOF) {
				logInfo("Client has disconnected");
				updateEvent(eventSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
				updateEvent(eventSocket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
			}
			else if (evList[i].filter == EVFILT_READ) {
				receiveRequest(eventSocket);
				updateEvent(evList[i].ident, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
				updateEvent(evList[i].ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
			}
			else if (evList[i].filter == EVFILT_WRITE) {
				Response response = Response(_request);
				sendResponse(response.build(), eventSocket);
				updateEvent(evList[i].ident, EVFILT_READ, EV_ENABLE, 0, 0, NULL);
				updateEvent(evList[i].ident, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
				close(eventSocket);
			}
		}
	}
}

/**
 * @brief Accepts a new connection and adds it to the kqueue
 */
void Server::acceptConnection() {
	sockaddr_in socketInfo;
	socklen_t socketInfoSize = sizeof(socketInfo);

	int clientSocket = accept(_listeningSocket, (sockaddr *)&socketInfo, &socketInfoSize);
	if (clientSocket == -1)
		exitWithError("Error accepting new connection");
	else {
		logInfo("New client connected: " + std::string(inet_ntoa(socketInfo.sin_addr)) + ":" + std::to_string(ntohs(socketInfo.sin_port)));
		fcntl(clientSocket, F_SETFL, O_NONBLOCK);
		updateEvent(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		updateEvent(clientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
	}
}

/**
 * @brief Receives a message from a client socket
 *
 * @param socket	The client socket to receive from
 * @return 			The received message
 */
void Server::receiveRequest(int socket) {
	std::string stash;
	char buffer[BUFFER_SIZE];
	int bytesReceived = 0;
	while (true) {
		bytesReceived = read(socket, buffer, BUFFER_SIZE);
		if (bytesReceived < 0) {
			logError("Failed to read request from client");
			close(socket);
		}
		stash.append(buffer, bytesReceived);
		if (bytesReceived < BUFFER_SIZE) {
			break;
		}
	}
	_request = Request(stash);
}

/**
 * @brief Sends a response to a client socket
 *
 * @param str		The response to send
 * @param socket	The client socket to send to
 */
void Server::sendResponse(std::string str, int socket) {
	long	bytesSent = write(socket, str.c_str(), str.size());
	if (bytesSent == (long)str.size())
		logSuccess("Response sent to client\n");
	else
		logError("Error sending response to client");
}
