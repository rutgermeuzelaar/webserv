#pragma once

#include <vector>
#include <iostream>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

class Socket {
private:
	std::vector<int> _serverSockets;
	std::map<int, std::string> _socketToServer;
	bool	_isRunning;
	int	_backlog; //backlog size for listen()
	int _optval; // option value for setsockopt()
	struct addrinfo _hints; 
	struct sockaddr_storage _peerAddr; //peer address info for accept();
	socklen_t	_peerSize;


	//? epoll related functions?

	int createSocket(const std::string& host, const std::string& port);
	void closeAllSockets();

public:
	Socket(int backlog);
	~Socket();

	//! bool initSocket(CONFIG) << init socket based on Config (wait for that to be done)
	bool	initTestSocket(const std::string& port = "8080");
	int		acceptConnection(int serverSocket);
	void	closeSocket(int socketFD);

	const std::vector<int>& getServerSockets() const;
	bool	isRunning();

	void	stop();
};
