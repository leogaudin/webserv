
#include "../include/Webserv.hpp"

CGI::CGI(Config config, Request request) {
	_config = config;
	_request = request;
	_env = std::map<std::string, std::string>();
	initEnv();
}

CGI::CGI(const CGI &other) {
    *this = other;
}

CGI::~CGI() {}

CGI &CGI::operator=(const CGI &other) {
    if (this != &other) {
        this->_env = other._env;
    }
    return (*this);
}

void CGI::initEnv() {
	std::map<std::string, std::vector<std::string> > headers = _request.getHeaders();

	_env["AUTH_TYPE"] = "BASIC";
	_env["CONTENT_LENGTH"] = std::to_string(_request.getBody().size());
	std::map<std::string, std::vector<std::string> >::iterator it = headers.find("content-type");
	if (it != headers.end())
		_env["CONTENT_TYPE"] = headers["content-type"][0];
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	// _env["PATH_INFO"] = rootPath(_config, _request.getPath());
	// _env["PATH_TRANSLATED"] = strcat(getcwd(0, 0), rootPath(_config, _request.getPath()).c_str());
	_env["QUERY_STRING"] = _request.getQueryString();
	_env["REMOTE_ADDR"] = headers["host"][0];
	_env["REQUEST_METHOD"] = _request.getMethod();
	// _env["REQUEST_URI"] = _request.getPath();
	_env["SCRIPT_NAME"] = rootPath(_config, _request.getPath());
	_env["SERVER_NAME"] = _config._serverName;
	_env["SERVER_PORT"] = std::to_string(_config._listen);
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["SERVER_SOFTWARE"] = "webserv/1.0";
}

void CGI::printEnv() {
	std::cout << "Printing env:" << std::endl;
	for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); it++) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
}

char **envToArray(std::map<std::string, std::string> env) {
	char **envArray = new char*[env.size() + 1];
	int i = 0;
	for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); it++) {
		std::string envVar = it->first + "=" + it->second;
		envArray[i] = new char[envVar.size() + 1];
		strcpy(envArray[i], envVar.c_str());
		i++;
	}
	envArray[i] = NULL;
	return (envArray);
}

int getIndexOfString(std::vector<std::string> v, std::string s) {
    int pos = std::find(v.begin(), v.end(), s) - v.begin();
	if (pos == (int)v.size())
		return (-1);
	return (pos);
}

std::string getExtension(std::string path) {
	size_t pos = path.find_last_of(".");
	if (pos == std::string::npos)
		return ("");
	return (path.substr(pos + 1));
}

std::string CGI::execute() {
	std::string path = rootPath(_config, _request.getPath());
	std::string pathExtension = getExtension(path);
	int cgiIndex = getIndexOfString(_config._cgiExtensions, pathExtension);
	if (cgiIndex == -1)
		return ("HTTP/1.1 500 Internal Server Error\r\n\r\nNo CGI extension found for path " + path);

	std::string cgiPath = _config._cgiPaths[cgiIndex];
	std::string scriptPath = strcat(getcwd(0, 0), path.c_str());

	char **envArray = envToArray(_env);
	char *args[] = {(char *)cgiPath.c_str(), (char *)scriptPath.c_str(), NULL};
	int fd[2];
	pipe(fd);
	pid_t pid = fork();
	if (pid == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		dup2(fd[1], 2);
		close(fd[1]);
		alarm(5);
		execve(args[0], args, envArray);
		exit(0);
	}
	else {
		close(fd[1]);
		std::string result;
		char buffer[1024];
		int bytesRead;

		while ((bytesRead = read(fd[0], buffer, sizeof(buffer))) > 0) {
			result.append(buffer, bytesRead);
		}
		int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            return (delete envArray, result);
        else
			return ("HTTP/1.1 500 Internal Server Error\r\n\r\nError: Script execution terminated unexpectedly.");
	}
}
