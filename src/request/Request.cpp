#include "Pch.hpp"
#include <cassert>
#include <iostream>
#include "Utilities.hpp"
#include "Http.hpp"
#include "Defines.hpp"

Request::Request()
    : _index {0}
    , _line_count {0}
{

}

Request::~Request()
{

}

void Request::parse(void)
{
    if (_raw.size() > MAX_HEADER_SIZE && (!_start_line.complete() || !_headers.complete()))
    {
        throw HTTPException(HTTPStatusCode::RequestHeaderFieldsTooLarge);
    }
    if (!_start_line.complete() || !_headers.complete())
    {
        size_t line_break_pos = _raw.find(LINE_BREAK, _index);
        while (line_break_pos != std::string::npos)
        {
            const std::string str = _raw.substr(_index, line_break_pos - _index + 2); // str with linebreak

            if (!_start_line.complete())
            {
                _start_line = HttpRequestStartLine(str);
            }
            else if (!_headers.complete())
            {
                _headers.add_header(str);
            }
            else
            {
                // manage data that's meant for body
                // happens with POST
                break;
            }
            _index += str.size();
            _line_count++;
            line_break_pos = _raw.find(LINE_BREAK, _index);
        }
    }
}

HttpRequestStartLine& Request::getStartLine()
{
    return _start_line;
}

const HttpRequestStartLine& Request::getStartLine() const
{
    return _start_line;
}

HttpHeaders& Request::getHeaders()
{
    return _headers;
}

const HttpHeaders& Request::getHeaders() const
{
    return _headers;
}

HttpBody& Request::getBody()
{
	return _body;
}

const HttpBody& Request::getBody() const
{
    return _body;
}

void Request::append(const char* buffer, size_t len, size_t client_max_body_size)
{
    [[maybe_unused]]size_t index_copy = _index;

    if (!_start_line.complete() || !_headers.complete())
    {
        _raw.append(buffer, len);
        parse();
    }
    if (!_body.complete() && _start_line.complete() && _headers.complete())
    {
        if (!_body.initialized())
        {
            _body = HttpBody(&_headers, _start_line.get_http_method(), client_max_body_size);
        }
    }
    if (complete() || _start_line.get_http_method() != HTTPMethod::POST)
    {
        return;
    }
    DEBUG("index: " << _index << "index_copy: " << index_copy);
    if (_start_line.complete() && _headers.complete())
    {
        std::string str_buffer;
        if (_raw.size() > 0)
        {
            str_buffer = _raw.substr(_index, _raw.size() - _index);
            // https://stackoverflow.com/questions/20457437/does-stdstringclear-reclaim-the-memory-associated-with-a-string
            std::string empty;
            _raw.swap(empty);
        }
        else
        {
            str_buffer = std::string(buffer, len);
        }
        _body.append(str_buffer);
    }
}

bool Request::complete(void) const
{
    return (_start_line.complete() && _headers.complete() && _body.complete());
}

bool Request::is_empty() const
{
	return _raw.empty();
}
