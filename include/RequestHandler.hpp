/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestHandler.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/05 14:13:45 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/13 13:07:18 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HANDLER_HPP
# define REQUEST_HANDLER_HPP
# include <optional>
# include <filesystem>
# include "Response.hpp"
# include "Request.hpp"
# include "ServerContext.hpp"

const std::string get_mime_type(const std::filesystem::path& extension);

class RequestHandler
{
	private:
		const ServerContext& m_config;
		Response handle_get(const Request& request);
		Response handle_post(const Request& request);
		Response handle_delete(const Request& request);

	public:
		RequestHandler(const ServerContext&);
		std::filesystem::path map_uri(std::string uri);
		Response handle(const Request& request);
};
#endif