/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utilities.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/04 16:14:40 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/04 16:56:51 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

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
#include <iostream>
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
