/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Lexer.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/01 16:41:23 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/21 17:31:43 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
# define LEXER_HPP

enum class TokenType: int
{
    OpenBrace, // {
    CloseBrace, // }
    Semicolon, // ;
    Path,
	Uri,
    String,
    Number,
	IPv4,
	On,
	Off,

    // NGINX keywords
    ClientMaxBodySize,
    ErrorPage,
    Http,
    Server,
    ServerName,
    Root,
    Location,
    Listen,
    Eof,
	Return,
	AutoIndex,
	//
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