#include "../include/Webserv.hpp"

int main() {
    ServerManager manager = ServerManager();
    Config* parsedConfs = parseConfig("conf.conf");
    int i = 0;
    while (i < MAX_SERVERS) {
        if (parsedConfs[i]._parsed) {
            Server *current = new Server(parsedConfs[i]);
            manager.addServer(current);
        }
        i++;
    }
    manager.runAllServers();
    return (EXIT_SUCCESS);
}
