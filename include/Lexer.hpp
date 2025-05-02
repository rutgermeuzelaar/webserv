/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Lexer.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/01 16:41:23 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/02 12:32:22 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
# define LEXER_HPP

enum class TokenType: int
{
    OpenBrace, // {
    CloseBrace, // }
    Semicolon, // ;
    String,
    Number,

    // NGINX keywords
    ClientMaxBodySize,
    ErrorPage,
    Http,
    Server,
    ServerName,
    Root,
    Location,
    Eof
};

class Token
{
    public:
        TokenType m_token_type;
        std::string m_str;
        Token(TokenType);
        Token(TokenType, std::string);
};
#endif