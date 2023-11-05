#include "../include/Server.hpp"

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
	exit(1);
}
