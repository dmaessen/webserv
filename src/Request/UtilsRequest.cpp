/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UtilsRequest.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dmaessen <dmaessen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/07 15:50:31 by dmaessen          #+#    #+#             */
/*   Updated: 2024/08/20 15:14:02 by dmaessen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/ParseRequest.hpp"

std::string& ParseRequest::capsOn(std::string &str) {
    for (size_t i = 0; i < str.size(); i++) {
        if (str[0])
            str[0] = toupper(str[0]);
        else if (str[i - 1] == '-' && isalpha(str[i]))
            str[i] = toupper(str[i]);
    }
    return str;
}

std::string ParseRequest::rmSpaces(std::string &str) {
    if (str.empty()) return str;
    size_t len = str.size() - 1;
    while (len > 0 && str[len] == ' ')
        --len;
    return str.substr(0, len + 1);
}

std::vector<std::string> ParseRequest::split(const std::string &str, char c) {
    std::vector<std::string> vec;
    std::string element;
    std::istringstream stream(str);

    while (std::getline(stream, element, c))
        vec.push_back(element);
    return vec;
}

bool cgiInvolved(const std::string& path) {
    std::size_t found = path.find("cgi-bin");

    if (found != std::string::npos)
        return true;
    return false;
}

bool isFileExists(const std::string& path) {
    struct stat buffer;
    
    if (stat(path.c_str(), &buffer) == 0)
        return true;
    return false;
}
