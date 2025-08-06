#include "Pch.hpp"
#include <fstream>
#include <cassert>
#include "Utilities.hpp"
#include "Defines.hpp"
#include "Http.hpp"

/**
 * @brief parses the request body based on Content-Length header
 * @param stream string stream containing the complete HTTP request including headers and body
 * @note stream should contain the entire request data, as we need to skip headers to reach the body
 * @note content-length header is required for POST requests to determine body size
 * @return true if parsing succeeded, false if Content-Length is invalid or body is incomplete
 */

HttpBody::HttpBody(const HttpHeaders* headers, HTTPMethod method, size_t client_max_body_size)
	: PartialWriter()
    , m_index {0}
	, m_data_start {0}
	, m_content_length {0}
	, m_client_max_body_size {client_max_body_size}
	, m_initialized {false}
	, m_complete {false}
	, m_headers {headers}
	, m_is_chunked {false}
    , m_content_type {""}
{
	switch (method)
	{
	case HTTPMethod::POST:
		break;
	default:
		m_complete = true;
		return;
	}
    //* parse and store Content-Type and multipart state first
    const std::string content_type_header = headers->get_header("content-type");
    if (content_type_header.empty())
        throw HTTPException(HTTPStatusCode::BadRequest);
    m_content_type = content_type_header;
    auto content_type = parse_content_type(content_type_header);
    if (headers->get_header("transfer-encoding") == "chunked") 
	{
        if (!headers->get_header("content-length").empty())
            throw HTTPException(HTTPStatusCode::BadRequest, "Chunked Encoding cannot have Content length");
        m_is_chunked = true;
        m_chunked_decoder = ChunkedDecoder();
        m_initialized = true;
        return;
    }
    const std::string content_len = headers->get_header("content-length");
    if (content_len.empty())
        throw HTTPException(HTTPStatusCode::BadRequest);
    try {
        m_content_length = std::stoul(content_len);
    } catch (const std::exception& e) {
        throw HTTPException(HTTPStatusCode::BadRequest, "Invalid Content-Length value");
    }
    if (std::get<std::string>(content_type) == "multipart/form-data")
    {
        const auto& attributes = std::get<std::unordered_map<std::string, std::string>>(content_type);
        auto bound = attributes.find("boundary");
        if (bound == attributes.end())
            throw HTTPException(HTTPStatusCode::BadRequest);
		m_boundary.emplace(bound->second);
        if (m_content_length == 0)
            throw HTTPException(HTTPStatusCode::BadRequest);
        m_initialized = true;
    }
    else if (std::get<std::string>(content_type) == "application/x-www-form-urlencoded")
    {
        m_initialized = true;
    }
}

HttpBody::HttpBody(void)
	: PartialWriter()
    , m_index {0}
	, m_data_start {0}
	, m_content_length {0}
	, m_client_max_body_size {0}
	, m_initialized {false}
	, m_complete {false}
	, m_headers {nullptr}
	, m_is_chunked{false}
{}

HttpBody::HttpBody(const HttpBody& http_body)
    : PartialWriter(http_body)
    , m_index {http_body.m_index}
    , m_data_start {http_body.m_data_start}
    , m_boundary {http_body.m_boundary}
    , m_content_length {http_body.m_content_length}
    , m_client_max_body_size {http_body.m_client_max_body_size}
    , m_initialized {http_body.m_initialized}
    , m_complete {http_body.m_complete}
    , m_chunk {http_body.m_chunk}
    , m_headers {http_body.m_headers}
	, m_is_chunked {http_body.m_is_chunked}
	, m_chunked_decoder {http_body.m_chunked_decoder}
{}

static std::string get_normal_boundary(const std::string& boundary)
{
	std::string normal_boundary = "--";
	normal_boundary.append(boundary);
	normal_boundary.append(LINE_BREAK);
	return normal_boundary;
}

