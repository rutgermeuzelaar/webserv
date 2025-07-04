#include "Pch.hpp"
#include "HTTPException.hpp"

HTTPException::HTTPException(HTTPStatusCode status_code, const std::string& message)
    : std::runtime_error(message.empty() ? get_http_status_text(status_code) : message),
      _status_code(status_code) {}

HTTPStatusCode HTTPException::getStatusCode() const {
    return _status_code;
} 