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
	std::string _locationName;
	std::string _version;
	int _status;
	bool _cgiBuilt;
	std::string _cgiResponse;
	std::map< std::string, std::vector<std::string> > _headers;
	std::string _body;

	void resolveMethod();
	void handleGet(std::string requestedPath);
	void handlePost(std::string requestedPath);
	void handleDelete(std::string requestedPath);
	void handleErrorStatus(int status);
	void addHeader(std::string key, std::string value);
	std::string outputHeaders();
	void uploadFromRequest(std::string path);
	Config getLocationConfig(std::string path, Config &config);
	std::string getLocationName(std::string path, Config &config);
	std::string autoindex(std::string path);
	void redirect(std::string path);
	bool checkRedirect();
	std::string resolveStatus(int status);
	std::string resolveMimeType(std::string path);
};

std::string rootPath(Config config, std::string requestedPath);

#endif
