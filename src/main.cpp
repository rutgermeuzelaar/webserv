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
#include <vector>
#ifndef TEST_PORT
# define TEST_PORT "1050"
#endif

volatile std::sig_atomic_t gSignum;
bool	gLive = true;

void testRequestParsing();

void signal_handler(int signal)
{
	gSignum = signal;
	gLive = false;
}

void testRequestParsing()
{
    std::cout << "\n=== Testing Request Parsing ===\n" << std::endl;

    //* GET
    std::string getRequest = 
        "GET /index.html?id=123&name=test#section1 HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: Mozilla/5.0\r\n"
        "Accept: text/html\r\n"
        "\r\n";

    std::cout << "Testing GET request parsing:" << std::endl;
    Request getReq;
    try {
        getReq.parse(getRequest);
        std::cout << "GET request parsed successfully!" << std::endl;
    } catch (const HTTPException& e) {
        std::cout << "Failed to parse GET request: " << e.what() << std::endl;
    }

    //* POST
    std::string postRequest = 
        "POST /submit-form HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 27\r\n"
        "\r\n"
        "username=john&password=secret";

    std::cout << "\nTesting POST request parsing:" << std::endl;
    Request postReq;
    try {
        postReq.parse(postRequest);
        std::cout << "POST request parsed successfully!" << std::endl;
    } catch (const HTTPException& e) {
        std::cout << "Failed to parse POST request: " << e.what() << std::endl;
    }

    //* DELETE
    std::string deleteRequest = 
        "DELETE /users/123 HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "\r\n";

    std::cout << "\nTesting DELETE request parsing:" << std::endl;
    Request deleteReq;
    try {
        deleteReq.parse(deleteRequest);
        std::cout << "DELETE request parsed successfully!" << std::endl;
    } catch (const HTTPException& e) {
        std::cout << "Failed to parse DELETE request: " << e.what() << std::endl;
    }

    std::cout << "\n=== Request Parsing Tests Completed ===\n" << std::endl;
}

// int main(int argc, char **argv)
// {
// 	struct addrinfo hints;
// 	struct addrinfo	*node;
// 	struct addrinfo *server_addr;
// 	int				sockfd;
// 	int				peerfd;
// 	int				optval;
// 	socklen_t		peer_size;
// 	struct sockaddr_storage peer_addr;
// 	(void)argv;
// 	optval = 1;

// 	//! easy testing as Rutger is doing config parsing
// 	if (argc > 2)
// 	{
// 		std::cout << "Too many arguments. Usage: ./webserv [config_file]" << std::endl;
// 		return (EXIT_FAILURE);
// 	}
// 	std::memset(&hints, 0, sizeof(struct addrinfo));
// 	hints.ai_family = AF_UNSPEC;
// 	hints.ai_socktype = SOCK_STREAM;
// 	hints.ai_flags = AI_PASSIVE;
// 	if (getaddrinfo(NULL, TEST_PORT, &hints, &server_addr) != 0)
// 	{
// 		return (EXIT_FAILURE);
// 	}
// 	for (node = server_addr; node != NULL; node = node->ai_next)
// 	{
// 		sockfd = socket(node->ai_family, node->ai_socktype, node->ai_protocol);
// 		if (sockfd == -1)
// 		{
// 			continue;
// 		}
// 		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
// 		{
// 			return (EXIT_FAILURE);
// 		}
// 		if (bind(sockfd, node->ai_addr, node->ai_addrlen) == -1)
// 		{
// 			close(sockfd);
// 			continue;
// 		}
// 		break;
// 	}
// 	freeaddrinfo(server_addr);
// 	if (node == NULL)
// 	{
// 		return (EXIT_FAILURE);
// 	}
// 	if (listen(sockfd, 10) == -1)
// 	{
// 		return (EXIT_FAILURE);
// 	}

// 	std::cout << "Server is listening on port " << TEST_PORT << std::endl;

// 	while (gLive)
// 	{
// 		peer_size = sizeof(peer_addr);
// 		peerfd = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_size);
// 		if (peerfd == -1)
// 		{
// 			if (gLive)
// 				std::cout << "accept failed: " << strerror(errno) << std::endl;
// 			continue;
// 		}
// 		std::cout << "Server received connection" << std::endl;

// 		pid_t child = fork();
// 		if (child == -1)
// 		{
// 			std::cout << "fork failed" << std::endl;
// 			close(peerfd);
// 			continue;
// 		}
// 		if (child == 0)
// 		{
// 			close(sockfd);
			
// 			// Read the incoming request
// 			std::vector<char> buffer(4096);
// 			std::string requestData;
// 			ssize_t bytesRead;
			
// 			while ((bytesRead = recv(peerfd, buffer.data(), buffer.size(), 0)) > 0)
// 			{
// 				requestData.append(buffer.data(), bytesRead);
// 				if (bytesRead < static_cast<ssize_t>(buffer.size()))
// 					break;
// 			}
// 			if (bytesRead < 0)
// 			{
// 				std::cout << "Error reading from socket: " << strerror(errno) << std::endl;
// 				close(peerfd);
// 				exit(EXIT_FAILURE);
// 			}
// 			std::cout << "Received request data: " << requestData << std::endl;
// 			Request request;
// 			try {
// 				request.parse(requestData);
// 				std::string response = "HTTP/1.1 200 OK\r\n"
// 									 "Content-Type: text/plain\r\n"
// 									 "Content-Length: 13\r\n"
// 									 "\r\n"
// 									 "Hello, world!";
// 				if (send(peerfd, response.c_str(), response.length(), 0) == -1)
// 					std::cout << "Error sending response: " << strerror(errno) << std::endl;
// 			}
// 			catch (const HTTPException& e) {
// 				std::cout << "Failed to parse request: " << e.what() << std::endl;
// 				std::string errorResponse = "HTTP/1.1 " + std::to_string(static_cast<int>(e.getStatusCode())) + " " + e.what() + "\r\n"
// 										  "Content-Type: text/plain\r\n"
// 										  "Content-Length: " + std::to_string(strlen(e.what())) + "\r\n"
// 										  "\r\n"
// 										  + e.what();
// 				send(peerfd, errorResponse.c_str(), errorResponse.length(), 0);
// 			}
// 			close(peerfd);
// 			exit(EXIT_SUCCESS);
// 		}
// 		char cheese[1024];
// 		const int bytes_read = read(peerfd, cheese, 1024);
// 		std::cout << std::setw(bytes_read) << cheese << '\n';
// 		close(peerfd);
// 	}

// 	close(sockfd);
// 	std::cout << "Server shutting down" << std::endl;
// 	return (EXIT_SUCCESS);
// }

int main()
{
	testRequestParsing();
	return EXIT_SUCCESS;
}