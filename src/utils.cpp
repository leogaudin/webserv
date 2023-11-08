#include "../include/Server.hpp"


// SERVER

void	log(const std::string &message)
{
	std::cout << message << std::endl;
}

void   logError(const std::string &message)
{
	std::cerr << BOLD_RED << "⚠ " << message << RESET << std::endl;
}

void logInfo(const std::string &message)
{
	std::cout << BOLD_CYAN << "ℹ " << message << RESET << std::endl;
}

void  logSuccess(const std::string &message)
{
	std::cout << BOLD_GREEN << "✔ " << message << RESET << std::endl;
}

void	exitWithError(const std::string &errorMessage)
{
	logError("ERROR: " + errorMessage);
	exit(EXIT_FAILURE);
}


// REQUEST

void printHeaders(std::map< std::string, std::vector<std::string> > headers) {
	std::map< std::string, std::vector<std::string> >::iterator it;
	for (it = headers.begin(); it != headers.end(); it++) {
		std::cout << it->first << " -> ";
		for (int i = 0; i < (int)it->second.size(); i++) {
			std::cout << it->second[i] << " ";
		}
		std::cout << std::endl;
	}
}

std::string toLowercase(std::string str) {
	std::string res = "";
	for (int i = 0; i < (int)str.size(); i++) {
		res += std::tolower(str[i]);
	}
	return res;
}