static std::string get_final_boundary(const std::string& boundary)
{
	std::string final_boundary = "--";
	final_boundary.append(boundary);
	final_boundary.append("--" LINE_BREAK);
	return final_boundary;
}

bool HttpBody::complete(void) const
{
	if (m_complete)
	{
		return (true);
	}
	return (false);
}

void HttpBody::append(const std::string& chunk)
{
	if (m_is_chunked)
	{
		std::cout << "[DEBUG] First 20 bytes of chunk: ";
		for (size_t i = 0; i < std::min(chunk.size(), size_t(20)); ++i) {
			unsigned char c = chunk[i];
			if (std::isprint(c))
				std::cout << c;
			else
				std::cout << "\\x" << std::hex << (int)c << std::dec;
		}
		std::cout << std::endl;
		std::cout << "[DEBUG] m_complete at start of append: " << m_complete << std::endl;
		m_chunked_decoder.append(chunk);
		
		const std::string& current_decoded = m_chunked_decoder.get_decoded();
		if (current_decoded.size() > m_client_max_body_size)
			throw HTTPException(HTTPStatusCode::ContentTooLarge);
		if (m_chunked_decoder.complete())
		{
			m_raw = current_decoded;
			std::cout <<"[HttpBody::append] chunk size: " << chunk.size() << ", m_raw size: " << m_raw.size() << std::endl;
			m_complete = true;
			if (m_content_type.find("multipart/form-data") != std::string::npos)
				parse();
		}
	}
	else
	{
		if ((m_raw.size() + chunk.size()) > m_client_max_body_size)
			throw HTTPException(HTTPStatusCode::ContentTooLarge);
		m_raw.append(chunk);
		if (m_raw.size() >= m_content_length)
		{
			m_complete = true;
			if (m_content_type.find("multipart/form-data") != std::string::npos)
				parse();
		}
	}
}

const std::string& HttpBody::get_raw(void) const
{
	return m_raw;
}

const MultiPartChunk& HttpBody::get_multi_part_chunk(void) const
{
	return m_chunk;
}

bool HttpBody::initialized() const
{
	return m_initialized;
}

// only multipart/form-data
void HttpBody::parse(void)
{
	// make up something better
	if (!m_boundary.has_value()) {
        std::cerr << "Error: boundary not set in multipart/form-data" << std::endl;
        throw HTTPException(HTTPStatusCode::BadRequest, "Missing boundary in multipart/form-data");
    }
	const std::string normal_boundary = get_normal_boundary(m_boundary.value());
	const std::string final_boundary = get_final_boundary(m_boundary.value());

	size_t line_break_pos = m_raw.find(LINE_BREAK, m_index);
	while (line_break_pos != std::string::npos)
	{
		if (line_break_pos > m_raw.size())
		{
			return;
		}
		// get str
		std::string str = m_raw.substr(m_index, line_break_pos - m_index + 2);
		// check if it's a boundary
		if (str == normal_boundary)
		{
			// add new chunk
		}
		else if (m_chunk.m_headers.complete())
		{
			// store raw data
			if (ends_with(str, final_boundary))
			{
				// victory
				m_chunk.m_data = m_raw.substr(m_data_start, m_raw.size() - final_boundary.size() - m_data_start);
				m_complete = true;
				return;
			}
		}
		else
		{
			m_chunk.m_headers.add_header(str);
			if (m_chunk.m_headers.complete())
			{
				m_data_start = m_index + 2;
				m_index += 2;
				m_chunk.parse_header_attributes();
			}
		}
		m_index += str.size();
		line_break_pos = m_raw.find(LINE_BREAK, m_index);
	}
}

//------------------------------------------------------------------------------//
//* Encoded Chunk Class*//

ChunkedDecoder::ChunkedDecoder()
	: m_state(READING_SIZE)
	, m_expected_chunk_size(0)
	, m_current_chunk_read(0)
    , m_complete(false)
{}

