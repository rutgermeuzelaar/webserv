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
{
    switch (method)
    {
        case HTTPMethod::POST:
            break;
        default:
            m_complete = true;
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

HttpBody::HttpBody(const HttpBody& http_body)
    : m_raw {http_body.m_raw}
    , m_index {http_body.m_index}
    , m_data_start {http_body.m_data_start}
    , m_boundary {http_body.m_boundary}
    , m_content_length {http_body.m_content_length}
    , m_client_max_body_size {http_body.m_client_max_body_size}
    , m_initialized {http_body.m_initialized}
    , m_complete {http_body.m_complete}
    , m_chunk {http_body.m_chunk}
    , m_headers {http_body.m_headers}
{

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
    m_raw.append(chunk);
    parse();
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
