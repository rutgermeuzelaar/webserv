#include "Pch.hpp"
#include "HttpMethod.hpp"
#include "HTTPException.hpp"

HTTPMethod StringToMethod(const std::string& method)
{
	if (method == "GET")
		return HTTPMethod::GET;
	else if (method == "POST")
		return HTTPMethod::POST;
	else if (method == "DELETE")
		return HTTPMethod::DELETE;
	else
		throw HTTPException(HTTPStatusCode::MethodNotAllowed, "Unsupported HTTP method: " + method);
}

std::string stringify(HTTPMethod http_method)
{
    switch (http_method)
    {
        case HTTPMethod::GET:
            return "GET";
        case HTTPMethod::POST:
            return "POST";
        case HTTPMethod::DELETE:
            return "DELETE";
        default:
            return "UNSUPPORTED";
    }
}

std::ostream& operator<<(std::ostream& os, HTTPMethod http_method)
{
    switch (http_method)
    {
        case HTTPMethod::GET:
            os << "GET";
            break;
        case HTTPMethod::POST:
            os << "POST";
            break;
        case HTTPMethod::DELETE:
            os << "DELETE";
            break;
        default:
            os << "UNSUPPORTED";
            break;
    }
    return os;
}