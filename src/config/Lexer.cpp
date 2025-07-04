#include "Pch.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <algorithm>
#include "Lexer.hpp"

const std::string single_token_char(TokenType TokenType)
{
	switch (TokenType)
	{
		case TokenType::OpenBrace:
			return "{";
		case TokenType::CloseBrace:
			return "}";
		case TokenType::Semicolon:
			return ";";
		case TokenType::Colon:
			return ":";
		default:
			return "";
	}
}

Token::Token(TokenType token_type, int linenum)
	: m_linenum {linenum}
    , m_token_type {token_type}
	, m_str {single_token_char(token_type)}
{
    
}

Token::Token(TokenType token_type, std::string str, int linenum)
	: m_linenum {linenum}
    , m_token_type {token_type}
    , m_str {str}
{

}

std::string stringify(TokenType type)
{
	switch (type)
	{
		case TokenType::OpenBrace: return "OpenBrace";
		case TokenType::CloseBrace: return "CloseBrace";
		case TokenType::Semicolon: return "Semicolon";
		case TokenType::Colon: return "Colon";
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
		case TokenType::IPv4: return "IPv4";
		case TokenType::Uri: return "Uri";
		case TokenType::AutoIndex: return "AutoIndex";
		case TokenType::Return: return "Return";
		case TokenType::On: return "On";
		case TokenType::Off: return "Off";
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