/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: robertrinh <robertrinh@student.codam.nl      +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 15:42:16 by robertrinh    #+#    #+#                 */
/*   Updated: 2025/05/19 16:10:40 by robertrinh    ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request() : _HTTPVersion(), _method_type(HTTPMethod::UNSUPPORTED), _method(), _uri(), _path(), _query(), _fragment(), _headers(), _body()
{}

Request::~Request()
{}

HTTPMethod Request::StringToMethod(const std::string& method)
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

/**
 * @brief parses Uniform Resource Identifier (URI) from the request line.
 * @param uri alias of the raw uri to parse
 * @note query is described after the `?`, and a framework after a `#`
 * @note example URI: https://www.example.com/forum/questions/?tag=networking&order=newest#top
 * @return true if URI is valid and parsing succeeded, false if URI is invalid
 */
void Request::parseURI(const std::string& uri)
{
	if (uri.empty() || uri[0] != '/')
		throw HTTPException(HTTPStatusCode::BadRequest, "Invalid URI format");
	if (uri.length() > 2048) //* common URI length limit
		throw HTTPException(HTTPStatusCode::URITooLong, "URI exceeds maximum length");
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
		//* no query string, check for fragment
		size_t fragmentPos = uri.find('#');
		if (fragmentPos != std::string::npos)
		{
			_path = uri.substr(0, fragmentPos);
			_fragment = uri.substr(fragmentPos + 1);
		}
		else
			_path = uri;
	}
}

void Request::parse(const std::string& rawRequest)
{
	std::istringstream requestStream(rawRequest);
	std::string currentLine;
	
	if (!std::getline(requestStream, currentLine) || currentLine.empty())
		throw HTTPException(HTTPStatusCode::BadRequest, "Empty request line");
	parseRequestLine(currentLine);
	parseHeaders(requestStream);
	if (_method_type == HTTPMethod::POST)
		parseBody(requestStream);
	else
		_body.clear();
	printRequest(); //* for testing
}

void Request::parseRequestLine(const std::string& requestLineStr)
{
	std::istringstream requestStream(requestLineStr);
	std::string methodStr;

	//* split in method, uri and version 
	if (!(requestStream >> methodStr >> _uri >> _HTTPVersion))
		throw HTTPException(HTTPStatusCode::BadRequest, "Invalid request line format");
	
	_method_type = StringToMethod(methodStr);
	_method = methodStr;
	parseURI(_uri);
	if (_HTTPVersion.substr(0, 8) != "HTTP/1.1")
		throw HTTPException(HTTPStatusCode::HTTPVersionNotSupported, "Unsupported HTTP version: " + _HTTPVersion);
}

/**
 * @brief parses HTTP headers from the request stream
 * @param requestStream string stream containing the HTTP request headers
 * @note headers are in format "Key: Value"
 * @note headers end with a newline
 * @return true if headers were parsed successfully
 */
void Request::parseHeaders(std::istream& stream)
{
	std::string line;
	size_t totalHeaderSize = 0;
	const size_t MAX_HEADER_SIZE = 8192; //* common header size limit
	
	while (std::getline(stream, line))
	{
		if (!line.empty() && line.back() == '\r') //* C11: back-> return reference to last element
			line.pop_back(); //* removes last element (carriage return = /r)
		if (line.empty())
			break;
		totalHeaderSize += line.length();
		if (totalHeaderSize > MAX_HEADER_SIZE)
			throw HTTPException(HTTPStatusCode::RequestHeaderFieldsTooLarge, "Request header fields too large");
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
			throw HTTPException(HTTPStatusCode::BadRequest, "Malformed header line: " + line);

		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		
		//* trim whitespace from key and value
		key.erase(0, key.find_first_not_of(" \t\r\n")); //* searches str for first char that does not match the given chars
		key.erase(key.find_last_not_of(" \t\r\n") + 1); //* same, but for last char
		value.erase(0, value.find_first_not_of(" \t\r\n"));
		value.erase(value.find_last_not_of(" \t\r\n") + 1);
		
		//* converting header keys to lowercase to combat case-insensivity
		std::string lowerKey = key;
		std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
		if (lowerKey.empty())
			throw HTTPException(HTTPStatusCode::BadRequest, "Empty header key");
		_headers[lowerKey] = value;
	}
}

/**
 * @brief parses the request body based on Content-Length header
 * @param stream string stream containing the complete HTTP request including headers and body
 * @note stream should contain the entire request data, as we need to skip headers to reach the body
 * @note content-length header is required for POST requests to determine body size
 * @return true if parsing succeeded, false if Content-Length is invalid or body is incomplete
 */
void Request::parseBody(std::istream& stream)
{
	std::string contentLengthStr = getHeader("content-length");
	if (contentLengthStr.empty())
		throw HTTPException(HTTPStatusCode::LengthRequired, "Content-Length header required for POST request");

	//* will check for how many bytes of data to expect in request, important for POST
	unsigned long contentLength;
	try {
		contentLength = std::stoul(contentLengthStr);
	} catch (const std::exception& e) {
		throw HTTPException(HTTPStatusCode::BadRequest, "Invalid Content-Length value");
	}
	if (contentLength == 0)
	{
		_body.clear();
		return;
	}

	//* reading body
	_body.resize(contentLength); //* allocate space in internal str buffer
	char* buffer = &_body[0]; //* C11: pointer to first char in internal (resized) str buffer
	stream.read(buffer, contentLength); //* reads directly into contentLength, pointed by buffer. efficient because avoids temp + copy

	std::streamsize bytesRead = stream.gcount();
	if (static_cast<unsigned long>(bytesRead) != contentLength)
		throw HTTPException(HTTPStatusCode::BadRequest, "Incomplete request body");
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
 * @note made headers lowcase for handling case insensitivity
 * @return The value of the requested header, or empty string if not found
 */
std::string Request::getHeader(const std::string& key) const
{
	std::string lowerKey = key;
	std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

	std::map<std::string, std::string>::const_iterator headerIt = _headers.find(lowerKey);
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
