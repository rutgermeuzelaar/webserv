#include "../include/Socket.hpp"

/**
 * @brief Constructor for the socket clsas
 * @details If you want to bind to a specific local IP address, drop the AI_PASSIVE
 * @details and put an IP address in for the first argument to getaddrinfo().
 */
Socket::Socket(int backlog) : _isRunning(false), _backlog(backlog), _optval(1)
{
	std::memset(&_hints, 0, sizeof(_hints));
	_hints.ai_family = AF_UNSPEC; //* Dont care if ipv4 or ipv6
	_hints.ai_socktype = SOCK_STREAM; //* TCP Stream sockets
	_hints.ai_flags = AI_PASSIVE; //* assign the address of local host to the socket structures
	_peerSize = sizeof(_peerAddr);
}

Socket::~Socket() {
	closeAllSockets();
}

int Socket::createSocket(const std::string& port)
{
	//*define struct
	struct addrinfo *node;
	struct addrinfo *server_addr;
	int socketFD = -1;
	
	//*get address info
	if (getaddrinfo(NULL, port.c_str(), &_hints, &server_addr) != 0)
		throw SocketException("Failed to get address info: " + std::string(gai_strerror(errno)));

	for (node = server_addr; node != NULL; node = node->ai_next)
	{
		//* create socket
		socketFD = socket(node->ai_family, node->ai_socktype, node->ai_protocol);
		if (socketFD == -1)
			continue ;
		
		//* set socket options
		if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &_optval, sizeof(int)) == -1)
		{
			close(socketFD);
			freeaddrinfo(server_addr);
			throw SocketException("Failed to set socket options: " + std::string(strerror(errno)));
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
			throw SocketException("Failed to listen on socket: " + std::string(strerror(errno)));
		}
		break ;
	}
	freeaddrinfo(server_addr);

	//* check if succesfully created
	if (node == NULL)
		throw SocketException("Failed to create socket - no valid address found");
	
	return socketFD;
}

void Socket::closeAllSockets()
{
	for (int socketFD : _serverSockets) 
		if (close(socketFD) == -1) 
			throw SocketException("Failed to close socket: " + std::string(strerror(errno)));
	_serverSockets.clear();
	_socketToServer.clear();
	_isRunning = false;
}

bool Socket::initTestSocket(const std::string& port)
{
	try {
		int socketFD = createSocket(port);
		_serverSockets.push_back(socketFD);
		_socketToServer[socketFD] = "TestServer";
		_isRunning = true;
		
		std::cout << "Server is listening on port " << port << std::endl;
		return true;
	} 
	catch (const SocketException& e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}
}

int	Socket::acceptConnection(int serverSocket)
{
	//? Nonblocking? how
	int peerFD = accept(serverSocket, (struct sockaddr*)&_peerAddr, &_peerSize);
	if (peerFD == -1) {
		throw SocketException("Accept failed: " + std::string(strerror(errno)));
	}
	return peerFD;
}

void Socket::closeSocket(int socketFD)
{
	if (close(socketFD) == -1) {
		throw SocketException("Failed to close socket: " + std::string(strerror(errno)));
	}
	
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
