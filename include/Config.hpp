
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <vector>
#include <string>
#include <iostream>

class Config {
public:
    Config();
    Config(const Config& other);
    Config& operator=(const Config& other);
    ~Config();
    void printConfig();

    int	_listen;
    std::string _serverName;
    std::string _errorPage;
    std::string _root;
    std::string _index;
    bool _autoindex;

    // std::vector<std::string> _locations;
};

#endif
