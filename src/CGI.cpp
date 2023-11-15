
#include "../include/Webserv.hpp"

CGI::CGI(Config config, Request request) {
	_config = config;
	_request = request;
	initEnv();
}

CGI::CGI(const CGI &other) {
    *this = other;
}

CGI::~CGI() {}

CGI &CGI::operator=(const CGI &other) {
    if (this != &other) {
		this->_config = other._config;
		this->_request = other._request;
        this->_env = other._env;
		this->_args = other._args;
    }
    return (*this);
}

/**
 * @brief	Initializes the environment variables for the CGI script
 */
void CGI::initEnv() {
	_env = std::map<std::string, std::string>();
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

/**
 * @brief	Prints the environment variables
 */
void CGI::printEnv() {
	std::cout << "Printing env:" << std::endl;
	for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); it++) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
}

/**
 * @brief	Converts the environment variables to an array of strings
 */
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

/**
 * @brief	Returns the index of a string in a vector of strings
 */
int getIndexOfString(std::vector<std::string> v, std::string s) {
    int pos = std::find(v.begin(), v.end(), s) - v.begin();
	if (pos == (int)v.size())
		return (-1);
	return (pos);
}

/**
 * @brief	Returns the string after the last dot in a path
 */
std::string getExtension(std::string path) {
	size_t pos = path.find_last_of(".");
	if (pos == std::string::npos)
		return ("");
	return (path.substr(pos + 1));
}

/**
 * @brief   Executes a CGI script and returns its output.
 *
 * This function executes a CGI script specified in the request path.
 * It constructs the full path of the script, converts the environment variables from a map to an array, and forks a new process to execute the script.
 * The output of the script is read from a pipe and returned as a string.
 * The child process is also set to terminate after 5 seconds to prevent scripts from running indefinitely.
 */
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
		delete envArray;
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
			return (delete envArray, "HTTP/1.1 500 Internal Server Error\r\n\r\nError: Script execution terminated unexpectedly.");
	}
}
