#pragma once

#include <string>
#include "Request.hpp"
#include "HTTPException.hpp"

class Server;

class Client {
private:
	//* network connection
	int m_socket_fd;
	bool m_is_connected;

	//* request handling
	Request m_request;
public:
	Client(int socket_fd);
	~Client();

	//* connection management
	void disconnect();
	bool isConnected() const;
	int getSocketFD() const;

	//* data handling
	void receiveData(const char* data, size_t len);
	bool hasCompleteRequest() const;
	const Request& getRequest() const;
    Request& getRequest();
	void clearRequest();
	void reset();  //* for re-use
};