#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Webserv.hpp"

class Request
{
  public:
	Request();
	Request(std::string raw, Config config);
	~Request();
	Request(const Request &other);
	void parse(std::string raw);
	std::string getRaw() const { return _raw; }
	Method getMethod() const { return _method; }
	std::string getPath() const { return _path; }
	std::string getVersion() const { return _version; }
	std::map< std::string, std::vector<std::string> > getHeaders() const { return _headers; }
	std::vector<unsigned char> getBody() const { return _body; }
	std::string getQueryString() const;

  private:
	Config _config;
	std::string _raw;
	Method _method;
	std::string _path;
	std::string _version;
	std::map< std::string, std::vector<std::string> > _headers;
	std::vector<unsigned char> _body;
	void addHeader(std::vector<std::string> line);
	void parseMethod(std::string token);
	void parsePath(std::string token);
	void parseVersion(std::string token);
};

#endif
