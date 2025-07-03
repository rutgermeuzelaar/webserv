#include <fstream>
#include <sstream>
#include <cassert>
#include "HttpBody.hpp"
#include "HTTPException.hpp"
#include "Utilities.hpp"
#include "HttpHeaders.hpp"
#include "Defines.hpp"
#include <stdio.h>

/**
 * @brief parses the request body based on Content-Length header
 * @param stream string stream containing the complete HTTP request including headers and body
 * @note stream should contain the entire request data, as we need to skip headers to reach the body
 * @note content-length header is required for POST requests to determine body size
 * @return true if parsing succeeded, false if Content-Length is invalid or body is incomplete
 */

HttpBody::HttpBody(const HttpHeaders* headers, HTTPMethod method, size_t client_max_body_size)
	: m_index {0}
	, m_data_start {0}
	, m_content_length {0}
	, m_client_max_body_size {client_max_body_size}
	, m_initialized {false}
	, m_complete {false}
	, m_headers {headers}
	, m_is_chunked {false}
{
	switch (method)
	{
	case HTTPMethod::POST:
		break;
	default:
		m_complete = true;
		return;
	}

	std::string transfer_encoding = headers->get_header("transfer-encoding");
	if (transfer_encoding == "chunked")
	{
		m_is_chunked = true;
		m_chunked_decoder = ChunkedDecoder();
		m_initialized = true;
		return;
	}

	if (headers->get_header("content-length") == "" || headers->get_header("content-type") == "")
	{
		throw HTTPException(HTTPStatusCode::BadRequest);
	}
	auto content_type = parse_content_type(headers->get_header("content-type"));
	if (std::get<std::string>(content_type) != "multipart/form-data") // only supported type for now
	{
		throw HTTPException(HTTPStatusCode::UnsupportedMediaType);
	}
	const auto& attributes = std::get<std::unordered_map<std::string, std::string>>(content_type);
	auto bound = attributes.find("boundary");
	if (bound == attributes.end())
	{
		throw HTTPException(HTTPStatusCode::BadRequest);
	}
	m_boundary.emplace(bound->second);
	try
	{
		m_content_length = std::stoul(headers->get_header("content-length"));
	}
	catch (const std::exception& e)
	{
		throw HTTPException(HTTPStatusCode::BadRequest, "Invalid Content-Length value");
	}
	if (m_content_length == 0)
	{
		throw HTTPException(HTTPStatusCode::BadRequest);
	}
	m_initialized = true;
}

HttpBody::HttpBody(void)
	: m_index {0}
	, m_data_start {0}
	, m_content_length {0}
	, m_client_max_body_size {0}
	, m_initialized {false}
	, m_complete {false}
	, m_headers {nullptr}
{

}

HttpBody& HttpBody::operator=(const HttpBody& http_body)
{
	m_raw = http_body.m_raw;
	m_index = http_body.m_index;
	m_data_start = http_body.m_data_start;
	if (http_body.m_boundary.has_value())
	{
		m_boundary.emplace(http_body.m_boundary.value());
	}
	m_content_length = http_body.m_content_length;
	m_client_max_body_size = http_body.m_client_max_body_size;
	m_initialized = http_body.m_initialized;
	m_complete = http_body.m_complete;
	m_chunk = http_body.m_chunk;
	m_headers = http_body.m_headers;
	return *this;
}

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
	if ((m_raw.size() + chunk.size()) > m_client_max_body_size)
	{
		throw HTTPException(HTTPStatusCode::ContentTooLarge);
	}
	if (m_is_chunked)
	{
		m_chunked_decoder.append(chunk);
		if (m_chunked_decoder.complete())
		{
			m_raw = m_chunked_decoder.get_decoded();
			m_complete = true;
			parse();
		}
	}
	else
	{
		m_raw.append(chunk);
		parse();
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
{}

void ChunkedDecoder::append(const std::string& data)
{
	m_buffer.append(data);

	size_t pos = 0;
	while (!m_complete && pos < m_buffer.size())
	{
		switch (m_state)
		{
		case READING_SIZE:
			if (parse_chunk_size(pos))
			{
				m_state = READING_DATA;
				m_current_chunk_read = 0;
			}
			break;

		case READING_DATA:
			if (parse_chunk_data(pos))
			{
				if (m_expected_chunk_size == 0)
					m_state = READING_TRAILER;
				else
					m_state = READING_SIZE;
			}
			break;

		case READING_TRAILER:
			if (parse_trailer(pos))
			{
				m_state = COMPLETE;
				m_complete = true;
			}
			break;

		case COMPLETE:
			return;
		}
	}
	if (pos > 0)
		m_buffer.erase(0, pos);
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
			return true;
		}
		return false;
	}

	//* check how much data needed per chunk
	size_t bytes_needed = m_expected_chunk_size - m_current_chunk_read;
	size_t available_data = m_buffer.size() - pos;

	//* not enough data for complete chunk + CRLF
	if (available_data < bytes_needed + 2)
	{
		size_t to_read = std::min(bytes_needed, available_data);
		m_decoded.append(m_buffer.substr(pos, to_read));
		m_current_chunk_read += to_read;
		pos += to_read;
		return false;
	}

	//* read remainder data
	if (bytes_needed > 0)
	{
		m_decoded.append(m_buffer.substr(pos, bytes_needed));
		pos += bytes_needed;
	}

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
	pos += 2;
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