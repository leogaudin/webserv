#include "../include/Server.hpp"

Server::Server(std::string ip_address, int port)
	: _ipAddress(ip_address), _port(port), _listeningSocket(),
		_socketInfo(), _socketInfoSize(sizeof(_socketInfo))
{
	initAndListen();
}

Server::~Server()
{
	close(_listeningSocket);
	exit(EXIT_SUCCESS);
}

void Server::initAndListen() {
	memset(&_socketInfo, 0, sizeof(_socketInfo));
	_socketInfo.sin_family = AF_INET;
	_socketInfo.sin_port = htons(_port);
	_socketInfo.sin_addr.s_addr = inet_addr(_ipAddress.c_str());

	_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listeningSocket < 0)
		exitWithError("Creating socket");
	if (bind(_listeningSocket, (sockaddr *)&_socketInfo, _socketInfoSize) < 0 && errno == EADDRINUSE)
		exitWithError("Address already in use");
	if (listen(_listeningSocket, MAX_CONNECTIONS) < 0)
		exitWithError("Socket listening");
	logSuccess("Listening at http://" + std::string(inet_ntoa(_socketInfo.sin_addr)) + ":" + std::to_string(ntohs(_socketInfo.sin_port)));
}

void Server::runLoop()
{
    struct kevent evSet;
    struct kevent evList[MAX_EVENTS];
	int kq = kqueue();

    EV_SET(&evSet, _listeningSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq, &evSet, 1, NULL, 0, NULL);

	while (true) {
		logInfo("Waiting for new events...");
		int newEvents = kevent(kq, NULL, 0, evList, MAX_EVENTS, NULL);
		if (newEvents == -1)
			exitWithError("Error checking for new events");
		for (int i = 0; i < newEvents; i++) {
			int eventSocket = evList[i].ident;
			if (eventSocket == _listeningSocket) {
				int fd = accept(_listeningSocket, (sockaddr *)&_socketInfo, &_socketInfoSize);
				if (fd == -1)
					exitWithError("Error accepting new connection");
				else {
					EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
					if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
						exitWithError("Failed to register kevent");
				}
			}
			else if (evList[i].flags & EV_EOF) {
				logInfo("Client has disconnected");
				EV_SET(&evSet, eventSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                kevent(kq, &evSet, 1, NULL, 0, NULL);
			}
			else if (evList[i].filter == EVFILT_READ) {
				std::string request = receiveMessage(eventSocket);
				logSuccess("Received the following request from client:\n" + request);
				sendResponse(buildResponse(request), eventSocket);
				close(eventSocket);
			}
		}
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
