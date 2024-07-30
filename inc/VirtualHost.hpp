#pragma once
#ifndef VIRTUAL_HOST_HPP
# define VIRTUAL_HOST_HPP

# include "defines.hpp"

class VirtualHost {
public:
	VirtualHost(const std::string& name, const ServerConfig& Config);

	const std::string& getName() const;
	const ServerConfig& getConfig() const;

private:
	std::string		_name;
	ServerConfig	_conf;
};
#endif