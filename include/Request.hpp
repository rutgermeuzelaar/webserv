/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Request.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/25 14:18:35 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/04/28 16:04:43 by robertrinh    ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <map>
#include <string>
#include <sstream>

class Request
{
private:
	//* status line components
	std::string _HTTPVersion;
	std::string _method;
	std::string _uri;
	std::map<std::string, std::string>_headers; //* key-value headers
	std::string _body; //* body

	bool parseRequestLine(const std::string& line);
	bool parseHeaders(const std::istringstream& requestStream);
	bool parseBody(const std::istringstream& requestStream);
public:
	Request();
	~Request(); 
	
	bool parse(const std::string& str);
	
	//* getters
	std::string getHTTPVersion() const;
	std::string getMethod() const;
	std::string getURI() const;
	std::string getHeader(const std::string& key) const;
	std::string getBody() const;

	void printRequest() const;
};
#endif