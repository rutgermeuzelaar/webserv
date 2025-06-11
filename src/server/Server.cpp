#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

Client::Client(int socket_fd, Server& server) 
	: m_socket_fd(socket_fd)
	, m_is_connected(true)
	, m_request_complete(false)
	, m_server(server)
{
	m_buffer.clear();
}

Client::~Client()
{
	disconnect();
}

void Client::disconnect()
{
	if (m_is_connected)
	{
		if (close(m_socket_fd) == -1) 
			std::cerr << "Warning: Error closing client socket: " << strerror(errno) << std::endl;
		m_is_connected = false;
		reset();
	}
}

bool Client::isConnected() const
{
	return m_is_connected;
}

int	Client::getSocketFD() const
{
	return m_socket_fd;
}

void Client::receiveData(const char* data, size_t len)
{
	if (!m_is_connected)
		return ;
	m_buffer.append(data, len);
	try {
		m_request.parse(m_buffer);
		m_request_complete = true;
	}
	catch (const HTTPException& e){
		m_request_complete = false;
	}
}

bool Client::hasCompleteRequest() const
{
	return m_request_complete;
}

const Request& Client::getRequest() const
{
	return m_request;
}

void Client::clearRequest()
{
	m_buffer.clear();
	m_request = Request();
	m_request_complete = false;
}

void Client::reset()
{
	clearRequest();
	//! no socket fd or server reference clear
	//! maybe better to use disconnect to reset connection state
}

//--------------//

Server::Server(const Config& config)
	: m_config(config)
	, m_listening_sockets()
	// , m_epoll()
	, m_clients()
	, m_running(false)
	, m_request_handler(m_config)
{
	initializeServerSockets();
	setupServerSockets();
}

Server::~Server()
{
	stop();
}

void Server::run()
{
	if (!m_running)
		return;
	
	//* Uncomment when implementing epoll
	// for (auto& socket: m_listening_sockets)
	// {
	//     setNonBlocking(socket.getFD());
	//     m_epoll.addFD(socket.getFD(), EPOLLIN);
	// }
	// while (m_running)
	// {
	//     //epoll related
	// }
	// If we have any clients, handle their data
	
	//* testing: accept one connection
	try {
		handleNewConnection();

		if (!m_clients.empty())
		{
			int client_fd = m_clients.begin()->first;
			handleClientData(client_fd);
			// Only remove the client, not the server socket
			removeClient(client_fd);
			// Stop the server after handling one request for testing
			stop();
			return;
		}
	}
	catch (const SocketException& e) {
		std::cerr << "Socket error: " << e.what() << std::endl;
		stop();
		return;
	}
}

void Server::start()
{
	m_running = true;
	std::cout << "Server started" << std::endl;
}

void Server::stop()
{
	m_running = false;
	for (auto& [fd, client] : m_clients)
		client.disconnect();
	m_clients.clear();
	m_listening_sockets.clear();
	// TODO: uncomment when implementing epoll
	// m_epoll.close();
}

bool Server::isRunning() const
{
	return m_running;
}

/**
 * @brief Adds a new client connection to the server's client map
 * @details This function is called when a new connection is accepted:
 *          1. Creates a new Client object with the socket fd and server reference (*this)
 *          2. Adds the client to the m_clients map using emplace
 * @param fd The socket file descriptor for the new client connection
 * @note The Client object needs a reference to the Server (*this) to:
 *       - Send responses back to the client
 *       - Access server configuration
 *       - Handle request processing
 */
void Server::addClient(int fd)
{
	if (m_clients.find(fd) != m_clients.end())
		return ;
	m_clients.emplace(fd, Client(fd, *this));
	//TODO add back later
	// setNonBlocking(fd);
	// m_epoll.addFd(fd, EPOLLIN);
}

void Server::removeClient(int fd)
{
	auto it = m_clients.find(fd);
	if (it != m_clients.end())
	{
		it->second.disconnect();
		m_clients.erase(it);
		// m_epoll.removeFD(fd);
	}
}

Client& Server::getClient(int fd)
{
	return m_clients.at(fd);
}

void Server::addListeningSocket(const std::string& port)
{
	//* create socket and store it directly -> see what good option is
	m_listening_sockets.emplace_back(10); //* create Socket with backlog of 10
	Socket& socket = m_listening_sockets.back();
	
	if (!socket.initTestSocket(port))
	{
		m_listening_sockets.pop_back(); //* remove the socket if initialization failed
		return;
	}
	std::cout << "Server is listening on port " << port << std::endl;
}

Socket& Server::getListeningSocket(size_t index)
{
	return m_listening_sockets.at(index);
}

size_t Server::getListeningSocketCount() const
{
	return m_listening_sockets.size();
}

void Server::processRequest(int client_fd, const Request& request)
{
	try {
		Response response = m_request_handler.handle_get(request); //! change to handle request later
		sendResponseToClient(client_fd, response);
	}
	catch (const HTTPException& e) {
		sendErrorResponse(client_fd, e);
	}
}

void Server::sendResponseToClient(int client_fd, const Response& response)
{
	std::string response_str = response.to_str();
	if (send(client_fd, response_str.c_str(), response_str.length(), 0) == -1)
		std::cerr << "Error sending response: " << strerror(errno) << std::endl;
}

void Server::handleNewConnection()
{
	for (auto& socket : m_listening_sockets)
	{
		const std::vector<int>& serverSockets = socket.getServerSockets();
		if (serverSockets.empty())
			continue;
			
		try {
			int client_fd = socket.acceptConnection(serverSockets[0]);  //! only accept one connection for testing
			if (client_fd != -1)
			{
				std::cout << "New connection accepted on fd: " << client_fd << std::endl;
				addClient(client_fd);
				return;
			}
		} catch (const SocketException& e) {
			std::cerr << "Warning: " << e.what() << std::endl;
			continue;
		}
	}
}

void Server::handleClientData(int client_fd)
{
	Client& client = getClient(client_fd);
	char buffer[4096];

	ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
	if (bytes_read == -1)
	{
		std::cerr << "Error reading from client: " << strerror(errno) << std::endl;
		removeClient(client_fd);
		return;
	}
	else if (bytes_read == 0)
	{
		std::cout << "Client closed connection on fd: " << client_fd << std::endl;
		removeClient(client_fd);
		return;
	}
	
	client.receiveData(buffer, bytes_read);
	if (client.hasCompleteRequest())
	{
		processRequest(client_fd, client.getRequest());
		client.clearRequest();
	}
}

void Server::setNonBlocking(int fd)
{
	fcntl(fd, F_SETFL, O_NONBLOCK);
}

void Server::initializeServerSockets()
{
	//* server sockets will be added by the caller for now
}

void Server::setupServerSockets()
{
	for (auto& socket : m_listening_sockets)
	{
		const std::vector<int>& serverSockets = socket.getServerSockets();
		if (!serverSockets.empty())
		{
			setNonBlocking(serverSockets[0]);
		}
	}
}

void Server::sendErrorResponse(int client_fd, const HTTPException& e)
{
	//! make better later
	Response errorResponse(e.getStatusCode());
	errorResponse.setBody(e.what());
	sendResponseToClient(client_fd, errorResponse);
}

