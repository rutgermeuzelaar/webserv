#ifndef CGI_HPP
# define CGI_HPP
# include <string>
# include <filesystem>
# include <vector>
# include <optional>
#endif

class Cgi
{
    private:
        const size_t m_timeout_seconds;
        std::optional<const std::string> find_binary(char *const *envp, const std::string& binary) const;
     
    public:
        Cgi();
        void execute_script(const std::filesystem::path& path, int fd, \
            std::vector<std::string>& argv, char **envp) const;
};