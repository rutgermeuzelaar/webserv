/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Lexer.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/01 16:41:23 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/11 18:04:49 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
# define LEXER_HPP

enum class TokenType: int
{
    OpenBrace, // {
    CloseBrace, // }
    Semicolon, // ;
    Path, // path or uri, begins with /
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
    Listen,
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