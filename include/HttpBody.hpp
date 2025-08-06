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
# include "PartialWriter.hpp"

class ChunkedDecoder 
{
private:
	enum State {
		READING_SIZE,
		READING_DATA,
		READING_TRAILER,
		COMPLETE
	};

	State m_state;
	size_t m_expected_chunk_size;
	size_t m_current_chunk_read;
	std::string m_buffer;
	std::string m_decoded;
	bool m_complete;

	bool parse_chunk_size(size_t& pos);
	bool parse_chunk_data(size_t& pos);
	bool parse_trailer(size_t& pos);

public:
	ChunkedDecoder();
	void append(const std::string& data);
	bool complete() const;
	size_t hex_to_size(const std::string& hex_str);

	const std::string& get_decoded() const;
};

class HttpBody: public PartialWriter
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
		bool m_is_chunked;
        std::string m_content_type;
		ChunkedDecoder m_chunked_decoder;

        void parse(void);
    public:
        HttpBody(void);
        HttpBody(const HttpHeaders* headers, HTTPMethod method, size_t client_max_body_size);
        HttpBody(const HttpBody&);
        HttpBody& operator=(const HttpBody&) = default;

        bool complete(void) const;
        bool initialized(void) const;
        void append(const std::string&);
        bool is_chunked(void) const;

        const std::string& get_raw(void) const;
        const MultiPartChunk& get_multi_part_chunk(void) const;
        const ChunkedDecoder& get_chunked_decoder(void) const;
};

#endif