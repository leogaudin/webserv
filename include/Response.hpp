#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Request.hpp"
# include <iostream>
# include <map>
# include <sstream>
# include <string>

# define CRLF "\r\n"

class Response
{
  public:
	Response(Request request);
	~Response();
	std::string build();

  private:
	std::string _version;
	int _status;
	std::map< std::string, std::vector<std::string> > _headers;
	std::string _body;
	std::string resolveStatus(int status);
};

#endif
