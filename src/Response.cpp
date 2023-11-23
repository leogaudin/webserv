#include "../include/Webserv.hpp"


// CONSTRUCTORS / DESTRUCTORS

/**
 * @brief   Constructs a Response object from a Request object.
 *
 * @param request The Request object to be used to construct the Response.
 */
Response::Response(Request request, Config config) {
	_config = getLocationConfig(request.getPath(), config);
    _locationName = getLocationName(request.getPath(), config);
	_request = request;
	_version = request.getVersion();
	_status = 500;
	_body = "";
    _cgiBuilt = false;
    _cgiResponse = "";
	resolveMethod();
}

Response::~Response() {}


// KEY METHODS

/**
 * @brief   Builds the HTTP response.
 */
std::string Response::build() {
    if (_cgiBuilt)
        return _cgiResponse;
    else {
        std::ostringstream ss;
        if (_headers.find("Content-Type") == _headers.end())
            addHeader("Content-Type", resolveMimeType(_request.getPath()));
        addHeader("Content-Length", std::to_string(_body.size()));
        ss
            << _version << " " << _status << " " << resolveStatus(_status) << CRLF
            << outputHeaders()
            << CRLF
            << _body;
        return (ss.str());
    }
}

/**
 * @brief   Resolves the HTTP method of the request and calls the appropriate handler function.
 *
 * It first checks if the size of the request body exceeds the maximum allowed size.
 * If it does, it handles a 413 Payload Too Large error and returns.
 *
 * If not, it checks if the request method is in the list of allowed methods.
 * If it's not, it handles a 405 Method Not Allowed error and returns.
 *
 * If it is, it checks if the host header of the request matches the server name or host in the configuration.
 * If it doesn't, it handles a 400 Bad Request error and returns.
 *
 * If it does, it switches on the request method and calls the appropriate handler function.
 */
void Response::resolveMethod() {
    if (_request.getBody().size() > (size_t)_config._client_max_body_size)
    {
		handleErrorStatus(413);
        return;
    }

    if (_config._allowedMethods.size() > 0) {
        if (std::find(_config._allowedMethods.begin(), _config._allowedMethods.end(), _request.getMethod()) == _config._allowedMethods.end()) {
            handleErrorStatus(405);
            return;
        }
    }

    if (_config._serverName.compare("") && _config._serverName.compare("_") && _request.getHeaders()["host"][0] != _config._serverName && _request.getHeaders()["host"][0] != _config._host) {
        if (!(
            !_config._host.compare("0.0.0.0")
            && (!_request.getHeaders()["host"][0].compare(getLocalIPAddress())
                || !_request.getHeaders()["host"][0].compare(LOOPBACK))
            )) {
            handleErrorStatus(400);
            return;
        }
    }

	switch (_request.getMethod()) {
		case GET:
			handleGet(_request.getPath());
			break;
		case POST:
			handlePost(_request.getPath());
			break;
		case DELETE:
			handleDelete(_request.getPath());
			break;
		default:
			_status = 405;
			break;
	}
}

/**
 * @brief   Handles a GET request for a specified path.
 *
 * It first checks if the path is a redirect.
 *
 * If it's a file, it checks if the file's extension is in the list of CGI extensions.
 *
 * If the path corresponds to a directory, it checks if an index file is specified in the configuration.
 * If it is, it redirects to the index file.
 * If not, it checks if autoindexing is enabled.
 * If not, it returns a 404 error.
 *
 * If the path doesn't correspond to a file or a directory, it checks if the path ends with a slash.
 * If it doesn't, it redirects to the path with a slash appended.
 * If it does, it returns a 404 error.
 */
void Response::handleGet(std::string requestedPath) {
    std::string rootedPath = rootPath(_config, requestedPath);
	char *path = strcat(getcwd(0, 0), rootedPath.c_str());
	struct stat s;

    if (checkRedirect())
        return;

	if (stat(path, &s) == 0) {
		_status = 200;
        if (s.st_mode & S_IFREG) {
            if (std::find(_config._cgiExtensions.begin(), _config._cgiExtensions.end(), getExtension(path)) != _config._cgiExtensions.end()) {
                _cgiBuilt = true;
                CGI cgi(_config, _request);
                _cgiResponse = cgi.execute();
            }
            else {
                std::ifstream file((std::string(path)));
                std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                _body = str;
            }
		}
		else if (s.st_mode & S_IFDIR) {
            if (_config._index != "") {
                std::string slash = "";
                if (path[strlen(path) - 1] != '/')
                        slash = "/";
                struct stat sIndex;
                std::string indexPath = path + slash + _config._index;
                if (stat(indexPath.c_str(), &sIndex) == 0) {
                    redirect(requestedPath + slash + _config._index);
                }
                else if (_config._autoindex)
                    _body = autoindex(requestedPath);
                else
                    handleErrorStatus(404);
            }
		}
		else if (path[strlen(path) - 1] != '/')
            redirect(requestedPath + "/");
	}
	else
		handleErrorStatus(404);
}


