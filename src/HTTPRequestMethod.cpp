/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPRequestMethod.cpp                              :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/25 14:24:53 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/04/25 14:28:23 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include "HTTPRequestMethod.hpp"

const std::string HTTPRequestMethod::_get = "GET";
const std::string HTTPRequestMethod::_post = "POST";
const std::string HTTPRequestMethod::_delete = "DELETE";

const std::string& HTTPRequestMethod::DELETE(void)
{
    return _delete;
}

const std::string& HTTPRequestMethod::GET(void)
{
    return _get;
}

const std::string& HTTPRequestMethod::POST(void)
{
    return _post;
}