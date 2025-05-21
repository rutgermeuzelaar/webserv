/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerConfig.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/11 14:15:37 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/21 17:20:23 by rmeuzela      ########   odam.nl         */
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

class Listen
{
	public:
		const int32_t m_ipv4;
		const int32_t m_port;
		Listen();
		Listen(const std::string ipv4);
		Listen(const std::string ipv4, const std::string port);
};

class ServerConfig
{
    public:
        ServerConfig();
        std::optional<ClientMaxBodySize> m_client_max_body_size;
        std::optional<Location> m_location;
        std::optional<ServerName> m_server_name;
		Listen m_listen;
		bool validate() const;
};

#endif