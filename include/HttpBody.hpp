#ifndef HTTP_BODY_HPP
# define HTTP_BODY_HPP
# include <istream>
# include <string>
# include <unordered_map>
# include <vector>
# include <optional>
# include "HttpMethod.hpp"
# include "HttpHeaders.hpp"
# include "MultiPartChunk.hpp"

class HttpBody
{
    private:
        std::string m_raw;
        size_t      m_index;
        size_t      m_data_start;
        std::optional<std::string> m_boundary;
        ulong m_content_length;
        size_t m_client_max_body_size;
        bool m_initialized;
        bool m_complete;
        MultiPartChunk m_chunk;
        const HttpHeaders* m_headers;
        void parse(void);
    public:
        HttpBody(void);
        HttpBody(const HttpHeaders* headers, HTTPMethod method, size_t client_max_body_size);
        HttpBody(const HttpBody&);
        HttpBody& operator=(const HttpBody&) = default;
        bool complete(void) const;
        bool initialized(void) const;
        void append(const std::string&);
        const std::string& get_raw(void) const;
        const MultiPartChunk& get_multi_part_chunk(void) const;
};
#endif