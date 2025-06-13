#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

//TODO Figure out

Server::Server(const Config& config)
	: m_config(config)
	, m_listening_sockets()
	, m_epoll()
	, m_clients()
	, m_running(false)
	, m_request_handler(m_config)
{}

Server::~Server()
{
	stop();
}

void Server::run()
{
	if (!m_running)
		return;
	
	std::cout << "Server running with epoll event loop" << std::endl;
	while (m_running)
	{
		try {
			int num_events = m_epoll.wait();
			//TODO timeout mechanism
			for (int i = 0; i < num_events; ++i)
			{
				const epoll_event& event = m_epoll.getEvents()[i];
				int fd = event.data.fd;
				std::cout << "Processing event for fd: " << fd << std::endl; //! TEST 
				
				if (m_epoll.isTypeEvent(event, EPOLLERR) || m_epoll.isTypeEvent(event, EPOLLHUP))
				{
					std::cerr << "Error or hangup on fd: " << fd << std::endl; //! TEST 
					removeClient(fd);
					continue;
				}
				if (m_epoll.isServerSocket(fd, m_listening_sockets[0].getServerSockets()))
				{
					std::cout << "Server socket event detected" << std::endl; //! TEST 
					handleNewConnection();
					continue;
				}
				if (m_epoll.isTypeEvent(event, EPOLLIN))
				{
					std::cout << "EPOLLIN event detected for fd: " << fd << std::endl; //! TEST 
					handleClientData(fd);
				}
			}
		}
		catch (const EpollException& e) {
			std::cerr << "Epoll error: " << e.what() << std::endl;
			stop();
			return;
		}
		catch (const SocketException& e) {
			std::cerr << "Socket error: " << e.what() << std::endl;
			return ;
		}
	}
}

void Server::start()
{
	m_running = true;
	addListeningSocket("1050"); //! testing port <- modify later
	configureServerSockets();
	std::cout << "Server started with epoll" << std::endl;
}

void Server::stop()
{
	m_running = false;
	for (auto& [fd, client] : m_clients)
		client.disconnect();
	m_clients.clear();
	m_listening_sockets.clear();
	m_epoll.close_epoll_instance();
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
		return;
	try {
		setNonBlocking(fd); //* client socket
		m_epoll.addFd(fd, EPOLLIN);
		m_clients.emplace(fd, Client(fd, *this));
	} catch (const SocketException& e) {
		std::cerr << "Error setting up client socket: " << e.what() << std::endl;
		close(fd);
		throw;
	}
}

void Server::removeClient(int fd)
{
	auto it = m_clients.find(fd);
	if (it != m_clients.end())
	{
		it->second.disconnect();
		m_clients.erase(it);
		m_epoll.removeFD(fd);
	}
}

Client& Server::getClient(int fd)
{
	return m_clients.at(fd);
}

/**
 * @note emplace_back vector method that constructs element directly in vector
 */
void Server::addListeningSocket(const std::string& port)
{
	m_listening_sockets.emplace_back(10);  //* create socket directly in vector
	if (!m_listening_sockets.back().initTestSocket(port))
		throw SocketException("Failed to initialize socket on port " + port);
	
	std::cout << "Server is listening on port " << port << std::endl;
}

Socket& Server::getListeningSocket(size_t index)
{
	return m_listening_sockets.at(index); //? check where to utilise
}

size_t Server::getListeningSocketCount() const
{
	return m_listening_sockets.size();
}

void Server::processRequest(int client_fd, const Request& request)
{
	try {
		std::cout << "Handling request for URI: " << request.getURI() << std::endl; //! TEST
		Response response = m_request_handler.handle_get(request);
		std::cout << "Response status: " << response.getStatusCode() << std::endl; //! TEST 
		std::cout << "Response body length: " << response.getBody().length() << std::endl; //! TEST 
		sendResponseToClient(client_fd, response);
	}
	catch (const HTTPException& e) {
		std::cerr << "HTTP Error: " << e.what() << std::endl; //! TEST
		sendErrorResponse(client_fd, e);
	}
}

void Server::sendResponseToClient(int client_fd, const Response& response)
{
	std::string response_str = response.to_str();
	std::cout << "Sending response of length: " << response_str.length() << std::endl; //! TEST 
	std::cout << "Response content: " << response_str << std::endl; //! TEST 
	ssize_t bytes_sent = send(client_fd, response_str.c_str(), response_str.length(), 0);
	if (bytes_sent == -1)
		std::cerr << "Error sending response: " << strerror(errno) << std::endl; //! TEST 
	else
		std::cout << "Successfully sent " << bytes_sent << " bytes" << std::endl; //! TEST 
}

void Server::handleNewConnection()
{
	for (auto& socket : m_listening_sockets)
	{
		const std::vector<int>& serverSockets = socket.getServerSockets();
		if (serverSockets.empty())
			continue;
			
		try {
			int client_fd = socket.acceptConnection(serverSockets[0]);
			std::cout << "New connection accepted on fd: " << client_fd << std::endl; //! TEST 
			try {
				addClient(client_fd);
				std::cout << "Client added successfully" << std::endl; //! TEST 
			} catch (const std::exception& e) {
				std::cerr << "Failed to set up client: " << e.what() << std::endl; //! TEST 
				close(client_fd);
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
		std::cerr << "Error reading from client: " << strerror(errno) << std::endl; //! TEST 
		removeClient(client_fd);
		return;
	}
	else if (bytes_read == 0)
	{
		std::cout << "Client closed connection on fd: " << client_fd << std::endl; //! TEST 
		removeClient(client_fd);
		return;
	}
	std::cout << "Received " << bytes_read << " bytes from client" << std::endl; //! TEST 
	std::cout << "Request data: " << std::string(buffer, bytes_read) << std::endl; //! TEST 
	
	client.receiveData(buffer, bytes_read);
	if (client.hasCompleteRequest())
	{
		std::cout << "Processing complete request" << std::endl; //! TEST 
		processRequest(client_fd, client.getRequest());
		client.clearRequest();
	}
	else
	{
		std::cout << "Request not complete yet" << std::endl; //! TEST 
	}
}

void Server::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		throw SocketException("Error setting non-blocking mode: " + std::string(strerror(errno)));
	}
}

void Server::configureServerSockets()
{
	for (auto& socket : m_listening_sockets)
	{
		const std::vector<int>& serverSockets = socket.getServerSockets();
		for (int server_fd : serverSockets)
		{
			setNonBlocking(server_fd); //* listening socket
			m_epoll.addFd(server_fd, EPOLLIN);
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

