
#include "../include/ServerManager.hpp"

ServerManager::ServerManager() {}

ServerManager::~ServerManager() {}

void ServerManager::addServer(Server *server) {
    _servers.push_back(server);
}

Server* ServerManager::findServerBySocket(int socket) {
	for (std::vector<Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
		if (socket == (*it)->getListeningSocket() || (*it)->isClientSocket(socket)) {
			return (*it);
		}
	}
	return nullptr;
}

/**
 * @brief The main event loop. Waits for new events and handles them
 * in a non-blocking way.
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
				fcntl(clientSocket, F_SETFL, O_NONBLOCK);
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

void updateEvent(int kqueue, int ident, short filter, u_short flags, u_int fflags, int data, void *udata) {
	struct kevent kev;
	EV_SET(&kev, ident, filter, flags, fflags, data, udata);
	if (kevent(kqueue, &kev, 1, NULL, 0, NULL) == -1)
		exitWithError("Failed to update kevent");
}
