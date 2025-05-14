/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/08 16:56:24 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/09 17:23:14 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include "Parser.hpp"

Parser::Parser(const std::vector<Token>& tokens)
    : m_tokens {tokens}
    , m_current {0}
{
    
}

bool Parser::at_end()
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

Token Parser::peek()
{
    return (m_tokens[m_current]);
}

Token Parser::advance()
{
    return m_tokens[m_current++];   
}

Token Parser::consume(TokenType type, const char *error)
{
    if (peek().m_token_type == type)
    {
        return (advance());
    }
    throw std::runtime_error(error);
}

void Parser::parse_server_name()
{
    consume(TokenType::String, "Expected string after server_name.");
}

void Parser::parse_error_page()
{
    while (check(TokenType::Number))
    {
        advance();
    }
    consume(TokenType::String, "Expected string after status code(s).");
}

void Parser::parse_listen()
{
    while (match({TokenType::Number, TokenType::String}))
    {
        advance();   
    }
}

void Parser::parse_location()
{
    consume(TokenType::String, "Expected string after location.");
    parse_block();
}

void Parser::parse_root()
{
    consume(TokenType::Path, "Expected path after root.");
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
        case TokenType::Location:
            parse_location();
            return;
        default:
            throw std::runtime_error("Unexpected token.");
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
        throw std::runtime_error("Config file should start with a http block.");
    }
    advance();
    parse_block();
    if (!at_end())
    {
        throw std::runtime_error("Expected end of file.");
    }
}
