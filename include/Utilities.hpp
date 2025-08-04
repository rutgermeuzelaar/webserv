#ifndef UTILITIES_HPP
# define UTILITIES_HPP
# include <vector>
# include <algorithm>
# include <string>
# include <optional>
# include <unordered_map>

const std::unordered_map<char, std::string> g_percent_encoding_table = {
    {':', "%3A"},
    {'/', "%2F"},
    {'?', "%3F"},
    {'#', "%23"},
    {'[', "%5B"},
    {']', "%5D"},
    {'@', "%40"},
    {'!', "%21"},
    {'$', "%24"},
    {'&', "%26"},
    {'\'', "%27"},
    {'(', "%28"},
    {')', "%29"},
    {'*', "%2A"},
    {'+', "%2B"},
    {',', "%2C"},
    {';', "%3B"},
    {'=', "%3D"},
    {'%', "%25"},
    {' ', "%20"}
};

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

template <typename Tkey, typename Tvalue> bool has_keys(const std::unordered_map<Tkey, Tvalue>& map, const std::vector<Tkey>& keys)
{
    if (map.size() < keys.size())
    {
        return false;
    }
    for (const auto& it: keys)
    {
        if (map.find(it) == map.end())
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
std::string& ltrim(std::string& str, const char* to_trim);
std::string& rtrim(std::string& str, const char* to_trim);
std::string& trim(std::string& str, const char* to_trim);
std::pair<std::string, std::string> parse_single_http_header(const std::string& header);
std::unordered_map<std::string, std::string> parse_http_headers(const std::string& str, size_t* chars_consumed);
void copy_str_bytes(std::vector<std::byte>& bytes, const std::string& str);
std::unordered_map<std::string, std::string> parse_query_string(const std::string& str);
#endif