#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>

#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>

#include "synchronous_websocket_client.hpp"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: testing_client <address> <port> <path>\n"
                  << "Example:\n"
                  << "    testing_client 0.0.0.0 8080 /\n";
        return EXIT_FAILURE;
    }

    const std::string address(argv[1]);
    const std::string port(argv[2]);
    const std::string path(argv[3]);

    try
    {
        const uint timeout_seconds = 15;
        std::cout << "Attempting to run client at '" << address << ":" << port << path << std::endl;

        connections::SynchronousWebSocketClient client;

        client.Connect(address, port, path);

        std::cout << "Type message to send:" << std::endl;

        std::string msg;
        while (std::getline(std::cin, msg))
        {
            if (msg == "end" || msg == "quit")
            {
                break;
            }
            client.Send(msg);
            std::cout << "Type message to send:" << std::endl;
        }

        std::cout << "\n\nClosing...";
        client.Close();
        std::cout << "Complete.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}