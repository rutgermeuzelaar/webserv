/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: robertrinh <robertrinh@student.codam.nl      +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 15:42:16 by robertrinh    #+#    #+#                 */
/*   Updated: 2025/05/08 18:55:17 by robertrinh    ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request() : _HTTPVersion(), _method_type(HTTPMethod::UNSUPPORTED), _method(), _uri(), _path(), _query(), _fragment(), _headers(), _body()
{}

Request::~Request()
{}

HTTPMethod Request::StringToMethod(const std::string& method)
{
	//? case sensitive. get -> toupper(), do we handle this?
	if (method == "GET")
		return HTTPMethod::GET;
	else if (method == "POST")
		return HTTPMethod::POST;
	else if (method == "DELETE")
		return HTTPMethod::DELETE;
	else
		return HTTPMethod::UNSUPPORTED; //! should have HTTP error codes
}

/**
 * @brief parses Uniform Resource Identifier (URI) from the request line.
 * @param uri alias of the uri to parse
 * @note query is described after the `?`, and a framework after a `#`
 * @note example URI: https://www.example.com/forum/questions/?tag=networking&order=newest#top
 * @return true if URI is valid and parsing succeeded, false if URI is invalid
 */
bool Request::parseURI(const std::string& uri)
{
	if (uri.empty() || uri[0] != '/')
		return false;
	_uri = uri;

	size_t queryPos = uri.find('?');
	if (queryPos != std::string::npos)
	{
		_path = uri.substr(0, queryPos);
		
		size_t fragmentPos = uri.find('#', queryPos);
		if (fragmentPos != std::string::npos)
		{
			_query = uri.substr(queryPos + 1, fragmentPos - queryPos - 1);
			_fragment = uri.substr(fragmentPos + 1);
		}
		else
			_query = uri.substr(queryPos + 1);
	}
	else
	{
		// No query string, check for fragment
		size_t fragmentPos = uri.find('#');
		if (fragmentPos != std::string::npos)
		{
			_path = uri.substr(0, fragmentPos);
			_fragment = uri.substr(fragmentPos + 1);
		}
		else
			_path = uri;
	}
	return true;
}

bool Request::parse(const std::string& str)
{
	std::istringstream stream(str);
	if (!parseRequestLine(str))
		return false;
	if (!parseHeaders(stream))
		return false;
	if (!parseBody(stream))
		return false;
	printRequest(); //* for testing
	return true;
}

bool Request::parseRequestLine(const std::string& line)
{
	std::istringstream lineStream(line);
	std::string requestLine;

	if (!std::getline(lineStream, requestLine) || requestLine.empty())
		return false;
	
	// Split the request line into method, URI, and HTTP version
	std::istringstream requestStream(requestLine);
	std::string methodStr;
	if (!(requestStream >> methodStr >> _uri >> _HTTPVersion))
		return false;

	// Convert method string to enum
	_method_type = StringToMethod(methodStr);
	_method = methodStr;
	if (!parseURI(_uri))
		return false;
	if (_HTTPVersion.substr(0, 5) != "HTTP/")
		return false;
	if (_method_type == HTTPMethod::UNSUPPORTED)
		return false; //* throw error?

	return true;
}

/**
 * @brief parses HTTP headers from the request stream
 * @param requestStream string stream containing the HTTP request headers
 * @note headers are in format "Key: Value"
 * @note headers end with a newline
 * @todo handling case insensitivity?
 * @return true if headers were parsed successfully
 */
bool Request::parseHeaders(const std::istringstream& requestStream)
{
	std::string line;
	std::istringstream streamCopy(requestStream.str());
	
	while (std::getline(streamCopy, line) && !line.empty())
	{
		// Remove carriage return if present
		if (!line.empty() && line[line.length()-1] == '\r')
			line.erase(line.length()-1);
		if (line.empty())
			continue;
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
			continue;

		//* get key and value
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);

		//* trim whitespace from key and value
		key.erase(0, key.find_first_not_of(" \t\r\n")); //* searches str for first char that does not match the given chars
		key.erase(key.find_last_not_of(" \t\r\n") + 1); //* same, but for last char
		value.erase(0, value.find_first_not_of(" \t\r\n"));
		value.erase(value.find_last_not_of(" \t\r\n") + 1);
		_headers[key] = value;
	}

	return true;
}

/**
 * @brief parses the request body based on Content-Length header
 * @param requestStream string stream containing the complete HTTP request including headers and body
 * @note stream should contain the entire request data, as we need to skip headers to reach the body
 * @note content-length header is required for POST requests to determine body size
 * @return true if parsing succeeded, false if Content-Length is invalid or body is incomplete
 */
bool Request::parseBody(const std::istringstream& requestStream)
{
	//* retrieve content length from header
	std::string contentLengthStr = getHeader("Content-Length");
	if (contentLengthStr.empty())
		return true; //* no body! <- GET has no body, so it should return true
	size_t contentLength; //* will check for how many bytes of data to expect in request, imnportant for POST
	contentLength = std::stoul(contentLengthStr); //* convert string to unsigned long
	if (!contentLength)
		return false; //! Better error hadnling

	//* skip headers
	std::string line;
    std::istringstream streamCopy(requestStream.str());
    while (std::getline(streamCopy, line) && !line.empty()) {
        if (line == "\r") break;
    }
	//* read the body
	std::string remainingContent;
	std::getline(streamCopy, remainingContent);
	if (remainingContent.length() >= contentLength)
	{
		_body = remainingContent.substr(0, contentLength);
		return true;
	}
	return false;
}

std::string Request::getHTTPVersion() const
{
	return _HTTPVersion;
}

HTTPMethod Request::getMethodType() const
{
	return _method_type;
}

std::string Request::getMethod() const
{
	return _method;
}

std::string Request::getURI() const
{
	return _uri;
}

std::string Request::getPath() const
{
	return _path;
}

std::string Request::getQuery() const
{
	return _query;
}

std::string Request::getFragment() const
{
	return _fragment;
}

/**
 * @brief retrieves the value of a specific HTTP header
 * @param key name of the header to retrieve
 * @todo headers are case insensitive, need to be handled?
 * @return The value of the requested header, or empty string if not found
 */
std::string Request::getHeader(const std::string& key) const
{
	//? HTTP headers are case insensitive. need to be handled?
	std::map<std::string, std::string>::const_iterator headerIt = _headers.find(key);
	if (headerIt != _headers.end())
		return headerIt->second;
	return "";
}

std::string Request::getBody() const
{
	return _body;
}

void Request::printRequest() const
{
	std::cout << "\n--- Request Details ---" << std::endl;
	std::cout << "Method: " << _method << std::endl;
	std::cout << "\nURI Components:" << std::endl;
	std::cout << "  Full URI: '" << _uri << "'" << std::endl;
	std::cout << "  Path:     '" << _path << "'" << std::endl;
	std::cout << "  Query:    '" << _query << "'" << std::endl;
	std::cout << "  Fragment: '" << _fragment << "'" << std::endl;
	std::cout << "\nHTTP Version: " << _HTTPVersion << std::endl;
	
	std::cout << "\nHeaders:" << std::endl;
	for (const auto& header : _headers)
		std::cout << "  " << header.first << ": " << header.second << std::endl;
	std::cout << "\nBody:" << std::endl;
	std::cout << _body << std::endl;
	std::cout << "---------------------\n" << std::endl;
}
