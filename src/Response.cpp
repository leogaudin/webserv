#include "../include/Response.hpp"

/**
 * @brief Constructs a Response object from a Request object.
 *
 * @param request The Request object to be used to construct the Response.
 */
Response::Response(Request request, Config config) {
	_config = config;
	_request = request;
	_version = request.getVersion();
	_status = 500;
	_body = "";
	resolveMethod();
}

Response::~Response() {}

/**
 * @brief Builds the HTTP response.
 *
 * @return The HTTP response.
 */
std::string Response::build() {
	std::ostringstream ss;
	ss
		<< _version << " " << _status << " " << resolveStatus(_status) << CRLF
		<< "Content-Type: text/html" << CRLF
		<< "Content-Length: " << _body.size() << CRLF
		<< CRLF
		<< _body;
	return (ss.str());
}

/**
 * @brief Resolves the method to its corresponding handler.
 */
void Response::resolveMethod() {
	switch (_request.getMethod()) {
		case GET:
			handleGet();
			break;
		// case POST:
		// 	handlePost();
		// 	break;
		// case DELETE:
		// 	handleDelete();
		// 	break;
		default:
			_status = 405;
			break;
	}
}

void Response::handleGet() {
	char *path = strcat(getcwd(0, 0), _request.getPath().c_str());
	struct stat s;

	if (stat(path, &s) == 0) {
		if (s.st_mode & S_IFDIR) {
			_status = 200;
		}
		else if (s.st_mode & S_IFREG) {
			std::ifstream file((std::string(path)));
			std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			_body = str;
			_status = 200;
		}
		else
			_status = 404;
	}
	else
		_status = 404;
}

/**
 * @brief Resolves the status code to its corresponding string.
 */
std::string Response::resolveStatus(int status) {
	switch (status) {
		case 100:
            return "Continue";
        case 101:
            return "Switching Protocols";
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 203:
            return "Non-Authoritative Information";
        case 204:
            return "No Content";
        case 205:
            return "Reset Content";
        case 206:
            return "Partial Content";
        case 300:
            return "Multiple Choices";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 303:
            return "See Other";
        case 304:
            return "Not Modified";
        case 305:
            return "Use Proxy";
        case 307:
            return "Temporary Redirect";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 402:
            return "Payment Required";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 406:
            return "Not Acceptable";
        case 407:
            return "Proxy Authentication Required";
        case 408:
            return "Request Time-out";
        case 409:
            return "Conflict";
        case 410:
            return "Gone";
        case 411:
            return "Length Required";
        case 412:
            return "Precondition Failed";
        case 413:
            return "Request Entity Too Large";
        case 414:
            return "Request-URI Too Large";
        case 415:
            return "Unsupported Media Type";
        case 416:
            return "Requested range not satisfiable";
        case 417:
            return "Expectation Failed";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Time-out";
        case 505:
            return "HTTP Version not supported";
        default:
            return "Unknown Status";
	}
}
