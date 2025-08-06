#pragma once
#include "Pch.hpp"
#include <map>
#include <string>
#include <memory>
#include <vector>
#include "Config.hpp"
#include "Socket.hpp"
#include "Epoll.hpp"
#include "RequestHandler.hpp"
#include "Client.hpp"
#include "Cgi.hpp"
#include "ResponseHandler.hpp"
#include "SessionHandler.hpp"
#include "Route.hpp"
#include "Http.hpp"

enum class CgiProcessEvent {
    ResponseReady,
    IsRemovable
};

class Server
{
    private:
        std::vector<ServerContext> m_configs;
        std::vector<Socket> m_listening_sockets;
        Epoll m_epoll;
        std::map<int, Client> m_clients;
        std::map<int, size_t> m_client_to_socket_index; //* <client_fd, listening_socket_i>
        std::vector<Route> m_routes;
        bool m_running;
        static constexpr std::chrono::seconds TIMEOUT{15}; //! change to appriorate timeout
        Cgi m_cgi;
        ResponseHandler m_response_handler;
        SessionHandler m_session_handler;

        //* server initialization
        void setupListeningSockets();

        //* connection handling
        void handleNewConnection(size_t socket_index);
        void handleClientData(int client_fd);
        void handleClientDisconnect(int client_fd);
        
        //* request processing
        void handleRequest(const Request& request, int client_fd);
        void sendErrorResponse(int client_fd, const HTTPException& e);

        //* utils
        void setNonBlocking(int fd);
        void cleanup();
        void timeout_clients();
        std::optional<size_t> getSocketIndex(int fd) const;
        
        void install_route(const std::string& url, route_act_t, const std::vector<HTTPMethod>& allowed_methods);
        std::vector<Route>::const_iterator find_route(const std::string& url);

        void epoll_loop(int num_events);
    public:
        Server(const std::vector<ServerContext>& configs, char **envp);
        Server& operator=(const Server&) = delete;
        Server(const Server&) = delete;
        ~Server();

        //* server control
        void start();
        void run();
        void stop();
        bool isRunning() const;

        //* client management
        void addClient(int fd);
        void removeClient(int fd);
        Client& getClient(int fd);
        bool isClient(int fd) const;
        
        //* socket management
        Socket& getListeningSocket(size_t index);
        size_t getListeningSocketCount() const;

        //* request handling
        void processRequest(int client_fd, Request& request);
        const Config& getConfig() const;
        RequestHandler& getRequestHandler();

        void notify(CgiProcess&, CgiProcessEvent);
        void notify_response_sent(int client_fd);

        SessionHandler& getSessionHandler(void);
};
