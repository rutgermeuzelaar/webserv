/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/08 16:54:16 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/21 16:24:00 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP
# include <stdexcept>
# include <vector>
# include <optional>
# include "HttpConfig.hpp"
# include "Lexer.hpp"
# include "Expression.hpp"

class Parser
{
    public:
        class                       Error;
        Parser(const std::vector<Token>& tokens, HttpConfig& http_config);
        void parse(void);

    private:
        const std::vector<Token>&   m_tokens;
		HttpConfig&					m_http_config;
        int                         m_current;
        bool                        at_end(void);
        bool                        at_end(void) const;
        bool                        match(const std::vector<TokenType>);
        bool                        check(TokenType);
        const Token&                advance(void);
        const Token&                previous(void) const;
        const Token&                consume(TokenType, const char *error);
        const Token&                peek(void) const;
        const Token&                next(void) const;
        void                        log_error(const std::string, const Token&) const;
        void                        log_error(const std::string) const;
        void                        parse_block(void);
        void                        parse_statement(void);
        bool                        is_valid_dir_path(const std::string) const;
        bool                        is_valid_file_path(const std::string) const;
       
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