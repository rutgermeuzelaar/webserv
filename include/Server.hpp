#pragma once

#include "Config.hpp"
#include "Socket.hpp"
// #include "Epoll.hpp" 
#include "Request.hpp"
#include "Response.hpp"
#include "RequestHandler.hpp"
#include <map>
#include <string>
#include <memory>
#include <vector>

class Server;

class Client {
private:
	Server& m_server;
	//* network connection
	int m_socket_fd;
	bool m_is_connected;

	//* request handling
	Request m_request;
	std::string m_buffer; //* raw data buffer
	bool m_request_complete;
public:
	Client(int socket_fd, Server& server);
	~Client();

	//* connection management
	void disconnect();
	bool isConnected() const;
	int getSocketFD() const;

	//* data handling
	void receiveData(const char* data, size_t len);
	bool hasCompleteRequest() const;
	const Request& getRequest() const;
	void clearRequest();
	void reset();  //* for re-use
};

class Server
{
private:
	Config m_config;
	std::vector<Socket> m_listening_sockets;
	// Epoll m_epoll;
	std::map<int, Client> m_clients;
	bool m_running;
	RequestHandler m_request_handler;

	//* server initialization
	void initializeServerSockets();
	void setupServerSockets();

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

