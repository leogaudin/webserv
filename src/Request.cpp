#include "../include/Webserv.hpp"

Request::Request() {}

/**
 * @brief	Constructs a Request object from a raw input.
 *
 * @param raw The raw input to be parsed.
 */
Request::Request(std::string raw, Config config)
{
	_config = config;
	parse(raw);
}

Request::~Request() {}

/**
 * @brief	Constructs a Request object from another Request object.
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
 * @brief	Parses a raw HTTP request string.
 *
 * This function takes a raw HTTP request string and parses it into its constituent parts, including the HTTP method, path, version, headers, and body.
 * The parsed information is stored in the Request object.
 * It reads the request character by character, building up tokens as it goes.
 * When it encounters a delimiter (a space, comma, semicolon, or colon), it finishes the current token and starts a new one.
 * When it encounters a newline character, it finishes the current line and starts a new one.
 * If the newline character is immediately followed by a carriage return character, it signifies the end of the headers and the start of the body.
 * The body is stored as a vector of unsigned chars.
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
	_body = std::vector<unsigned char>(raw.begin() + iss.tellg(), raw.end());
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
 * @brief	Parses the HTTP method from a given token and sets it to the Request object.
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
 * @brief	Parses the HTTP path from a given token and sets it to the Request object.
 */
void Request::parsePath(std::string token) {
	_path = token;
	// std::cout << "PATH: " << _path << std::endl;
}

/**
 * @brief	Parses the HTTP version from a given token and sets it to the Request object.
 */
void Request::parseVersion(std::string token) {
	_version = token;
	// std::cout << "VERSION: " << _version << std::endl;
}

/**
 * @brief	Returns the query string (everything after the ?) from the path.
 */
std::string Request::getQueryString() const {
	std::string queryString = "";
	std::string path = _path;
	size_t pos = path.find("?");
	if (pos != std::string::npos) {
		queryString = path.substr(pos + 1);
	}
	return queryString;
}

/**
 * @brief	Prints the headers to stdout.
 */
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

/**
 * @brief	Converts a string to lowercase.
 */
std::string toLowercase(std::string str) {
	std::string res = "";
	for (int i = 0; i < (int)str.size(); i++) {
		res += std::tolower(str[i]);
	}
	return res;
}
