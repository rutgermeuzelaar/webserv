#pragma once

#include "Config.hpp"
#include "Socket.hpp"
#include "Epoll.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "RequestHandler.hpp"
#include <map>
#include <string>
#include <memory>

// Forward declarations
class Server;

class Client {
private:
	int m_fd;
	Request m_request;
	bool m_request_complete;
	Server& m_server;

public:
	Client(int fd, Server& server);
	void appendData(const char* data, size_t len);
	bool isRequestComplete() const;
	const Request& getRequest() const;
	void clearRequest();
	int getFD() const;
};

class Server
{
private:
	Config m_config;
	Socket m_socket;
	Epoll m_epoll;
	std::map<int, Client> m_clients;
	bool m_running;
	RequestHandler m_request_handler;

	//* server initialization
	void initializeServerSockets();
	void setupServerSockets();
	void setupSignalHandling();

	//* connection handling
	void handleNewConnection();
	void handleClientData(int client_fd);
	void handleClientDisconnect(int client_fd);
	
	//* request processing
	void handleRequest(const Request& request, int client_fd);
	void sendResponse(int client_fd, const Response& response);
	void sendErrorResponse(int client_fd, const HTTPException& e);

	//* utils
	void setNonBlocking(int fd);
	void cleanup();

public:
	Server(const Config& config);
	~Server();

	//* server control
	void run();
	void stop();
	bool isRunning() const;

	//* accessors for Client
	const Config& getConfig() const { return m_config; }
	RequestHandler& getRequestHandler() { return m_request_handler; }
	void removeClient(int fd) { m_clients.erase(fd); }
};

