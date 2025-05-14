/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Scanner.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/01 16:57:18 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/14 17:47:00 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCANNER_HPP
# define SCANNER_HPP
# include <string>
# include <vector>
# include "Lexer.hpp"

class Scanner
{
    private:
        size_t m_index;
		size_t m_linenum;
        const std::string m_in;
        const size_t    m_inlen;
        char advance(void);
        char peek(void) const;
        void skip_whitespace(void);
        std::vector<Token> m_tokens;
        void scan_token(void);
        void scan_string(void);
        void scan_number(void);
        void add_token(TokenType);
        void add_token(TokenType, std::string);
        TokenType get_token_type(const std::string& str);
        bool at_end(void) const;
    public:
        Scanner(std::string);
        TokenType get_token(void);
        const std::vector<Token>& scan(void);
		void print_tokens(void) const;
};
#endif