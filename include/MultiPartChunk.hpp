#ifndef MULTI_PART_CHUNK_HPP
# define MULTI_PART_CHUNK_HPP
# include <string>
# include <tuple>
# include <unordered_map>
# include "HttpHeaders.hpp"

class MultiPartChunk
{
    private:
        std::string m_mime_type;
        std::tuple<std::string, std::unordered_map<std::string, std::string>> m_content_disposition;
    public:
        MultiPartChunk(void);
        MultiPartChunk& operator=(const MultiPartChunk&);
        ~MultiPartChunk(void);
        std::string m_data;
        HttpHeaders m_headers;
        void parse_header_attributes(void);
        const std::string& get_mime_type(void) const;
};
#endif