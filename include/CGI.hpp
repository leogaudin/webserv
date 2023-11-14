#ifndef CGI_HPP
# define CGI_HPP

# include "Webserv.hpp"

class CGI
{
	public:
		CGI(Config config, Request request);
		CGI(const CGI &other);
		CGI &operator=(const CGI &other);
		~CGI();

		void setEnv(std::map<std::string, std::string> env) { _env = env; }
		void setArgs(const std::string &args) { _args = args; }
		void setBody(const std::string &body) { _body = body; }
		std::map<std::string, std::string> getEnv() const { return (_env); }
		std::string getArgs() const { return (_args); }
		std::string getBody() const { return (_body); }
		std::string execute();

	private:
		Config _config;
		Request _request;
		std::map<std::string, std::string> _env;
		std::string _args;
		std::string _body;

		void initEnv();
		void printEnv();
};

int getIndexOfString(std::vector<std::string> v, std::string s);
std::string getExtension(std::string path);

#endif
