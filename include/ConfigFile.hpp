/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConfigFile.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/02 11:24:15 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/11 17:51:35 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_FILE_HPP
# define CONFIG_FILE_HPP
# include <string>
# include <map>
# include "Lexer.hpp"

const std::map<std::string, TokenType> keywords {
    {"client_max_body_size", TokenType::ClientMaxBodySize},
    {"error_page", TokenType::ErrorPage},
    {"http", TokenType::Http},
    {"server", TokenType::Server},
    {"server_name", TokenType::ServerName},
    {"root", TokenType::Root},
    {"location", TokenType::Location},
    {"listen", TokenType::Listen}
};
#endif