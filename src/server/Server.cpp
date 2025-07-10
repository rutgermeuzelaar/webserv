#include "Pch.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

Server::Server(const std::vector<ServerContext>& config, char** envp)
	: m_configs(config)
	, m_listening_sockets()
	, m_epoll()
	, m_clients()
	, m_client_to_socket_index()
	, m_running(false)
    , m_cgi {envp, CGI_TIMEOUT_MS}
{

}

Server::~Server()
{
	stop();
}

void Server::run()
{
	if (!m_running)
		return;

	while (m_running)
	{
		try {
			int num_events = m_epoll.wait();
            m_cgi.timeout();
			for (int i = 0; i < num_events; ++i)
			{
				const epoll_event& event = m_epoll.getEvents()[i];
				std::cout << "Event for fd: " << event.data.fd << ", events: " << event.events << std::endl;
				if (event.events & EPOLLIN) std::cout << "  EPOLLIN" << std::endl;
				if (event.events & EPOLLHUP) std::cout << "  EPOLLHUP" << std::endl;
				if (event.events & EPOLLERR) std::cout << "  EPOLLERR" << std::endl;
				int fd = event.data.fd;
				std::cout << "Processing event for fd: " << fd << std::endl;
	            
                if (m_cgi.is_cgi_fd(fd))
                {
                    std::cout << "Is CGI fd\n";
                    m_cgi.read_pipes();
                    auto cgi_response = m_cgi.reap();
                    if (cgi_response.has_value())
                    {
                        sendResponseToClient(cgi_response.value().first, cgi_response.value().second);
                    }
                    continue;
                }
				if (m_epoll.isTypeEvent(event, EPOLLERR) || m_epoll.isTypeEvent(event, EPOLLHUP))
				{
					std::cerr << "Error or hangup on fd: " << fd << std::endl;
					removeClient(fd);
					continue;
				}
				bool is_server_socket = false;
				for (size_t socket_i = 0; socket_i < m_listening_sockets.size(); ++socket_i) 
				{
					const std::vector<int>& serverSockets = m_listening_sockets[socket_i].getServerSockets();
					if (m_epoll.isServerSocket(fd, serverSockets)) {
						is_server_socket = true;
						handleNewConnection(socket_i);
						break;
					}
				}
				if (is_server_socket)
					continue;

				if (m_epoll.isTypeEvent(event, EPOLLIN)) 
				{
					try
					{
						std::cout << "EPOLLIN event detected for fd: " << fd << std::endl;
						handleClientData(fd);
					}
					catch(const HTTPException& e)
					{
						const auto& conf = m_configs[m_client_to_socket_index[fd]];
						const std::string& uri = getClient(fd).getRequest().getStartLine().get_uri();
						const LocationContext* location = find_location(uri, conf);
				
						Response response = build_error_page(e.getStatusCode(), location, conf);
						sendResponseToClient(fd, response);
						std::cerr << e.what() << '\n';
					}
					continue ;
				}
			}

			//* timeout
			auto now = std::chrono::steady_clock::now();
			for (auto it = m_clients.begin(); it != m_clients.end();)
			{
				int fd = it->first;
				Client& client = it->second;
				if (now - client.getLastActivity() > TIMEOUT)
				{
					std::cout << "Client " << it->first << " timed out" << std::endl;
					//* partial request
					if (!client.hasCompleteRequest() && !client.getRequest().is_empty())
					{
						std::cout << "going in this block" << std::endl;
						HTTPException timeout(HTTPStatusCode::RequestTimeout, "Request timed out\n");
						sendErrorResponse(fd, timeout);
					}
					++it;
					removeClient(fd);
				}
				else
					++it;
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
	setupListeningSockets();
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
 *          1. Sets the client socket to non-blocking mode
 *          2. Adds the client fd to the epoll instance with the following event types:
 *             - EPOLLIN: Notifies when data is available to read from the client
 *             - EPOLLHUP: Notifies when the client has closed the connection (hangup)
 *             - EPOLLERR: Notifies if an error occurs on the client socket
 *          3. Constructs a new Client object in-place within the m_clients map using try_emplace,
 *             avoiding temp object destruction and ensuring the socket remains valid
 * @param fd The socket file descriptor for the new client connection
 * @note Adding the client fd to epoll with EPOLLIN, EPOLLHUP, and EPOLLERR ensures the server is notified for epoll types.
 */
void Server::addClient(int fd)
{
	if (m_clients.find(fd) != m_clients.end())
		return;
	try {
		setNonBlocking(fd); //* client socket
		m_epoll.addFd(fd, EPOLLIN | EPOLLHUP | EPOLLERR);
		std::cout << "Added client fd " << fd << " to epoll" << std::endl;
		m_clients.try_emplace(fd, fd);
	} catch (const SocketException& e) {
		std::cerr << "Error setting up client socket: " << e.what() << std::endl;
		close(fd);
		throw;
	} //TODO change later for good exception throwing
}

void Server::removeClient(int fd)
{
	auto it = m_clients.find(fd);
	if (it != m_clients.end())
	{
		m_epoll.removeFD(fd);
		m_client_to_socket_index.erase(fd);
		m_clients.erase(it);
	}
}

Client& Server::getClient(int fd)
{
	return m_clients.at(fd);
}

void Server::setupListeningSockets()
{	
	m_listening_sockets.reserve(m_configs.size());
	for (const ServerContext& config : m_configs)
	{
		m_listening_sockets.emplace_back(10); //! 10 backlog
		if (!m_listening_sockets.back().initSocket(config))
			throw SocketException("Failed to init socket");
	
		//* straight config and add to epoll
		const std::vector<int>& serverSockets = m_listening_sockets.back().getServerSockets();
		for (int server_fd : serverSockets)
		{
				setNonBlocking(server_fd);
				m_epoll.addFd(server_fd, EPOLLIN);
		}
	}
}

Socket& Server::getListeningSocket(size_t index)
{
	return m_listening_sockets.at(index); //? check where to utilise
}

size_t Server::getListeningSocketCount() const
{
	return m_listening_sockets.size(); //? need still?
}

void Server::processRequest(int client_fd, const Request& request)
{
	//* check which socket to handle
	size_t socket_index = 0;
	auto it = m_client_to_socket_index.find(client_fd);
	if (it != m_client_to_socket_index.end())
		socket_index = it->second;
	// else
	// 	socket_index = 0; //! handle error, comment in if it works
	const ServerContext& config = m_configs[socket_index];
	RequestHandler handler(config);
	std::cout << "Handling request for URI: " << request.getStartLine().get_uri() << std::endl; //! TEST
	const std::string& uri = request.getStartLine().get_uri();
	const LocationContext* location = find_location(uri, config);

	if (request_method_allowed(location, request.getStartLine().get_http_method()) && is_cgi_request(uri))
	{
		std::cout << "CGI request\n";
		m_cgi.add_process(request, m_epoll, client_fd, location, config);
	}
	else
	{
		Response response = handler.handle(request, uri, location);
		std::cout << "Response status: " << response.getStatusCode() << std::endl; //! TEST
		std::cout << "Response body length: " << response.getBody().length() << std::endl; //! TEST
		sendResponseToClient(client_fd, response);
	}
}

void Server::sendResponseToClient(int client_fd, const Response& response)
{
	std::string response_str = response.to_str();
	std::cout << "Sending response of length: " << response_str.length() << std::endl; //! TEST
	ssize_t bytes_sent = send(client_fd, response_str.c_str(), response_str.length(), 0);
	if (bytes_sent == -1)
		std::cerr << "Error sending response: " << strerror(errno) << std::endl; //! TEST 
	else
		std::cout << "Successfully sent " << bytes_sent << " bytes" << std::endl; //! TEST
}

void Server::handleNewConnection(size_t socket_index)
{
	Socket& socket = m_listening_sockets[socket_index];
	const std::vector<int>& serverSockets = socket.getServerSockets();
	for (int server_fd : serverSockets)
	{
		try {
			int client_fd = socket.acceptConnection(server_fd);
			std::cout << "New connection accepted on fd: " << client_fd << std::endl;
			try {
				addClient(client_fd);
				m_client_to_socket_index[client_fd] = socket_index;
				std::cout << "Client added successfully" << std::endl;
			} catch (const std::exception& e) {
				std::cerr << "Failed to set up client: " << e.what() << std::endl;
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
	std::cout << "\n=== Handling Client Data for fd: " << client_fd << " ===" << std::endl;
	
	Client& client = getClient(client_fd);
	char buffer[RECV_BUFFER_SIZE];

	ssize_t bytes_read = recv(client_fd, buffer, RECV_BUFFER_SIZE, 0);
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

	// std::cout << "\n--- Raw Request Data ---" << std::endl;
	// std::cout << std::string(buffer, bytes_read) << std::endl;
	// std::cout << "------------------------\n" << std::endl;

    const auto& conf = m_configs[m_client_to_socket_index[client_fd]];
	client.receiveData(buffer, bytes_read, conf.m_client_max_body_size.value().m_size);
	
	std::cout << "Checking if request is complete..." << std::endl;
	if (client.hasCompleteRequest())
	{
		std::cout << "Processing complete request" << std::endl;
		Request& req = client.getRequest();
		std::cout << "Request method: " << req.getStartLine().get_http_method() << std::endl;
		std::cout << "Request URI: " << req.getStartLine().get_uri() << std::endl;
		processRequest(client_fd, req);
		client.clearRequest();
	}
	else
	{
		std::cout << "Request not complete yet" << std::endl;
	}
	std::cout << "=== End of Client Data Handling ===\n" << std::endl;
}

void Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw SocketException("Error getting flags: " + std::string(strerror(errno)));
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw SocketException("Error setting non-blocking mode: " + std::string(strerror(errno)));
}

void Server::sendErrorResponse(int client_fd, const HTTPException& e)
{
	//! make better later
	Response errorResponse(e.getStatusCode());
	errorResponse.setBody(e.what());
	sendResponseToClient(client_fd, errorResponse);
}
