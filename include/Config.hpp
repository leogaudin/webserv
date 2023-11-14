
#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Webserv.hpp"

class Config
{
  public:
	Config();
	Config(const Config &other);
	Config &operator=(const Config &other);
	~Config();
	void printConfig(int indent = 0);
	Config toLocationConfig();
	// void parseConfig(std::string filename);

	// General
	int _listen;
	std::string _host;
	std::string _serverName;
	std::string _root;
	std::string _index;
	int _client_max_body_size;
	std::map<int, std::string> _errorPages;
	std::map< std::string, Config > _locations;
	std::vector<std::string> _cgiExtensions;
	std::vector<std::string> _cgiPaths;

	// Location-specific
	bool _autoindex;
	std::vector<Method> _allowedMethods;
	std::string _redirect;
	std::string _alias;
};

#endif
