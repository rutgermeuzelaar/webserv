#include "Pch.hpp"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>

#include <string>
#include <stdexcept>
#include <list>
#include <iostream>
#include <filesystem>
#include <optional>
#include <cassert>
#include <chrono>

#include "Utilities.hpp"
#include "Epoll.hpp"
#include "Server.hpp"
#include "Http.hpp"
#include "Defines.hpp"
#include "Cgi.hpp"

Cgi::Cgi(char **envp, size_t timeout_ms)
    : m_envp {envp}
    , m_timeout_ms {timeout_ms}
{

}

void Cgi::reap_dtor(void)
{
    int exit_status;
    auto it = m_children.begin();

    while (it != m_children.end())
    {
        (void)kill((*it)->m_pid, SIGINT);
        (void)waitpid((*it)->m_pid, &exit_status, 0);
        it = m_children.erase(it);
    }
}

Cgi::~Cgi()
{
    reap_dtor();
}

void Cgi::timeout(void)
{
    const auto time_now = std::chrono::steady_clock::now();
    for (auto it = m_children.begin(); it != m_children.end(); ++it)
    {
        if ((*it)->get_reaped()) continue;
        const auto ms_diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - (*it)->m_start);
        if (ms_diff > m_timeout_ms)
        {
            if (kill((*it)->m_pid, SIGTERM) == -1)
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
        auto process = std::find_if(m_children.begin(), m_children.end(), [pid](auto p){return p->m_pid == pid;}); 
        assert(process != m_children.end());

        (*process)->m_exit_status = exit_status;
        (*process)->set_reaped(true);
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

static std::vector<std::string> get_cgi_envvar(const Request& request, const LocationContext* location, const Root& root)
{
    std::vector<std::string> envvar;
    const std::string request_method = stringify(request.getStartLine().get_http_method());
    const std::string content_length = request.getHeaders().get_header("content-length");
    const std::string content_type = request.getHeaders().get_header("content-type");
    const std::string path_info = extract_path_info_envvar(request.getStartLine().get_uri());
    const std::string path_translated = map_uri(path_info, location, root);

    envvar.reserve(5);
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

static std::optional<const std::string> find_binary(char *const *envp, const std::string& binary)
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

static const std::string get_script_name(const std::string& uri)
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

static int execve_wrapper(const std::vector<std::string> envvar, const std::string& binary_path, const std::filesystem::path& cgi_file_path)
{
    std::vector<char*> cstring_envvar;
    char* argv[3];
    char* binary_cstring = const_cast<char*>(binary_path.c_str());

    cstring_envvar.reserve(envvar.size() + 1); // + 1 for nullptr
    argv[0] = binary_cstring;
    argv[1] = const_cast<char*>(cgi_file_path.c_str());
    argv[2] = nullptr;
    for (auto &it: envvar)
    {
        cstring_envvar.push_back(const_cast<char*>(it.c_str()));
    }
    cstring_envvar.push_back(nullptr);
    return (execve(binary_cstring, argv, cstring_envvar.data()));
}

static void child_process_post(int socket_pair[2], const std::vector<std::string>& envvar, \
    const std::string& binary_path, const std::filesystem::path& cgi_file_path)
{
    // close parent socket in child
    if (close(socket_pair[0]) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (dup2(socket_pair[1], STDOUT_FILENO) == -1)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    if (dup2(socket_pair[1], STDIN_FILENO) == -1)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    execve_wrapper(envvar, binary_path, cgi_file_path);
    perror("execve");
    exit(EXIT_FAILURE);
}

static void child_process_default(int socket_pair[2], const std::vector<std::string>& envvar, \
    const std::string& binary_path, const std::filesystem::path& cgi_file_path)
{
    // close parent socket in child
    if (close(socket_pair[0]) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (dup2(socket_pair[1], STDOUT_FILENO) == -1)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    execve_wrapper(envvar, binary_path, cgi_file_path);
    perror("execve");
    exit(EXIT_FAILURE);
}

void Cgi::add_process(Client& client, Request& request, Epoll& epoll, const LocationContext* location, \
    const ServerContext& config, Server& server)
{   
    const int client_fd = client.getSocketFD();
    (void)client_fd;
    assert("Client can't have multiple processes" && std::find_if(
        m_children.begin(),
        m_children.end(),
        [client_fd](auto ptr){
            return ptr->m_client_fd == client_fd && ptr->get_client_connected();
        }
    )
    == m_children.end());
 
    const HTTPMethod http_method = request.getStartLine().get_http_method();
    const std::string script_name = get_script_name(request.getStartLine().get_uri());
    const char* interpreter = get_interpreter(script_name);
    const std::vector<std::string> envvar = get_cgi_envvar(request, location, config.m_root.value());

    std::string binary_path;
    std::filesystem::path cgi_file_path = std::filesystem::current_path();

    cgi_file_path /= "root/" CGI_DIR "/";
    cgi_file_path.append(script_name); 
    assert(std::filesystem::exists(cgi_file_path));
    if (interpreter == nullptr)
    {
        binary_path = cgi_file_path;
    }
    else
    {
        auto binary_opt = find_binary(m_envp, interpreter);
        if (binary_opt.has_value())
        {
            binary_path = binary_opt.value();
        }
        else
        {
            binary_path = cgi_file_path;
        }
    }
    // socket_pair[0] parent
    // socket_pair[1] child
    int socket_pair[2];
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, socket_pair) == -1)
    {
        perror("socketpair");
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    if (http_method == HTTPMethod::POST)
    {
        epoll.addFd(socket_pair[0], EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLRDHUP | EPOLLERR);
    }
    else
    {
        epoll.addFd(socket_pair[0], EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR);
    }
    pid_t pid = fork();
    if (pid == 0)
    {
        if (http_method == HTTPMethod::POST)
        {
            child_process_post(socket_pair, envvar, binary_path, cgi_file_path);
        }
        else
        {
            child_process_default(socket_pair, envvar, binary_path, cgi_file_path);
        }
    }
    if (pid == -1)
    {
        perror("fork");
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    if (close(socket_pair[1]) == 1)
    {
        perror(nullptr);
        throw HTTPException(HTTPStatusCode::InternalServerError);
    }
    auto cgi_process = std::make_shared<CgiProcess>(socket_pair[0], client.getSocketFD(), pid, location, config, server, request.getBody());
    if (http_method == HTTPMethod::POST)
    {
        cgi_process->set_is_post(true);
    }
    else
    {
        cgi_process->set_writing_complete(true);
    }
    m_children.push_back(cgi_process);
    client.setProcessPtr(m_children.back());
}

bool Cgi::is_cgi_fd(int fd) const
{
    auto pos = std::find_if(
        m_children.begin(),
        m_children.end(),
        [fd](std::shared_ptr<CgiProcess> process) {
            return (process->get_fd() == fd);
        }
    );
    return (pos != m_children.end());    
}

bool Cgi::has_children(void) const
{
    return (!m_children.empty());
}

CgiProcess& Cgi::get_child(int fd)
{
    assert(fd != -1);
    auto child = std::find_if(
        m_children.begin(),
        m_children.end(),
        [fd](std::shared_ptr<CgiProcess> process){
            return (process->get_fd() == fd);
        }
    );

    assert("This function should only be called for existing childprocesses" && child != m_children.end());
    return *(*child);
}

void Cgi::erase_child(pid_t pid, bool require_connection)
{
    const size_t old_size = m_children.size();
    (void)old_size; // assert can get removed
    if (require_connection)
    {
        m_children.erase(
            std::remove_if(
                m_children.begin(),
                m_children.end(),
                [pid](std::shared_ptr<CgiProcess> p){
                    return p->get_client_connected() && p->m_pid == pid;
                }
            ),
            m_children.end()
        );
    }
    else
    {
        m_children.erase(
            std::remove_if(
                m_children.begin(),
                m_children.end(),
                [pid](std::shared_ptr<CgiProcess> p){
                    return p->m_pid == pid;
                }
            ),
            m_children.end()
        );
    }
    assert("One element should be deleted" && (old_size - m_children.size()) == 1);
}
