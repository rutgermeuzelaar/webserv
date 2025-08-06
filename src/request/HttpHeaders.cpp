#include "Pch.hpp"
#include <vector>
#include <cassert>
#include "Defines.hpp"
#include "Utilities.hpp"
#include "HttpHeaders.hpp"
#include "HTTPException.hpp"

HttpHeaders::HttpHeaders()
    : m_line_break_count {0}
{

}

HttpHeaders::HttpHeaders(const HttpHeaders& http_headers)
    : m_line_break_count {http_headers.m_line_break_count}
    , m_headers {http_headers.m_headers}
{

}

HttpHeaders::~HttpHeaders()
{

}

const std::string HttpHeaders::get_header(const std::string& header_name) const
{
    std::string header_name_lower = header_name;
    std::transform(
        header_name_lower.begin(),
        header_name_lower.end(),
        header_name_lower.begin(),
        tolower
    );
    const auto pos = m_headers.find(header_name_lower);

    if (pos == m_headers.end())
    {
        return "";
    }
    return pos->second;
}

void HttpHeaders::add_header(const std::string& header_name)
{
    if (header_name == LINE_BREAK)
    {
        m_line_break_count++;
        return;
    }
    m_headers.insert(parse_single_http_header(header_name));
}

void HttpHeaders::add_header(const std::string& key, const std::string& value)
{
    m_headers[key] = value;
}

bool HttpHeaders::complete() const
{
    return (m_line_break_count == 1);
}

std::tuple<std::string, std::unordered_map<std::string, std::string>> parse_header_arguments(const std::string& header_contents)
{
    assert(header_contents.size() > 0);
    std::unordered_map<std::string, std::string> attributes;
    std::vector<std::string> raw_split = split(header_contents, ';');

    assert(raw_split.size() > 0);
    std::string media_type = raw_split.front();
    raw_split.erase(raw_split.begin());
    for (auto it: raw_split)
    {
        std::vector<std::string> kv_pair = split(it, '=');
        assert(kv_pair.size() == 2);
        for (auto& str: kv_pair) trim(str, WHITE_SPACE);
        attributes[kv_pair[0]] = kv_pair[1];
    }
    return (std::tuple(media_type, attributes));
}

std::tuple<std::string, std::unordered_map<std::string, std::string>> parse_content_type(const std::string& content_type)
{
    return parse_header_arguments(content_type);
}

std::tuple<std::string, std::unordered_map<std::string, std::string>> parse_content_disposition(const std::string& content_disposition)
{
    auto arguments = parse_header_arguments(content_disposition);
    const auto& media_type = std::get<std::string>(arguments);
    const auto& attributes = std::get<std::unordered_map<std::string, std::string>>(arguments);

    if (media_type == "form-data")
    {
        if (attributes.find("name") == attributes.end())
        {
            throw HTTPException(HTTPStatusCode::BadRequest);
        }
        return arguments;
    }
    else if (media_type == "inline" || media_type == "attachment")
    {
        return arguments;
    }
    throw HTTPException(HTTPStatusCode::BadRequest);
}

void HttpHeaders::set_headers(const std::unordered_map<std::string, std::string>& headers)
{
    m_line_break_count = 1;
    m_headers = headers;
}

const std::unordered_map<std::string, std::string>& HttpHeaders::get_headers(void) const
{
    return m_headers;
}
