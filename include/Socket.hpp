#pragma once

#include <vector>
#include <iostream>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <cstring>
#include "ServerContext.hpp"

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

	int createSocket(const std::string& port);
	void closeAllSockets();

public:
	Socket(int backlog);
	~Socket();

	//! consider socket configuration - TCP_NODELAY, no timeout, keep alive, no socket buffer size
	bool	initSocket(const ServerContext& config);
	bool	initTestSocket(const std::string& port = "1050"); //! delete later
	int		acceptConnection(int serverSocket);
	void	closeSocket(int socketFD);

	const std::vector<int>& getServerSockets() const;
	bool	isRunning();

	void	stop();
};
