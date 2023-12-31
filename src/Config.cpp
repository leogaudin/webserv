
#include "../include/Webserv.hpp"

Config::Config() {
    _parsed = false;
    _serverName = "";
    _host = "0.0.0.0";
    _listen = 8080;
    _root = "";
    _index = "index.html";
    _client_max_body_size = 1000000;
    _cgiPaths = std::vector<std::string>();
    _cgiExtensions = std::vector<std::string>();
    _locations = std::map<std::string, Config>();
    _autoindex = false;
    _allowedMethods = std::vector<Method>();
    _redirect = "";
    _alias = "";
}

Config::Config(const Config& other) {
    *this = other;
}

Config& Config::operator=(const Config& other) {
    if (this != &other) {
        _host = other._host;
        _listen = other._listen;
        _serverName = other._serverName;
        _root = other._root;
        _index = other._index;
        _client_max_body_size = other._client_max_body_size;
        _errorPages = other._errorPages;
        _locations = other._locations;
        _autoindex = other._autoindex;
        _allowedMethods = other._allowedMethods;
        _redirect = other._redirect;
        _cgiExtensions = other._cgiExtensions;
        _cgiPaths = other._cgiPaths;
        _alias = other._alias;
        _parsed = other._parsed;
    }
    return *this;
}

Config::~Config() {}

/**
 * @brief   Returns a config object without any location-specific information
 * to avoid infinite recursion.
 */
Config Config::toLocationConfig() {
    Config locationConfig = *(this);
    locationConfig._locations.clear();
    return locationConfig;
}

/**
 * @brief   Prints the config object to stdout.
 */
void Config::printConfig(int indent, bool printLocs) {
    std::string indentStr = "";
    for (int i = 0; i < indent; i++) {
        indentStr += "\t";
    }
    std::cout << indentStr << "Parsed: " <<  _parsed << std::endl;
    std::cout << indentStr << "Listen: " <<  _listen << std::endl;
    std::cout << indentStr << "Host: " <<  _host << std::endl;
    std::cout << indentStr << "ServerName: " <<  _serverName << std::endl;
    std::cout << indentStr << "Root: " <<  _root << std::endl;
    std::cout << indentStr << "Index: " <<  _index << std::endl;
    std::cout << indentStr << "Autoindex: " <<  _autoindex << std::endl;
    std::cout << indentStr << "ClientMaxBodySize: " <<  _client_max_body_size << std::endl;
    std::cout << indentStr << "ErrorPages: " << std::endl;
    for (std::map<int, std::string>::iterator it =  _errorPages.begin(); it !=  _errorPages.end(); it++) {
        std::cout << indentStr << "\t" << it->first << ": " << it->second << std::endl;
    }
    std::cout << indentStr << "Locations: " << std::endl;
    if (printLocs)
        for (std::map<std::string, Config>::iterator it =  _locations.begin(); it !=  _locations.end(); it++) {
            std::cout << indentStr << "\t" << it->first << ": " << std::endl;
            it->second.printConfig(indent + 1, false);
        }
    std::cout << indentStr << "AllowedMethods: " << std::endl;
    for (std::vector<Method>::iterator it =  _allowedMethods.begin(); it !=  _allowedMethods.end(); it++) {
        std::cout << indentStr << "\t" << *it << std::endl;
    }
    std::cout << indentStr << "Return: " <<  _redirect << std::endl;
    std::cout << indentStr << "CGIExtensions: " << std::endl;
    for (std::vector<std::string>::iterator it =  _cgiExtensions.begin(); it !=  _cgiExtensions.end(); it++) {
        std::cout << indentStr << "\t" << *it << std::endl;
    }
    std::cout << indentStr << "CGIPaths: " << std::endl;
    for (std::vector<std::string>::iterator it =  _cgiPaths.begin(); it !=  _cgiPaths.end(); it++) {
        std::cout << indentStr << "\t" << *it << std::endl;
    }
    std::cout << indentStr << "Alias: " <<  _alias << std::endl;
}
