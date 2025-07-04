#ifndef SCANNER_HPP
# define SCANNER_HPP
# include <optional>
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
		bool scan_on_off(void);
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