/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseRequest.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dmaessen <dmaessen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/07 15:50:05 by dmaessen          #+#    #+#             */
/*   Updated: 2024/07/08 14:19:18 by dmaessen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Request/parseRequest.hpp"

// this will also take an instance of the server part to be able to retreive _path if it was overwritten/updated
parseRequest::parseRequest(std::string &info) : _methodType(""), _version(""), _returnValue(200),
                              _bodyMsg(""), _port(80), _path(""), _query("") {
    initHeaders();
    // init _path to a certain default location but depends on the config file
    parseStr(info);
    if (_returnValue != 200)
        std::cout << "Parse error: " << _returnValue << '\n';
}

parseRequest::~parseRequest() {

}

parseRequest&	parseRequest::operator=(const parseRequest &cpy)
{
	this->_headers = cpy.getHeaders();
	this->_methodType = cpy.getMethod();
	this->_version = cpy.getVersion();
	this->_returnValue = cpy.getRetVal();
	this->_bodyMsg = cpy.getBodyMsg();
	this->_port = cpy.getPort();
	this->_path = cpy.getPath();
    this->_query = cpy._query;
	return (*this);
}

std::string parseRequest::parseStr(std::string &info) {
    size_t i = 0;
    std::string line;
    std::string bodyLine;
    std::string value;
    std::string key;

    parseFirstline(readLine(info, i));
    while ((line = readLine(info, i)) != "\r" && line != "" && _returnValue != 400) {
        key = setKey(line);
        value = setValue(line);
        if (_headers.count(key)) {
            _headers[key] = value;
        }
    }

	size_t startBody = info.find("\r\n\r\n") + 4;
	_bodyMsg = info.substr(startBody, std::string::npos);

    setPort(_headers["Host"]);
    setQuery();
    setLanguage();

    if (_headers["Cookie"] != "")
        _cookies = parseCookies(_headers["Cookie"]);
    
    _cgiresponse = "";
    if (cgiInvolved(_headers["Path"]) == true)
        _cgiresponse = cgiHandler(*this); // check with laura, she needs the config things tooo + store it in Lou's struct directly
    //CHECK IF SOMEONE FAILED ON LAURA'S SIDE??
    
    // DO I NEED/WANT TO CHECK IF THE ERROR CODE IS BAD ALREADY??
    Response res;
    return res.giveResponse(*this); // ++ an instance of the struct ++ is this really how we want to return??
}


std::string parseRequest::readLine(const std::string &str, size_t &i) {
    std::string res;
    size_t j;

    if (i == std::string::npos) 
        return "";
    j = str.find_first_of('\n', i);
    res = str.substr(i, j - i);
    if (res[res.size() - 1] == '\r')
        res.pop_back();
    if (j == std::string::npos)
        i = j;
    else
        i = j + 1;
    return res;
}

/* SETS THE HEADER VALUES */
std::string parseRequest::setKey(const std::string &line) {
    size_t i;
    std::string res;

    i = line.find_first_of(":", 1);
    res.append(line, 0, i);
    capsOn(res);
    return res;
}

std::string parseRequest::setValue(const std::string &line) {
    size_t i;
    size_t endline;
    std::string res;

    i = line.find_first_of(":", 1);
    i = line.find_first_not_of(" ", i + 1);
    endline = line.find_first_of("\r", i);
    line.substr(i, endline - 1);
    if (i != std::string::npos)
        res.append(line, i, std::string::npos);
    return rmSpaces(res);
}

void parseRequest::setLanguage() {
    std::vector<std::string> vec;
    std::string header;
    size_t i;

    if ((header = _headers["Accept-Language"]) != "")
    {
        vec = split(header, ',');
        for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++)
        {
            float weight = 0.0;
			std::string	language;
			language = (*it).substr(0, (*it).find_first_of('-'));
			rmSpaces(language);
			if ((i = language.find_last_of(';')) != std::string::npos) {
				weight = atof((*it).substr(i + 4).c_str());
			}
            if (i > 2)
                language.resize(2);
            else
                language.resize(i);
			_language.push_back(std::pair<std::string, float>(language, weight));
		}
        _language.sort(std::greater<std::pair<std::string, float>>());
    }
}

std::string parseRequest::readBody(const std::string &str, size_t &i) {
    std::string res;

    if (i == std::string::npos)
        return "";
    for (size_t j = 0; str[i] != std::string::npos; i++) {
        res[j] = str[i];
        j++;
    }
    return res;
}

void parseRequest::setQuery() {
    size_t i;

    i = _path.find_first_of('?');
    if (i != std::string::npos) {
        _query.assign(_path, i + 1, std::string::npos);
        _path = _path.substr(0, i);
    }
}

/* ACCESSSORS SETTERS-GETTERS */
void parseRequest::setMethod(std::string type) {
    _methodType = type;
}

std::string parseRequest::getMethod(void) const {
    return _methodType;
}

void parseRequest::setPath(std::string path) {
    _path = path;
}

