#pragma once

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include "HTTPStatusCode.hpp"
#include "HTTPStatusLine.hpp"
#include "HTTPException.hpp"
#include "defines.h"

class Response
{
private:
    //* status line components
    HTTPStatusLine    _status_line;
    std::map<std::string, std::string> _headers; //* key-value headers
    std::string      _body;
    std::string      _raw_response; //* cached response string

public:
    Response(HTTPStatusCode status_code = HTTPStatusCode::OK);
    ~Response();

    //* setters
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& content);
    void setBodyFromFile(const std::string& file_path);
    void redirect(const std::string& location, HTTPStatusCode status_code = HTTPStatusCode::Found);
    void setContentType(const std::string& type, const std::string& charset = "UTF-8");
    void setDate();
    void setLastModified(const std::string& date);
    void setCacheControl(const std::string& value);
    void setServer(const std::string& server = "webserv/1.0");

    //* getters
    HTTPStatusCode getStatusCode() const;
    std::string getStatusText() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
    const std::string& to_str(); //* returns the complete response as a string

    void printResponse() const; //* for debugging
};
