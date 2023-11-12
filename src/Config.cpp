
#include "../include/Webserv.hpp"

Config::Config() {
    _serverName = "_";
    _root = "";
    _index = "index.html";
    _autoindex = false;
    _client_max_body_size = 1000000;
}

Config::Config(const Config& other) {
    *this = other;
}

Config& Config::operator=(const Config& other) {
    if (this != &other) {
        _serverName = other._serverName;
        _listen = other._listen;
        _root = other._root;
        _index = other._index;
        _autoindex = other._autoindex;
        _client_max_body_size = other._client_max_body_size;
        _errorPages = other._errorPages;
    }
    return *this;
}

Config::~Config() {}

void Config::printConfig() {
    std::cout << "ServerName: " << _serverName << std::endl;
    std::cout << "Listen: " << _listen << std::endl;
    std::cout << "Root: " << _root << std::endl;
    std::cout << "Index: " << _index << std::endl;
    std::cout << "Autoindex: " << _autoindex << std::endl;
    std::cout << "ClientMaxBodySize: " << _client_max_body_size << std::endl;
    std::cout << "ErrorPages: " << std::endl;
    for (std::map<int, std::string>::iterator it = _errorPages.begin(); it != _errorPages.end(); it++) {
        std::cout << "\t" << it->first << ": " << it->second << std::endl;
    }
}
