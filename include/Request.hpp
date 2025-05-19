/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/25 14:18:35 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/19 15:55:49 by robertrinh    ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "HTTPStatusCode.hpp"

enum class HTTPMethod {
	GET,
	POST,
	DELETE,
	UNSUPPORTED
};

class HTTPException : public std::exception {
private:
	HTTPStatusCode _status_code;
	std::string _message;

public:
	HTTPException(HTTPStatusCode status_code, const std::string& message = "")
		: _status_code(status_code), _message(message) {}

	HTTPStatusCode getStatusCode() const { return _status_code; }
	const char* what() const noexcept override {
		return _message.empty() ? get_http_status_text(_status_code) : _message.c_str();
	}
};

class Request
{
private:
	//* status line components
	std::string _HTTPVersion;
	HTTPMethod	_method_type;
	std::string _method;
	std::string _uri;
	std::string _path;
	std::string _query;
	std::string _fragment;
	std::map<std::string, std::string>_headers; //* key-value headers
	std::string _body;

	HTTPMethod StringToMethod(const std::string& method);
	void parseRequestLine(const std::string& line);
	void parseHeaders(std::istream& requestStream);
	void parseBody(std::istream& requestStream);
	void parseURI(const std::string& uri); 

public:
	Request();
	~Request(); 
	
	void parse(const std::string& str);

	//* getters
	HTTPMethod getMethodType() const;
	std::string getHTTPVersion() const;
	std::string getMethod() const;
	std::string getURI() const;
	std::string getPath() const;     
	std::string getQuery() const;
	std::string getFragment() const;
	std::string getHeader(const std::string& key) const;
	std::string getBody() const;

	void printRequest() const;
};
#endif