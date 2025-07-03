#include "MultiPartChunk.hpp"
#include "Utilities.hpp"
#include "HTTPException.hpp"

MultiPartChunk::MultiPartChunk()
{

}

MultiPartChunk::~MultiPartChunk()
{

}

void swap(MultiPartChunk& a, MultiPartChunk& b) noexcept
{
    using std::swap;

    swap(a.m_mime_type, b.m_mime_type);
    swap(a.m_content_disposition, b.m_content_disposition);
    swap(a.m_data, b.m_data);
    swap(a.m_headers, b.m_headers);
}

MultiPartChunk::MultiPartChunk(const MultiPartChunk& multi_part_chunk)
    : m_mime_type {multi_part_chunk.m_mime_type}
    , m_content_disposition {multi_part_chunk.m_content_disposition}
    , m_data {multi_part_chunk.m_data}
    , m_headers {multi_part_chunk.m_headers}
{

}

MultiPartChunk& MultiPartChunk::operator=(MultiPartChunk multi_part_chunk)
{
    swap(*this, multi_part_chunk);
    return *this;
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