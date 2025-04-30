/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: robertrinh <robertrinh@student.codam.nl      +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 15:42:16 by robertrinh    #+#    #+#                 */
/*   Updated: 2025/04/28 16:37:19 by robertrinh    ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

//? add GET-POST-DELETE method in form of a switch statement?

Request::Request() : _HTTPVersion(), _method(), _uri(), _headers(), _body()
{}

Request::~Request()
{}

bool Request::parse(const std::string& str)
{
	std::istringstream stream(str);
	if (!parseRequestLine(str))
		return false;
	if (!parseHeaders(stream))
		return false;
	if (!parseBody(stream))
		return false;
	return true;
}

bool Request::parseRequestLine(const std::string& line)
{
	std::istringstream lineStream(line);
	std::string requestLine;

	if (!std::getline(lineStream, requestLine) || requestLine.empty())
		return false;
	
	return true;
}

std::string Request::getHTTPVersion() const
{
	return _HTTPVersion;
}

std::string Request::getMethod() const
{
	return _method;
}

std::string Request::getURI() const
{
	return _uri;
}

std::string Request::getHeader(const std::string& key) const
{
	//!find key, else return empty string?
	return "";
}

std::string Request::getBody() const
{
	return _body;
}
