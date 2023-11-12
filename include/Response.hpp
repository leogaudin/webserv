#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Webserv.hpp"
class Response
{
  public:
	Response(Request request, Config config);
	~Response();
	std::string build();

  private:
	Request _request;
	Config _config;
	std::string _version;
	int _status;
	std::map< std::string, std::vector<std::string> > _headers;
	std::string _body;
	void addHeader(std::string key, std::string value);
	std::string outputHeaders();
	std::string resolveStatus(int status);
	std::string resolveMimeType(std::string path);
	void resolveMethod();
	void handleErrorStatus(int status);
	void handleGet(std::string requestedPath);
	void handlePost(std::string requestedPath);
	void uploadFromRequest(std::string path);
	void handleDelete(std::string requestedPath);
};

#endif
