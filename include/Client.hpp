#pragma once
#include <memory>
#include <string>
#include <chrono>
#include "Http.hpp"

class Server;
class CgiProcess;

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