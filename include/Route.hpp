#pragma once
#ifndef ROUTE_HPP
# define ROUTE_HPP
# include <vector>
# include "Http.hpp"

class Server;
class Response;
class Request;
class LocationContext;
class ServerContext;
typedef Response (*route_act_t)(Server& server, Request& request, const LocationContext* location, const ServerContext& config);

class Route
{
    private:
        const std::string               m_url;
        const std::vector<HTTPMethod>   m_allowed_methods;
        route_act_t                     m_action;

    public:
        Route(const std::string& url, const std::vector<HTTPMethod>& allowed_methods, route_act_t action);
        Route(const Route& other);
        Route& operator=(const Route& other) = delete;
        ~Route() = default;

        const std::string& get_url(void) const;
        const std::vector<HTTPMethod>& get_allowed_methods(void) const;
        route_act_t get_action(void) const;
};

Response create_session(Server& server, Request& request, const LocationContext* location, const ServerContext& config);
Response file_upload_success(Server& server, Request& request, const LocationContext* location, const ServerContext& config);
Response whoami(Server& server, Request& request, const LocationContext* location, const ServerContext& config);
#endif