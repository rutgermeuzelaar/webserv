#include <random>
#include <ctime>
#include "Utilities.hpp"

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
