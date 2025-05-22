/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Scanner.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/01 17:11:15 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/22 16:18:47 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdexcept>
#include <cctype>
#include <sstream>
#include <utility>
#include "Scanner.hpp"
#include "ConfigFile.hpp"

Scanner::Scanner(std::string in)
    : m_index {0} 
	, m_linenum {1}
    , m_in {in}
    , m_inlen {in.length()}
{
    
}

void Scanner::skip_whitespace()
{
    while (!at_end() && std::isspace(m_in[m_index]))
    {
        if (m_in[m_index] == '\n')
        {
            m_linenum++;
        }
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
    m_tokens.push_back(Token(token_type, m_linenum));
}

void Scanner::add_token(TokenType token_type, std::string str)
{
    m_tokens.push_back(Token(token_type, str, m_linenum));
}

void Scanner::scan_string()
{
    const bool is_path = (m_in.compare(m_index - 1, 2, "./") == 0);
	const bool is_uri = (m_in.compare(m_index - 1, 1, "/") == 0);
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
	else if (is_uri)
	{
		add_token(TokenType::Uri, substr);
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

static size_t consume_octet(const std::string& str, size_t pos)
{
	const size_t len = str.length();
	size_t i;
	
	i = 0;
	while (i < len && i < 3 && std::isdigit(str.at(pos + i)))
	{
		i++;
	}
	return i;
}

static std::pair<bool, size_t> is_ipv4(const std::string& str, size_t pos)
{
	const size_t len = str.length();
	size_t octet_count;
	size_t octet_len;
	size_t i;
	
	i = pos;
	octet_count = 0;
	if (str.compare(pos, 10, "localhost") == 0)
	{
		return std::pair<bool, size_t>(true, 10);
	}
	while (i < len && octet_count < 4)
	{
		octet_len = consume_octet(str, i);
		if (octet_len == 0)
		{
			return std::pair<bool, size_t>(false, 0);
		}
		i += octet_len;
		if (str.at(i) == '.' && octet_count < 3)
		{
			i++;
		}
		else if (octet_count != 3)
		{
			return std::pair<bool, size_t>(false, 0);
		}
		octet_count++;
	}
	return (std::pair<bool, size_t>(true, i - pos));
}

void Scanner::scan_number()
{
    const size_t start = m_index;
	const std::pair<bool, size_t> ipv4 = is_ipv4(m_in, m_index - 1);
    std::string substr;

	if (ipv4.first)
	{
		while ((m_index - start) < ipv4.second - 1)
		{
			advance();
		}
		substr = m_in.substr(start - 1, ipv4.second);
		add_token(TokenType::IPv4, substr);
		return;
	}
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
        case ':':
            add_token(TokenType::Colon);
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
    if (m_index >= m_inlen)
    {
        return (true);
    }
    return (false);
}

const std::vector<Token>& Scanner::scan()
{
    if (at_end())
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
