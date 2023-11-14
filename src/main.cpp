#include "../include/Webserv.hpp"

int main() {
    Config config;
    Config config2;

    config._serverName = "localhost";
    config._listen = 8082;
    // config._host = LOOPBACK;
    config._root = "docs";
    config._client_max_body_size = INT32_MAX;
    config._errorPages[404] = "error/404.html";
    config._locations["/subsite"] = config.toLocationConfig();
    config._locations["/subsite"]._allowedMethods.push_back(GET);
    config._locations["/subsite"]._allowedMethods.push_back(POST);
    config._locations["/subsite"]._redirect = "contact.html";

    config.printConfig();

    // config2.printConfig();

    Server server = Server(config);
    // Server server2 = Server(config2);

    ServerManager manager = ServerManager();
    manager.addServer(&server);
    // manager.addServer(&server2);

    manager.runAllServers();

    return 0;
}