void ChunkedDecoder::append(const std::string& data)
{
	m_buffer.append(data);
    std::cout << "[DEBUG] Appending data, buffer size now: " << m_buffer.size() << std::endl;
	std::cout << "[DEBUG]m_complete: " << m_complete << std::endl;
	// m_complete = false; //? WHY DOES IT TURN INTO TRUE IN THIS FUNCTION?!
	
	size_t pos = 0;
	while (!m_complete && pos < m_buffer.size())
	{
		std::cout << "going in this loop" << std::endl;
		switch (m_state)
		{
		case READING_SIZE:
			if (parse_chunk_size(pos))
			{
				m_state = READING_DATA;
				m_current_chunk_read = 0;
                std::cout << "[DEBUG] Parsed chunk size: " << m_expected_chunk_size << std::endl;
			}
            [[fallthrough]];
		case READING_DATA:
			if (parse_chunk_data(pos))
			{
				if (m_expected_chunk_size == 0)
					m_state = READING_TRAILER;
				else
					m_state = READING_SIZE;
                std::cout << "[DEBUG] Finished reading chunk, decoded size: " << m_decoded.size() << std::endl;
			}
            [[fallthrough]];
		case READING_TRAILER:
			if (parse_trailer(pos))
			{
				m_state = COMPLETE;
				m_complete = true;
                    std::cout << "[DEBUG] Finished reading trailers." << std::endl;
			}
			break;

		case COMPLETE:
			return;
		}
	}
	if (pos > 0)
		m_buffer.erase(0, pos);
	std::cout << "[ChunkedDecoder] m_decoded size: " << m_decoded.size() << std::endl;
}

/**
 * @brief Parses the next chunk size line from the buffer.
 *        Handles optional chunk extensions (after ';').
 *        On success, updates m_expected_chunk_size and advances pos past CRLF.
 * @param pos Reference to current buffer position.
 * @return true if a complete size line was parsed, false if more data is needed.
 * @throws HTTPException on invalid or empty chunk size.
 */
bool ChunkedDecoder::parse_chunk_size(size_t& pos)
{
	size_t crlf_pos = m_buffer.find(LINE_BREAK, pos);
	if (crlf_pos == std::string::npos)
		return false;

	//* extract size line (no CRLF: \r\n)
	std::string size_line = m_buffer.substr(pos, crlf_pos - pos);

	//* chunk extensions (f.e. optional param after ;)
	size_t semicolon_pos = size_line.find(';');
	if (semicolon_pos != std::string::npos)
		size_line = size_line.substr(0, semicolon_pos);

	//* validate hexstring
	if (size_line.empty())
		throw HTTPException(HTTPStatusCode::BadRequest, "Empty chunk size");
	try
	{
		m_expected_chunk_size = hex_to_size(size_line);
        std::cout << "[DEBUG] parse_chunk_size: size_line='" << size_line << "', m_expected_chunk_size=" << m_expected_chunk_size << std::endl;
	}
	catch (const std::exception &)
	{
		throw HTTPException(HTTPStatusCode::BadRequest, "Invalid chunk size");
	}

	//* move pos past size line + CRLF
	pos = crlf_pos + 2;
	return true;
}

/**
 * @brief Reads chunk data from the buffer according to m_expected_chunk_size.
 *        Handles partial data and ensures CRLF after each chunk.
 *        Appends data to m_decoded.
 * @param pos Reference to current buffer position.
 * @return true if the entire chunk (including trailing CRLF) was read, false if more data is needed.
 * @throws HTTPException if CRLF is missing after chunk data.
 */
