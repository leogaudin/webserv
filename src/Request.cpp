#include "../include/Request.hpp"

Request::Request() {}

/**
 * @brief Constructs a Request object from a raw input.
 *
 * @param raw The raw input to be parsed.
 */
Request::Request(std::string raw)
{
	parse(raw);
}

Request::~Request() {}

/**
 * @brief Constructs a Request object from another Request object.
 *
 * @param other The Request object to be copied.
 */
Request::Request(const Request &other) {
	_raw = other._raw;
	_method = other._method;
	_path = other._path;
	_version = other._version;
	_headers = other._headers;
	_body = other._body;
}

/**
 * @brief Parses the raw HTTP request and extracts the method, path, headers and body.
 */
void Request::parse(std::string raw) {
	std::istringstream iss(raw);
	std::string token;
	std::vector<std::string> line;
	std::string delimiters = " ,;:";
	int tokenIndex = -1;

    while (true) {
		char c;
		iss.get(c);
		if (iss.eof()) break;
		if (c == '\n') { // End of line
			if (!token.compare("\r")) { // End of headers
				token.clear();
				break;
			}
			if (!token.empty()) { // Last token before newline
				tokenIndex++;
				token.resize(token.size() - 1);
				if (tokenIndex == 2)
					parseVersion(token);
				else
				{
					line.push_back(token);
					addHeader(line);
				}
				token.clear();
			}
			line.clear();
		}
		else if (delimiters.find(c) == std::string::npos) token += c;
		else {
			if (!token.empty()) { // Normal end of token
				tokenIndex++;
				if (tokenIndex == 0)
					parseMethod(token);
				else if (tokenIndex == 1)
					parsePath(token);
				else
					line.push_back(token);
				token.clear();
			}
		}
	}
	std::getline(iss, _body, '\0');
}


/**
 * Adds a header to the request.
 * @param line A vector of strings containing the header key and its values.
 */
void Request::addHeader(std::vector<std::string> line) {
	std::string key = toLowercase(line[0]);
	std::vector<std::string> values;
	for (int i = 1; i < (int)line.size(); i++) {
		values.push_back(line[i]);
	}
	_headers[key] = values;
}

/**
 * @brief Parses the HTTP method from a given token and sets it to the Request object.
 */
void Request::parseMethod(std::string token) {
	if (!token.compare("GET"))
		_method = GET;
	else if (!token.compare("POST"))
		_method = POST;
	else if (!token.compare("DELETE"))
		_method = DELETE;
	else
		_method = UNKNOWN;
	// std::cout << "METHOD: " << _method << std::endl;
}

/**
 * @brief Parses the HTTP path from a given token and sets it to the Request object.
 */
void Request::parsePath(std::string token) {
	_path = token;
	// std::cout << "PATH: " << _path << std::endl;
}

/**
 * @brief Parses the HTTP version from a given token and sets it to the Request object.
 */
void Request::parseVersion(std::string token) {
	_version = token;
	// std::cout << "VERSION: " << _version << std::endl;
}

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