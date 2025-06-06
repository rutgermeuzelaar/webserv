/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Cgi.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/03 17:29:32 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/04 17:14:54 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <filesystem>
#include <optional>
#include <unistd.h>
#include <sys/wait.h>
#include "Cgi.hpp"
#include "Utilities.hpp"

Cgi::Cgi()
    : m_timeout_seconds {10}
{
    
}

static char**vector_to_char_ptr_arr(const std::vector<std::string>& vector)
{
    char **ptr_arr = new char*[vector.size() + 1];
    size_t i;
    std::string str;
    
    i = 0;
    while (i < vector.size())
    {
        ptr_arr[i] = strdup((vector.at(i).c_str()));
        i++;
    }
    ptr_arr[i] = nullptr;
    return ptr_arr;
}

static const char* get_interpreter(const std::filesystem::path& extension)
{
    if (extension == ".py")
    {
        return "python3";
    }
    if (extension == ".pl")
    {
        return "perl";   
    }
    return nullptr;
}



static int execute_binary(const std::filesystem::path& path, std::vector<std::string>& argv, char **envp)
{
    return (execve(path.c_str(), vector_to_char_ptr_arr(argv), envp));
}


std::optional<const std::string> find_binary(char *const *envp, const std::string& binary)
{
    std::vector<std::string> paths;
    std::optional<const std::string> path_envvar = get_envvar(envp, "PATH");
    std::string bin_path;

    if (!path_envvar.has_value())
    {
        throw std::runtime_error("Missing PATH environment variable.");
    }
    paths = split(path_envvar.value(), ':');
    for (auto& it: paths)
    {
        bin_path = it;
        bin_path.append("/");
        bin_path.append(binary);
        if (std::filesystem::exists(bin_path))
        {
            return std::optional<const std::string>(bin_path);
        }
    }
    return std::optional<const std::string>(std::nullopt);
}

static int execute_interpreted(const char* interpreter, const std::filesystem::path& path, \
std::vector<std::string>& argv, char *const *envp)
{
    std::optional<const std::string> bin_path = find_binary(envp, interpreter);

    if (!bin_path.has_value())
    {
        throw std::runtime_error("Could not find requested interpreter.");
    }
    argv.insert(argv.begin(), path.string());
    argv.insert(argv.begin(), bin_path.value());
    return (execve(bin_path.value().c_str(), vector_to_char_ptr_arr(argv), envp));
}
void Cgi::execute_script(const std::filesystem::path& path, int fd, std::vector<std::string>& argv, char **envp) const
{
    pid_t child;
    int status;
    const char* interpreter;
    (void)fd; // I think we need a fd to write to a specific connection
    interpreter = get_interpreter(path.extension());
    child = fork();
    if (child == 0)
    {
        if (interpreter == nullptr)
        {
            execute_binary(path, argv, envp);
        }
        else
        {
            execute_interpreted(interpreter, path, argv, envp);
        }
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    if (waitpid(child, &status, 0) == -1)
    {
        throw std::runtime_error("A cgi script error occured.");
    }
}
