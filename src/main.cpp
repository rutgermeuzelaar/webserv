#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <csignal>
#ifndef TEST_PORT
# define TEST_PORT "1050"
#endif

volatile std::sig_atomic_t gSignum;
bool	gLive = true;

void signal_handler(int signal)
{
	gSignum = signal;
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
	if (argc != 2)
	{
		std::cout << "Insufficient arguments, please supply a configuration"
		" file.\n";
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
			std::cout << "server: socket" << strerror(errno) << '\n';
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
		{
			return (EXIT_FAILURE);
		}
		if (bind(sockfd, node->ai_addr, node->ai_addrlen) == -1)
		{
			close(sockfd);
			std::cout << "server: bind" << strerror(errno) << '\n';
			continue;
		}
		break ;
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
	while (gLive)
	{
		peer_size = sizeof(peer_addr);
		peerfd = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_size);
		if (peerfd == -1)
		{
			return (EXIT_FAILURE);
		}
		printf("Server received something\n");
		pid_t child = fork();
		if (child == -1)
		{
			return (EXIT_FAILURE);
		}
		if (child == 0)
		{
			close(sockfd);
			if (send(peerfd, "Hello, world!", 13, 0) == -1)
			{
				std::cout << "Error\n";
			}
			close(peerfd);
			exit (EXIT_SUCCESS);
		}
		close(peerfd);
	}
	return (EXIT_SUCCESS);
}
