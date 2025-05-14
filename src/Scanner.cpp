/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Scanner.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/01 17:11:15 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/14 17:58:02 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include <cctype>
#include "Scanner.hpp"
#include "ConfigFile.hpp"

Scanner::Scanner(std::string in)
    : m_index {0} 
	, m_linenum {0}
    , m_in {in}
    , m_inlen {in.length()}
{
    
}

void Scanner::skip_whitespace()
{
    while (m_in[m_index] && std::isspace(m_in[m_index]))
    {
        m_index++;
    }
}

// empty string would overflow
char Scanner::advance()
{
    return (m_in[m_index++]);
}

char Scanner::peek() const
{
    if (at_end())
    {
        return ('\0');
    }
    return (m_in[m_index]);
}

void Scanner::add_token(TokenType token_type)
{
    m_tokens.push_back(Token(token_type));
}

void Scanner::add_token(TokenType token_type, std::string str)
{
    m_tokens.push_back(Token(token_type, str));
}

void Scanner::scan_string()
{
    const bool is_path = m_in[m_index] == '/';
    const std::string single_token("{};");
    const size_t start = m_index;
    std::string substr;
    const std::string keyword;
    std::map<std::string, TokenType>::const_iterator it;

    while (!at_end() && !std::isspace(m_in[m_index]) && \
    single_token.find(m_in[m_index]) == std::string::npos)
    {
        advance();
    }
    substr = m_in.substr(start - 1, m_index - start + 1);
    it = keywords.find(substr);
    if (is_path)
    {
        add_token(TokenType::Path, substr);
    }
    else if (it != keywords.end())
    {
        add_token(it->second, it->first);
    }
    else
    {
        add_token(TokenType::String, substr);
    }
}

void Scanner::scan_number()
{
    const size_t start = m_index;
    std::string substr;
    
    while (!at_end() && std::isdigit(m_in[m_index]) && !std::isspace(m_in[m_index]))
    {
        advance();
    }
    substr = m_in.substr(start - 1, m_index - start + 1);
    add_token(TokenType::Number, substr);
}

void Scanner::scan_token()
{
    const char glyph = advance();

    switch (glyph)
    {
        case '{':
            add_token(TokenType::OpenBrace);
            break;
        case '}':
            add_token(TokenType::CloseBrace);
            break;
        case ';':
            add_token(TokenType::Semicolon);
            break;
        case '\0':
            add_token(TokenType::Eof);
            break;
        default:
            if (std::isdigit(glyph))
            {
                scan_number();
            }
            else
            {
                scan_string();
            }
    }
}

bool Scanner::at_end() const
{
    return (m_index == m_inlen);
}

const std::vector<Token>& Scanner::scan()
{
    if (m_inlen == 0)
    {
        throw std::runtime_error("Configuration file cannot be empty.");
    }
    while (!at_end())
    {
        skip_whitespace();
        scan_token();
    }
	add_token(TokenType::Eof);
    return m_tokens;
}

void Scanner::print_tokens(void) const
{
	const size_t	size = m_tokens.size();
	size_t			i;
	
	i = 0;
	while (i < size)
	{
		m_tokens[i].print();
		i++;
	}
}