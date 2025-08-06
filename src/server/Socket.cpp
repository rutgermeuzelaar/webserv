#include "Pch.hpp"
#include <unistd.h>
#include <cassert>
#include "Config.hpp"
#include "Server.hpp"

/**
 * @brief Constructor for the Socket class
 * @details Initializes a new Socket instance with the following:
 *          - Sets up address hints for socket creation:
 *            * AF_UNSPEC: Supports both IPv4 and IPv6
 *            * SOCK_STREAM: TCP connection-oriented protocol
 *            * AI_PASSIVE: Socket will be used for binding
 *          - Initializes member variables:
 *            * _isRunning: Set to false (server not running)
 *            * _backlog: Maximum length of pending connections queue
 *            * _optval: Set to 1 for socket options
 *            * _peerSize: Size of peer address structure
 * @param backlog The maximum number of pending connections that can be queued
 * @note The AI_PASSIVE flag means the socket will be bound to all available interfaces.
 *       To bind to a specific interface, remove AI_PASSIVE and provide the IP address.
 */
Socket::Socket(int backlog) : _isRunning(false), _backlog(backlog), _optval(1)
{
	std::memset(&_hints, 0, sizeof(_hints));
	_hints.ai_family = AF_UNSPEC;
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_flags = AI_PASSIVE;
	_peerSize = sizeof(_peerAddr);
}

Socket::~Socket() {
	closeAllSockets();
}

/**
 * @brief Creates and initializes a server socket
 * @details This function performs the following steps:
 *          1. Gets address information using getaddrinfo
 *          2. Creates a socket with the appropriate family and type
 *          3. Sets socket options (SO_REUSEADDR)
 *          4. Binds the socket to the specified port
 *          5. Sets the socket to listen for incoming connections
 * @param port The port number to bind the socket to
 * @return int The file descriptor of the created socket
 * @throw SocketException if any step fails (getaddrinfo, socket creation, 
 *        setsockopt, bind, or listen)
 */
int Socket::createSocket(const std::string &port, const std::string &ip_addr)
{
	//*define struct
	struct addrinfo *node;
	struct addrinfo *server_addr;
	int socketFD = -1;
	
	assert("IP address gotta be a string" && !ip_addr.empty());
	//*get address info
	if (getaddrinfo(ip_addr.c_str(), port.c_str(), &_hints, &server_addr) != 0)
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
		if (bind(socketFD, node->ai_addr, node->ai_addrlen) == -1)
		{
			close(socketFD);
			continue ;
		}
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

bool Socket::initSocket(const ServerContext& config)
{
	try {
        assert("Listen object should be fully initialized here" && config.m_listen.has_value() && config.m_listen.value().m_port.has_value());
        assert("Config should contain server_name here" && config.m_server_name.has_value());
        const std::string port = config.m_listen.value().m_port.value().to_string();
        const std::string server_name = config.m_server_name.value().m_name;
		const std::string ip_address = config.m_listen.value().m_ipv4.value().to_string();

		std::cout << ip_address << '\n';
		int socketFD = createSocket(port, ip_address);
		_serverSockets.push_back(socketFD);
		_socketToServer[socketFD] = server_name;
		_isRunning = true;
		return true;
	}
	catch (const SocketException& e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}
}

int Socket::acceptConnection(int serverSocket)
{
	_peerSize = sizeof(_peerAddr); //! test
	int peerFD = accept(serverSocket, (struct sockaddr*)&_peerAddr, &_peerSize);
	if (peerFD == -1) 
		throw SocketException("Accept failed: " + std::string(strerror(errno)));
	return peerFD;
}

void Socket::closeSocket(int socketFD)
{
	if (socketFD == -1)
		return;
		
	if (close(socketFD) == -1)
			throw SocketException("Failed to close socket: " + std::string(strerror(errno)));

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
