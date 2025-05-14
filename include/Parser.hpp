/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/08 16:54:16 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/09 17:13:28 by rmeuzela      ########   odam.nl         */
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
        Token                       consume(TokenType, const char *error);
        Token                       peek(void);
        void                        parse_block(void);
        void                        parse_statement(void);

        // config options
        void                        parse_server_name(void);
        void                        parse_error_page(void);
        void                        parse_listen(void);
        void                        parse_location(void);
        void                        parse_root(void);
    public:
        Parser(const std::vector<Token>& tokens);
        void parse(void);
};
#endif