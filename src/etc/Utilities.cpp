#include <random>
#include <ctime>
#include <iostream>
#include "Utilities.hpp"
#include "Defines.hpp"
#include "HTTPException.hpp"

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
    if (ending > str)
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

std::istream& getline_delim(std::istream& is, std::string& str, const std::string& delim)
{
    std::string buffer;
    char glyph;

    buffer.reserve(delim.size() + 1);
    while (is.get(glyph))
    {
        str += glyph;
        buffer += glyph;

        if (buffer.size() > delim.size())
        {
            buffer.erase(0, 1);
        }
        if (buffer == delim)
        {
            return is;
        }
    }
    return is;
}

std::unordered_map<std::string, std::string> parse_http_headers(std::istream& stream)
{
	static const size_t MAX_HEADER_SIZE = 8192; //* common header size limit
	std::string line;
	size_t totalHeaderSize = 0;
	std::unordered_map<std::string, std::string> http_headers;

	while (getline_delim(stream, line, LINE_BREAK))
	{
        // header end is indicated by double CRLF
		if (line == LINE_BREAK)
        {
            // https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Host
            if (http_headers.find("host") == http_headers.end())
            {
                throw HTTPException(HTTPStatusCode::BadRequest);
            }
            return http_headers;
        }
		totalHeaderSize += line.length();
		if (totalHeaderSize > MAX_HEADER_SIZE)
			throw HTTPException(HTTPStatusCode::RequestHeaderFieldsTooLarge, "Request header fields too large");
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
			throw HTTPException(HTTPStatusCode::BadRequest, "Malformed header line: " + line);
        if (!ends_with(line, LINE_BREAK))
            throw HTTPException(HTTPStatusCode::BadRequest);
        std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		
		//* trim whitespace from key and value
		key.erase(0, key.find_first_not_of(" \t\r\n")); //* searches str for first char that does not match the given chars
		key.erase(key.find_last_not_of(" \t\r\n") + 1); //* same, but for last char
		value.erase(0, value.find_first_not_of(" \t\r\n"));
		value.erase(value.find_last_not_of(" \t\r\n") + 1);
		
		//* converting header keys to lowercase to combat case-insensivity
		std::string lowerKey = key;
		std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
		if (lowerKey.empty())
			throw HTTPException(HTTPStatusCode::BadRequest, "Empty header key");
        http_headers[lowerKey] = value;
        line = "";
	}
    throw HTTPException(HTTPStatusCode::BadRequest);
}