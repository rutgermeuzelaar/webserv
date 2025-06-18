/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utilities.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/04 16:13:54 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/11 17:21:32 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILITIES_HPP
# define UTILITIES_HPP
# include <vector>
# include <algorithm>
# include <string>
# include <optional>

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
#endif