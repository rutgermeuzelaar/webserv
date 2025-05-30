/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/02 11:24:15 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 19:00:38 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP
# include <string>
# include <map>
# include <vector>
# include <optional>
# include "Lexer.hpp"
# include "ServerContext.hpp"
# include "HttpContext.hpp"
# include "ConfigStatement.hpp"

const std::map<std::string, TokenType> keywords {
    {"client_max_body_size", TokenType::ClientMaxBodySize},
    {"error_page", TokenType::ErrorPage},
    {"http", TokenType::Http},
    {"server", TokenType::Server},
    {"server_name", TokenType::ServerName},
    {"root", TokenType::Root},
    {"location", TokenType::Location},
    {"listen", TokenType::Listen},
    {"return", TokenType::Return},
    {"autoindex", TokenType::AutoIndex},
    {"on", TokenType::On},
    {"off", TokenType::Off}
};

class Config
{
    public:
        HttpContext m_http_context;
        Config();
        ServerContext& get_server(void);
};

#endif