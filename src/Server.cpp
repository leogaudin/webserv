#include "../include/Server.hpp"

Server::Server(std::string ip_address, int port): _ipAddress(ip_address), _port(port), _listeningSocket()
{
	initAndListen();
}

Server::~Server()
{
	close(_listeningSocket);
	exit(EXIT_SUCCESS);
}

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

void Server::updateEvent(int ident, short filter, u_short flags, u_int fflags, int data, void *udata) {
	struct kevent kev;
	EV_SET(&kev, ident, filter, flags, fflags, data, udata);
	if (kevent(_kq, &kev, 1, NULL, 0, NULL) == -1)
		exitWithError("Failed to update kevent");
}

void Server::runLoop()
{
    struct kevent evList[MAX_EVENTS];
	_kq = kqueue();

    updateEvent(_listeningSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);

	while (true) {
		logInfo("Waiting for new events...");
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
				std::string request = receiveMessage(eventSocket);
				updateEvent(evList[i].ident, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
				updateEvent(evList[i].ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
			}
			else if (evList[i].filter == EVFILT_WRITE) {
				std::string response = buildResponse("Hello World!");
				sendResponse(response, eventSocket);
				updateEvent(evList[i].ident, EVFILT_READ, EV_ENABLE, 0, 0, NULL);
				updateEvent(evList[i].ident, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
				close(eventSocket);
			}
		}
	}
}

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

std::string Server::receiveMessage(int socket) {
	char buffer[BUFFER_SIZE];
	int bytesReceived = read(socket, buffer, BUFFER_SIZE);
	if (bytesReceived < 0)
		exitWithError("Failed to read bytes from client socket connection");
	buffer[bytesReceived] = '\0';
	return (std::string(buffer));
}

std::string Server::buildResponse(std::string str)
{
	std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p>" + str + "</p></body></html>";
	std::ostringstream ss;
	ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
		<< htmlFile;
	return (ss.str());
}

void Server::sendResponse(std::string str, int socket)
{
	long	bytesSent = write(socket, str.c_str(), str.size());
	if (bytesSent == (long)str.size())
		logSuccess("Response sent to client\n");
	else
		logError("Error sending response to client");
}

// void setAsNonBlocking(int socket)
// {
// 	int existingFlags = fcntl(socket, F_GETFL, 0);
// 	if (existingFlags == -1)
// 	{
// 		exitWithError("Failed to set socket as non-blocking");
// 	}
// 	int s = fcntl(socket, F_SETFL, existingFlags | O_NONBLOCK);
// 	if (s == -1)
// 	{
// 		exitWithError("Failed to set socket as non-blocking");
// 	}
// }
