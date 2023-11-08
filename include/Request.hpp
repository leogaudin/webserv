#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <map>
# include <sstream>
# include <string>
# include <vector>

enum	Method
{
	GET,
	POST,
	DELETE,
	UNKNOWN
};

class Request
{
  private:
	std::string _raw;
	Method _method;
	std::string _path;
	std::string _version;
	std::map< std::string, std::vector<std::string> > _headers;
	std::string _body;

  public:
	Request(std::string raw);
	~Request();
	void parse(std::string raw);
	void addHeader(std::vector<std::string> line);
	void parseMethod(std::string token);
	void parsePath(std::string token);
	void parseVersion(std::string token);
};

void	printHeaders(std::map< std::string, std::vector<std::string> > headers);
std::string toLowercase(std::string str);

#endif
