#pragma once

#include "Config.hpp"
#include "Socket.hpp"
#include "Epoll.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "RequestHandler.hpp"
#include "Client.hpp"
#include <map>
#include <string>
#include <memory>
#include <vector>

class Server
{
private:
	Config m_config;
	std::vector<Socket> m_listening_sockets;
	Epoll m_epoll;
	std::map<int, Client> m_clients;
	bool m_running;
	RequestHandler m_request_handler;

	//* server initialization
	void configureServerSockets();

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
	void start();
	void run();
	void stop();
	bool isRunning() const;

	//* client management
	void addClient(int fd);
	void removeClient(int fd);
	Client& getClient(int fd);
	
	//* socket management
	void addListeningSocket(const std::string& port);
	Socket& getListeningSocket(size_t index);
	size_t getListeningSocketCount() const;

	//* request handling
	void processRequest(int client_fd, const Request& request);
	void sendResponseToClient(int client_fd, const Response& response);
	const Config& getConfig() const;
	RequestHandler& getRequestHandler();
};

