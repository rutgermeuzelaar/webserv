#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

Client::Client(int socket_fd) 
	: m_socket_fd(socket_fd)
	, m_is_connected(true)
	, m_request_complete(false)
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