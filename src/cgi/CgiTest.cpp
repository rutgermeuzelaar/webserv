#include "Pch.hpp"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include "Cgi.hpp"

std::vector<std::string> char_ptr_arr_to_vector(char **argv)
{
    int i;
    std::vector<std::string> vector;
    
    i = 0;
    while (argv[i] != nullptr)
    {
        vector.push_back(argv[i]);
        i++;
    }
    return vector;
}

int main(int argc, char **argv, char **envp)
{
    (void)argc;

    if (argc < 2)
    {
        return (EXIT_FAILURE);
    }
    Cgi cgi;
    std::vector<std::string> args;
    try
    {
        cgi.execute_script(argv[1], 0, args, envp);
    }
    catch (const std::exception& error)
    {
        std::cout << error.what() << '\n';
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}
