
#include "../include/Webserv.hpp"

ServerManager::ServerManager() {}

ServerManager::~ServerManager() {}

/**
 * @brief	Adds a server to the end of the _servers vector.
 */
void ServerManager::addServer(Server *server) {
    _servers.push_back(server);
}

/**
 * @brief	Finds a server by its socket.
 *
 * This function finds a server by its socket.
 * It iterates over the _servers vector and for every server, checks if the socket matches the listening socket or if it is one of the client sockets.
 *
 * If a match is found, it returns the server.
 */
Server* ServerManager::findServerBySocket(int socket) {
	for (std::vector<Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
		if (socket == (*it)->getListeningSocket() || (*it)->isClientSocket(socket)) {
			return (*it);
		}
	}
	return nullptr;
}

/**
 * @brief	Runs all servers managed by the ServerManager.
 *
 * This function creates a new kqueue and adds the listening sockets of all servers to it.
 * It then enters an infinite loop where it waits for events on the kqueue.
 * When an event occurs, it identifies the server associated with the event socket and handles the event accordingly.
 *
 * If the event is a disconnect event (EV_EOF), it logs the disconnection, removes the client socket from the kqueue.
 * If the event socket is a listening socket, it accepts the new connection, adds the client socket to the kqueue.
 * If the event is a read event, it receives the request, disables further read events, enables write events.
 * If the event is a write event, it sends the response, enables further read events, disables write events, closes the connection.
 */
void ServerManager::runAllServers() {
    struct kevent evList[MAX_EVENTS];
	int kq = kqueue();
	if (kq == -1)
		exitWithError("Failed to create kqueue");

	for (std::vector<Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
		updateEvent(kq, (*it)->getListeningSocket(), EVFILT_READ, EV_ADD, 0, 0, NULL);
	}

	while (true) {
		int newEvents = kevent(kq, NULL, 0, evList, MAX_EVENTS, NULL);
		if (newEvents <= 0)
			continue;
		for (int i = 0; i < newEvents; i++) {
			int eventSocket = evList[i].ident;
			Server* server = findServerBySocket(eventSocket);

			if (server == nullptr)
				exitWithError("Server not found");
			if (evList[i].flags & EV_EOF) {
				server->logInfo("Client has disconnected");
				updateEvent(kq, eventSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
				updateEvent(kq, eventSocket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
			}
			else if (eventSocket == server->getListeningSocket()) {
				int clientSocket = server->acceptConnection();
				// fcntl(clientSocket, F_SETFL, O_NONBLOCK);
				updateEvent(kq, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
				updateEvent(kq, clientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
			}
			else if (evList[i].filter == EVFILT_READ) {
				server->receiveRequest(eventSocket);
				updateEvent(kq, eventSocket, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
				updateEvent(kq, eventSocket, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
			}
			else if (evList[i].filter == EVFILT_WRITE) {
				Response response = Response(server->_request, server->_config);
				server->sendResponse(response.build(), eventSocket);
				updateEvent(kq, eventSocket, EVFILT_READ, EV_ENABLE, 0, 0, NULL);
				updateEvent(kq, eventSocket, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
				server->closeConnection(eventSocket);
			}
		}
	}
}

/**
 * @brief	Updates an event in the kqueue.
 */
void updateEvent(int kqueue, int ident, short filter, u_short flags, u_int fflags, int data, void *udata) {
	struct kevent kev;
	EV_SET(&kev, ident, filter, flags, fflags, data, udata);
	if (kevent(kqueue, &kev, 1, NULL, 0, NULL) == -1)
		exitWithError("Failed to update kevent");
}
