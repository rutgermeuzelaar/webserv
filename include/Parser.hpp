/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/08 16:54:16 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/18 20:30:09 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP
# include <stdexcept>
# include <vector>
# include "Lexer.hpp"
# include "Expression.hpp"

class Parser
{
    public:
        class                       Error;
        Parser(const std::vector<Token>& tokens);
        void parse(void);

    private:
        const std::vector<Token>&   m_tokens;
        int                         m_current;
        bool                        at_end(void);
        bool                        match(const std::vector<TokenType>);
        bool                        check(TokenType);
        Token                       advance(void);
        const Token&                previous(void) const;
        Token                       consume(TokenType, const char *error);
        Token                       peek(void) const;
        void                        log_error(const char*) const;
        void                        parse_block(void);
        void                        parse_statement(void);
        
        // config options
        void                        parse_server_name(void);
        void                        parse_error_page(void);
        void                        parse_listen(void);
        void                        parse_location(void);
        void                        parse_root(void);
        void                        parse_client_max_body_size(void);
        void                        parse_server(void);
};

class Parser::Error: public std::runtime_error
{
    public:
        Error();
};
#endif