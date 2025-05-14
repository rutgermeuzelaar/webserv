/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Lexer.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/02 12:33:07 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/14 17:45:01 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <algorithm>
#include "Lexer.hpp"

Token::Token(TokenType token_type)
    : m_token_type {token_type}
{
    
}

Token::Token(TokenType token_type, std::string str)
    : m_token_type {token_type}
    , m_str {str}
{

}

static const std::string stringify(TokenType type)
{
	switch (type)
	{
		case TokenType::OpenBrace: return "OpenBrace";
		case TokenType::CloseBrace: return "CloseBrace";
		case TokenType::Semicolon: return "Semicolon";
		case TokenType::Path: return "Path";
		case TokenType::String: return "String";
		case TokenType::Number: return "Number";
		case TokenType::ClientMaxBodySize: return "ClientMaxBodySize";
		case TokenType::ErrorPage: return "ErrorPage";
		case TokenType::Http: return "Http";
		case TokenType::Server: return "Server";
		case TokenType::ServerName: return "ServerName";
		case TokenType::Root: return "Root";
		case TokenType::Location: return "Location";
		case TokenType::Listen: return "Listen";
		case TokenType::Eof: return "Eof";
		default: assert(false);
	}
}

void Token::print() const
{
	const std::vector<TokenType> stringless = {
		TokenType::OpenBrace,
		TokenType::CloseBrace,
		TokenType::Semicolon
	};
	std::cout << "\"" << stringify(m_token_type) << \
	"\", \"";
	if (m_token_type == TokenType::Eof || \
		std::find(stringless.begin(), stringless.end(), m_token_type) \
		!= stringless.end())
	{
		switch (m_token_type)
		{
			case TokenType::OpenBrace:
				std:: cout << '{';
				break;
			case TokenType::CloseBrace:
				std::cout << '}';
				break;
			case TokenType::Semicolon:
				std::cout << ';';
				break;
			default:
				break;
		}
	}
	else
	{
		std::cout << m_str;
	}
	std::cout << "\"\n";
}