/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/08 16:56:24 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/17 14:44:11 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <cassert>
#include "Parser.hpp"

Parser::Parser(const std::vector<Token>& tokens, Config& config)
    : m_tokens {tokens}
	, m_config {config}
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
    try
    {
        set_server_name(ServerName(previous().m_str));
    }
    catch (const std::runtime_error& error)
    {
        log_error(error.what());
        throw Error();
    }
}

void Parser::parse_error_page()
{
    consume(TokenType::Number, "Expected status code.");
    consume(TokenType::Path, "Expected path after status code(s).");
    if (!is_valid_file_path(previous().m_str))
    {
        throw Parser::Error();
    }
    try
    {
        set_error_page(ErrorPage(m_tokens[m_current - 2].m_str, std::filesystem::path(previous().m_str)));
    }
    catch (const std::exception& error)
    {
        log_error(error.what());
        throw Error();
    }
}

void Parser::parse_listen()
{
    const TokenType next = peek().m_token_type;

    if (next == TokenType::Number)
    {
        consume(TokenType::Number, "Expected port number.");
        try
        {
            set_listen(Listen("127.0.0.1", previous().m_str));
        }
        catch (const std::runtime_error& error)
        {
            log_error(error.what());
            throw Error();
        }
        return ;
    }
    consume(TokenType::IPv4, "Expected IPv4 address.");
    if (peek().m_token_type == TokenType::Semicolon)
    {
        try
        {
            set_listen(Listen(previous().m_str));
        }
        catch (const std::runtime_error& error)
        {
            log_error(error.what());
            throw Error();
        }
        return ;
    }
    consume(TokenType::Colon, "Expected colon after IPv4 address.");
    consume(TokenType::Number, "Expected port number after IPv4 address.");
    try
    {
        set_listen(Listen(m_tokens[m_current - 3].m_str, previous().m_str));
    }
    catch (const std::runtime_error& error)
    {
        log_error(error.what());
        throw Error();
    }
}

void Parser::parse_location()
{
    consume(TokenType::Uri, "Expected URI after location.");
    try
    {
        set_location(LocationContext(previous().m_str));
    }
    catch (const std::exception& error)
    {
        log_error(error.what());
        throw Parser::Error();
    }
    push_context(ContextName::Location);
    parse_block();
    pop_context();
}

void Parser::parse_return()
{
    consume(TokenType::Number, "Expected status code.");
    consume(TokenType::Uri, "Expected URL after status code(s).");
    try
    {
        set_return(Return(m_tokens[m_current - 2].m_str, previous().m_str));
    }
    catch (const std::exception& error)
    {
        log_error(error.what(), previous());
        throw Parser::Error();
    }
}

void Parser::parse_autoindex()
{
    consume({TokenType::On, TokenType::Off}, "Expected 'on' or 'off'.");
    try
    {
        set_auto_index(AutoIndex(previous().m_str));
    }
    catch (const std::exception& error)
    {
        log_error(error.what(), previous());
        throw Parser::Error();
    }
}

void Parser::parse_root()
{
    consume(TokenType::Path, "Expected path after root.");
    if (!is_valid_dir_path(previous().m_str))
    {
        throw Parser::Error();
    }
    try
    {
        set_root(Root(previous().m_str));
    }
    catch (const std::exception& error)
    {
        log_error(error.what(), previous());
        throw Parser::Error();
    }
}

void Parser::parse_client_max_body_size()
{
    consume(TokenType::Number, "Expected number after client_max_body_size.");
    try
    {
        set_client_max_body_size(ClientMaxBodySize(previous().m_str));
    }
    catch (const std::exception& error)
    {
        log_error(error.what(), previous());
        throw Parser::Error();
    }
}

