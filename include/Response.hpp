#pragma once

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <cstddef>
#include <vector>
#include <filesystem>
#include "HTTPStatusCode.hpp"
#include "HTTPStatusLine.hpp"
#include "HTTPException.hpp"
#include "Defines.hpp"

class Response
{
private:
    HTTPStatusLine                      _status_line;
    std::map<std::string, std::string>  _headers; //* key-value headers
    std::string                         _body;
    std::vector<std::byte>              _bytes;
    size_t                              _bytes_sent;
    int                                 _client_fd;
    bool                                _headers_complete;
    size_t                              _headers_size; // Byte count of status line, headers and linebreak
    size_t                              _body_size;

    void headersToBytes(void);
public:
    Response(HTTPStatusCode status_code = HTTPStatusCode::OK);
	Response(const Response& src);
    Response& operator=(const Response& src);
    Response(Response&& other) noexcept;
    ~Response();

    //* setters
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& content);
    void setBodyFromFile(const std::filesystem::path& file_path);
    void redirect(const std::string& location, HTTPStatusCode status_code = HTTPStatusCode::Found);
    void setContentType(const std::string& type, const std::string& charset = "UTF-8");
    void setDate();
    void setLastModified(const std::string& date);
    void setCacheControl(const std::string& value);
    void setServer(const std::string& server = "webserv/1.0");
    void setClientFD(int client_fd);

    //* getters
    HTTPStatusCode getStatusCode() const;
    std::string getStatusText() const;
    std::string getHeader(const std::string& key) const;
    int getClientFD(void) const;
    size_t getHeadersSize(void) const;
    size_t getBodySize(void) const;
	bool getHeadersComplete(void) const;

    const std::byte* getNextBytes(size_t* length);
    void incrementBytesSent(size_t amount);
    void printResponse() const; //* for debugging
    bool fullySent(void) const;

    void printHeaders() const;
    void printBody() const;
};
