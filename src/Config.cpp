
#include "../include/Config.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

Config::Config() {}

Config::Config(const Config& other) {
    *this = other;
}

Config& Config::operator=(const Config& other) {
    if (this != &other) {
        _serverName = other._serverName;
        _listen = other._listen;
        _errorPage = other._errorPage;
        _root = other._root;
        _index = other._index;
        _autoindex = other._autoindex;
    }
    return *this;
}

Config::~Config() {}

void Config::printConfig() {
    std::cout << "ServerName: " << _serverName << std::endl;
    std::cout << "Listen: " << _listen << std::endl;
    std::cout << "ErrorPage: " << _errorPage << std::endl;
    std::cout << "Root: " << _root << std::endl;
    std::cout << "Index: " << _index << std::endl;
    std::cout << "Autoindex: " << _autoindex << std::endl;
}
