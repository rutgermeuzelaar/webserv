/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/08 16:56:24 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/08 20:11:08 by rmeuzela      ########   odam.nl         */
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

bool Parser::consume(TokenType type, const char *error)
{
    if (peek().m_token_type == type)
    {
        advance();
        return (true);
    }
    throw std::runtime_error(error);
    return (false);
}

bool Parser::parse_server_name()
{
    consume(TokenType::String, "Expected string after server_name.");
}

bool Parser::parse_statement()
{
    const TokenType current = advance().m_token_type;
    
    switch (current)
    {
        case TokenType::ServerName:
            
    }
}

bool Parser::parse_block()
{
    if (consume(TokenType::OpenBrace, "Expected '{'."))
    {
        return (false);
    }
    while (!at_end() && !match({TokenType::CloseBrace}))
    {
        if (!parse_statement())
        {
            return (false);
        };
    }
    if (consume(TokenType::CloseBrace, "Expected '}'."))
    {
        return (false);
    }
    return (true);
}

void Parser::parse()
{
    const int   len = m_tokens.size();
    TokenType   current;
    int         open_braces;

    open_braces = 0;
    if (peek().m_token_type != TokenType::Http)
    {
        throw std::runtime_error("Config file should start with a http block.");
    }
}
