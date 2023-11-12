#include "../include/Webserv.hpp"

int main() {
    Config config;
    Config config2;

    config._serverName = "localhost";
    config._listen = 8080;
    config._root = "docs";
    config._client_max_body_size = 10;
    config._errorPages[404] = "error/404.html";

    config2._serverName = "localhost2";
    config2._listen = 8081;
    config2._root = "docs";

    Server server = Server(config);
    Server server2 = Server(config2);

    ServerManager manager = ServerManager();
    manager.addServer(&server);
    manager.addServer(&server2);

    manager.runAllServers();

    return 0;
}
