/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/25 14:18:35 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/26 15:00:41 by qtrinh        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include "defines.h"
#include "HTTPStatusCode.hpp"
#include "HTTPException.hpp"

enum class HTTPMethod {
	GET,
	POST,
	DELETE,
	UNSUPPORTED
};

class Request
{
private:
	//* chunk limits
	//? configurable or hard setting?
	const size_t _MAX_CHUNKS = 1000;
	const size_t _MAX_TOTAL_SIZE = 10 * 1024 * 1024; //* 10MB
	const size_t _MAX_CHUNK_SIZE = 1024 * 1024; //* 1MB per chunk
	
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
	void parseChunkedBody(std::istream& stream);
	void parseContentLength(std::istream& stream, const std::string& contentLengthStr);

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