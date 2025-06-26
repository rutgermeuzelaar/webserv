#ifndef UTILITIES_HPP
# define UTILITIES_HPP
# include <vector>
# include <algorithm>
# include <string>
# include <optional>
# include <unordered_map>

std::vector<std::string> split(const std::string to_split, const char delimiter);

template <typename T> bool is_unique(const std::vector<T>& vector)
{
    for (auto it = vector.begin(); it != vector.end(); ++it)
    {
        if (std::find(std::next(it), vector.end(), *it) != vector.end())
        {
            return false;
        }
    }
    return true;
}

std::optional<const std::string> get_envvar(char *const *envp, const std::string& var_name);
const std::string create_file_name(const std::string extension);
bool starts_with(const std::string& str, const std::string& start);
bool ends_with(const std::string& str, const std::string& ending);
std::istream& getline_delim(std::istream& is, std::string& str, const std::string& delim);
std::string& ltrim(std::string& str, const char* to_trim);
std::string& rtrim(std::string& str, const char* to_trim);
std::string& trim(std::string& str, const char* to_trim);
std::pair<std::string, std::string> parse_single_http_header(const std::string& header);
std::unordered_map<std::string, std::string> parse_http_headers(std::istream& stream);
#endif