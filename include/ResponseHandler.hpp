#pragma once
#ifndef RESPONSE_HANDLER_HPP
# define RESPONSE_HANDLER_HPP
# include "Pch.hpp" // IWYU pragma: keep
# include <vector>
# include "Http.hpp"

class Server;
class Epoll;

enum class ResponseEvent
{
    MarkEpollOut,
    UnmarkEpollOut
};

class ResponseHandler
{
    private:
        std::vector<Response>  m_pending_responses;
        Server&                     m_server;
        Epoll&                      m_epoll;

        void notify_observer(int client_fd);
        void notify_epoll(int client_fd, ResponseEvent);
        std::vector<Response>::iterator find_response(int client_fd);
    public:
        ResponseHandler(Server& server, Epoll& epoll);
        ResponseHandler(const ResponseHandler&) = delete;
        ResponseHandler& operator=(const ResponseHandler&) = delete;
        ~ResponseHandler() = default;

        void add_response(int client_fd, Response& response);
        void add_response(int client_fd, Response&& response);
        void send_response(int client_fd); 
        void remove_response(int client_fd);
        void remove_if_exists(int client_fd);
};
#endif