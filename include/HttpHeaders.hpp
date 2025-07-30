#ifndef HTTP_HEADERS_HPP
# define HTTP_HEADERS_HPP
# include <tuple>
# include <unordered_map>
# include <string>

class HttpHeaders
{
    private:
        size_t m_line_break_count;
        std::unordered_map<std::string, std::string> m_headers;

    public:
        HttpHeaders(void);
        HttpHeaders(const HttpHeaders&);
        HttpHeaders& operator=(const HttpHeaders&) = default;
        ~HttpHeaders(void);

        /**
         * @brief retrieves the value of a specific HTTP header
         * @param key name of the header to retrieve
         * @note made headers lowcase for handling case insensitivity
         * @return The value of the requested header, or empty string if not found
         */
        const std::string get_header(const std::string& header_name) const;
        const std::unordered_map<std::string, std::string>& get_headers() const;

        void set_headers(const std::unordered_map<std::string, std::string>& headers);
        void add_header(const std::string& raw_string);
        void add_header(const std::string& key, const std::string& value);
        bool complete(void) const;
};

std::tuple<std::string, std::unordered_map<std::string, std::string>> parse_content_type(const std::string&);
std::tuple<std::string, std::unordered_map<std::string, std::string>> parse_content_disposition(const std::string&);
#endif

