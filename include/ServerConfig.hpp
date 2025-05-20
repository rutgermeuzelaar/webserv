/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerConfig.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:15:37 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/20 20:28:37 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP
# include <string>
# include <vector>
# include <map>
# include <filesystem>
# include <optional>
# include "Lexer.hpp"

class Root
{
    public:
        std::filesystem::path m_path;
        Root();
};

class ClientMaxBodySize
{
    public:
        size_t    m_size;
        ClientMaxBodySize();
};

class Location
{
    public:
        std::string m_uri;
        std::optional<Root> m_root;
        Location();
};

class ServerName
{
    public:
        std::string m_name;
};

class ServerConfig
{
    public:
        ServerConfig();
        std::optional<ClientMaxBodySize> m_client_max_body_size;
        std::optional<Location> m_location;
        std::optional<ServerName> m_server_name;
};

#endif