std::string parseRequest::getPath(void) const {
    return _path;
}

void parseRequest::setVersion(std::string v) {
    _version = v;
}

std::string parseRequest::getVersion(void) const {
    return _version;
}

void parseRequest::setPort(std::string port) {
    size_t start;

    start = port.find_first_of(":");

    if (start != 0 && port.find("localhost:") != std::string::npos)
        port = port.substr(start + 1);
    if (port.size() < 5)
        _port = std::stoul(port);
    else
        std::cout << "Error: in port\n"; // this will be checked later as well right
}

unsigned int parseRequest::getPort(void) const {
    return _port;
}

void parseRequest::setRetVal(int value) {
    _returnValue = value;
}

int parseRequest::getRetVal(void) const {
    return _returnValue;
}

void parseRequest::setBodyMsg(std::string b) {
	_bodyMsg = b;
}

std::string parseRequest::getBodyMsg(void) const {
    return _bodyMsg;
}

std::string parseRequest::getLanguageStr(void) const {
    std::ostringstream oss;

    for (auto it = _language.begin(); it != _language.end(); ++it) {
        oss << it->first;
        if (std::next(it) != _language.end()) {
            oss << ", ";
        }
    }
    return oss.str();
}

std::string parseRequest::getQuery(void) const {
    return _query;
}

std::string parseRequest::getCgiResponse(void) const {
    return _cgiresponse;
}

/* HEADERS */
void parseRequest::initHeaders() {
    _headers.clear();
    _headers["Accept-Charsets"] = "";
    _headers["Accept-Language"] = "";
    _headers["Allow"] = "";
	_headers["Auth-Scheme"] = "";
	_headers["Authorization"] = "";
	_headers["Connection"] = "Keep-Alive";
    _headers["Cookie"] = ""; // will need to be stored somewhere for later
	_headers["Content-Language"] = "";
	_headers["Content-Length"] = "";
	_headers["Content-Location"] = "";
	_headers["Content-Type"] = "";
	_headers["Date"] = "";
	_headers["Host"] = "";
	_headers["Last-Modified"] = "";
	_headers["Location"] = "";
	_headers["Referer"] = "";
	_headers["Retry-After"] = "";
	_headers["Server"] = "";
	_headers["Transfer-Encoding"] = "";
	_headers["User-Agent"] = "";
	_headers["Www-Authenticate"] = "";
}

const std::map<std::string, std::string>&	parseRequest::getHeaders(void) const {
	return _headers;
}

const std::map<std::string, std::string>&	parseRequest::getCookies(void) const {
	return _cookies;
}

/* PARSING REQUEST */
int parseRequest::parseFirstline(const std::string &info) {
    size_t i;
    std::string line;

    i = info.find_first_of('\n');
    line = info.substr(0, i);
    i = line.find_first_of(' ');
    
    if (i == std::string::npos) {
        _returnValue = 400;
        std::cerr << "Error: wrong syntax of HTTP method\n"; // OR WHAT??
        return _returnValue;
    }
    _methodType.assign(line, 0, i);
    return parsePath(line, i);
}

int parseRequest::parsePath(const std::string &line, size_t i) {
    size_t j;

    if ((j = line.find_first_not_of(' ', i)) == std::string::npos) {
        _returnValue = 400;
        std::cerr << "Error: no path\n"; // do we want this here, should go further not?
        return _returnValue;
    }
    if ((j = line.find_first_of(' ', j)) == std::string::npos) {
        _returnValue = 400;
        std::cerr << "Error: no HTTP version\n"; // do we want this here, should go further not?
        return _returnValue;
    }
    _path.assign(line, i + 1, j - i);
    return parseVersion(line, j);
}

int parseRequest::parseVersion(const std::string &line, size_t i) {
    if ((i = line.find_first_not_of(' ', i)) == std::string::npos) {
        _returnValue = 400;
        std::cerr << "Error: no HTTP version\n";
        return _returnValue;
    }
    if (line[i] == 'H' && line[i + 1] == 'T' && line[i + 2] == 'T' && line[i + 3] == 'P'
        && line[i + 4] == '/')
        _version.assign(line, i  + 5, 3);
    if (_version != "1.0" && _version != "1.1") {
        _returnValue = 400;
        std::cerr << "Error: wrong HTTP version\n";
        return _returnValue;
    }
    return validateMethodType();
}

/* METHOD */
std::string initMethodString(Method method)
{
    switch(method)
    {
        case Method::GET:
            return "GET";
        case Method::POST:
            return "POST";
        case Method::DELETE:
            return "DELETE";
    }
	return "DONE INITING METHOD STRING";
}

int parseRequest::validateMethodType() {
    if (_methodType == initMethodString(Method::GET) 
    || _methodType == initMethodString(Method::POST) 
    || _methodType == initMethodString(Method::DELETE))
          return _returnValue;
    std::cerr << "Error: invalid method requested\n";
    _returnValue = 400;
    return _returnValue;
}
