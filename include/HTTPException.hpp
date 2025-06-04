#pragma once

#include <stdexcept>
#include <string>
#include "HTTPStatusCode.hpp"

class HTTPException : public std::runtime_error {
private:
    HTTPStatusCode _status_code;

public:
    HTTPException(HTTPStatusCode status_code, const std::string& message = "");
    HTTPStatusCode getStatusCode() const;
};