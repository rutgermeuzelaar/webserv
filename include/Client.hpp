#pragma once

#include <string>
#include "Request.hpp"
#include "HTTPException.hpp"
#include <chrono>

class Server;

class Client {
private:
	//* network connection
	int m_socket_fd;
	bool m_is_connected;
	std::chrono::steady_clock::time_point m_last_activity;

	//* request handling
	Request m_request;
public:
	Client(int socket_fd);
	~Client();

	//* connection management
	void disconnect();
	bool isConnected() const;
	int getSocketFD() const;
	std::chrono::steady_clock::time_point getLastActivity() const;
	void updateActivity();

	//* data handling
	void receiveData(const char* data, size_t len);
	bool hasCompleteRequest() const;
	const Request& getRequest() const;
    Request& getRequest();
	void clearRequest();
	void reset();  //* for re-use
};