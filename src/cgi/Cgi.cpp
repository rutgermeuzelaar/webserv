#include "Pch.hpp"
#include <string.h>
#include <string>
#include <stdexcept>
#include <list>
#include <iostream>
#include <filesystem>
#include <optional>
#include <unistd.h>
#include <sys/wait.h>
#include <cassert>
#include <chrono>
#include "CgiProcess.hpp"
#include "Cgi.hpp"
#include "Utilities.hpp"
#include "Request.hpp"
#include "Epoll.hpp"
#include "Server.hpp"

Cgi::Cgi(char **envp, size_t timeout_ms)
    : m_envp {envp}
    , m_timeout_ms {timeout_ms}
{

}

void Cgi::reap_dtor(void)
{
    int exit_status;

    for (const auto& it = m_children.begin(); it != m_children.end();)
    {
        (void)kill(it->m_pid, SIGINT);
        (void)waitpid(it->m_pid, &exit_status, 0);
        m_children.erase(it);
    }
}

Cgi::~Cgi()
{
    reap_dtor();
}

void Cgi::timeout(void)
{
    const auto time_now = std::chrono::steady_clock::now();

    for (std::vector<CgiProcess>::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        const auto ms_diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - it->m_start);
        if (ms_diff > m_timeout_ms)
        {
            if (kill(it->m_pid, SIGTERM) == -1)
            {
                perror("kill");
                throw HTTPException(HTTPStatusCode::InternalServerError);
            };
        }
    }
}

void Cgi::reap(void)
{
    int exit_status = 0;

    pid_t pid = waitpid(-1, &exit_status, WNOHANG);

    if (pid == -1)
    {
        if (errno == ECHILD) return;
        perror("waitpid");
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    if (pid == 0)
    {
        return; 
    }
    if (pid > 0)
    {
        auto process = std::find_if(m_children.begin(), m_children.end(), [pid](const CgiProcess& p){return p.m_pid == pid;}); 
        assert(process != m_children.end());

        process->m_reaped = true;
        process->m_exit_status = exit_status;
    }
}

static std::string extract_path_info_envvar(const std::string& uri)
{
    static const std::string cgi_uri = "/" CGI_DIR "/";

    size_t cgi_pos = uri.find(cgi_uri);
    assert(cgi_pos != std::string::npos);

    size_t next_fslash = uri.find('/', cgi_pos + cgi_uri.size());
    if (next_fslash == std::string::npos || next_fslash == cgi_uri.size())
    {
        return "";
    }
    return uri.substr(next_fslash + 1, uri.size() - next_fslash - 1);
}

std::vector<std::string> get_cgi_envvar(const Request& request, const LocationContext* location, const Root& root)
{
    std::vector<std::string> envvar;
    const std::string request_method = stringify(request.getStartLine().get_http_method());
    const std::string content_length = request.getHeaders().get_header("content-length");
    const std::string content_type = request.getHeaders().get_header("content-type");
    const std::string path_info = extract_path_info_envvar(request.getStartLine().get_uri());
    const std::string path_translated = map_uri(path_info, location, root);

    assert(request_method.size() > 0);
    if (content_length.size() > 0)
    {
        envvar.push_back("CONTENT_LENGTH=" + content_length);
    }
    if (content_type.size() > 0)
    {
        envvar.push_back("CONTENT_TYPE=" + content_type);
    }
    if (path_info.size() > 0)
    {
        envvar.push_back("PATH_INFO=" + path_info);
    }
    if (path_translated.size() > 0)
    {
        envvar.push_back("PATH_TRANSLATED=" + path_translated);
    }
    envvar.push_back("REQUEST_METHOD=" + request_method);
    return envvar;
}

// would be nice if this dynamic, so by extension or by shebang
static const char* get_interpreter(const std::string& script_name)
{
    if (ends_with(script_name, ".py"))
    {
        return "python3";
    }
    if (ends_with(script_name, ".pl"))
    {
        return "perl";   
    }
    return nullptr;
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

const std::string Cgi::get_script_name(const std::string& uri) const
{
    const std::string dir_str = "/" CGI_DIR "/";
    size_t dir_pos = uri.find(dir_str, 0);
    assert(dir_pos != std::string::npos);
    assert("A request for the cgi directory should never get here" && (dir_pos + dir_str.size()) < uri.size());
    size_t fslash_pos = uri.find('/', dir_pos + dir_str.size());

    if (fslash_pos == std::string::npos)
    {
        return uri.substr(dir_pos + dir_str.size(), uri.size());
    }
    return uri.substr(dir_pos + dir_str.size(), fslash_pos - dir_pos - dir_str.size());
}

void Cgi::add_process(Client& client, const Request& request, Epoll& epoll, int client_fd, const LocationContext* location, const ServerContext& config)
{
    const std::vector envvar = get_cgi_envvar(request, location, config.m_root.value());
    const std::string script_name = get_script_name(request.getStartLine().get_uri());
    auto binary = find_binary(m_envp, get_interpreter(script_name));
    std::filesystem::path cgi_file_path = std::filesystem::current_path();
    cgi_file_path /= "root/" CGI_DIR "/";
    std::cout << cgi_file_path << '\n';
    char *argv[3]; // no extra arguments for now
    char *envp[envvar.size() + 1];

    cgi_file_path.append(script_name);

    int fd_pair[2];
    assert(std::filesystem::exists(cgi_file_path));
    assert(binary.has_value());
    if (pipe(fd_pair) == -1)
    {
        perror("pipe");
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    epoll.addFd(fd_pair[0], EPOLLIN); // mark read end of pipe for reading
    // setting argv
    argv[0] = const_cast<char*>(binary.value().c_str());
    argv[1] = const_cast<char*>(cgi_file_path.c_str());
    argv[2] = NULL;
    // setting envp
    for (size_t i = 0; i < envvar.size(); ++i)
    {
        envp[i] = const_cast<char*>(envvar[i].c_str());   
    }
    envp[envvar.size()] = NULL;
    pid_t pid = fork();
    if (pid == 0)
    {
        // close read end of pipe in child
        if (close(fd_pair[0]) == -1)
        {
            exit(EXIT_FAILURE);
        }
        if (dup2(fd_pair[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        execve(binary.value().c_str(), argv, envp);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    if (pid == -1)
    {
        perror("fork");
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    if (close(fd_pair[1]) == 1)
    {
        perror(nullptr);
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    m_children.emplace_back(CgiProcess(fd_pair[0], client_fd, pid, location, config));
    client.m_cgi_process = &m_children.back();
}

bool Cgi::is_cgi_fd(int fd) const
{
    if (std::find_if(m_children.begin(), m_children.end(), [fd](const CgiProcess& process){return process.m_read_fd == fd;}) == m_children.end())
    {
        return (false);
    }
    return (true);
}

bool Cgi::has_children(void) const
{
    return (!m_children.empty());
}

CgiProcess& Cgi::get_child(int fd)
{
    auto child = std::find_if(m_children.begin(), m_children.end(), [fd](auto& process){return process.m_read_fd == fd;});

    assert("This function should only be called for existing childprocesses" && child != m_children.end());
    return *child;
}

void Cgi::erase_child(int fd)
{
    auto child = std::find_if(m_children.begin(), m_children.end(), [fd](auto& process){return process.m_read_fd == fd;});

    m_children.erase(child);
}

std::list<CgiProcess>& Cgi::get_children(void)
{
    return m_children;
}
