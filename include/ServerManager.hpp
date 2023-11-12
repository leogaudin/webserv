#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include "Webserv.hpp"
class ServerManager
{
  public:
	ServerManager();
	~ServerManager();

	void addServer(Server *server);
	void runAllServers();

  private:
	std::vector<Server *> _servers;
	Server *findServerBySocket(int socket);
};

void	updateEvent(int kqueue, int ident, short filter, u_short flags,
			u_int fflags, int data, void *udata);

#endif
