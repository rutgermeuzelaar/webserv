/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parser.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/02 13:37:57 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/05/23 11:54:19 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include "HttpContext.hpp"
#include "Parser.hpp"
#include "Scanner.hpp"

int main(int argc, char **argv)
{
	Config config;
    if (argc != 2)
    {
        std::cout << "Please supply exactly one configuration file.\n";
        return (EXIT_FAILURE);
    }
    std::ifstream config_file(argv[1]);
    std::stringstream buffer;

    if (config_file.fail())
    {
        std::cout << "Failed to open file, please try again.\n";
    }
    buffer << config_file.rdbuf();
    Scanner scanner(buffer.str());
    const std::vector<Token>& tokens = scanner.scan();
    scanner.print_tokens();
    Parser parser(tokens, config);
    try
    {
        parser.parse();
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "An exception occured: " << e.what() << '\n';
    }
    return (EXIT_SUCCESS);
}