/**
 * @brief   Handles a POST request for a specified path.
 *
 * It first checks if the path is a redirect.
 * If not, it checks if the path corresponds to a file or a directory.
 * If it's a file, it checks if the file's extension is in the list of CGI extensions.
 * If it is, it executes the CGI script and sends the output as HTTP response.
 * If it's not, that means the user is trying to create an existing file, it returns a 409 Conflict error.
 * If the path corresponds to a directory, it returns a 405 Method Not Allowed error.
 * If the path doesn't correspond to a file or a directory, it uploads the content of the request to the path and sets the status to 201 Created.
 */
void Response::handlePost(std::string requestedPath) {
    std::string rootedPath = rootPath(_config, requestedPath);
    char *path = strcat(getcwd(0, 0), rootedPath.c_str());
    struct stat s;

    if (checkRedirect())
        return;

    if (stat(path, &s) == 0) {
        if (s.st_mode & S_IFREG) {
            if (std::find(_config._cgiExtensions.begin(), _config._cgiExtensions.end(), getExtension(path)) != _config._cgiExtensions.end()) {
                _cgiBuilt = true;
                CGI cgi(_config, _request);
                _cgiResponse = cgi.execute();
            }
            else
                handleErrorStatus(409);
        }
        else if (s.st_mode & S_IFDIR)
            handleErrorStatus(405);
        else
            handleErrorStatus(404);
    }
    else {
        uploadFromRequest(path);
        _status = 201;
    }
}

/**
 * @brief   Handles a DELETE request for a specified path.
 *
 * It first checks if the path is a redirect.
 * If not, it checks if the path corresponds to a file or a directory.
 * If it's a file, it attempts to delete the file.
 * If the deletion is successful, it sets the status to 200 OK.
 * If it's not successful, it sets the status to 500 Internal Server Error.
 * If the path corresponds to a directory, it sets the status to 405 Method Not Allowed.
 * If the path doesn't correspond to a file or a directory, it sets the status to 404 Not Found.
 */
void Response::handleDelete(std::string requestedPath) {
    std::string rootedPath = rootPath(_config, requestedPath);
    char *path = strcat(getcwd(0, 0), rootedPath.c_str());
    struct stat s;

    if (checkRedirect())
        return;

    if (stat(path, &s) == 0) {
        if (s.st_mode & S_IFREG) {
            if (remove(path) == 0)
                _status = 200;
            else
                handleErrorStatus(500);
        }
        else if (s.st_mode & S_IFDIR)
            handleErrorStatus(405);
    }
    else
        handleErrorStatus(404);
}

/**
 * @brief   Resolves the status code to its corresponding error page.
 */
void Response::handleErrorStatus(int status) {
    std::string path = "/" + _config._root + "/" + _config._errorPages[status];
    char *errorPath = strcat(getcwd(0, 0), path.c_str());

    std::ifstream file((std::string(errorPath)));
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    _body = str;
    _status = status;
    addHeader("Content-Type", "text/html");
}


// UTILS

/**
 * @brief   Adds a header to the headers map.
 */
void Response::addHeader(std::string key, std::string value) {
    if (_headers.find(key) == _headers.end())
        _headers[key] = std::vector<std::string>();
    _headers[key].push_back(value);
}

/**
 * @brief   Outputs the headers map.
 *
 * @return The headers map.
 */
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
 * @brief   Uploads the body of the request to the specified path.
 */
void Response::uploadFromRequest(std::string path) {
    std::ofstream file((std::string(path)));
    std::vector<unsigned char> body = _request.getBody();
    for (int i = 0; i < (int)body.size(); i++)
        file << body[i];
    file.close();
}

