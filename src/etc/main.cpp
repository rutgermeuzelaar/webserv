/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 15:42:16 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/13 13:57:35 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <csignal>
#include <vector>
#include <iomanip>
#include "Request.hpp"
#include "Socket.hpp"
#include "Scanner.hpp"
#include "Config.hpp"
#include "RequestHandler.hpp"

volatile std::sig_atomic_t gSignum;
bool	gLive = true;

void signal_handler(int signal)
{
	gSignum = signal;
	gLive = false;
}

static void exec_connection_process(Socket& server, const ServerContext& server_config, int peerfd)
{
    std::vector<char> buffer(4096);
    std::string requestData;
    ssize_t bytesRead;
    Request request;
    RequestHandler request_handler(server_config);

    //* close all server sockets
    for (int sock: server.getServerSockets())
    {
        server.closeSocket(sock);
    }
    //* read the incoming request

    while ((bytesRead = recv(peerfd, buffer.data(), buffer.size(), 0)) > 0)
    {
        requestData.append(buffer.data(), bytesRead);
        if (bytesRead < static_cast<ssize_t>(buffer.size()))
            break;
    }
    if (bytesRead < 0)
    {
        std::cout << "Error reading from socket: " << strerror(errno) << std::endl;
        server.closeSocket(peerfd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Received request data: " << requestData << std::endl;
    try
    {
        request.parse(requestData);
        Response response = request_handler.handle(request);
        const std::string response_str = response.to_str();

        if (send(peerfd, response_str.c_str(), response_str.size(), 0) == -1)
            std::cout << "Error sending response: " << strerror(errno) << std::endl;
    } 
    catch (std::exception& e)
    { 
        std::cout << "Failed to parse request: " << e.what() << '\n';
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

static pid_t fork_connection(Socket& server, const ServerContext& server_config, int peerfd)
{
    pid_t child = fork();
    if (child == -1)
    {
        std::cout << "fork failed" << std::endl;
        server.closeSocket(peerfd);
    }
    if (child == 0)
    {
        exec_connection_process(server, server_config, peerfd);
    }
    return child;
}

static void process_connections(Socket& server, const ServerContext& server_config)
{
    int peerfd;
    int exit_status;
    pid_t child;

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
        child = fork_connection(server, server_config, peerfd);
        waitpid(child, &exit_status, 0);
        server.closeSocket(peerfd);
    }
}

static void setup_server(const ServerContext& server_config, Socket& server)
{
    if (!server.initTestSocket(server_config.m_listen.value().m_port.value().to_string()))
    {
        throw std::runtime_error("Failed to initialize server socket.");
    }
}

int main(int argc, char **argv)
{
    std::vector<ServerContext> server_configs;

	if (argc != 2)
	{
		std::cout << "Wrong arguments. Usage: ./webserv [config_file]" << std::endl;
		return (EXIT_FAILURE);
	}
	try
	{
		server_configs = get_server_config(argv[1]);
	}
	catch (const std::exception& error)
	{
		std::cerr << "A problem occured whilst processing configuration file: " << error.what() << '\n';
		return (EXIT_FAILURE);
	}
    //* initialize socket with backlog of 10
    Socket server(10);
    try
    {
        setup_server(server_configs[0], server);
        process_connections(server, server_configs[0]);
    }
    catch (const std::exception& error)
    {
        std::cout << error.what() << '\n';
        return (EXIT_FAILURE);
    }
	std::cout << "Server shutting down" << std::endl;
	return (EXIT_SUCCESS);
}
