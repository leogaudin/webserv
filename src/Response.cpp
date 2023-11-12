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
 * @brief Adds a header to the headers map.
 *
 * @param line The header to be added.
 */
void Response::addHeader(std::string key, std::string value) {
    _headers[key].push_back(value);
}

std::string Response::outputHeaders() {
    std::ostringstream ss;
    for (std::map< std::string, std::vector<std::string> >::iterator it = _headers.begin(); it != _headers.end(); it++) {
        ss << it->first << ": ";
        for (std::vector<std::string>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            ss << *it2;
            if (it2 + 1 != it->second.end())
                ss << ", ";
        }
        ss << CRLF;
    }
    return (ss.str());
}

/**
 * @brief Builds the HTTP response.
 *
 * @return The HTTP response.
 */
std::string Response::build() {
	std::ostringstream ss;
    addHeader("Content-Type", resolveMimeType(_request.getPath()));
    addHeader("Content-Length", std::to_string(_body.size()));
	ss
		<< _version << " " << _status << " " << resolveStatus(_status) << CRLF
        << outputHeaders()
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
			handleGet(_request.getPath());
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

void Response::handleGet(std::string requestedPath) {
    std::string rootedPath = "/" + _config._root + requestedPath;
	char *path = strcat(getcwd(0, 0), rootedPath.c_str());
	struct stat s;

	if (stat(path, &s) == 0) {
		_status = 200;
        if (s.st_mode & S_IFREG) {
			std::ifstream file((std::string(path)));
			std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			_body = str;
		}
		else if (s.st_mode & S_IFDIR) {
            if (_config._autoindex)
                (void)_body;
                // _body = autoindex(path);
            else
            {
                if (path[strlen(path) - 1] != '/')
                    requestedPath += "/";
                _status = 301;
                addHeader("Location", requestedPath + _config._index);
            }
		}
		else if (path[strlen(path) - 1] != '/')
        {
            _status = 301;
            addHeader("Location", requestedPath + "/");
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

std::string Response::resolveMimeType(std::string path) {
    std::string extension = path.substr(path.find_last_of(".") + 1);
    if (extension == "aac") return "audio/aac";
    if (extension == "abw") return "application/x-abiword";
    if (extension == "arc") return "application/x-freearc";
    if (extension == "avif") return "image/avif";
    if (extension == "avi") return "video/x-msvideo";
    if (extension == "azw") return "application/vnd.amazon.ebook";
    if (extension == "bin") return "application/octet-stream";
    if (extension == "bmp") return "image/bmp";
    if (extension == "bz") return "application/x-bzip";
    if (extension == "bz2") return "application/x-bzip2";
    if (extension == "cda") return "application/x-cdf";
    if (extension == "csh") return "application/x-csh";
    if (extension == "css") return "text/css";
    if (extension == "csv") return "text/csv";
    if (extension == "doc") return "application/msword";
    if (extension == "docx") return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if (extension == "eot") return "application/vnd.ms-fontobject";
    if (extension == "epub") return "application/epub+zip";
    if (extension == "gz") return "application/gzip";
    if (extension == "gif") return "image/gif";
    if (extension == "htm" || extension == "html") return "text/html";
    if (extension == "ico") return "image/vnd.microsoft.icon";
    if (extension == "ics") return "text/calendar";
    if (extension == "jar") return "application/java-archive";
    if (extension == "jpeg" || extension == "jpg") return "image/jpeg";
    if (extension == "js") return "text/javascript";
    if (extension == "json") return "application/json";
    if (extension == "jsonld") return "application/ld+json";
    if (extension == "mid" || extension == "midi") return "audio/midi";
    if (extension == "mjs") return "text/javascript";
    if (extension == "mp3") return "audio/mpeg";
    if (extension == "mp4") return "video/mp4";
    if (extension == "mpeg") return "video/mpeg";
    if (extension == "mpkg") return "application/vnd.apple.installer+xml";
    if (extension == "odp") return "application/vnd.oasis.opendocument.presentation";
    if (extension == "ods") return "application/vnd.oasis.opendocument.spreadsheet";
    if (extension == "odt") return "application/vnd.oasis.opendocument.text";
    if (extension == "oga") return "audio/ogg";
    if (extension == "ogv") return "video/ogg";
    if (extension == "ogx") return "application/ogg";
    if (extension == "opus") return "audio/opus";
    if (extension == "otf") return "font/otf";
    if (extension == "png") return "image/png";
    if (extension == "pdf") return "application/pdf";
    if (extension == "php") return "application/x-httpd-php";
    if (extension == "ppt") return "application/vnd.ms-powerpoint";
    if (extension == "pptx") return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    if (extension == "rar") return "application/vnd.rar";
    if (extension == "rtf") return "application/rtf";
    if (extension == "sh") return "application/x-sh";
    if (extension == "svg") return "image/svg+xml";
    if (extension == "tar") return "application/x-tar";
    if (extension == "tif" || extension == "tiff") return "image/tiff";
    if (extension == "ts") return "video/mp2t";
    if (extension == "ttf") return "font/ttf";
    if (extension == "txt") return "text/plain";
    if (extension == "vsd") return "application/vnd.visio";
    if (extension == "wav") return "audio/wav";
    if (extension == "weba") return "audio/webm";
    if (extension == "webm") return "video/webm";
    if (extension == "webp") return "image/webp";
    if (extension == "woff") return "font/woff";
    if (extension == "woff2") return "font/woff2";
    if (extension == "xhtml") return "application/xhtml+xml";
    if (extension == "xls") return "application/vnd.ms-excel";
    if (extension == "xlsx") return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    if (extension == "xml") return "application/xml";
    if (extension == "xul") return "application/vnd.mozilla.xul+xml";
    if (extension == "zip") return "application/zip";
    if (extension == "3gp") return "video/3gpp";
    if (extension == "3g2") return "video/3gpp2";
    if (extension == "7z") return "application/x-7z-compressed";
    return "application/octet-stream";
}
