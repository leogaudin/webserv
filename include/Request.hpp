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
	void addHeader(std::vector<std::string> line);
	void parseMethod(std::string token);
	void parsePath(std::string token);
	void parseVersion(std::string token);

  public:
	Request();
	Request(std::string raw);
	~Request();
	Request(const Request &other);
	void parse(std::string raw);
	std::string getRaw() const { return _raw; };
	Method getMethod() const { return _method; };
	std::string getPath() const { return _path; };
	std::string getVersion() const { return _version; };
	std::map< std::string, std::vector<std::string> > getHeaders() const { return _headers; };
	std::string getBody() const { return _body; };
};

void	printHeaders(std::map< std::string, std::vector<std::string> > headers);
std::string toLowercase(std::string str);

#endif