/**
 * @brief   Retrieves the configuration for a specific location.
 *
 * This function retrieves the configuration for a specific location from a given configuration object.
 * It iterates over the locations in the configuration object and checks if the requested path matches the location.
 * If a match is found, it clears the locations in the matched configuration and returns it.
 * If no match is found, it returns the original configuration object.
 */
Config Response::getLocationConfig(std::string path, Config &config) {
    std::map< std::string, Config >::iterator it;
    for (it = config._locations.begin(); it != config._locations.end(); it++) {
        if (
            (path.find(it->first) == 0 && (path.size() == it->first.size() || (path[it->first.size()] && path[it->first.size()] == '/')))
            || (path.find("/" + it->first) == 0 && (path.size() == it->first.size() + 1 || (path[it->first.size() + 1] && path[it->first.size() + 1] == '/')))
        )
        {
            it->second._locations.clear();
            return it->second;
        }
    }
    return config;
}

/**
 * @brief   Retrieves the name of the location for a specific path.
 *
 * This function retrieves the name of the location for a specific path from a given configuration object.
 * It iterates over the locations in the configuration object and checks if the requested path matches the location.
 * If a match is found, it returns the name of the location.
 * If no match is found, it returns an empty string.
 */
std::string Response::getLocationName(std::string path, Config &config) {
    std::map< std::string, Config >::iterator it;
    for (it = config._locations.begin(); it != config._locations.end(); it++) {
        if (
            (path.find(it->first) == 0 && (path.size() == it->first.size() || (path[it->first.size()] && path[it->first.size()] == '/')))
            || (path.find("/" + it->first) == 0 && (path.size() == it->first.size() + 1 || (path[it->first.size() + 1] && path[it->first.size() + 1] == '/')))
        )
            return it->first;
    }
    return "";
}

/**
 * @brief   Generates an HTML page listing the contents of a directory.
 *
 * This function generates an HTML page that lists the contents of a directory specified by the path parameter.
 * It first constructs the full path of the directory, then opens the directory and reads its contents.
 * For each entry in the directory, it adds a link to the HTML page.
 * The HTML page is then returned as a string.
 */
std::string Response::autoindex(std::string path) {
    std::string rootedPath = rootPath(_config, path);
    char *dirPath = strcat(getcwd(0, 0), rootedPath.c_str());
    DIR *dir;
    struct dirent *ent;
    std::string html = "<html><head><title>Index of " + path + "</title></head><body><h1>Index of " + path + "</h1><hr><pre>";
    if ((dir = opendir(dirPath)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.' && path == "/")
                continue;
            html += "<a href=\"" + path + (path[path.size() - 1] == '/' ? "" : "/") + ent->d_name + "\">" + ent->d_name + "</a><br>";
        }
        closedir(dir);
    } else {
        perror("Could not open directory");
    }
    html += "</pre><hr></body></html>";
    addHeader("Content-Type", "text/html");
    return html;
}

/**
 * @brief   Constructs a path based on a configuration object and a requested path.
 *
 * This function constructs a path based on a configuration object and a requested path.
 * If the configuration object has an alias, the alias is prepended to the requested path.
 * If not, but the configuration object has a root, the root is prepended to the requested path.
 * If neither an alias nor a root is present, the requested path is returned as is.
 */
std::string rootPath(Config config, std::string requestedPath) {
    std::string rootedPath;

    if (config._alias != "")
        rootedPath = "/" + config._alias + requestedPath;
    else if (config._root != "")
        rootedPath = "/" + config._root + requestedPath;
    else
        rootedPath = requestedPath;

    return rootedPath;
}

/**
 * @brief   Sets a redirect header and status.
 */
void Response::redirect(std::string path) {
    addHeader("location", path);
    _status = 302;
}

/**
 * @brief   Checks if a redirect is set in the configuration.
 */
bool Response::checkRedirect() {
    if (_config._redirect != "") {
        redirect(_config._redirect);
        return true;
    }
    return false;
}

/**
 * @brief   Resolves the status code to its corresponding string.
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

/**
 * @brief   Resolves the extension to its corresponding MIME type.
 */
std::string Response::resolveMimeType(std::string path) {
    std::string extension = path.substr(path.find_last_of(".") + 1);
    std::vector<std::string>::iterator it = std::find(_config._cgiExtensions.begin(), _config._cgiExtensions.end(), extension);
    if (it != _config._cgiExtensions.end())
        return "text/html";
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
