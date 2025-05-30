/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parseTests.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: robertrinh <robertrinh@student.codam.nl      +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/19 16:49:58 by robertrinh    #+#    #+#                 */
/*   Updated: 2025/05/21 14:49:07 by robertrinh    ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include "Request.hpp"
#include "Response.hpp"

void testRequestParsing()
{
    std::cout << "\n=== Testing Request Parsing ===\n" << std::endl;

    //* GET
    std::string getRequest = 
        "GET /index.html?id=123&name=test#section1 HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: Mozilla/5.0\r\n"
        "Accept: text/html\r\n"
        "\r\n";

    std::cout << "Testing GET request parsing:" << std::endl;
    Request getReq;
    if (getReq.parse(getRequest))
        std::cout << "GET request parsed successfully!" << std::endl;
    else
        std::cout << "Failed to parse GET request" << std::endl;

    //* POST
    std::string postRequest = 
        "POST /submit-form HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 27\r\n"
        "\r\n"
        "username=john&password=secret";

    std::cout << "\nTesting POST request parsing:" << std::endl;
    Request postReq;
    if (postReq.parse(postRequest))
        std::cout << "POST request parsed successfully!" << std::endl;
    else
        std::cout << "Failed to parse POST request" << std::endl;

    //* DELETE
    std::string deleteRequest = 
        "DELETE /users/123 HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "\r\n";

    std::cout << "\nTesting DELETE request parsing:" << std::endl;
    Request deleteReq;
    if (deleteReq.parse(deleteRequest))
        std::cout << "DELETE request parsed successfully!" << std::endl;
    else
        std::cout << "Failed to parse DELETE request" << std::endl;

    std::cout << "\n=== Request Parsing Tests Completed ===\n" << std::endl;
}

void testResponseBuilding()
{
    std::cout << "\n=== Testing Response Building ===\n" << std::endl;

    try {
        //* test 1: basic OK response
        std::cout << "Testing basic OK response:" << std::endl;
        Response okResponse(HTTPStatusCode::OK);
        okResponse.setBody("Hello, World!");
        std::cout << okResponse.to_str() << std::endl;

        //* test 2: response with custom headers
        std::cout << "\nTesting response with custom headers:" << std::endl;
        Response customResponse(HTTPStatusCode::OK);
        customResponse.setHeader("X-Custom-Header", "test-value");
        customResponse.setHeader("Cache-Control", "no-cache");
        customResponse.setBody("Custom headers test");
        std::cout << customResponse.to_str() << std::endl;

        //* test 3: error response
        std::cout << "\nTesting error response:" << std::endl;
        Response errorResponse(HTTPStatusCode::NotFound);
        errorResponse.setBody("404 - Page not found");
        std::cout << errorResponse.to_str() << std::endl;

        //* test 4: file response (this should throw if file doesn't exist)
        std::cout << "\nTesting file response (should throw if file doesn't exist):" << std::endl;
        Response fileResponse(HTTPStatusCode::OK);
        try {
            fileResponse.setBodyFromFile("nonexistent.html");
        } catch (const HTTPException& e) {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
            std::cout << "Status code: " << static_cast<int>(e.getStatusCode()) << std::endl;
        }

        //* test 5: invalid header
        std::cout << "\nTesting invalid header (should throw):" << std::endl;
        Response invalidHeaderResponse(HTTPStatusCode::OK);
        try {
            invalidHeaderResponse.setHeader("", "value");
        } catch (const HTTPException& e) {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
            std::cout << "Status code: " << static_cast<int>(e.getStatusCode()) << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << "Unexpected error: " << e.what() << std::endl;
    }

    std::cout << "\n=== Response Building Tests Completed ===\n" << std::endl;
}

int main()
{
    // testRequestParsing();
    testResponseBuilding();
    return EXIT_SUCCESS;
} 