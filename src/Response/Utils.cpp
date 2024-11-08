/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dmaessen <dmaessen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/07 15:50:11 by dmaessen          #+#    #+#             */
/*   Updated: 2024/08/19 12:51:50 by dmaessen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Response.hpp"

/* generate an HTML page that lists the contents of a specified directory. 
This directory listing includes the names of files and subdirectories within the specified directory, 
along with their sizes and last modified dates. */
std::string Response::autoIndexPageListing(const std::string& path, const std::string& dir) {
    std::stringstream html;

    html << "<!DOCTYPE html>\n";
    html << "<html>\n";
    html << "<head>\n";
    html << "<title>Index of " << path << "</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; }\n";
    html << "table { width: 100%; border-collapse: collapse; }\n";
    html << "th, td { padding: 8px 12px; border: 1px solid #ddd; text-align: left; }\n";
    html << "th { background-color: #f2f2f2; }\n";
    html << "</style>\n";
    html << "</head>\n";
    html << "<body>\n";
    html << "<h1>Index of " << path << "</h1>\n";
    html << "<table>\n";
    html << "<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string name = entry.path().filename().string();
        std::string size = entry.is_directory() ? "-" : std::to_string(std::filesystem::file_size(entry.path()));
        std::string lastModified = "-";

        html << "<tr>";
        html << "<td><a href=\"" << dir + "/" + name << "\">" << name << "</a></td>";
        html << "<td>" << size << "</td>";
        html << "<td>" << lastModified << "</td>";
        html << "</tr>\n";
    }

    html << "</table>\n";
    html << "</body>\n";
    html << "</html>\n";

    return html.str();
}
