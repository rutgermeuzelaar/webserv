/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/23 11:38:40 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/05 13:40:19 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include <fstream>
#include <iostream>
#include "Config.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"

Config::Config()
{
      
}

ServerContext& Config::get_server()
{
    return (m_http_context.m_servers.m_vector.back());
}

void read_config_file(Config& config, const char *path)
{
    std::ifstream config_file(path);
    std::stringstream buffer;

    if (config_file.fail())
    {
		throw std::runtime_error("An error occured when opening the supplied"
		" configuration file.");
    }
    buffer << config_file.rdbuf();
    Scanner scanner(buffer.str());
    const std::vector<Token>& tokens = scanner.scan();
    Parser parser(tokens, config);
    parser.parse();
}
