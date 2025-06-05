/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utilities.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/06/04 16:13:54 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/06/04 16:15:42 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILITIES_HPP
# define UTILITIES_HPP
# include <vector>
# include <string>
# include <optional>

std::vector<std::string> split(const std::string to_split, const char delimiter);
std::optional<const std::string> get_envvar(char *const *envp, const std::string& var_name);
#endif