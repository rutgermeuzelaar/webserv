/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/08 16:56:24 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/22 16:41:14 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include "Parser.hpp"

Parser::Parser(const std::vector<Token>& tokens, HttpContext& http_config)
    : m_tokens {tokens}
	, m_http_config {http_config}
    , m_current {0}
{
    
}

bool Parser::at_end()
{
    return peek().m_token_type == TokenType::Eof;
}

bool Parser::at_end() const
{
    return peek().m_token_type == TokenType::Eof;
}

bool Parser::check(TokenType type)
{
    if (at_end())
    {
        return (false);
    }
    return (type == peek().m_token_type);
}

bool Parser::match(const std::vector<TokenType> types)
{
    for (const TokenType& i: types)
    {
        if (check(i))
        {
            return (true);
        }   
    }
    return (false);
}

const Token& Parser::peek() const
{
    return (m_tokens[m_current]);
}

const Token& Parser::next() const
{
    if (at_end())
    {
        return (peek());           
    }
    return (m_tokens[m_current + 1]);
}

const Token& Parser::advance()
{
    return m_tokens[m_current++];   
}

const Token& Parser::previous() const
{
    if (m_current == 0)
    {
        return (m_tokens[m_current]);
    }
    return (m_tokens[m_current - 1]);
}

const Token& Parser::consume(std::vector<TokenType> types, const char *error)
{
    if (match(types))
    {
        return (advance());
    }
    log_error(error);
    throw Error();
}

const Token& Parser::consume(TokenType type, const char *error)
{
    if (peek().m_token_type == type)
    {
        return (advance());
    }
    log_error(error);
    throw Error();
}

void Parser::parse_server_name()
{
    consume(TokenType::String, "Expected string after server_name.");
}

void Parser::parse_error_page()
{
    consume(TokenType::Number, "Expected status code.");
    while (check(TokenType::Number))
    {
        advance();
    }
    consume(TokenType::Path, "Expected path after status code(s).");
    if (!is_valid_file_path(previous().m_str))
    {
        throw Parser::Error();
    }
}

void Parser::parse_listen()
{
    const TokenType next = peek().m_token_type;

    if (next == TokenType::Number)
    {
        consume(TokenType::Number, "Expected port number.");
        return ;
    }
    consume(TokenType::IPv4, "Expected IPv4 address.");
    if (peek().m_token_type == TokenType::Semicolon)
    {
        return ;
    }
    consume(TokenType::Colon, "Expected colon after IPv4 address.");
    consume(TokenType::Number, "Expected port number after IPv$ address.");
}

void Parser::parse_location()
{
    consume(TokenType::Uri, "Expected URI after location.");
    parse_block();
}

void Parser::parse_return()
{
    consume(TokenType::Number, "Expected status code.");
    while (check(TokenType::Number))
    {
        advance();
    }
    consume(TokenType::String, "Expected URL after status code(s).");
}

void Parser::parse_autoindex()
{
    consume({TokenType::On, TokenType::Off}, "Expected 'on' or 'off'.");
}

void Parser::parse_root()
{
    consume(TokenType::Path, "Expected path after root.");
    if (!is_valid_dir_path(previous().m_str))
    {
        throw Parser::Error();
    }
}

void Parser::parse_client_max_body_size()
{
    consume(TokenType::Number, "Expected number after client_max_body_size.");
}

void Parser::parse_server()
{
    parse_block();
}

void Parser::parse_statement()
{
    const TokenType current = advance().m_token_type;
    
    switch (current)
    {
        case TokenType::ServerName:
            parse_server_name();
            break;
        case TokenType::ErrorPage:
            parse_error_page();
            break;
        case TokenType::Listen:
            parse_listen();
            break;
        case TokenType::Root:
            parse_root();
            break;
        case TokenType::AutoIndex:
            parse_autoindex();
            break;
        case TokenType::Return:
            parse_return();
            break;
        case TokenType::Location:
            parse_location();
            return;
        case TokenType::Server:
            parse_server();
            return;
        case TokenType::ClientMaxBodySize:
            parse_client_max_body_size();
            break;
        default:
            log_error("Unexpected token.");
            throw Error();
    }
    consume(TokenType::Semicolon, "Expected ';' after statement.");
}

void Parser::parse_block()
{
    consume(TokenType::OpenBrace, "Expected '{'.");
    while (!check(TokenType::CloseBrace))
    {
        parse_statement();
    }
    consume(TokenType::CloseBrace, "Expected '}'.");
}

void Parser::parse()
{
    if (peek().m_token_type != TokenType::Http)
    {
        log_error("Config file should start with a http block.");
        throw Error();
    }
    advance();
    parse_block();
    if (!at_end())
    {
        log_error("Expected end of file.");
        throw Error();
    }
}

void Parser::log_error(const std::string reason) const
{
    const Token& token = previous();
    std::stringstream stream;
    
    stream << "Parsing error occured near token \'" << token.m_str <<"\' of type \'" \
    << stringify(token.m_token_type) << "\', at line " << token.m_linenum << ": " << reason;
    std::cerr << stream.str() << '\n';
}

void Parser::log_error(const std::string reason, const Token& token) const
{
    std::stringstream stream;
    
    stream << "Parsing error occured near token \'" << token.m_str <<"\' of type \'" \
    << stringify(token.m_token_type) << "\', at line " << token.m_linenum << ": " << reason;
    std::cerr << stream.str() << '\n';
}

Parser::Error::Error()
    : std::runtime_error("A parsing error occured.")
{
      
}

static bool owner_read(const std::filesystem::path& path)
{
    std::filesystem::perms perms = std::filesystem::status(path).permissions();
    if (std::filesystem::perms::none == (std::filesystem::perms::owner_read & perms))
    {
        return (false);
    }
    return (true);
}

bool Parser::is_valid_dir_path(const std::string path) const
{
    const std::filesystem::path path_obj(path);
    std::error_code             error;
    
    if (!std::filesystem::is_directory(path_obj, error))
    {
        log_error(error.message(), next());
        return (false); 
    }
    if (!owner_read(path_obj))
    {
        log_error("Missing read rights for owner.", previous());
        return (false);
    }
    return (true);
}

bool Parser::is_valid_file_path(const std::string path) const
{
    const std::filesystem::path path_obj(path);
    std::error_code             error;
    
    if (!std::filesystem::is_regular_file(path_obj, error))
    {
        log_error(error.message(), previous());
        return (false); 
    }
    if (!owner_read(path_obj))
    {
        log_error("Missing read rights for owner.", previous());
        return (false);
    }
    return (true);
}
