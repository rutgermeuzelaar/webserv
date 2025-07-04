#include "Pch.hpp"
#include <iostream>
#include <csignal>
#include "Server.hpp"
#include "Config.hpp"

volatile std::sig_atomic_t gSignum;
bool gLive = true;

void signal_handler(int signal)
{
    gSignum = signal;
    gLive = false;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Wrong arguments. Usage: ./webserv [config_file]" << std::endl;
        return (EXIT_FAILURE);
    }
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        //* server init
        std::vector<ServerContext> config = get_server_config(argv[1]);

        //* create + start server
        Server server(config);
        std::cout << "Starting server..." << std::endl;
        server.start();
		server.run();

        //* cleanup
        std::cout << "Shutting down server..." << std::endl;
        server.stop();
        return (EXIT_SUCCESS);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (EXIT_FAILURE);
    }
}

