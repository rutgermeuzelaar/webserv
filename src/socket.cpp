#include "../include/Socket.hpp"

//! change errors in throws

/**
 * @brief Constructor for the socket clsas
 * @details If you want to bind to a specific local IP address, drop the AI_PASSIVE
 * @details and put an IP address in for the first argument to getaddrinfo().
 */
Socket::Socket(int backlog) : _isRunning(false), _backlog(backlog), _optval(1)
{
	addrinfo _hints{}; //* c++ bzero function
	_hints.ai_family = AF_UNSPEC; //* Dont care if ipv4 or ipv6
	_hints.ai_socktype = SOCK_STREAM; //* TCP Stream sockets
	_hints.ai_flags = AI_PASSIVE; //* assign the address of local host to the socket structures
	_peerSize = sizeof(_peerAddr);
}

Socket::~Socket() {
	closeAllSockets();
}

int	Socket::createSocket(const std::string& host, const std::string& port)
{
	//*define struct
	struct addrinfo	*node;
	struct addrinfo *server_addr;
	int socketFD = -1;
	
	//*get address info
	if (getaddrinfo(NULL, port.c_str(), &_hints, &server_addr) != 0)
	{
		std::cerr << "getaddr info fail" << std::endl;
		return -1;
	}
	//* try address until bind
	for (node = server_addr; node != NULL; node = node->ai_next)
	{
		//* create socket
		socketFD = socket(node->ai_family, node->ai_socktype, node->ai_protocol);
		if (socketFD == -1)
			continue ;
		
		//* set socket options
		if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &_optval, sizeof(int) == -1))
		{
			close(socketFD);
			freeaddrinfo(server_addr);
			std::cerr << "setsockopt failed" << std::endl;
			return -1;
		}

		//* bind
		if (bind(socketFD, node->ai_addr, node->ai_addrlen) == -1)
		{
			close(socketFD);
			continue ;
		}

		//* listen
		if (listen(socketFD, _backlog) == -1)
		{
			close(socketFD);
			freeaddrinfo(server_addr);
			std::cerr << "listen failed" << std::endl;
			return -1;
		}
		break ;
	}
	freeaddrinfo(server_addr);

	//* check if succesfully created
	if (node == NULL) {
		std::cerr << "failed to create node" << std::endl;
		return -1;
	}
	return socketFD;
}

void Socket::closeAllSockets()
{
	for (int socketFD : _serverSockets)
		close(socketFD);
	_serverSockets.clear();
	_socketToServer.clear();
	_isRunning = false;
}

bool Socket::initTestSocket(const std::string& port)
{
	int socketFD = createSocket("0.0.0.0", port);
	if (socketFD == -1)
	{
		std::cerr << "failed to create socket on port" << port << std::endl;
		return false;
	}

	_serverSockets.push_back(socketFD);
	_socketToServer[socketFD] = "TestServer";
	_isRunning = true;
	
	std::cout << "Server is listening on port " << port << std::endl;
	return true;
}

int	Socket::acceptConnection(int serverSocket)
{
	//? Nonblocking? how
	int peerFD = accept(serverSocket, (struct sockaddr*)&_peerAddr, &_peerSize);
	if (peerFD == -1)
		std::cout << "accept failed" << std::endl;
	return peerFD;
}

void Socket::closeSocket(int socketFD)
{
	  close(socketFD);
    
	  //* remove from server sockets if it's a server socket
	  for (auto it = _serverSockets.begin(); it != _serverSockets.end(); ++it) {
		  if (*it == socketFD) {
			  _serverSockets.erase(it);
			  break;
		  }
	  }
	  //* remove from socket map
	  _socketToServer.erase(socketFD);
}

const std::vector<int>& Socket::getServerSockets() const
{
	return _serverSockets;
}

bool Socket::isRunning()
{
	return _isRunning;
}

void Socket::stop()
{
	_isRunning = false;
}
