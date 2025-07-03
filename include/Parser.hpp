/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Parser.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/08 16:54:16 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 14:20:58 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP
# include <stdexcept>
# include <vector>
# include <stack>
# include <optional>
# include "HttpContext.hpp"
# include "Lexer.hpp"
# include "Config.hpp"
# include "ContextName.hpp"

class Parser
{
    public:
        class                       Error;
        Parser(const std::vector<Token>& tokens, Config& config);
        void parse(void);

    private:
        const std::vector<Token>&   m_tokens;
        std::stack<ContextName>     m_contexts;
        void                        push_context(ContextName);
        void                        pop_context(void);
		Config&                     m_config;
        int                         m_current;
        bool                        at_end(void);
        bool                        at_end(void) const;
        bool                        match(const std::vector<TokenType>);
        bool                        check(TokenType);
        const Token&                advance(void);
        const Token&                previous(void) const;
        const Token&                consume(TokenType, const char *error);
        const Token&                consume(std::vector<TokenType>, const char *error);
        const Token&                peek(void) const;
        const Token&                next(void) const;
        void                        log_error(const std::string, const Token&) const;
        void                        log_error(const std::string) const;
        void                        parse_block(void);
        void                        parse_statement(void);
        bool                        is_valid_dir_path(const std::string) const;
        bool                        is_valid_file_path(const std::string) const;
        
        void                        parse_server_name(void);
        void                        parse_error_page(void);
        void                        parse_listen(void);
        void                        parse_location(void);
        void                        parse_root(void);
        void                        parse_client_max_body_size(void);
        void                        parse_server(void);
        void                        parse_return(void);
        void                        parse_autoindex(void);
        void                        parse_index(void);
        void                        parse_limit_except(void);
        void                        parse_upload_store(void);

        void                        set_server_name(ServerName);
        void                        set_error_page(ErrorPage);
        void                        set_listen(Listen);
        void                        set_location(LocationContext);
        void                        set_root(Root);
        void                        set_client_max_body_size(ClientMaxBodySize);
        void                        set_server(ServerContext);
        void                        set_return(Return);
        void                        set_auto_index(AutoIndex);
        void                        set_index(Index);
        void                        set_limit_except(LimitExcept);
        void                        set_upload_store(UploadStore);
};

class Parser::Error: public std::runtime_error
{
    public:
        Error();
};

#endif