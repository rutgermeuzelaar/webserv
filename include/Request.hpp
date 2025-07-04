#ifndef REQUEST_HPP
# define REQUEST_HPP
#include <unordered_map>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include "Defines.hpp"
#include "HTTPStatusCode.hpp"
#include "HTTPException.hpp"
#include "HttpBody.hpp"
#include "HttpMethod.hpp"
#include "HttpRequestStartLine.hpp"
#include "HttpHeaders.hpp"

class Request
{
    private:
        HttpRequestStartLine _start_line; //* status line components
        HttpHeaders _headers; //* key-value headers
        HttpBody _body;
        std::string _raw;
        size_t _index; // index of current byte
        size_t _line_count; // count parsed_lines

        void parse(void);
    public:
        Request();
        ~Request(); 
        friend void swap(Request&, Request&) noexcept;
        Request& operator=(const Request&) = default;

        void append(const char* buffer, size_t len);
        bool complete(void) const;

        //* getters
        HttpRequestStartLine& getStartLine();
        HttpHeaders& getHeaders();
        HttpBody& getBody();

        const HttpRequestStartLine& getStartLine() const;
        const HttpHeaders& getHeaders() const;
        const HttpBody& getBody() const;

        void printRequest() const;
};
#endif