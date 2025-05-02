/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Lexer.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/02 12:33:07 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/02 12:34:28 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string>
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