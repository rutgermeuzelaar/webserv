/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 15:42:16 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/19 16:51:06 by robertrinh    ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <csignal>
#include "Request.hpp"
#include <vector>
#include <iomanip>

#ifndef TEST_PORT
# define TEST_PORT "8080"
#endif

volatile std::sig_atomic_t gSignum;
bool	gLive = true;

void signal_handler(int signal)
{
	gSignum = signal;
	gLive = false;
}

int main(int argc, char **argv)
{
	struct addrinfo hints;
	struct addrinfo	*node;
	struct addrinfo *server_addr;
	int				sockfd;
	int				peerfd;
	int				optval;
	socklen_t		peer_size;
	struct sockaddr_storage peer_addr;
	(void)argv;
	optval = 1;

	//! easy testing as Rutger is doing config parsing
	if (argc > 2)
	{
		std::cout << "Too many arguments. Usage: ./webserv [config_file]" << std::endl;
		return (EXIT_FAILURE);
	}
	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, TEST_PORT, &hints, &server_addr) != 0)
	{
		return (EXIT_FAILURE);
	}
	for (node = server_addr; node != NULL; node = node->ai_next)
	{
		sockfd = socket(node->ai_family, node->ai_socktype, node->ai_protocol);
		if (sockfd == -1)
		{
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
		{
			return (EXIT_FAILURE);
		}
		if (bind(sockfd, node->ai_addr, node->ai_addrlen) == -1)
		{
			close(sockfd);
			continue;
		}
		break;
	}
	freeaddrinfo(server_addr);
	if (node == NULL)
	{
		return (EXIT_FAILURE);
	}
	if (listen(sockfd, 10) == -1)
	{
		return (EXIT_FAILURE);
	}

	std::cout << "Server is listening on port " << TEST_PORT << std::endl;

	while (gLive)
	{
		peer_size = sizeof(peer_addr);
		peerfd = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_size);
		if (peerfd == -1)
		{
			if (gLive)
				std::cout << "accept failed: " << strerror(errno) << std::endl;
			continue;
		}
		std::cout << "Server received connection" << std::endl;

		pid_t child = fork();
		if (child == -1)
		{
			std::cout << "fork failed" << std::endl;
			close(peerfd);
			continue;
		}
		if (child == 0)
		{
			close(sockfd);
			
			// Read the incoming request
			std::vector<char> buffer(4096);
			std::string requestData;
			ssize_t bytesRead;
			
			while ((bytesRead = recv(peerfd, buffer.data(), buffer.size(), 0)) > 0)
			{
				requestData.append(buffer.data(), bytesRead);
				if (bytesRead < static_cast<ssize_t>(buffer.size()))
					break;
			}
			if (bytesRead < 0)
			{
				std::cout << "Error reading from socket: " << strerror(errno) << std::endl;
				close(peerfd);
				exit(EXIT_FAILURE);
			}
			std::cout << "Received request data: " << requestData << std::endl;
			Request request;
			if (request.parse(requestData))
			{
				std::string response = "HTTP/1.1 200 OK\r\n"
									 "Content-Type: text/plain\r\n"
									 "Content-Length: 13\r\n"
									 "\r\n"
									 "Hello, world!";
				if (send(peerfd, response.c_str(), response.length(), 0) == -1)
					std::cout << "Error sending response: " << strerror(errno) << std::endl;
			}
			else
			{
				std::cout << "Failed to parse request\n";
				std::string errorResponse = "HTTP/1.1 400 Bad Request\r\n"
										  "Content-Type: text/plain\r\n"
										  "Content-Length: 15\r\n"
										  "\r\n"
										  "Invalid request";
				send(peerfd, errorResponse.c_str(), errorResponse.length(), 0);
			}
			close(peerfd);
			exit(EXIT_SUCCESS);
		}
		char cheese[1024];
		const int bytes_read = read(peerfd, cheese, 1024);
		std::cout << std::setw(bytes_read) << cheese << '\n';
		close(peerfd);
	}

	close(sockfd);
	std::cout << "Server shutting down" << std::endl;
	return (EXIT_SUCCESS);
}