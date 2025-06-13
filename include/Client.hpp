#pragma once

#include <string>
#include "Request.hpp"
#include "HTTPException.hpp"

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