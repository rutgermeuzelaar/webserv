/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/08 16:54:16 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/08 20:07:11 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP
# include <vector>
# include "Lexer.hpp"
# include "Expression.hpp"

class Parser
{
    private:
        const std::vector<Token>&   m_tokens;
        int                         m_current;
        bool                        at_end(void);
        bool                        match(const std::vector<TokenType>);
        bool                        check(TokenType);
        Token                       advance(void);
        Token                       previous(void);
        bool                        consume(TokenType, const char *error);
        Token                       peek(void);
        bool                        parse_block(void);
        bool                        parse_statement(void);

        bool                        parse_server_name(void);
    public:
        Parser(const std::vector<Token>& tokens);
        void parse(void);
};
#endif