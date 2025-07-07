#ifndef HTTP_METHOD_HPP
# define HTTP_METHOD_HPP
# include <string>
# include <iostream>

enum class HTTPMethod {
	GET,
	POST,
	DELETE,
	UNSUPPORTED
};

HTTPMethod StringToMethod(const std::string& method);
std::ostream& operator<<(std::ostream& os, HTTPMethod http_method);
std::string stringify(HTTPMethod);
#endif