void Parser::parse_server()
{
    try
    {
        set_server(ServerContext());
    }
    catch (const std::exception& error)
    {
        log_error(error.what());
        throw Parser::Error();
    }
    push_context(ContextName::Server);
    parse_block();
    pop_context();
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
    push_context(ContextName::Http);
    advance();
    parse_block();
    pop_context();
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

void Parser::push_context(ContextName context)
{
    m_contexts.push(context);
}

void Parser::pop_context(void)
{
    m_contexts.pop();
}

static const std::string context_forbidden = "Keyword not allowed in current context";

void Parser::set_server_name(ServerName server_name)
{
    const ContextName context = m_contexts.top();

    switch (context)
    {
        case ContextName::Server:
            add_to_context<ServerName>(m_config.get_server().m_server_name, server_name);
            return;
        default:
            throw std::runtime_error(context_forbidden);
    }
}

void Parser::set_error_page(ErrorPage error_page)
{
    const ContextName context = m_contexts.top();

    switch (context)
    {
        case ContextName::Http:
            add_to_context<ErrorPage>(m_config.m_http_context.m_error_pages, error_page);
            return;
        case ContextName::Server:
            add_to_context<ErrorPage>(m_config.get_server().m_error_pages, error_page);
            return;
        case ContextName::Location:
            add_to_context<ErrorPage>(m_config.get_server().get_location().m_error_pages, error_page);
            return;
    }
}

void Parser::set_listen(Listen listen)
{
    const ContextName context = m_contexts.top();

    switch (context)
    {
        case ContextName::Server:
            add_to_context<Listen>(m_config.get_server().m_listen, listen);
            return;
        default:
            throw std::runtime_error(context_forbidden);
    }
}
void Parser::set_location(LocationContext location)
{
    const ContextName context = m_contexts.top();

    switch (context)
    {
        case ContextName::Server:
            add_to_context<LocationContext>(m_config.get_server().m_location_contexts, location);
            return;
        // location in location is allowed according to NGINX but we won't implement that
        default:
            throw std::runtime_error(context_forbidden);
    }
}


void Parser::set_root(Root root)
{
    const ContextName context = m_contexts.top();

    switch (context)
    {
        case ContextName::Http:
            add_to_context<Root>(m_config.m_http_context.m_root, root);
            return;
        case ContextName::Server:
            add_to_context<Root>(m_config.get_server().m_root, root);
            return;
       case ContextName::Location:
            add_to_context<Root>(m_config.get_server().get_location().m_root, root);
            return;
    }
}

void Parser::set_client_max_body_size(ClientMaxBodySize client_max_body_size)
{
    const ContextName context = m_contexts.top();

    switch (context)
    {
        case ContextName::Http:
            add_to_context<ClientMaxBodySize>(m_config.m_http_context.m_client_max_body_size, client_max_body_size);
            return;
        case ContextName::Server:
            add_to_context<ClientMaxBodySize>(m_config.get_server().m_client_max_body_size, client_max_body_size);
            return;
        case ContextName::Location:
            add_to_context<ClientMaxBodySize>(m_config.get_server().get_location().m_client_max_body_size, client_max_body_size);
            return;
        }
}

void Parser::set_server(ServerContext server)
{
    const ContextName context = m_contexts.top();

    switch (context)
    {
        case ContextName::Http:
            add_to_context<ServerContext>(m_config.m_servers, server);
            return;
        default:
            throw std::runtime_error(context_forbidden);
    }
}

void Parser::set_return(Return return_obj)
{
    const ContextName context = m_contexts.top();

    switch (context)
    {
        case ContextName::Server:
            add_to_context<Return>(m_config.get_server().m_return, return_obj);
            return;
        case ContextName::Location:
            add_to_context<Return>(m_config.get_server().get_location().m_return, return_obj);
            return;
        default:
            throw std::runtime_error(context_forbidden);
        }
}

void Parser::set_auto_index(AutoIndex auto_index)
{
    const ContextName context = m_contexts.top();

    switch (context)
    {
        case ContextName::Http:
            add_to_context<AutoIndex>(m_config.m_http_context.m_auto_index, auto_index);
            return;
        case ContextName::Server:
            add_to_context<AutoIndex>(m_config.get_server().m_auto_index, auto_index);
            return;
        case ContextName::Location:
            add_to_context<AutoIndex>(m_config.get_server().get_location().m_auto_index, auto_index);
            return;
    }
}
