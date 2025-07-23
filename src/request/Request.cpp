#include "Pch.hpp"
#include <cassert>
#include <iostream>
#include "Request.hpp"
#include "Utilities.hpp"
#include <unistd.h>

Request::Request()
    : _index {0}
    , _line_count {0}
	, _body_write_offset {0}
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


void Request::printRequest() const
{
	// std::cout << "\n--- Request Details ---" << std::endl;
	// std::cout << "Method: " << _method << std::endl;
	// std::cout << "\nURI Components:" << std::endl;
	// std::cout << "  Full URI: '" << _uri << "'" << std::endl;
	// std::cout << "  Path:     '" << _path << "'" << std::endl;
	// std::cout << "  Query:    '" << _query << "'" << std::endl;
	// std::cout << "  Fragment: '" << _fragment << "'" << std::endl;
	// std::cout << "\nHTTP Version: " << _HTTPVersion << std::endl;
	
	// std::cout << "\nHeaders:" << std::endl;
	// for (const auto& header : _headers)
	// 	std::cout << "  " << header.first << ": " << header.second << std::endl;
	// std::cout << "\nBody:" << std::endl;
	// std::cout << _body.get_raw() << std::endl;
	// std::cout << "---------------------\n" << std::endl;
}

void Request::append(const char* buffer, size_t len, size_t client_max_body_size)
{
    size_t index_copy = _index;

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
    // assert(_start_line.get_http_method() == HTTPMethod::POST); //! finish @rutger
	if (_start_line.get_http_method() == HTTPMethod::POST)
	{
		std::cout << "";	
	}
    std::cout << "index: " << _index << "index_copy: " << index_copy << '\n';
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

//* call this function when EPOLLOUT is triggered!
bool Request::writeChunkToFD(int fd)
{
	const std::string& transfer_encoding = _headers.get_header("transfer-encoding");
	const std::string& content_length_str = _headers.get_header("content-length");
	const HttpBody& body = _body;
	std::string data_to_write;

	if (transfer_encoding == "chunked")
		data_to_write = body.get_raw();
	else if (!content_length_str.empty())
	{
		size_t content_length = std::stoul(content_length_str);
		const std::string& raw = body.get_raw();
		data_to_write = raw.substr(0, std::min(content_length, raw.size()));
	}
	else
		data_to_write = body.get_raw(); //* no chunked - content length for security

	if (_body_write_offset >= data_to_write.size())
		return true;
	
	ssize_t written = write(fd, data_to_write.data(), data_to_write.size());
	if (written < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return false;
		throw HTTPException(HTTPStatusCode::InternalServerError, "Failed to write request request body to fd");
	}
	_body_write_offset += written;
	std::cout << "[writeChunkToFD] Wrote " << written << "bytes, total sent: " << _body_write_offset << "/" << data_to_write.size() << std::endl;
	//* return true when entire body is written, false if there is more
	return _body_write_offset == data_to_write.size();
}

