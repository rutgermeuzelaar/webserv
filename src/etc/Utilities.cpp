#include "Pch.hpp"
#include <random>
#include <ctime>
#include <iostream>
#include "Utilities.hpp"
#include "Defines.hpp"
#include "Http.hpp"

std::vector<std::string> split(const std::string to_split, const char delimiter)
{
    const size_t length = to_split.length();
    std::vector<std::string> split_str;
    size_t                      i;
    size_t                      j;
    
    i = 0;
    j = 0;
    while (i < length)
    {
        if (to_split.at(i) == delimiter)
        {
            split_str.push_back(to_split.substr(j, i - j));
            j = i + 1;            
        }
        i++;
    }
    split_str.push_back(to_split.substr(j, length - j));
    return split_str;
}

std::optional<const std::string> get_envvar(char *const *envp, const std::string& var_name)
{
    std::optional<const std::string> envvar;
    int i;
    
    i = 0;
    while (envp[i] != nullptr)
    {
        std::string str(envp[i]);
        if (str.find(var_name, 0) == 0)
        {
            str.erase(0, str.find_first_of('=', 0) + 1);
            envvar.emplace(str);
            return (envvar);
        }
        i++;
    }
    return envvar;
}

// https://softwareengineering.stackexchange.com/questions/61683/standard-format-for-using-a-timestamp-as-part-of-a-filename
const std::string create_file_name(const std::string extension)
{
    static std::random_device device;
    static std::mt19937 generator(device());
    static std::uniform_int_distribution<> numbers(1, 9);
    time_t rawtime;
    struct tm* timeinfo;
    char timestamp[80];
    std::string file_name = "";

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);
    std::strftime(timestamp, 80, "%F-T%H%M%S", timeinfo);

    file_name.reserve(35);
    for (size_t i = 0; i < 10; ++i)
    {
        file_name.append(std::to_string(numbers(generator)));
    }
    file_name += "-" + std::string(timestamp) + extension;
    return file_name;
}

bool ends_with(const std::string& str, const std::string& ending)
{
    if (ending.size() > str.size())
    {
        return (false);
    }
    return (str.compare(str.size() - ending.size(), ending.size(), ending) == 0);
}

bool starts_with(const std::string& str, const std::string& start)
{
    if (str.rfind(start, 0) == std::string::npos)
    {
        return (false);
    }
    return (true);
}

static size_t getline_delim(const std::string& input, const size_t start, std::string& str, const std::string& delim)
{
    std::string buffer;
    size_t i = start;

    buffer.reserve(delim.size() + 1);
    while (i < input.size())
    {
        char glyph = input[i];
        str += glyph;
        buffer += glyph;

        if (buffer.size() > delim.size())
        {
            buffer.erase(0, 1);
        }
        if (buffer == delim)
        {
            return i;
        }
        i++;
    }
    return i;
}

std::string& ltrim(std::string& str, const char* to_trim)
{
	str.erase(0, str.find_first_not_of(to_trim));
    return str;
}

std::string& rtrim(std::string& str, const char* to_trim)
{
    str.erase(str.find_last_not_of(to_trim) + 1);
    return str;
}

std::string& trim(std::string& str, const char* to_trim)
{
    ltrim(str, to_trim);
    rtrim(str, to_trim);
    return str;
}

std::pair<std::string, std::string> parse_single_http_header(const std::string& header)
{
    size_t colonPos = header.find(':');
    if (colonPos == std::string::npos)
        throw HTTPException(HTTPStatusCode::BadRequest, "Malformed header line: " + header);
    if (!ends_with(header, LINE_BREAK))
        throw HTTPException(HTTPStatusCode::BadRequest);
    std::string key = header.substr(0, colonPos);
    std::string value = header.substr(colonPos + 1);
    
    //* trim whitespace from key and value
    trim(key, WHITE_SPACE);
    trim(value, WHITE_SPACE);
    
    //* converting header keys to lowercase to combat case-insensivity
    std::string lowerKey = key;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
    if (lowerKey.empty())
        throw HTTPException(HTTPStatusCode::BadRequest, "Empty header key");
    return std::pair(lowerKey, value);
}

std::unordered_map<std::string, std::string> parse_http_headers(const std::string& str, size_t* chars_consumed)
{
    const size_t str_size = str.size();
    size_t i = 0;
	std::string line;
	std::unordered_map<std::string, std::string> http_headers;

	while (i < str_size)
	{
        i = getline_delim(str, i, line, LINE_BREAK);
        i += 1;
        // header end is indicated by double CRLF
		if (line == LINE_BREAK)
        {
            if (chars_consumed != nullptr)
            {
                *chars_consumed = i;
            }
            return http_headers;
        }
        std::pair<std::string, std::string> pair = parse_single_http_header(line);
        http_headers.insert(pair);
        line = "";
	}
    throw HTTPException(HTTPStatusCode::BadRequest);
}

void copy_str_bytes(std::vector<std::byte>& bytes, const std::string& str)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        bytes.push_back(static_cast<std::byte>(str[i]));
    }
}

static std::filesystem::path& resolve_overlap(std::filesystem::path& root, const std::filesystem::path& to_join)
{
	std::filesystem::path::iterator root_it = root.begin();
	std::filesystem::path::iterator join_it = to_join.begin();

	if (*root_it == ".")
	{
		root_it++;
	}
	while (root_it != root.end() && join_it != to_join.end() && *root_it == *join_it)
	{
		root_it++;
		join_it++;
	}
	while (join_it != to_join.end())
	{
		root /= *join_it;
		join_it++;
	}
	return root;
}

std::filesystem::path map_uri_helper(std::filesystem::path root_path, std::filesystem::path& uri_path)
{
	if (uri_path.is_absolute())
	{
		uri_path = uri_path.relative_path();
	}
	resolve_overlap(root_path, uri_path);
	return root_path;
}

static void replace_all(std::string& str, const std::string& what, char by)
{
    size_t i = 0;

    while (i < str.size())
    {
        const size_t what_pos = str.find(what, i);

        if (what_pos != std::string::npos)
        {
            str.replace(what_pos, what.size(), 1, by);
        }
        i++;
    }
}

static std::string& percent_decode(std::string& str)
{
    for (const auto& it: g_percent_encoding_table)
    {
        replace_all(str, it.second, it.first);
    }
    return str;
}

std::unordered_map<std::string, std::string> parse_query_string(const std::string& query_string)
{
    auto split_str = split(query_string, '&');
    std::unordered_map<std::string, std::string> attributes;

    for (auto& it: split_str)
    {
        auto kv_pair = split(it, '=');
        if (kv_pair.size() != 2)
        {
            return attributes;
        }
        percent_decode(kv_pair[0]);
        percent_decode(kv_pair[1]);
        attributes.insert({kv_pair[0], kv_pair[1]});
    }
    return attributes;
}
