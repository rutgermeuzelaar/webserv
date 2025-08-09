#include "Pch.hpp" // IWYU pragma: keep
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cassert>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <exception>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Server.hpp"
#include "Cgi.hpp"
#include "Defines.hpp"
#include "Config.hpp"
#include "Epoll.hpp"
#include "Http.hpp"
#include "RequestHandler.hpp"
#include "ResponseHandler.hpp"

extern bool gLive;

Server::Server(const std::vector<ServerContext>& config, char** envp)
	: m_configs(config)
	, m_listening_sockets()
	, m_epoll()
	, m_clients()
	, m_client_to_socket_index()
	, m_running(false)
    , m_cgi {envp, CGI_TIMEOUT_MS}
    , m_response_handler {*this, m_epoll}
{

}

Server::~Server()
{
	stop();
}

void Server::epoll_loop(int num_events)
{
    for (int i = 0; i < num_events; ++i)
    {
        const epoll_event& event = m_epoll.getEvents()[i];
        DEBUG("Event for fd: " << event.data.fd << ", events: " << event.events);
        if (event.events & EPOLLIN) { DEBUG("  EPOLLIN") ;}
        if (event.events & EPOLLOUT) { DEBUG("  EPOLLOUT") ;}
        if (event.events & EPOLLHUP) { DEBUG("  EPOLLHUP") ;}
        if (event.events & EPOLLERR) { DEBUG("  EPOLLERR") ;}
        if (event.events & EPOLLRDHUP) { DEBUG("  EPOLLRDHUP") ;}
        
        int fd = event.data.fd;
        DEBUG("Processing event for fd: " << fd);
        if (m_cgi.is_cgi_fd(fd))
        {
            CgiProcess& process = m_cgi.get_child(fd);
            if (m_epoll.isTypeEvent(event, EPOLLIN))
            {
                process.read_from_fd(m_epoll);
                if (!m_cgi.is_cgi_fd(fd))
                {
                    continue;
                }
            }
			if (m_epoll.isTypeEvent(event, {EPOLLHUP, EPOLLRDHUP, EPOLLERR}))
            {
				process.close_read_write_fd(m_epoll);
				continue;
            }
            if (m_epoll.isTypeEvent(event, EPOLLOUT))
            {
                process.write_to_fd(m_epoll);
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
            if (m_epoll.isTypeEvent(event, {EPOLLRDHUP, EPOLLHUP, EPOLLERR}))
            {
                removeClient(fd);
                continue;
            }
            if (m_epoll.isTypeEvent(event, EPOLLOUT))
            {
                m_response_handler.send_response(fd);
            }
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
                    m_response_handler.add_response(fd, response);
                    std::cerr << e.what() << '\n';
                }
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
        timeout_clients();
        epoll_loop(num_events);
  }
}

void Server::start()
{
	m_running = true;
	setupListeningSockets();
    install_route("/create-session", create_session, {HTTPMethod::POST});
    install_route("/file-upload-success", file_upload_success, {HTTPMethod::GET});
    install_route("/whoami", whoami, {HTTPMethod::GET});
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
		m_epoll.addFd(fd, EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLRDHUP);
		DEBUG("Added client fd " << fd << " to epoll");
		m_clients.try_emplace(fd, fd);
	} catch (const SocketException& e) {
		std::cerr << "Error setting up client socket: " << e.what() << std::endl;
		close(fd);
		throw e;
	}
}

