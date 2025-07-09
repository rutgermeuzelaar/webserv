#include "Pch.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

Client::Client(int socket_fd) 
	: m_socket_fd(socket_fd)
	, m_is_connected(true)
	, m_last_activity(std::chrono::steady_clock::now())
{}

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

std::chrono::steady_clock::time_point Client::getLastActivity() const
{
	return m_last_activity;
}

void Client::updateActivity()
{
	m_last_activity = std::chrono::steady_clock::now();
}

void Client::receiveData(const char* data, size_t len)
{
	if (!m_is_connected)
		return ;
	updateActivity();
    m_request.append(data, len);
}

bool Client::hasCompleteRequest() const
{
	return m_request.complete();
}

const Request& Client::getRequest() const
{
	return m_request;
}

Request& Client::getRequest()
{
	return m_request;
}

void Client::clearRequest()
{
	m_request = Request();
}

void Client::reset()
{
	clearRequest();
	//! no socket fd or server reference clear
	//! maybe better to use disconnect to reset connection state
}