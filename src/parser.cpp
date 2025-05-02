/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parser.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/02 13:37:57 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/02 13:48:59 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include "Scanner.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Please supply exactly one configuration file.\n";
        return (EXIT_FAILURE);
    }
    std::ifstream config(argv[1]);
    std::stringstream buffer;

    if (config.fail())
    {
        std::cout << "Failed to open file, please try again.\n";
    }
    buffer << config.rdbuf();
    Scanner scanner(buffer.str());
    const std::vector<Token>& tokens = scanner.scan();
    for (const auto& it: tokens)
    {
        std::cout << it.m_str << '\n';
    }
    return (EXIT_SUCCESS);
}