void Server::removeClient(int fd)
{
	auto it = m_clients.find(fd);
	if (it != m_clients.end())
	{
        if (it->second.getProcessPtr().use_count() > 0)
        {
            it->second.getProcessPtr()->set_client_connected(false);
        }
        m_response_handler.remove_if_exists(fd);
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
		m_listening_sockets.emplace_back(10);
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

void Server::processRequest(int client_fd, Request& request)
{
	//* check which socket to handle
	size_t socket_index = 0;
	auto it = m_client_to_socket_index.find(client_fd);
	assert("If it is end of client to socket index, we made a mistake lol" && (it != m_client_to_socket_index.end()));
	if (it != m_client_to_socket_index.end())
		socket_index = it->second;
	const ServerContext& config = m_configs[socket_index];
	RequestHandler handler(config);
	DEBUG("Handling request for URI: " << request.getStartLine().get_uri()); 
	const std::string& uri = request.getStartLine().get_uri();
	const LocationContext* location = find_location(uri, config);
    auto session_id_opt = get_header_value(request.getHeaders().get_header("Cookie"), "session");
    if (session_id_opt.has_value())
    {
        auto session_it = m_session_handler.find_session(session_id_opt.value());
        if (session_it != m_session_handler.get_sessions().end())
        {
            session_it->second.add_request(request.getStartLine().get_http_method());
            session_it->second.print();
        }
    }
	if (request_method_allowed(location, request.getStartLine().get_http_method()) && is_cgi_request(config.m_root.value().m_path, uri))
	{
		DEBUG("CGI request");
        HttpBody& http_body = request.getBody();

        if (http_body.is_chunked())
        {
            http_body.append_bytes(http_body.get_chunked_decoder().get_decoded());
        }
        else
        {
            http_body.append_bytes(http_body.get_raw());
        }
        m_cgi.add_process(getClient(client_fd), request, m_epoll, location, config, *this);
        return;
	}
    const auto& route_it = std::find_if(
        m_routes.begin(),
        m_routes.end(),
        [&uri](const Route& route) {
            return route.get_url() == uri;
        }
    );
    if (route_it == m_routes.end())
    {
        Response response = handler.handle(request, uri, location);
        DEBUG("Response status: " << response.getStatusCode()); 
        DEBUG("Response body length: " << response.getBodySize()); 
        m_response_handler.add_response(client_fd, response);
    }
    else
    {
        const HTTPMethod http_method = request.getStartLine().get_http_method();
        const auto& allowed_methods = route_it->get_allowed_methods();

        if (std::find(allowed_methods.begin(), allowed_methods.end(), http_method) == allowed_methods.end())
        {
            m_response_handler.add_response(client_fd, build_error_page(HTTPStatusCode::MethodNotAllowed, location, config));  
        }
        else
        {
            m_response_handler.add_response(client_fd, route_it->get_action()(*this, request, location, config));
        }
    }
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
				DEBUG("Client added successfully");
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
	DEBUG("\n=== Handling Client Data for fd: " << client_fd << " ===");
	
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
    const auto& conf = m_configs[m_client_to_socket_index[client_fd]];
	client.receiveData(buffer, bytes_read, conf.m_client_max_body_size.value().m_size);
	
	DEBUG("Checking if request is complete...");
	if (client.hasCompleteRequest())
	{
		DEBUG("Processing complete request");
		Request& req = client.getRequest();
		DEBUG("Request method: " << req.getStartLine().get_http_method());
		DEBUG("Request URI: " << req.getStartLine().get_uri());
		processRequest(client_fd, req);
	}
	else
	{
		DEBUG("Request not complete yet");
	}
	DEBUG("=== End of Client Data Handling ===\n");
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
	Response errorResponse(e.getStatusCode());
	errorResponse.setBody(e.what());
    m_response_handler.add_response(client_fd, errorResponse);
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
                DEBUG("going in this block");
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

std::optional<size_t> Server::getSocketIndex(int fd) const
{
    for (size_t i = 0; i < m_listening_sockets.size(); ++i) 
	{
        const auto& server_sockets = m_listening_sockets[i].getServerSockets();
        if (std::find(server_sockets.begin(), server_sockets.end(), fd) != server_sockets.end())
            return i;
    }
    return std::nullopt;
}

void Server::notify(CgiProcess& process, CgiProcessEvent event)
{
    switch (event)
    {
        case CgiProcessEvent::ResponseReady:
        {
            getClient(process.m_client_fd).resetProcessPtr();
            Response response = process.get_response();
            m_response_handler.add_response(process.m_client_fd, response);
            m_cgi.erase_child(process.m_pid, true);
            return;
        }
        case CgiProcessEvent::IsRemovable: // Initiator is disconnected
            m_cgi.erase_child(process.m_pid, false);
    }
}

void Server::notify_response_sent(int client_fd)
{
    Client& client = getClient(client_fd);
    const std::string& connection_header = client.getRequest().getHeaders().get_header("connection");

    if (connection_header == "close")
    {
        removeClient(client_fd);
    }
    else
    {
        client.clearRequest();
    }
}

SessionHandler& Server::getSessionHandler(void)
{
    return m_session_handler;
}

void Server::install_route(const std::string& url, route_act_t action, const std::vector<HTTPMethod>& allowed_methods)
{
    m_routes.push_back(Route(url, allowed_methods, action));
}
