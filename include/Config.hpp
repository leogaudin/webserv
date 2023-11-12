
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
	void printConfig();

	int _listen;
	std::string _serverName;
	std::string _root;
	std::string _index;
	bool _autoindex;
	int _client_max_body_size;
	std::map<int, std::string> _errorPages;

	// std::vector<std::string> _locations;
};

#endif
