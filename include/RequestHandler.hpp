/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestHandler.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/05 14:13:45 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/17 15:16:47 by rmeuzela      ########   odam.nl         */
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

const std::string create_directory_listing(const std::filesystem::path& directory);

class RequestHandler
{
	private:
		const ServerContext& m_config;
		Response handle_get(const Request& request);
		Response handle_post(const Request& request);
		Response handle_delete(const Request& request);
        const LocationContext* find_location(const std::string& folder_path) const;
        Response build_error_page(HTTPStatusCode status_code, const LocationContext* location);

	public:
		RequestHandler(const ServerContext&);
		std::filesystem::path map_uri(std::string uri, const LocationContext* location);
		Response handle(const Request& request);
};
#endif