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
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <csignal>
#include "Request.hpp"
#include "Socket.hpp"
#include <vector>
#include <iomanip>

#ifndef TEST_PORT
# define TEST_PORT "1050"
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
	(void)argv;
	if (argc > 2)
	{
		std::cout << "Too many arguments. Usage: ./webserv [config_file]" << std::endl;
		return (EXIT_FAILURE);
	}

	try {
		//* initialize socket with backlog of 10
		Socket server(10);
		//* initialize test socket on default port
		if (!server.initTestSocket(TEST_PORT)) {
			std::cerr << "Failed to initialize server socket" << std::endl;
			return (EXIT_FAILURE);
		}
		int peerfd;
		while (gLive)
		{
			//* get the server socket
			const std::vector<int>& serverSockets = server.getServerSockets();
			if (serverSockets.empty()) {
				std::cerr << "No server sockets available" << std::endl;
				break;
			}

			//* accept connection on the first server socket
			peerfd = server.acceptConnection(serverSockets[0]);
			if (peerfd == -1) {
				if (gLive)
					std::cout << "accept failed: " << strerror(errno) << std::endl;
				continue;
			}
			std::cout << "Server received connection" << std::endl;

			pid_t child = fork();
			if (child == -1) {
				std::cout << "fork failed" << std::endl;
				server.closeSocket(peerfd);
				continue;
			}
			if (child == 0) {
				//* close all server sockets
				for (int sock : serverSockets) {
					server.closeSocket(sock);
				}
				
				//* read the incoming request
				std::vector<char> buffer(4096);
				std::string requestData;
				ssize_t bytesRead;
			
				while ((bytesRead = recv(peerfd, buffer.data(), buffer.size(), 0)) > 0) {
					requestData.append(buffer.data(), bytesRead);
					if (bytesRead < static_cast<ssize_t>(buffer.size()))
						break;
				}

				if (bytesRead < 0) {
					std::cout << "Error reading from socket: " << strerror(errno) << std::endl;
					server.closeSocket(peerfd);
					exit(EXIT_FAILURE);
				}

				std::cout << "Received request data: " << requestData << std::endl;
				Request request;
				try {
					request.parse(requestData); 
					std::string response = "HTTP/1.1 200 OK\r\n"
											"Content-Type: text/plain\r\n"
											"Content-Length: 13\r\n"
											"\r\n"
											"Hello, world!";
					if (send(peerfd, response.c_str(), response.length(), 0) == -1)
						std::cout << "Error sending response: " << strerror(errno) << std::endl;
					} 
					catch (std::exception& e){ 
						std::cout << "Failed to parse request\n";
						std::string errorResponse = "HTTP/1.1 400 Bad Request\r\n"
												  "Content-Type: text/plain\r\n"
												  "Content-Length: 15\r\n"
												  "\r\n"
												  "Invalid request";
						send(peerfd, errorResponse.c_str(), errorResponse.length(), 0);
					}
					server.closeSocket(peerfd);
					exit(EXIT_SUCCESS);
				}

			}
			//* parent process
			server.closeSocket(peerfd);

		std::cout << "Server shutting down" << std::endl;
		return (EXIT_SUCCESS);

	} catch (const SocketException& e) {
		std::cerr << "Socket error: " << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
}
