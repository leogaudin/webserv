#include "../include/ServerManager.hpp"

int main() {
    Config config;
    Config config2;

    config._serverName = "localhost";
    config._listen = 8080;
    config._errorPage = "error.html";
    config._root = "www";
    config._index = "index.html";
    config._autoindex = "on";

    config2._serverName = "localhost2";
    config2._listen = 8081;
    config2._errorPage = "error.html";
    config2._root = "www";
    config2._index = "index.html";
    config2._autoindex = "on";

    // config.printConfig();
    // config2.printConfig();

    Server server = Server(config);
    Server server2 = Server(config2);

    ServerManager manager = ServerManager();
    manager.addServer(&server);
    manager.addServer(&server2);

    manager.runAllServers();

    return 0;
}
