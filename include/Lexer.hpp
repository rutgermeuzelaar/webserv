/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Lexer.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/01 16:41:23 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/22 15:43:14 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
# define LEXER_HPP

enum class TokenType: int
{
    OpenBrace, // {
    CloseBrace, // }
    Semicolon, // ;
    Colon, // :
    Path,
	Uri,
    String,
    Number,
	IPv4,
	On,
	Off,

    // HTTP methods
    Get,
    Post,
    Delete,

    // NGINX keywords
    ClientMaxBodySize,
    ErrorPage,
    Http,
    Server,
    ServerName,
    Root,
    Location,
    Listen,
	Return,
	AutoIndex,
    Index,
    LimitExcept,

    Eof,
	TokenTypeCount
};

class Token
{
    public:
        const int m_linenum;
        TokenType m_token_type;
        std::string m_str;
        Token(TokenType, int);
        Token(TokenType, std::string, int);
		void print(void) const;
};

std::string stringify(TokenType type);
#endif