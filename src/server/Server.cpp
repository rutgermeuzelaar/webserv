#include "Pch.hpp"
#include <cassert>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include "Utilities.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "CgiProcess.hpp"

extern bool gLive;

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

void Server::epoll_event_loop(int num_events)
{
    for (int i = 0; i < num_events; ++i)
    {
        const epoll_event& event = m_epoll.getEvents()[i];
        std::cout << "Event for fd: " << event.data.fd << ", events: " << event.events << std::endl;
        if (event.events & EPOLLIN) std::cout << "  EPOLLIN" << std::endl;
        if (event.events & EPOLLHUP) std::cout << "  EPOLLHUP" << std::endl;
        if (event.events & EPOLLERR) std::cout << "  EPOLLERR" << std::endl;
        if (event.events & EPOLLRDHUP) std::cout << "  EPOLLRDHUP" << std::endl;
        
        int fd = event.data.fd;
        std::cout << "Processing event for fd: " << fd << std::endl;
        if (m_cgi.is_cgi_fd(fd))
        {
            CgiProcess& process = m_cgi.get_child(fd);
            if (m_epoll.isTypeEvent(event, EPOLLIN))
            {
                process.read_pipe(m_epoll);
            }
            if (m_epoll.isTypeEvent(event, EPOLLERR) || m_epoll.isTypeEvent(event, EPOLLHUP))
            {
                process.close_pipe_read_end(m_epoll);
            }
            continue;
        }
        auto socket_index = getSocketIndex(fd);
        if (socket_index.has_value())
        {
            handleNewConnection(socket_index.value());
            continue;
        }
        if (isClient(fd))
        {
            if (m_epoll.isTypeEvent(event, EPOLLIN))
            {
                try
                {
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
            }
            if (!m_epoll.isTypeEvent(event, EPOLLIN))
            {
                removeClient(fd);
            }
        }
    }
}

void Server::run()
{
	if (!m_running)
		return;

	while (m_running && gLive)
	{
        int num_events = m_epoll.wait();
        m_cgi.timeout();
        if (m_cgi.has_children())
        {
            m_cgi.reap();
        }
        send_cgi_responses();
        timeout_clients();
        epoll_event_loop(num_events);
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
    assert("Client should exist" && std::find_if(m_clients.begin(), m_clients.end(), [fd](const auto& pair){return pair.second.getSocketFD() == fd;}) 
    !=  m_clients.end());
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
		m_cgi.add_process(getClient(client_fd), request, m_epoll, client_fd, location, config);
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
    {
        assert("We made a mistake if errno is EBADF" && errno != EBADF);
		std::cerr << "Error sending response: " << strerror(errno) << std::endl; //! TEST 
    }
	else
		std::cout << "Successfully sent " << bytes_sent << " bytes" << std::endl; //! TEST
    const std::string &connection_header = getClient(client_fd).getRequest().getHeaders().get_header("connection");
	if (connection_header == "close")
		removeClient(client_fd);
	else
		getClient(client_fd).clearRequest();
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

bool Server::isClient(int fd) const
{
    return (
        std::find_if(m_clients.begin(), m_clients.end(), [fd](const auto& pair){ return pair.first == fd; })
         != m_clients.end());
}

void Server::timeout_clients()
{
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

void Server::send_cgi_responses()
{
    auto& processes = m_cgi.get_children();
    auto it = processes.begin();

    while (it != processes.end())
    {
        if (it->response_ready() && it->m_client_connected)
        {
            std::cout << "Sending a cgi response\n";
            sendResponseToClient(it->m_client_fd, it->get_response());
            it = processes.erase(it);
        }
        else
        {    
            it++;
        }
    }
}

std::optional<int> Server::getSocketIndex(int fd) const
{
    std::optional<int> socket_index;

    for (const auto& it: m_listening_sockets)
    {
        const auto& server_sockets = it.getServerSockets();
        auto pos = std::find_if(server_sockets.begin(), server_sockets.end(), [fd](int sock){ return fd == sock ;});
        if (pos != server_sockets.end())
        {
            socket_index.emplace(std::distance(server_sockets.begin(), pos));
            return socket_index;
        }
    }
    return socket_index;
}