bool ChunkedDecoder::parse_chunk_data(size_t& pos)
{
	//* zero-sized chunk
	if (m_expected_chunk_size == 0)
	{
		if (pos + 2 <= m_buffer.size() && m_buffer.substr(pos, 2) == LINE_BREAK)
		{
			pos += 2;
            std::cout << "[DEBUG] parse_chunk_data: zero-sized chunk, pos now " << pos << std::endl;
			return true;
		}
		return false;
	}

	//* check how much data needed per chunk
	size_t bytes_needed = m_expected_chunk_size - m_current_chunk_read;
	size_t available_data = m_buffer.size() - pos;
    std::cout << "[DEBUG] parse_chunk_data: bytes_needed=" << bytes_needed << ", available_data=" << available_data << std::endl;

	//* all chunks read, only CRLF validation
	if (bytes_needed == 0)
	{
		//* at least 2 bytes available for /r/n
		if (available_data < 2)
			return false;
		if (m_buffer.substr(pos,2) != LINE_BREAK)
			throw HTTPException(HTTPStatusCode::BadRequest, "Missing CRLF after chunk data");
		pos += 2;
            std::cout << "[DEBUG] parse_chunk_data: chunk data complete, pos now " << pos << std::endl;
		return true;
	}

	//* not enough data for complete chunk + CRLF
	if (available_data < bytes_needed + 2)
	{
		size_t to_read = std::min(bytes_needed, available_data);
		if (to_read > 0)
		{
			m_decoded.append(m_buffer.substr(pos, to_read));
			m_current_chunk_read += to_read;
			pos += to_read;
            std::cout << "[DEBUG] parse_chunk_data: partial read, to_read=" << to_read << ", m_current_chunk_read=" << m_current_chunk_read << std::endl;
		}
		return false;
	}

	//* enough data, read remainder data
	m_decoded.append(m_buffer.substr(pos, bytes_needed));
	pos += bytes_needed;
    std::cout << "[DEBUG] parse_chunk_data: full read, bytes_needed=" << bytes_needed << ", pos now " << pos << std::endl;

	//* CRLF check after chunk data
	if (m_buffer.substr(pos, 2) != LINE_BREAK)
		throw HTTPException(HTTPStatusCode::BadRequest, "Missing CRLF after chunk data");

	pos += 2;
	return true;
}

/**
 * @brief Parses trailer headers after the last (zero-sized) chunk.
 *        Advances pos past each trailer line and CRLF.
 *        Returns true when an empty line (end of trailers) is found.
 *        Currently, trailer headers are not processed.
 * @param pos Reference to current buffer position.
 * @return true if trailers are complete, false if more data is needed.
 */
bool ChunkedDecoder::parse_trailer(size_t& pos)
{
	while (pos < m_buffer.size())
	{
		size_t crlf_pos = m_buffer.find(LINE_BREAK, pos);
		if (crlf_pos == std::string::npos)
			return false;
	
		std::string trailer_line = m_buffer.substr(pos, crlf_pos - pos);
		if (trailer_line.empty())
		{
			pos += 2;
			return true;
		}
		//? implement process trailer headers?
		pos = crlf_pos + 2;
	}
	return false;
}

size_t ChunkedDecoder::hex_to_size(const std::string& hex_str)
{
	//* remove ws
	std::string clean_hex = hex_str;
	clean_hex = trim(clean_hex, WHITE_SPACE);

	//* validate hex
	for (char c : clean_hex)
		if (!std::isxdigit(c))
			throw HTTPException(HTTPStatusCode::BadRequest, "Invalid hex char");

	//* convert hex to size_t
	std::stringstream ss;
	size_t result;
	ss << std::hex << clean_hex;
	ss >> result;
	if (ss.fail())
		throw HTTPException(HTTPStatusCode::BadRequest, "Hex conversion failed");
	return result;
}

bool ChunkedDecoder::complete() const
{
	return m_complete;
}

const std::string &ChunkedDecoder::get_decoded() const
{
	return m_decoded;
}

bool HttpBody::is_chunked(void) const
{
    return m_is_chunked;
}

const ChunkedDecoder& HttpBody::get_chunked_decoder(void) const
{
    return m_chunked_decoder;
}
