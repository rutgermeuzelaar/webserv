#pragma once
#include "Pch.hpp" // IWYU pragma: keep
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "Config.hpp"
#include "Epoll.hpp"
#include "RequestHandler.hpp"
#include "Cgi.hpp"
#include "ResponseHandler.hpp"
#include "Http.hpp"

class Epoll;

//-----------------------------------------------------------------------------
// Classes to parse HTTP requests
//-----------------------------------------------------------------------------

class SocketException : public std::runtime_error {
public:
	explicit SocketException(const std::string& message) 
		: std::runtime_error("Socket Error: " + message) {}
};

class Socket {
private:
	std::vector<int> _serverSockets;
	std::map<int, std::string> _socketToServer;
	bool	_isRunning;
	int	_backlog; //* backlog size for listen()
	int _optval; //* option value for setsockopt()
	struct addrinfo _hints; 
	struct sockaddr_storage _peerAddr; //* peer address info for accept();
	socklen_t	_peerSize;

	int createSocket(const std::string& port, const std::string& ip_address);
	void closeAllSockets();

public:
	Socket(int backlog);
	~Socket();

	//! consider socket configuration - TCP_NODELAY, no timeout, keep alive, no socket buffer size
	bool	initSocket(const ServerContext& config);
	int		acceptConnection(int serverSocket);
	void	closeSocket(int socketFD);

	const std::vector<int>& getServerSockets() const;
	bool	isRunning();

	void	stop();
};


//-----------------------------------------------------------------------------
// Client class
//-----------------------------------------------------------------------------
class Client {
private:
	//* network connection
	int m_socket_fd;
	bool m_is_connected;
	std::chrono::steady_clock::time_point m_last_activity;

	//* request handling
	Request m_request;

    std::shared_ptr<CgiProcess> m_process_ptr;
public:
	Client(int socket_fd);
    Client(const Client& client) = delete;
	~Client();

	//* connection management
	void disconnect();
	bool isConnected() const;
	int getSocketFD() const;
	std::chrono::steady_clock::time_point getLastActivity() const;
	void updateActivity();

	//* data handling
	void receiveData(const char* data, size_t len, size_t client_max_body_size);
	bool hasCompleteRequest() const;
	const Request& getRequest() const;
    Request& getRequest();
	void clearRequest();
	void reset();  //* for re-use

    void setProcessPtr(std::shared_ptr<CgiProcess>);
    std::shared_ptr<CgiProcess> getProcessPtr(void);

    void resetProcessPtr(void);
};

//-----------------------------------------------------------------------------
// Internal routing class
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Session classes
//-----------------------------------------------------------------------------
class Session
{
	private:
		const std::string m_id;
		std::unordered_map<std::string, std::string> m_data;
        std::unordered_map<HTTPMethod, size_t> m_request_history;
	public:
		Session(const std::string& id);
        Session& operator=(const Session&) = delete;
        Session(const Session&);
        ~Session();

		const std::string& get_id(void) const;
		void set_data_pair(const std::string& key, const std::string& value);
		std::optional<const std::string> get_value(const std::string& key) const;
        const std::unordered_map<HTTPMethod, size_t>& get_request_history() const;
        void add_request(HTTPMethod http_method);
        void print(void) const;
};

class SessionHandler
{
    private:
        std::unordered_map<std::string, Session> m_sessions;
		const std::string generate_session_id(void);

    public:
        std::unordered_map<std::string, Session>::iterator find_session(const std::string& id);
        std::unordered_map<std::string, Session>::const_iterator find_session_const(const std::string& id) const;
        void clear_session(const std::string& id);
        const std::string add_session(void);
        const std::string get_cookie(const std::string& id) const;
        bool has_session(const std::string& id) const;
        const std::unordered_map<std::string, Session>& get_sessions(void) const;
};

//-----------------------------------------------------------------------------
// Server class
//-----------------------------------------------------------------------------
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

        //* request handling
        void processRequest(int client_fd, Request& request);
        const Config& getConfig() const;
        RequestHandler& getRequestHandler();

        void notify(CgiProcess&, CgiProcessEvent);
        void notify_response_sent(int client_fd);

        SessionHandler& getSessionHandler(void);
};
