#include "MultiPartChunk.hpp"
#include "Utilities.hpp"
#include "HTTPException.hpp"

MultiPartChunk::MultiPartChunk()
{

}

MultiPartChunk::~MultiPartChunk()
{

}

MultiPartChunk::MultiPartChunk(const MultiPartChunk& multi_part_chunk)
    : m_mime_type {multi_part_chunk.m_mime_type}
    , m_content_disposition {multi_part_chunk.m_content_disposition}
    , m_data {multi_part_chunk.m_data}
    , m_headers {multi_part_chunk.m_headers}
{

}

void MultiPartChunk::parse_header_attributes(void)
{
    if (m_headers.get_header("content-disposition") == "")
    {
        throw HTTPException(HTTPStatusCode::BadRequest);
    }
    if (m_headers.get_header("content-type") == "")
    {
        throw HTTPException(HTTPStatusCode::BadRequest);   
    }
    m_mime_type = m_headers.get_header("content-type");
    m_content_disposition = parse_content_disposition(m_headers.get_header("content-disposition"));
}

const std::string& MultiPartChunk::get_mime_type(void) const
{
    return m_mime_type;
}