#include "Server.hpp"
#include <sys/epoll.h>
#include <algorithm>

Server::Server() : _fd(-1), _timeout(0.0), _maxNrOfRequests(0) {
	_shared = std::make_shared<SharedData>();
}

// Server::Server(Server const & src) {
//     // Copy constructor implementation if needed
// }

Server::~Server() {
	if (_fd != -1) {
		close(_fd);
	}
}

// Dit kan netter, time ? doen : skip;
int Server::initServer(const ServerConfig *config, int epollFd, double timeout, int maxNrOfRequests) {
	_timeout = timeout;
	_maxNrOfRequests = maxNrOfRequests;

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	// _serverAddr.sin_addr.s_addr = inet_addr(config->host.c_str()); voor de echte host?
	_serverAddr.sin_port = htons(config->port);

    if (config->port == 0 || config->port > 65535) {
        throw ServerException("Port number out of range: " + std::to_string(config->port));
    }
	_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_fd == -1) {
		throw ServerException("Failed to create socket: ");
	}

    try {
        _setSocketOptions();
        _bindSocket();
        _listenSocket(BACKLOG);

        _registerWithEpoll(epollFd, _fd, EPOLLIN); // Should I use edge-triggered?
    } catch (std::exception & e) {
        throw e.what();
    }
	_configs = config;
    
    std::cout << GREEN << "Server initialized on port " << config->port << RESET << std::endl;
	return 0;
}

void Server::_setSocketOptions() {
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw ServerException("Failed to set SO_REUSEADDR socket option.");
    }
    #ifdef SO_REUSEPORT
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        throw ServerException("Failed to set SO_REUSEPORT socket option.");
    }
    #endif
}

void Server::_bindSocket() {
    if (bind(_fd, reinterpret_cast<struct sockaddr*>(&_serverAddr), sizeof(_serverAddr)) == -1)
        throw ServerException("Failed to bind socket");
}

void Server::_listenSocket(int backlog) {
    if (listen(_fd, backlog) == -1)
        throw ServerException("Failed to listen on socket");
}

#include <cstdio>
void Server::_registerWithEpoll(int epollFd, int fd, uint32_t events) {
    epoll_event	event;
	
	_shared->cgi_fd = -1;	//Laura??
	_shared->cgi_pid = -1;	//Laura??

	_shared->request = "";
	_shared->response = "";
	_shared->response_code = 200; // Heeft Domi hier een enum ofzo voor?

	_shared->fd =fd;
	_shared->epoll_fd = epollFd;

	_shared->status = Status::listening;
	_shared->server_config = _configs; // Hier heb ik een lijst, maar dit moet denk ik iets anders zijn..
	_shared->connection_closed = false; // Should I set this..? No I think domi..

	_shared->timestamp_last_request = std::time(nullptr);

    event.data.fd = fd;
    event.events = events;
    event.data.ptr = _shared.get();
    printf("--------------- %p ----------------\n", event.data.ptr);
    printf("--------------- %p ----------------\n", _shared.get());
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) < 0) //might need to do sth about lifetime.
        throw ServerException("Failed to register with epoll");
}

// void Server::setConnection(SharedData *shared) {
// 	_shared = shared;
// }

uint16_t Server::getPort() const {
	return ntohs(_serverAddr.sin_port);
}

double Server::getTimeout() const {
	return _timeout;
}

int Server::getMaxNrOfRequests() const {
	return _maxNrOfRequests;
}

std::map<std::string, int> Server::getKnownClientIds() const {
	return _knownClientIds;
}

std::string Server::getIndex(const std::string &host, const std::string &location) const {
	// const ServerConfig* configs = _getHostConfigs(host);
	if (_configs) {
		auto it = std::find_if(_configs->locations.begin(), _configs->locations.end(),
            [&location](const Locations& loc) { return loc.path == location; });
        if (it != _configs->locations.end()) {
            return it->default_file;
		}
		return _configs->index;
	}
	return "index.html";
}

bool Server::getDirListing(const std::string &host, const std::string &location) const {
	// ServerConfig* configs = _getHostConfigs(host);
	if (_configs) {
		auto it = std::find_if(_configs->locations.begin(), _configs->locations.end(),
            [&location](const Locations& loc) { return loc.path == location; });
        if (it != _configs->locations.end()) {
            return it->dir_listing;
        }
	}
	return false; // Dit had ik afgesproken met Domi, right?
}

std::string Server::getRootFolder(const std::string &host, const std::string &location) const {
    // const ServerConfig* _configs = _getHostConfigs(host);
    if (_configs) {
        auto it = std::find_if(_configs->locations.begin(), _configs->locations.end(),
                               [&location](const Locations& loc) { return loc.path == location; });
        if (it != _configs->locations.end()) {
            return it->root_dir.empty() ? _configs->root_dir : it->root_dir; // Use location-specific or fallback to general
        }
    }
    return "/";
}

std::set<std::string> Server::getAllowedMethods(const std::string &host, const std::string &location) const {
    // const ServerConfig* _configs = _getHostConfigs(host);
    if (_configs) {
        auto it = std::find_if(_configs->locations.begin(), _configs->locations.end(),
                               [&location](const Locations& loc) { return loc.path == location; });
        if (it != _configs->locations.end() && !it->allowed_methods.empty()) {
            return it->allowed_methods;
        }
    }
    return {"GET", "POST"};
}

std::map<int, std::string> Server::getRedirect(const std::string &host, const std::string &location) const {
    // const ServerConfig* _configs = _getHostConfigs(host);
    if (_configs) {
        auto it = std::find_if(_configs->locations.begin(), _configs->locations.end(),
                               [&location](const Locations& loc) { return loc.path == location; });
        if (it != _configs->locations.end()) {
            return it->redirect;
        }
    }
    return std::map<int, std::string>();
}

size_t Server::getMaxBodySize(const std::string &host, const std::string &location) const {
	// ServerConfig* _configs = _getHostConfigs(host);
	if (_configs) {
		return _configs->max_client_body_size;
	}
	return ONE_MB;
}

std::string Server::getUploadDir(const std::string &host, const std::string &location) const {
    // const ServerConfig* _configs = _getHostConfigs(host);
    if (_configs) {
        for (auto it = _configs->locations.rbegin(); it != _configs->locations.rend(); ++it) {
            if (location.find(it->path) == 0) {
                if (!it->upload_dir.empty()) {
                    return it->upload_dir;
                }
            }
        }
        if (!_configs->upload_dir.empty()) {
            return _configs->upload_dir;
        }
    }
    return "/uploads";
}

// Private Methods

int Server::_getFD() const {
	return _fd;
}

struct sockaddr_in Server::_getServerAddr() const {
	return _serverAddr;
}

std::shared_ptr<SharedData> Server::_getSharedData() const {
	return _shared;
}

// ServerConfig* Server::_getHostConfigs(const std::string &host) const {
// 	for (ServerConfig* _configs : _configs) {
// 		if (_configs->host == host) {
// 			return _configs;
// 		}
// 	}
// 	return nullptr;
// }