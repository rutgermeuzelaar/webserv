/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Expression.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/06 13:47:00 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/08 17:24:48 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXPRESSION_HPP
# define EXPRESSION_HPP
# include "Lexer.hpp"

class Expression
{

};

// (server_name | location) Literal
class ConfigOption: public Expression
{
    
};

// Literal / Primary
// webserv
// NUMBER | STRING | on | off | ( expression )
class Literal: public Expression
{
    
};

#endif