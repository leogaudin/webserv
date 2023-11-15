#include "../include/Webserv.hpp"

int main() {
    Config config;
    Config config2;

    config._serverName = "nomdelion";
    config._listen = 8080;
    config._host = LOOPBACK;
    config._root = "docs";
    config._autoindex = true;
    config._errorPages[400] = "error/400.html";
    config._errorPages[404] = "error/404.html";
    config._errorPages[405] = "error/405.html";
    config._errorPages[409] = "error/409.html";
    config._errorPages[413] = "error/413.html";
    config._errorPages[500] = "error/500.html";
    config._cgiPaths.push_back("/opt/homebrew/bin/python3");
    config._cgiExtensions.push_back("py");
    config._cgiPaths.push_back("/bin/bash");
    config._cgiExtensions.push_back("sh");
    config._locations["/subsite"] = config.toLocationConfig();
    config._locations["/subsite"]._redirect = "/efewffe";
    config._locations["/subsite"]._allowedMethods.push_back(GET);
    config._locations["/subsite"]._allowedMethods.push_back(POST);

    config2 = config;
    config2._serverName = "nomdefou";
    config2._listen = 8081;

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
